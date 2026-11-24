"""Run the evidence laboratories and connected pricing and market-design studies.

The input data are synthetic mathematical examples, not live market observations.
Usage: python quant/research/run_evidence_labs.py KLINE_QUANT OUTPUT_DIRECTORY
"""
import argparse
import csv
import io
import json
import math
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent / "evidence-examples"
CAT = json.loads((ROOT / "catalog.json").read_text(encoding="utf-8"))


def write(path, header, rows):
    with path.open("w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(header)
        w.writerows(rows)


def read(path):
    with path.open(newline="", encoding="utf-8") as f:
        return [{k: float(v) for k, v in r.items()} for r in csv.DictReader(f)]


def execute(binary, command, path, params, output):
    r = subprocess.run([str(binary), command, str(path), *map(str, params)],
                       check=True, capture_output=True, text=True, timeout=120)
    data = list(csv.DictReader(io.StringIO(r.stdout)))
    if not data or any(not math.isfinite(float(v)) for row in data for v in row.values()):
        raise ValueError("Invalid numerical report: " + command)
    output.write_text(r.stdout, encoding="utf-8")
    return [{k: float(v) for k, v in row.items()} for row in data]


def near(x, y):
    if not math.isclose(x, y, rel_tol=1e-8, abs_tol=1e-9):
        raise ValueError(f"Numerical identity failed: {x} versus {y}")


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("binary", type=Path)
    p.add_argument("output", type=Path)
    args = p.parse_args()
    binary = args.binary.resolve()
    out = args.output.resolve()
    if out.exists() and any(out.iterdir()):
        raise ValueError("Output directory must be new or empty")
    out.mkdir(parents=True, exist_ok=True)
    studies = out / "studies"
    studies.mkdir()
    reports = {}
    for e in CAT:
        reports[e["fn"]] = execute(binary, e["command"], ROOT/e["file"], e["params"], out/(e["fn"]+".csv"))
    # Recover a pricing kernel, then independently evaluate its moment constraints.
    x = read(ROOT/"minimum-norm-sdf.csv")
    m = reports["minimum_norm_sdf"]
    source = studies/"sdf_payoffs.csv"
    write(source, ["SDF","Cash","Payoff"], [[v["SDF"],r["Cash"],r["Payoff"]] for v,r in zip(m,x)])
    errors = execute(binary,"evidence-sdf-pricing-errors",source,[1,.5],studies/"sdf_moment_check.csv")
    for r in errors:
        near(r["PricingError"],0)
    # Reconstruct a complete call curve from the inferred state probabilities.
    atoms = reports["call_implied_atoms"]
    original = read(ROOT/"call-implied-atoms.csv")
    reconstructed = []
    for r in original:
        k = r["Strike"]
        c = sum(a["StatePrice"] * max(a["State"]-k,0) for a in atoms)
        near(c,r["Call"])
        reconstructed.append([k,r["Call"],c,c-r["Call"]])
    write(studies/"call_curve_reconstruction.csv",["Strike","Observed","Reconstructed","Error"],reconstructed)
    # Search auction price, then allocate exactly the matched volume on each side.
    auction = next(r for r in reports["call_auction_grid"] if r["Selected"] == 1)
    fills = execute(binary,"evidence-auction-pro-rata",ROOT/"call-auction-grid.csv",
                    [auction["Price"]],studies/"cleared_orders.csv")
    for s in [-1,1]:
        near(sum(r["Filled"] for r in fills if r["Side"]==s),auction["Matched"])
    write(studies/"auction_conservation.csv",["Side","Filled","Target"],
          [[s,sum(r["Filled"] for r in fills if r["Side"]==s),auction["Matched"]] for s in [-1,1]])
    # Put the FDR and FWER adjustments for the identical family in one table.
    bh=reports["fdr_adjustment"]
    holm=reports["holm_adjustment"]
    write(studies/"multiple_testing_comparison.csv",
          ["Hypothesis","PValue","BHAdjusted","BYAdjusted","HolmAdjusted","BHReject","BYReject","HolmReject"],
          [[a["Hypothesis"],a["PValue"],a["BHAdjusted"],a["BYAdjusted"],b["HolmAdjusted"],a["BHReject"],a["BYReject"],b["Reject"]]
           for a,b in zip(bh,holm)])
    # Compute the PBO diagnostic from the actual partition output.
    splits=reports["cscv_mean_pbo"]
    write(studies/"cscv_summary.csv",["Splits","BelowMedian","PBO"],
          [[len(splits),sum(r["BelowMedian"] for r in splits),sum(r["BelowMedian"] for r in splits)/len(splits)]])
    # Separate external contributions from investment profit in the margin ledger.
    margin=reports["futures_margin_cashbook"]
    running=300
    audit=[]
    for r in margin:
        running+=r["VariationMargin"]-r["Fee"]
        near(running,r["EquityNetOfTopups"])
        audit.append([r["Period"],r["Cash"],r["ExternalTopup"],running,r["EquityNetOfTopups"]])
    write(studies/"margin_economic_equity.csv",["Period","Cash","Topup","ReconstructedEquity","ReportedEquity"],audit)
    print("Completed 32 evidence laboratories and 6 linked research studies.")


if __name__ == "__main__":
    main()
