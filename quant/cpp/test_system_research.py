"""Analytical fixture regression tests and model-domain input guards."""
import sys
import tempfile
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"research"))
from system_study_common import CATALOG,INDEX,ROOT,csvfile,run,verify

binary=Path(sys.argv[1]).resolve()
assert len(CATALOG)==len(INDEX)==48
with tempfile.TemporaryDirectory() as tmp:
    f=Path(tmp)/"input.csv"
    for c in CATALOG:
        name=c["command"][7:]
        rows,_=verify(binary,c)
        for j,h in enumerate(c["header"]):
            if h in {"ConservationResidual","ReconstructionError","StationarityResidual","BudgetResidual","KKTResidual","LinearResidual","PriceResidual","ProbabilityResidual","ResidualNorm"}:
                for r in rows:assert abs(r[j])<=1e-6,(name,h,r[j])
        original=ROOT/c["file"]
        run(binary,name,original.with_suffix(".absent"),good=False)
        run(binary,name,params=c["params"]+["bad"],good=False)
        lines=original.read_text(encoding="utf-8").splitlines()
        row=lines[1].split(",");row[0]="nan"
        f.write_text("\n".join([lines[0],",".join(row),*lines[2:]])+"\n",encoding="utf-8")
        run(binary,name,f,good=False)

    bad=[
      ("haar-energy",[[1],[2],[3]],[1]),
      ("fractional-filter",[[1],[2]],[-1,1]),
      ("ou-irregular-profile",[[0,1],[1,2],[1,3],[2,4]],[.1,1,3]),
      ("fourier-seasonal-fit",[[0,1,0],[1,2,0],[2,3,0]],[4,1,.1]),
      ("garch-horizon-risk",[[.04,.01,.6,.5,0]],[3]),
      ("rolling-forecast-comparison",[[1,1,1],[2,2,2]],[2]),
      ("factor-replicating-gls",[[1,0,1,2],[0,1,1,2]],[1,2]),
      ("rolling-risk-scaling",[[.01],[-1.1],[.03]],[2,.1,1,0,1]),
      ("minimum-variance-hedge",[[1,1,0],[2,2,0],[3,3,1]],[.1]),
      ("scenario-tail-attribution",[[.4,1],[.4,2]],[.9,1]),
      ("cashflow-portfolio-unitization",[[-100,0]],[100,1]),
      ("portfolio-brinson-linked",[[1,1,.5,1,.1,.1],[1,2,.4,0,.1,.1]],[]),
      ("tridiagonal-cn-option",[[100,100,0,0,.2,1]],[20,20,90,1]),
      ("asian-geometric-exact",[[100,100,0,0,-.2,1]],[4]),
      ("asian-arithmetic-control",[[100,100,0,0,.2,1]],[4,1,100,1]),
      ("cashflow-zspread-fit",[[1,100,1]],[500,-.1,.1,1e-9]),
      ("swap-curve-keyrate",[[2,1,1],[1,1,1]],[100,.03,1e-5]),
      ("credit-tranche-loss",[[1,.2]],[0,.5,.4,1]),
      ("venue-convex-routing",[[1,0,1,2]],[3]),
      ("fifo-position-ledger",[[1,10,10,-1]],[100]),
      ("futures-margin-ledger",[[100,1,0]],[100,100,10,.1,.2]),
      ("fx-cash-ledger",[[1,0,0,0,0]],[100,0,2]),
      ("integer-lot-allocation",[[1,1.5,2,4]],[10]),
      ("batch-auction-clearing",[[1,10,1],[1,11,2]],[10]),
      ("expert-mixture-logwealth",[[.01,-1]],[.5,.5]),
      ("gaussian-crps-score",[[1,0,0]],[]),
      ("cost-sensitive-threshold",[[.1,0,0],[.9,1,0]],[1,1]),
      ("conformal-classification-set",[[1,1,.5,.8],[0,2,.2,.8]],[.1]),
      ("pairwise-rank-logistic",[[1,1,1],[1,2,1]],[]),
      ("weighted-gaussian-discriminant",[[1,0,1],[1,0,2],[0,1,3]],[.1]),
      ("ols-hc3-inference",[[1,1,0],[2,1,0],[3,1,1]],[]),
      ("quantile-crossing-projection",[[0,1,2]],[.8,.2,1,1]),
      ("permutation-randomization",[[1,0],[2,0]],[]),
      ("kaplan-meier-rmst",[[1,1],[2,0]],[3]),
      ("bunching-rd-local",[[1,1],[2,2],[3,3],[4,4]],[0,5,0]),
      ("newey-west-long-run",[[1],[2]],[2]),
      ("ces-cost-demand",[[1,.6],[2,.6]],[1,10]),
      ("logit-market-share",[[1,-1]],[1]),
      ("solow-transition",[[.2,.1,0,0,1]],[4,2]),
      ("two-period-consumption",[[10,10,0,1,1,10]],[]),
      ("input-output-carbon",[[0,1.1,1,1],[.2,0,2,1]],[]),
      ("market-clearing-tatonnement",[[1,0,-1],[0,1,1]],[]),
      ("corporate-action-ledger",[[100,2,0],[50,1,0]],[1]),
      ("survivorship-cohort",[[1,.5,.1,1],[2,.5,-.1,1]],[]),
      ("lagged-asof-panel",[[1,1,0,1],[1,1,0,2],[0,1,1,0]],[1]),
      ("session-ohlcv-bars",[[0,10,1],[3,10,1]],[0,3,1]),
      ("cross-sectional-weighted-standardize",[[1,1,0]],[]),
      ("label-interval-purge-folds",[[0,1,1],[2,3,1]],[1]),
    ]
    assert len(bad)==48 and {v[0] for v in bad}==set(INDEX)
    for name,rows,p in bad:
        header=INDEX[name]["columns"]
        if len(header)!=len(rows[0]):header=[f"V{i+1}" for i in range(len(rows[0]))]
        run(binary,name,csvfile(f,header,rows),p,False)
print("PASS: 48 analytical reference cases, 144 malformed-input tests, 48 model-domain guards")
