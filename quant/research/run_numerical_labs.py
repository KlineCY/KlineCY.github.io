"""Execute the complete synthetic numerical laboratory and parameter studies.

Usage:
    python quant/research/run_numerical_labs.py PATH_TO_KLINE_QUANT OUTPUT_DIRECTORY

The standard library suffices; input fixtures are constructed examples.
"""
import argparse
import csv
import io
import json
import math
import subprocess
from pathlib import Path

DATA=Path(__file__).resolve().parent/"numerical-examples"
CATALOG=json.loads((DATA/"catalog.json").read_text(encoding="utf-8"))


def execute(binary,command,source,params,target):
    proc=subprocess.run([str(binary),command,str(source),*map(str,params)],
                        check=True,capture_output=True,text=True,timeout=120)
    rows=[{k:float(v) for k,v in row.items()}
          for row in csv.DictReader(io.StringIO(proc.stdout))]
    if not rows or not all(math.isfinite(v) for row in rows for v in row.values()):
        raise ValueError("Invalid numerical output: "+command)
    target.write_text(proc.stdout,encoding="utf-8")
    return rows


def write(path,header,rows):
    with path.open("w",encoding="utf-8",newline="") as f:
        w=csv.writer(f)
        w.writerow(header)
        w.writerows(rows)


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("binary",type=Path)
    ap.add_argument("output",type=Path)
    args=ap.parse_args()
    binary=args.binary.resolve()
    out=args.output.resolve()
    if out.exists() and any(out.iterdir()):
        raise ValueError("Output directory must be new or empty")
    out.mkdir(parents=True,exist_ok=True)
    studies=out/"studies"
    studies.mkdir()
    results={}
    for e in CATALOG:
        results[e["fn"]]=execute(binary,e["command"],DATA/e["fixture"],e["params"],out/(e["fn"]+".csv"))

    # European call/put parity is a separate price-domain accuracy diagnostic.
    e=next(e for e in CATALOG if e["fn"]=="pde_european")
    p=e["params"][:]
    p[-1]=0
    put=execute(binary,e["command"],DATA/e["fixture"],p,studies/"european_put.csv")
    parity=p[0]*math.exp(-p[3]*p[5])-p[1]*math.exp(-p[2]*p[5])
    rows=[]
    for a,b in zip(results["pde_european"],put):
        residual=a["GridPrice"]-b["GridPrice"]-parity
        rows.append([a["SpaceSteps"],a["TimeSteps"],a["GridPrice"],b["GridPrice"],parity,residual])
    write(studies/"put_call_parity.csv",["SpaceSteps","TimeSteps","Call","Put","ParityValue","ParityResidual"],rows)
    assert abs(rows[-1][-1])<.02

    # A wider upper barrier can only enlarge the set of surviving paths.
    rows=[]
    for B in [130,150,180]:
        r=execute(binary,"num-pde-barrier",DATA/"pde-barrier.csv",
                  [100,.03,.01,.2,1,B,360,400],studies/("barrier_"+str(B)+".csv"))
        rows.extend([B,z["Spot"],z["KnockOutCall"],z["VanillaCall"]] for z in r)
    write(studies/"barrier_sensitivity.csv",["Barrier","Spot","KnockOutPrice","VanillaPrice"],rows)
    for spot in sorted(set(row[1] for row in rows)):
        v=[row[2] for row in rows if row[1]==spot]
        assert all(b>=a-1e-5 for a,b in zip(v,v[1:]))

    # Larger turnover penalties shrink the L1 trade from the same old portfolio.
    rows=[]
    for lam in [0,.025,.05,.1,.2]:
        r=execute(binary,"num-turnover-admm",DATA/"turnover-admm.csv",
                  [1,lam,1,5000],studies/("admm_"+str(lam)+".csv"))
        rows.append([lam,r[0]["Weight"],r[1]["Weight"],r[0]["TurnoverL1"],
                     r[0]["Objective"],r[0]["PrimalResidual"],r[0]["DualResidual"]])
    write(studies/"turnover_penalty_path.csv",["Penalty","WeightA","WeightB","TurnoverL1","Objective","PrimalResidual","DualResidual"],rows)
    assert all(b[3]<=a[3]+1e-7 for a,b in zip(rows,rows[1:]))

    # Reuse estimated zero-yield nodes in a separate cash-flow risk calculation.
    params=[.0001]
    for z in results["nelson_siegel"]:
        params.extend([z["Maturity"],z["FittedYield"]])
    execute(binary,"num-keyrate-curve",DATA/"keyrate-curve.csv",params,studies/"fitted_curve_keyrate.csv")

    # Hold point estimates fixed when comparing alternative covariance estimators.
    hc3=results["multivariate_hc3"]
    hac=results["multivariate_hac"]
    cr1=results["multivariate_cluster"]
    rows=[]
    for a,b,c in zip(hc3,hac,cr1):
        assert abs(a["Estimate"]-b["Estimate"])<1e-9 and abs(a["Estimate"]-c["Estimate"])<1e-9
        rows.append([a["Coefficient"],a["Estimate"],a["StandardError"],b["StandardError"],c["StandardError"]])
    write(studies/"covariance_comparison.csv",["Coefficient","Estimate","HC3SE","HACSE","CR1SE"],rows)

    # Evaluate robust weights as the uncertainty radius moves through the zero-position threshold.
    rows=[]
    for radius in [0,1,2,4,5,6]:
        r=execute(binary,"num-robust-mean-ellipsoid",DATA/"robust-mean-ellipsoid.csv",
                  [2,radius],studies/("robust_"+str(radius)+".csv"))
        rows.append([radius,r[0]["Weight"],r[1]["Weight"],r[0]["Volatility"],r[0]["WorstCaseExpectedReturn"]])
    write(studies/"uncertainty_radius_path.csv",["Radius","WeightA","WeightB","Volatility","WorstCaseReturn"],rows)
    assert all(b[3]<=a[3]+1e-8 for a,b in zip(rows,rows[1:])) and abs(rows[-1][3])<1e-8

    print("Completed 40 numerical laboratories and 6 integrated parameter studies.")
    print("Constructed teaching samples only. Output directory:",out)


if __name__=="__main__":
    main()
