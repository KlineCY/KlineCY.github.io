"""Reference, invalid-domain and structural tests for the 96 research methods."""
import csv
import io
import json
import math
import subprocess
import sys
import tempfile
from pathlib import Path

root=Path(__file__).resolve().parents[1]/"research"/"depth-examples"
binary=str(Path(sys.argv[1]).resolve())
cat=[c for name in ["foundations","learning","finance","research"] for c in json.loads((root/f"catalog-{name}.json").read_text(encoding="utf-8"))]
index={c["command"][6:]:c for c in cat}
assert len(cat)==len(index)==96

def run(name,file=None,params=None,good=True):
    c=index[name]
    p=subprocess.run([binary,c["command"],str(file or root/c["file"]),*map(str,c["params"] if params is None else params)],capture_output=True,text=True,timeout=30)
    if not good:
        assert p.returncode==1,(name,p.returncode,p.stderr)
        return
    assert p.returncode==0,(name,p.stderr)
    reader=csv.reader(io.StringIO(p.stdout))
    return next(reader),[list(map(float,r)) for r in reader]

def near(a,b,rel=1e-8,absolute=1e-9):
    assert math.isclose(a,b,rel_tol=rel,abs_tol=absolute),(a,b)

with tempfile.TemporaryDirectory() as d:
    path=Path(d)/"input.csv"
    def data(header,rows):
        with path.open("w",newline="",encoding="utf-8") as f:
            w=csv.writer(f)
            w.writerow(header)
            w.writerows(rows)
        return path

    for c in cat:
        name=c["command"][6:]
        f=root/c["file"]
        h,rows=run(name)
        assert h==c["header"],(name,h,c["header"])
        assert len(rows)==len(c["expected"]),name
        for a,b in zip(rows,c["expected"]):
            assert len(a)==len(b),name
            for u,v in zip(a,b):
                try: near(u,v)
                except AssertionError as e: raise AssertionError((name,a,b)) from e
        run(name,file=f.with_suffix(".missing"),good=False)
        run(name,params=c["params"]+["bad"],good=False)
        lines=f.read_text().splitlines()
        row=lines[1].split(",")
        row[0]="nan"
        path.write_text("\n".join([lines[0],",".join(row),*lines[2:]])+"\n")
        run(name,file=path,good=False)

    # Each row is one model-specific domain failure, distinct from malformed CSV.
    bad=[
        ("backward-error",[[1,2,1,0],[2,1,2,0]],[]),
        ("row-equilibration",[[0,0,1]],[]),
        ("two-by-two-spectrum",[[1,2],[3,1]],[]),
        ("precision-conditional",[[1,2],[2,1]],[]),
        ("group-prox",[[1,2]],[-1]),
        ("affine-projection",[[1,0],[2,0]],[1]),
        ("bounded-budget",[[1,1,0,.4],[2,1,0,.4]],[1]),
        ("huber-prox",[[1]],[1,0]),
        ("discrete-convolution",[[.5,.5],[.4,.5]],[]),
        ("latent-binomial",[[1.1,1]],[2]),
        ("exponential-tilt",[[0,.2],[1,.3]],[1]),
        ("randomized-pit",[[.7,.5,.2]],[]),
        ("reliability-variance",[[1,1],[2,0]],[]),
        ("pairwise-missing",[[1,1,1,0],[2,2,0,1]],[]),
        ("group-jackknife",[[1,1],[1,2],[2,3]],[]),
        ("trim-winsor",[[1],[2]],[1]),
        ("ar-impulse",[[1]],[1]),
        ("ar-forecast-variance",[[0]],[.5,1]),
        ("filter-frequency",[[1]],[-.1]),
        ("lag-aligned-cov",[[1,2],[3,4]],[1]),
        ("paired-student",[[2,1],[3,2]],[]),
        ("equivalence-tost",[[0,0]],[.5,1.96]),
        ("survey-hajek",[[1,0]],[10]),
        ("fisher-exact",[[1,0],[2,0]],[]),
        ("nystrom-schur",[[1,0,2],[0,1,2]],[1]),
        ("rff-pair",[[1,0]],[1]),
        ("mahalanobis-residual",[[1,2,1],[2,1,1]],[]),
        ("outofsample-kernel-center",[[1,2],[2,4]],[2]),
        ("binary-focal",[[0,2]],[1]),
        ("huberized-pinball",[[1]],[1,1]),
        ("logit-intercept-score",[[0,2]],[0]),
        ("temperature-curvature",[[2,1,0]],[1]),
        ("logistic-hvp",[[1,0,2]],[0,0,1,1]),
        ("softmax-jvp",[[0,1]],[]),
        ("gradient-noise",[[1,0],[-1,0]],[]),
        ("global-gradient-clip",[[1]],[0]),
        ("purge-interval",[[2,1]],[0,1,0]),
        ("train-only-standardization",[[1,2],[1,2],[0,3]],[]),
        ("forecast-oos-r2",[[1,2,1]],[]),
        ("block-loss-decomposition",[[1,-1,1]],[]),
        ("mixture-predictive-variance",[[1,1,-1]],[]),
        ("ensemble-entropy-mi",[[1,.6,.6]],[]),
        ("mondrian-calibration",[[1,1]],[.1]),
        ("wilson-coverage",[[3,2]],[1.96]),
        ("weighted-group-demean",[[1,2,0]],[]),
        ("train-missing-indicator",[[1,0,0],[0,1,2]],[]),
        ("ew-prior-zscore",[[1]],[.5,0,0]),
        ("regime-covariance",[[.5,1,2]],[]),
        ("digital-decomposition",[[100,100,0,0,0,1]],[]),
        ("vanna-volga",[[100,100,0,0,.2,0]],[]),
        ("delta-hedge-variance",[[100,.02,.01,0]],[.2]),
        ("spot-vol-taylor",[[1,.5,.02,10,.1,2]],[1]),
        ("log-discount-forward",[[1,1],[1,.9]],[]),
        ("forward-swap-annuity",[[0,.9]],[1,100,.03]),
        ("fra-settlement",[[-2,.5]],[100,.03]),
        ("overnight-accrual-bridge",[[-2,.5]],[]),
        ("node-martingale",[[1,100,100,.8,1]],[]),
        ("numeraire-measure",[[1,2,1]],[]),
        ("stopping-policy-value",[[1,2,3,2,0]],[]),
        ("dual-stopping-bound",[[1,2,3,1,0,0]],[]),
        ("forward-default",[[0,1,.8,.9]],[]),
        ("margin-period-risk",[[1,-1,2]],[]),
        ("heterogeneous-credit-mixture",[[1,.5,10,.1],[1,.4,20,.2]],[]),
        ("exponential-hazard-mle",[[0,1]],[]),
        ("ellipsoid-worst-mean",[[1,.1,-.2]],[1]),
        ("stochastic-dominance",[[1,.5,.5],[1,.5,.5]],[]),
        ("single-asset-no-trade",[[2,.5,1]],[.1,0,1]),
        ("mean-estimation-risk",[[1,2,.5],[2,1,.5]],[100]),
        ("forward-variance",[[1,2,.4,.1]],[]),
        ("noise-signature",[[1]],[.04,-.01]),
        ("lognormal-vol-convexity",[[.04,-.1]],[]),
        ("gjr-news-impact",[[1]],[.1,.1,-.2,.7,1]),
        ("execution-kkt",[[1,1,0],[1,1,0]],[3]),
        ("competing-execution-hazards",[[0,0]],[1]),
        ("queue-cancel-bounds",[[-1,0]],[5,3]),
        ("quote-bayesian-selection",[[.5,1.1]],[90,110]),
        ("bitemporal-asof",[[0,1,1,2,3],[0,1,1,2,4],[1,1,2,2,0]],[10]),
        ("event-watermark",[[1]],[-1]),
        ("event-idempotency",[[1,1,100,0],[1,2,100,0]],[]),
        ("tick-grid",[[100]],[.05,.03]),
        ("unbalanced-two-way-projection",[[1,1,2],[1,1,3]],[1e-12,100]),
        ("manski-missing-bounds",[[1,11]],[0,10]),
        ("lee-trimming-bounds",[[1,1,2],[1,0,0],[0,1,3],[0,1,4]],[]),
        ("synthetic-control-gap",[[0,1,1,1],[1,2,1,1]],[.5,.5]),
        ("leontief-demand",[[1,0,1],[0,.1,1]],[1e-12,100]),
        ("fisher-price-index",[[1,0,1,0]],[]),
        ("shift-share-exposure",[[1,1,1,.1],[2,1,1,.2]],[]),
        ("taylor-inertia",[[.02,0]],[.01,.02,1.5,.5,1,.03]),
        ("mixed-nash-interior",[[1,1,0,1],[0,0,1,0]],[]),
        ("cournot-active-set",[[1]],[10,0]),
        ("auction-uniform-bid",[[1.1]],[3]),
        ("kyle-linear-equilibrium",[[1,0]],[]),
        ("tie-aware-tail-weights",[[1],[2]],[.6]),
        ("size-value-six-cells",[[0,0,1,.1]],[]),
        ("beta-posterior-shrink",[[1,0]],[1,1]),
        ("constant-mix-diversification",[[-1,0]],[.5]),
    ]
    assert len(bad)==96 and {r[0] for r in bad}==set(index)
    for name,rows,params in bad:
        run(name,data(index[name]["columns"],rows),params,False)

    def custom(name,rows,params):
        return run(name,data(index[name]["columns"],rows),params)[1]

    # Independent mathematical invariants, one for each of the 24 topics.
    a=custom("backward-error",[[3,2,1,6],[1,4,2,10]],[])[0]
    b=custom("backward-error",[[30,20,1,60],[10,40,2,100]],[])[0]
    near(a[1],b[1]);near(a[2],b[2]);near(10*a[0],b[0])
    r=custom("bounded-budget",[[.3,2,.1,.5],[.4,1,0,.8],[.1,3,.1,.4]],[1])
    near(sum(x[1] for x in r),1)
    assert all(x[-1]<1e-9 for x in r)
    a=[.2,.3,.5];b=[.6,.1,.3]
    r=custom("discrete-convolution",list(zip(a,b)),[])
    near(sum(x[1] for x in r),1)
    near(sum(x[0]*x[1] for x in r),sum(i*(u+v) for i,(u,v) in enumerate(zip(a,b))))
    a=custom("reliability-variance",[[1,2],[4,5],[9,3]],[])[0]
    b=custom("reliability-variance",[[1,20],[4,50],[9,30]],[])[0]
    for u,v in zip(a,b): near(u,v)
    r=custom("ar-forecast-variance",[[1],[4],[9]],[-.6,3])
    for h,v,longrun in r:
        near(v,3*sum(.6**(2*j) for j in range(int(h))))
        assert v<=longrun+1e-12
    a=custom("fisher-exact",[[1,9],[11,3]],[])[0]
    b=custom("fisher-exact",[[1,11],[9,3]],[])[0]
    c=custom("fisher-exact",[[11,3],[1,9]],[])[0]
    near(a[1],b[1]);near(a[1],c[1])
    exact=sum(math.comb(12,k)*math.comb(12,10-k)/math.comb(24,10) for k in range(11) if math.comb(12,k)*math.comb(12,10-k)<=math.comb(12,1)*math.comb(12,9))
    near(a[1],exact)
    r=custom("outofsample-kernel-center",[[1,2],[4,3],[7,4]],[3])
    near(sum(x[1] for x in r),0)
    r=custom("binary-focal",[[-3,0],[.7,1],[4,0]],[0])
    for z,y,L,g in r:
        near(L,math.log1p(math.exp(z))-y*z)
        near(g,1/(1+math.exp(-z))-y)
    sample=[[1,2,1],[-1,3,0],[2,-2,1]]
    w=[.2,-.4];v=[.3,.6];eps=1e-5
    a=custom("logistic-hvp",sample,w+v)
    b=custom("logistic-hvp",sample,[w[i]+eps*v[i] for i in range(2)]+v)
    c=custom("logistic-hvp",sample,[w[i]-eps*v[i] for i in range(2)]+v)
    for i in range(2): near(a[i][2],(b[i][1]-c[i][1])/(2*eps))
    assert a[0][3]>=0
    sample=[[1,1],[1,3],[0,5]]
    a=custom("train-only-standardization",sample,[])
    b=custom("train-only-standardization",sample+[[0,10000]],[])
    for r,s in zip(a,b):
        for u,v in zip(r,s): near(u,v)
    r=custom("ensemble-entropy-mi",[[.2,.3,.7],[.8,.3,.7]],[])
    near(r[0][2],0)
    a=custom("ew-prior-zscore",[[1],[2],[3]],[.2,0,1])
    b=custom("ew-prior-zscore",[[1],[2],[3],[999]],[.2,0,1])
    for r,s in zip(a,b):
        for u,v in zip(r,s): near(u,v)
    s,k,r,q,v,T=105,100,.03,.01,.3,2
    a=custom("digital-decomposition",[[s,k,r,q,v,T]],[])
    d1=(math.log(s/k)+(r-q+.5*v*v)*T)/(v*math.sqrt(T))
    d2=d1-v*math.sqrt(T)
    N=lambda z:.5*math.erfc(-z/math.sqrt(2))
    near(a[0][2],s*math.exp(-q*T)*N(d1)-k*math.exp(-r*T)*N(d2))
    a=custom("fra-settlement",[[-.01,.25],[.07,.5]],[1200,.02])
    for row in a: near(row[3]*(1+row[0]*row[1]),row[2])
    a=custom("dual-stopping-bound",[[1,4,3,0,0,0],[1,2,5,0,0,0]],[])
    near(a[-1][-1],4.5)
    a=custom("exponential-hazard-mle",[[2,0],[3,0]],[])
    near(a[0][2],0);near(a[0][3],0);near(a[0][4],0)
    w=[.6,-.2,.6];mu=[.1,.2,.05];scale=[.02,.03,.01];radius=1.5
    a=custom("ellipsoid-worst-mean",list(zip(w,mu,scale)),[radius])
    z=[-radius*u*v/a[0][1] for u,v in zip(w,scale)]
    near(math.sqrt(sum(v*v for v in z)),radius)
    near(sum(u*(v+s*t) for u,v,s,t in zip(w,mu,scale,z)),a[0][2])
    a=custom("forward-variance",[[0,1,0,.2],[1,3,.2,.25],[0,3,0,.25]],[])
    near(a[0][4]+2*a[1][4],3*a[2][4])
    a=custom("queue-cancel-bounds",[[1,0],[2,2],[1,4],[4,0]],[6,2])
    for i,row in enumerate(a):
        assert 0<=row[3]<=row[4]<=2 and row[1]<=row[2]
        if i: assert row[3]>=a[i-1][3] and row[4]>=a[i-1][4]
    a=custom("bitemporal-asof",[[0,1,1,2,10],[1,1,3,3,0]],[100])[0]
    b=custom("bitemporal-asof",[[0,1,1,2,10],[1,1,3,3,0],[0,1,1,5,20]],[100])[0]
    assert a==b
    panel=[[1,1,1],[1,2,3],[2,1,2],[2,2,6],[2,3,8],[3,2,4],[3,3,5]]
    a=custom("unbalanced-two-way-projection",panel,[1e-12,1000])
    b=custom("unbalanced-two-way-projection",[[i,t,y+3*i-2*t] for i,t,y in panel],[1e-12,1000])
    for r,s in zip(a,b): near(r[3],s[3])
    for k in [1,2]:
        for g in set(r[k] for r in a): near(sum(r[3] for r in a if r[k]==g),0)
    a=custom("leontief-demand",[[.1,.2,2],[.3,.1,4]],[1e-12,200])
    b=custom("leontief-demand",[[.1,.2,6],[.3,.1,12]],[1e-12,200])
    for r,s in zip(a,b): near(3*r[2],s[2])
    costs=[1,4,8,20]
    a=custom("cournot-active-set",[[v] for v in costs],[15,2])
    near(a[0][2],15-2*sum(r[1] for r in a))
    for c,r in zip(costs,a):
        assert r[1]>=0 and r[-1]<1e-9
        near(r[3],(r[2]-c)*r[1])
    scores=[1,1,2,2,2,4,5]
    a=custom("tie-aware-tail-weights",[[v] for v in scores],[.3])
    b=custom("tie-aware-tail-weights",[[v] for v in reversed(scores)],[.3])
    near(sum(r[2] for r in a),1);near(sum(r[3] for r in a),1);near(sum(r[4] for r in a),0)
    for r,s in zip(a,reversed(b)):
        for u,v in zip(r[1:],s[1:]): near(u,v)

print("PASS: 96 reference workflows, 288 malformed inputs, 96 model-domain failures and 24 independent structural checks")
