"""Run the public synthetic research examples and chained evaluations.

Usage:
    python quant/research/run_researchflow.py PATH_TO_KLINE_QUANT OUTPUT_DIRECTORY

No external data provider or third-party Python package is required.
The input files are teaching fixtures, not observations of traded assets.
"""
import argparse
import csv
import json
import math
import statistics
import subprocess
from pathlib import Path

EXAMPLES=Path(__file__).resolve().parent/"examples"
CATALOG=json.loads((EXAMPLES/"catalog.json").read_text(encoding="utf-8"))


def write_csv(path,header,rows):
    with path.open("w",newline="",encoding="utf-8") as f:
        w=csv.writer(f)
        w.writerow(header)
        w.writerows(rows)


def read_csv(path):
    with path.open(newline="",encoding="utf-8") as f:
        r=csv.DictReader(f)
        return [{k:float(v) for k,v in row.items()} for row in r]


def execute(binary,command,source,parameters,target):
    result=subprocess.run([str(binary),command,str(source),*map(str,parameters)],
                          check=True,capture_output=True,text=True,timeout=120)
    target.write_text(result.stdout,encoding="utf-8")
    rows=read_csv(target)
    if not rows or not all(math.isfinite(v) for row in rows for v in row.values()):
        raise ValueError("Missing or nonfinite output: "+command)
    return rows


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("binary",type=Path)
    parser.add_argument("output",type=Path)
    args=parser.parse_args()
    binary=args.binary.resolve()
    out=args.output.resolve()
    if out.exists() and any(out.iterdir()):
        raise ValueError("Output directory must be new or empty")
    out.mkdir(parents=True,exist_ok=True)
    derived=out/"derived"
    derived.mkdir()
    reports={}
    for entry in CATALOG:
        fn=entry["fn"]
        reports[fn]=execute(binary,entry["command"],EXAMPLES/entry["fixture"],
                            entry["params"],out/(fn+".csv"))
    assets=read_csv(EXAMPLES/"multiasset.csv")
    names=["cta","cross_section","minimum_variance","equal_risk"]
    paths={name:{int(r["Period"]):r for r in reports[name]} for name in names}
    common=sorted(set.intersection(*(set(p) for p in paths.values())))
    summary=[]
    for name in names:
        source=derived/(name+"_aligned_returns.csv")
        write_csv(source,["Strategy","Benchmark"],
                  [[paths[name][t]["NetReturn"],statistics.mean(assets[t].values())] for t in common])
        metrics=execute(binary,"flow-performance",source,[252,.02,.8],
                        derived/(name+"_performance.csv"))[0]
        summary.append([name,common[0],common[-1],len(common),metrics["TotalReturn"],
                        metrics["AnnualVol"],metrics["SharpeIID"],metrics["MaxDrawdown"],
                        statistics.mean(paths[name][t]["Turnover"] for t in common)])
    write_csv(derived/"strategy_comparison.csv",
              ["Strategy","StartPeriod","EndPeriod","Observations","TotalReturn",
               "AnnualVol","SharpeIID","MaxDrawdown","MeanTurnover"],summary)

    # Both models are compared on the intersection of prediction dates.
    fixed={int(r["Period"]):r for r in reports["rolling_ridge"]}
    nested={int(r["Period"]):r for r in reports["nested_ridge"]}
    dates=sorted(set(fixed)&set(nested))
    source=derived/"aligned_predictions.csv"
    write_csv(source,["Model1","Model2","Actual"],
              [[fixed[t]["Prediction"],nested[t]["Prediction"],nested[t]["Actual"]] for t in dates])
    execute(binary,"flow-forecast-comparison",source,[2],derived/"ridge_comparison.csv")

    # The nested predictions are already out of sample; calibration uses earlier residuals.
    source=derived/"nested_predictions.csv"
    write_csv(source,["Prediction","Actual"],
              [[nested[t]["Prediction"],nested[t]["Actual"]] for t in sorted(nested)])
    execute(binary,"flow-conformal-rolling",source,[8,2,.2],derived/"nested_intervals.csv")

    source=derived/"rolling_var_breaches.csv"
    write_csv(source,["Breach"],[[r["VaRBreach"]] for r in reports["rolling_tail"]])
    execute(binary,"flow-coverage",source,[.2],derived/"rolling_var_coverage.csv")

    source=derived/"cta_net_returns.csv"
    write_csv(source,["Return"],[[paths["cta"][t]["NetReturn"]] for t in common])
    execute(binary,"flow-moving-block-paths",source,[5,20,128,2026],derived/"cta_bootstrap.csv")

    print("Completed 25 research flows and 8 chained model evaluations.")
    print("Synthetic examples only. Output directory:",out)


if __name__=="__main__":
    main()
