"""Reference fixtures, malformed CSV inputs and explicit model-domain tests."""
import sys
import tempfile
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"research"))
from advanced_study_common import CATALOG,INDEX,ROOT,near,csvfile,run,verify

binary=Path(sys.argv[1]).resolve()
assert len(CATALOG)==len(INDEX)==48
residual_names={"FixedPointResidual","ScoreResidual","ProjectedScoreResidual","StationarityResidual","ProjectedGradient","AffineResidual","PrimalResidual","DualResidual","KKTResidual","BellmanResidual","EqualityResidual","ReconstructionResidual","OrthogonalityResidual","RowMassResidual","MarginalResidual","ComplementarityResidual","MassResidual","PriceResidual","VolResidual"}
with tempfile.TemporaryDirectory() as tmp:
    f=Path(tmp)/"input.csv"
    for c in CATALOG:
        name=c["command"][9:]
        rows,_=verify(binary,c)
        for j,h in enumerate(c["header"]):
            if h in residual_names:
                for r in rows:assert abs(r[j])<=1e-6,(name,h,r[j])
        original=ROOT/c["file"]
        run(binary,name,original.with_suffix(".absent"),good=False)
        run(binary,name,params=c["params"]+["bad"],good=False)
        lines=original.read_text(encoding="utf-8").splitlines()
        row=lines[1].split(",");row[0]="nan"
        f.write_text("\n".join([lines[0],",".join(row),*lines[2:]])+"\n",encoding="utf-8")
        run(binary,name,f,good=False)

    bad=[
      ("tyler-shape",[[0,0],[1,1]],[.2,1e-9,100]),
      ("linear-gmm-two-step",[[1,1,0],[2,2,0],[3,3,0],[4,4,0]],[1,.1]),
      ("ar1-gls-profile",[[1,1],[2,1],[3,1]],[-1,.5,3]),
      ("random-intercept-reml",[[1,1,1],[1,2,1],[1,3,1]],[0,1,3]),
      ("sur-feasible-gls",[[1,2,1,1],[2,3,1,1],[3,4,1,1]],[2,1,.1]),
      ("james-stein-means",[[1],[2]],[1,0]),
      ("masked-als",[[1,1,1,2],[1,1,0,3]],[1,1,1e-9,100]),
      ("probit-ridge-fit",[[1,0],[1,2]],[.1,1e-9,100]),
      ("poisson-mixture-weight",[[1],[2]],[3,1,1e-9,100]),
      ("bernoulli-hmm-filter",[[1],[0]],[.5,0,.2,.2,.8]),
      ("recursive-least-squares",[[1,2],[1,3]],[0,1]),
      ("dirichlet-multinomial-grid",[[1,2],[3,4]],[1,2,3,.8,.8]),
      ("merton-jump-mixture",[[100,90,0,0,.2,-1,-.1,.2,1]],[10]),
      ("vasicek-bond-option",[[0,1,0,.1,2,1,.9]],[]),
      ("callable-rate-lattice",[[1,0,5,0],[3,0,5,0]],[100,.01,1]),
      ("structural-credit-fit",[[10,.5,100,0,1]],[.01,.02,1e-9]),
      ("kirk-spread",[[120,100,10,.2,.2,0,1,200]],[1e-9]),
      ("heston-full-truncation",[[100,90,0,0,.04,1,.04,.2,1.1,1]],[12,100,1]),
      ("ellipsoid-robust-qp",[[1,0,0,0,.4],[0,1,0,0,.4]],[1,1,.2,1e-9,100]),
      ("affine-box-admm",[[1,0,0,-1,1,1,1],[0,1,0,-1,1,1,1]],[2,1,1,1e-9,100,0,0]),
      ("scenario-minimax-variance",[[1,1,0],[1,0,1],[2,1,2],[2,2,1]],[1,1e-8,100]),
      ("multiasset-execution-lq",[[1,0,1,0,4],[0,-1,0,1,6]],[2,1]),
      ("log-optimal-box",[[.1,-1,.5],[-.1,.1,.5]],[1,1e-9,100]),
      ("stochastic-execution-dp",[[1.1,4,1,0]],[4,10]),
      ("gmres-restarted",[[1,0,1],[0,1,2]],[3,1e-9,100]),
      ("jacobi-eigensystem",[[1,2],[0,1]],[1e-9,100]),
      ("equality-ridge",[[0,1,1,1],[1,1,1,0],[1,2,2,0]],[.1]),
      ("ctmc-uniformization",[[-1,2],[0,0]],[1,10]),
      ("sinkhorn-log",[[0,1,.5],[1,0,.5]],[1,1e-9,100,.2,.2]),
      ("trust-region-quadratic",[[1,0,-1],[0,1,-2]],[0,1e-9]),
      ("acd-duration-grid",[[1],[1],[0],[1]],[3,.2,.2,3]),
      ("flow-propagator-ridge",[[1,1],[1,2],[1,2],[1,2]],[3,4,1]),
      ("queue-fill-uniformization",[[1]],[3,0,1,2,10]),
      ("bivariate-hawkes-likelihood",[[1,1],[2,2]],[3,1,1,1,1,0,0,1]),
      ("parzen-realized-kernel",[[.01],[-.01]],[2]),
      ("var-structural-irf",[[.5,0,1,2],[0,.2,2,1]],[2]),
      ("local-projection-hac",[[1,2],[-1,1],[1,2],[-1,1]],[3,1]),
      ("twfe-two-way-cluster",[[1,1,0,2],[1,2,1,5],[2,1,1,6],[2,1,0,5]],[]),
      ("crossfit-aipw",[[1,0,1,0],[1,0,2,1],[2,1,3,0],[2,1,4,1]],[.1,.1,.05,1e-9,100]),
      ("weibull-survival-profile",[[1,0],[2,0]],[1,2,3]),
      ("dynamic-panel-diff-iv",[[1,1,1],[1,2,2],[1,4,3],[2,1,2],[2,2,3],[2,3,4]],[]),
      ("supf-break-grid",[[0,1],[1,1],[2,1],[3,1],[4,1],[5,1]],[4]),
      ("bayesian-linear-nig",[[0,1,1],[0,1,2]],[1,2,1]),
      ("gaussian-process-grid",[[1,0,1],[0,1,2],[0,2,3]],[1,2,3,.1,1,3]),
      ("student-t-regression-em",[[1,1],[1,2]],[0,1,.1,1e-9,100]),
      ("hmm-transition-em",[[1],[0]],[.5,.5,.5,1,1e-9,100]),
      ("bayesian-online-change",[[0],[1]],[0,0,1,1]),
      ("dynamic-regression-kalman",[[1,1],[1,2]],[-1,1,1]),
    ]
    assert len(bad)==48 and {v[0] for v in bad}==set(INDEX)
    for name,rows,p in bad:
        header=INDEX[name]["columns"]
        if len(header)!=len(rows[0]):header=[f"V{i+1}" for i in range(len(rows[0]))]
        run(binary,name,csvfile(f,header,rows),p,False)
print("PASS: 48 analytical reference models, 144 malformed-input checks, 48 domain-specific guards")
