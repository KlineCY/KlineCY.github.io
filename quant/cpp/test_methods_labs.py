import csv
import io
import json
import math
import subprocess
import sys
import tempfile
from pathlib import Path

root=Path(__file__).resolve().parents[1]/"research"/"methods-examples"
binary=str(Path(sys.argv[1]).resolve())
cat=[c for name in ["catalog-foundations.json","catalog-finance.json"] for c in json.loads((root/name).read_text(encoding="utf-8"))]

def run(command,file,params,good=True):
    p=subprocess.run([binary,command,str(file),*map(str,params)],capture_output=True,text=True,timeout=30)
    if not good:
        assert p.returncode==1,(command,p.stderr)
        return
    assert p.returncode==0,(command,p.stderr)
    reader=csv.reader(io.StringIO(p.stdout))
    return next(reader),[list(map(float,r)) for r in reader]

def near(a,b):
    assert math.isclose(a,b,rel_tol=1e-8,abs_tol=1e-9),(a,b)

with tempfile.TemporaryDirectory() as d:
    p=Path(d)/"input.csv"
    for c in cat:
        f=root/c["file"]
        h,rows=run(c["command"],f,c["params"])
        assert h==c["header"],(c["command"],h,c["header"])
        assert len(rows)==len(c["expected"])
        for a,b in zip(rows,c["expected"]):
            assert len(a)==len(b)
            for u,v in zip(a,b):
                try: near(u,v)
                except AssertionError as e: raise AssertionError((c["command"],a,b)) from e
        run(c["command"],f.with_suffix(".missing"),c["params"],False)
        run(c["command"],f,c["params"]+["bad"],False)
        data=f.read_text().splitlines()
        row=data[1].split(",")
        row[0]="nan"
        p.write_text("\n".join([data[0],",".join(row),*data[2:]])+"\n")
        run(c["command"],p,c["params"],False)

    # Direct matrix product of the rank-one inverse with the original matrix.
    _,rows=run("methods-rank-one-inverse",root/"rank-one-inverse.csv",[])
    near(rows[0][1],7/17)
    near(rows[1][1],3/17)
    # Convex projection must preserve budget and satisfy stationarity.
    p.write_text("Score\n-2\n2\n1\n.2\n")
    _,rows=run("methods-simplex-projection",p,[])
    near(sum(r[1] for r in rows),1)
    for r,z in zip(rows,[-2,2,1,.2]):
        near(r[1],max(0,z-r[2]))
    # OU conditional variances obey the Markov semigroup.
    _,rows=run("methods-ou-transition",root/"ou-transition.csv",[.7,1,2,3])
    near(rows[1][2],rows[0][2]*(1+math.exp(-1.4)))
    # Fourier conjugate symmetry for a real signal.
    a=run("methods-harmonic-power",root/"harmonic-power.csv",[1])[1][0]
    b=run("methods-harmonic-power",root/"harmonic-power.csv",[3])[1][0]
    near(a[0],b[0]);near(a[1],-b[1]);near(a[2],b[2])
    # Pairwise AUC calculated by direct enumeration with ties.
    p.write_text("Score,Event\n.2,1\n.3,0\n.4,1\n.4,0\n.5,0\n")
    _,rows=run("methods-pairwise-auc",p,[])
    expected=sum((a>b)+.5*(a==b) for a in [.2,.4] for b in [.3,.4,.5])/6
    near(rows[0][-1],expected)
    # Exchange option reduces to deterministic payoff with identical diffusion.
    p.write_text("S1,S2,Vol1,Vol2,Correlation\n110,100,.3,.3,1\n")
    _,rows=run("methods-margrabe-exchange",p,[1,0,0])
    near(rows[0][0],0);near(rows[0][1],10)
    # Tranche loss additivity for a full partition.
    total=run("methods-tranche-loss",root/"tranche-loss.csv",[0,1])[1][0][1]
    parts=sum(run("methods-tranche-loss",root/"tranche-loss.csv",[a,b])[1][0][1] for a,b in [(0,.05),(.05,.25),(.25,1)])
    near(total,parts)
    # Directionally symmetric risk attribution.
    r=run("methods-variance-shapley",root/"variance-shapley.csv",[1,2,0,2,1,0])[1][0]
    near(r[1]-r[0],r[2]+r[3]);near(r[4],0)
    # Chronology and cumulative position conservation.
    _,rows=run("methods-position-cash-reconcile",root/"position-cash-reconcile.csv",[1000])
    near(rows[-1][1],0);near(rows[-1][2],rows[-1][3])

    bad=[
        ("methods-gram-schmidt","X1,X2\n1,1\n2,2\n",[ ]),
        ("methods-rank-one-inverse","D,U\n0,1\n",[]),
        ("methods-gershgorin","A1,A2\n1,2\n3,4\n",[]),
        ("methods-spectral-participation","Eigenvalue\n-1\n",[]),
        ("methods-orthogonal-lasso","XTy,NormSquared\n1,0\n",[1]),
        ("methods-ridge-effective-df","SingularSquared\n1\n",[0]),
        ("methods-stability-frequency","Selected\n2\n",[.75]),
        ("methods-diagonal-kkt","Return,Variance\n1,-1\n",[1]),
        ("methods-quadratic-line-search","D,G,Direction\n1,1,1\n",[]),
        ("methods-scalar-riccati","Observation\n0\n",[1,1,0,0]),
        ("methods-ou-transition","DeltaT\n-1\n",[1,1,1,1]),
        ("methods-markov-occupation","Horizon\n1\n",[.2,1.1,0]),
        ("methods-innovation-likelihood","Innovation,Variance\n0,0\n",[]),
        ("methods-harmonic-power","Value\n1\n2\n",[2]),
        ("methods-weighted-effective-n","Weight\n0\n0\n",[]),
        ("methods-stratified-mean","Population,Sample,Mean,Variance\n10,11,1,1\n",[]),
        ("methods-median-of-means","Value\n1\n2\n3\n",[2]),
        ("methods-prior-adjustment","Probability\n1.1\n",[.5,.2]),
        ("methods-pairwise-auc","Score,Event\n1,1\n2,1\n",[]),
        ("methods-multiclass-brier","Label,P0,P1\n0,.5,.6\n",[]),
        ("methods-log-utility-grid","Return,Probability\n-1,1\n",[0,1,2]),
        ("methods-cara-ce","Payoff,Probability\n1,.5\n",[1]),
        ("methods-margrabe-exchange","S1,S2,V1,V2,Rho\n100,100,.2,.2,1.1\n",[1,0,0]),
        ("methods-forward-start","Start,Expiry\n1,.5\n",[100,0,0,.2]),
        ("methods-duration-immunization","Price,Duration\n100,2\n100,3\n",[1000,4]),
        ("methods-discrete-cva","Discount,Exposure,Survival\n1,100,.9\n1,100,.95\n",[.6]),
        ("methods-joint-default","P1,P2,JointPD\n.1,.2,.3\n",[]),
        ("methods-tranche-loss","Loss,Probability\n.1,1\n",[.3,.2]),
        ("methods-quote-time-weighted","Time,Bid,Ask\n0,101,100\n",[1]),
        ("methods-depth-walk","Ask,Size\n101,1\n100,1\n",[1]),
        ("methods-variance-shapley","Old,New\n.5,.5\n.5,.5\n",[1,1,2,1,1,0]),
        ("methods-binary-kelly","Return,Probability\n.2,.5\n-.1,.5\n",[0,10]),
        ("methods-ipw-balance","D,E,X\n1,1,2\n0,.5,1\n",[]),
        ("methods-debt-dynamics","I,G,S,A\n.1,-1,0,0\n",[.6]),
        ("methods-asof-staleness","Decision,Available,Value\n1,2,3\n",[1]),
        ("methods-latency-tail","Send,Exchange,Ack\n1,0,2\n",[.9]),
    ]
    for command,data,params in bad:
        p.write_text(data)
        run(command,p,params,False)
print(f"PASS: {len(cat)} analytic workflows, 192 malformed-input checks, {len(bad)} model-domain failures and independent invariants")
