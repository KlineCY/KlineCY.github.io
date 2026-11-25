"""Model-specific mathematical-domain rejection and boundary contracts."""
import csv
import sys
import tempfile
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"research"))
from comprehensive_study_common import CATALOG,INDEX,ROOT,csvfile,run,near

# Each rejection is tied to the model's own mathematical or economic assumptions.
# Parameter tuples: position, prohibited value. Data tuples: row, column, prohibited value.
PARAM={
    "rrqr-regression":(0,1),
    "pcg-jacobi":(2,2),
    "lanczos-krylov":(0,1000),
    "polytope-analytic-center":(2,1e9),
    "pchip-monotone":(0,-1e9),
    "chebyshev-interpolant":(0,2),
    "softmax-newton-fit":(0,1),
    "ranknet-linear-fit":(0,0),
    "gamma-log-glm":(0,0),
    "ordinal-logit-fixed-cuts":(4,2),
    "ridge-loocv-gcv-path":(0,-1),
    "label-shift-prior-em":(2,.7),
    "mixture-regression-em":(4,1),
    "multitask-group-lasso":(0,1),
    "levinson-durbin-path":(0,1000),
    "arma11-profile-grid":(0,-1),
    "garch-targeted-grid":(0,1),
    "threshold-ar-fit":(0,1),
    "dynamic-factor-sequential":(0,1.01),
    "local-trend-filter-grid":(6,0),
    "markov-cumulative-reward":(0,0),
    "svi-profile-calibration":(0,-1),
    "call-curve-convex-projection":(0,0),
    "ho-lee-curve-tree":(0,0),
    "variance-strip-quadrature":(1,0),
    "heston-fourier-inversion":(0,0),
    "cir-zero-curve-grid":(1,0),
    "factor-covariance-estimate":(3,0),
    "black-litterman-correlated-views":(2,0),
    "active-tracking-frontier":(0,-1),
    "mean-semivariance-simplex":(1,0),
    "maximum-diversification-kkt":(0,0),
    "entropic-risk-allocation":(0,0),
    "empirical-bayes-alpha-grid":(0,-1),
    "rolling-panel-ridge-longshort":(3,5),
    "queue-competing-hazard-fit":(1,-1),
    "trade-signing-asof":(0,-1),
    "market-maker-cara-dp":(4,-1),
    "transient-impact-schedule":(2,0),
    "hawkes-network-em":(2,0),
    "spread-resiliency-profile":(0,0),
    "two-scale-realized-variance":(0,1),
    "anderson-rubin-inversion":(0,0),
    "quantile-regression-admm":(0,1),
    "interval-censored-discrete-em":(2,3),
    "overlap-weighted-effects":(0,-1),
    "rosenbaum-pair-bounds":(0,.5),
    "normal-hierarchical-grid":(2,-1),
    "logistic-laplace-posterior":(0,0),
    "dirichlet-transition-posterior":(1,0),
    "beta-binomial-hyper-grid":(0,0),
    "particle-sv-filter":(1,1),
    "gaussian-tail-importance":(1,0),
    "bayesian-multivariate-regression":(2,-1),
    "beta-bandit-finite-horizon":(1,0),
    "household-saving-bellman":(4,1),
    "mccall-reservation-wage":(0,1),
    "dynamic-logit-replacement":(0,1),
    "matching-free-entry-equilibrium":(1,1),
    "ces-armington-trade-counterfactual":(0,1),
    "cournot-capacity-equilibrium":(1,0),
    "ramsey-steady-state-transition":(0,1),
    "bitemporal-vintage-join":(0,-1),
    "watermark-window-ledger":(1,-1),
    "label-window-availability":(0,0),
    "train-only-winsor-standardize":(0,1.1),
}
DATA={
    "woodbury-risk-solve":(0,0,-1),
    "simplex-active-set":(0,0,-1),
    "arx-recursive-fit":(0,0,2),
    "multicurve-forward-bootstrap":(0,0,0),
    "quanto-lognormal-option":(0,5,-1),
    "price-time-matching-replay":(0,2,0),
    "aalen-johansen-incidence":(0,1,3),
    "leontief-dual-price-system":(0,0,1),
    "overlapping-label-uniqueness":(0,1,0),
    "corporate-action-return-ledger":(0,3,-1),
    "multi-currency-cash-reconciliation":(0,4,1),
}

def main(binary):
    special={"cluster-wild-sign-test","empirical-likelihood-mean","sequence-gap-replay"}
    assert set(PARAM)|set(DATA)|special==set(INDEX)
    assert len(PARAM)+len(DATA)+len(special)==80
    with tempfile.TemporaryDirectory() as tmp:
        path=Path(tmp)/"domain.csv"
        for name,(i,value) in PARAM.items():
            p=INDEX[name]["params"][:];p[i]=value
            run(binary,name,params=p,good=False)
        for name,(i,j,value) in DATA.items():
            c=INDEX[name]
            rows=list(csv.reader((ROOT/c["file"]).read_text(encoding="utf-8").splitlines()))[1:]
            rows[i][j]=value
            run(binary,name,csvfile(path,c["columns"],rows),good=False)
        run(binary,"cluster-wild-sign-test",csvfile(path,["Y","X","Cluster"],[[1,1,1],[-1,-1,1]]),[0],good=False)
        run(binary,"empirical-likelihood-mean",params=[],good=False)
        run(binary,"sequence-gap-replay",csvfile(path,["Partition","Sequence","Value"],[[1,1,1],[1,1,2]]),[1],good=False)
        # Boundary cases must be accepted rather than confused with invalid inputs.
        def calc(name,header,rows,params):
            return run(binary,name,csvfile(path,header,rows),params)[1]
        r=calc("rrqr-regression",["Y","X1","X2"],[[1,1,2],[2,2,4],[3,3,6]],[1e-10])
        assert all(row[2]==1 and row[3]<1e-8 for row in r)
        r=calc("pcg-jacobi",["A1","A2","B"],[[1,0,0],[0,2,0]],[1e-12,10,1])
        assert len(r)==1 and r[0][-1]==1
        r=calc("empirical-likelihood-mean",["Y"],[[1],[1]],[1,2])
        assert r[0][-1]==1 and r[1][-1]==0
        r=calc("mean-semivariance-simplex",["R1","R2"],[[.1,.1],[.2,.2]],[0,1,1e-10,100])
        near(r[0][3],0)
        r=calc("label-shift-prior-em",["P1","P2"],[[.5,.5],[.5,.5]],[1e-10,100,.5,.5])
        near(r[0][5],.5)
        r=calc("interval-censored-discrete-em",["L","U"],[[1,2],[1,2]],[1e-10,100,1,2])
        near(r[0][1]+r[1][1],1);near(r[0][3],0)
        r=calc("overlapping-label-uniqueness",["Start","End"],[[1,1],[2,2]],[])
        assert all(abs(row[4]-1)<1e-10 for row in r)
        r=calc("label-window-availability",["Type","Asset","Time","Value"],[[0,1,1,100],[1,1,1,2]],[1,0])
        assert r[0][-1]==r[0][-2]==0
        r=calc("watermark-window-ledger",["Event","Arrival","Value"],[[0,1,1],[2,2,2],[1,3,3]],[2,0])
        assert sum(z[4] for z in r)==1
        r=calc("corporate-action-return-ledger",["Time","Price","Split","Dividend"],[[1,50,1,0],[2,100,.5,0]],[])
        near(r[-1][-1],100)
    print("PASS: 80 model-specific domain rejections and 10 admissible boundary cases")

if __name__=="__main__":main(Path(sys.argv[1]).resolve())
