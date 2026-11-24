"""Reproduce the mathematical and financial laboratories and linked studies."""
import csv
import io
import json
import math
import subprocess
import sys
import tempfile
from pathlib import Path

root=Path(__file__).resolve().parent/"methods-examples"
binary=Path(sys.argv[1]).resolve()
out=Path(sys.argv[2])
cases=out/"cases"
studies=out/"studies"
cases.mkdir(parents=True,exist_ok=True)
studies.mkdir(parents=True,exist_ok=True)
catalog=[c for name in ["catalog-foundations.json","catalog-finance.json"] for c in json.loads((root/name).read_text(encoding="utf-8"))]
index={c["command"]:c for c in catalog}

def run(command, params=None, file=None):
    c=index[command]
    result=subprocess.run([str(binary),command,str(file or root/c["file"]),*map(str,c["params"] if params is None else params)],capture_output=True,text=True,check=True,timeout=30)
    rows=list(csv.reader(io.StringIO(result.stdout)))
    return rows[0],[[float(v) for v in r] for r in rows[1:]],result.stdout

def report(name, header, rows):
    with (studies/(name+".csv")).open("w",encoding="utf-8",newline="") as f:
        w=csv.writer(f)
        w.writerow(header)
        w.writerows(rows)

def close(a,b):
    assert math.isclose(a,b,rel_tol=1e-8,abs_tol=1e-9),(a,b)

for c in catalog:
    h,r,text=run(c["command"])
    assert h==c["header"]
    (cases/c["file"]).write_text(text,encoding="utf-8")

# 1. Compare orthogonal sparsity and spectral shrinkage on a common design.
rows=[]
with tempfile.TemporaryDirectory() as tmp:
    f=Path(tmp)/"design.csv"
    f.write_text("XTy,NormSquared\n3,9\n-1,1\n.5,1\n")
    spec=Path(tmp)/"spectrum.csv"
    spec.write_text("SingularSquared\n9\n1\n1\n")
    for lam in [.01,.1,.5,1,2,5,10]:
        _,a,_=run("methods-orthogonal-lasso",[lam],f)
        _,b,_=run("methods-ridge-effective-df",[lam],spec)
        active=sum(r[2] for r in a)
        df=b[0][2]
        close(df,sum(v/(v+lam) for v in [9,1,1]))
        rows.append([lam,active,df,*[r[1] for r in a]])
    report("sparsity_shrinkage",["Lambda","LassoActive","RidgeEffectiveDF","LassoBeta1","LassoBeta2","LassoBeta3"],rows)

# 2. Contrast state persistence and accumulated risk at multiple horizons.
rows=[]
with tempfile.TemporaryDirectory() as tmp:
    f=Path(tmp)/"h.csv"
    f.write_text("Horizon\n1\n2\n5\n10\n20\n")
    for k in [.1,.5,1.0]:
        _,a,_=run("methods-ou-transition",[k,1,.3,2],f)
        _,b,_=run("methods-markov-occupation",[.2,.4,1],f)
        for r,v in zip(a,b):
            close(r[1],1+math.exp(-k*r[0]))
            rows.append([k,r[0],r[1],r[2],v[1]])
    report("state_persistence",["Kappa","Horizon","OUConditionalMean","OUConditionalVariance","HighStateProbability"],rows)

# 3. Model prior drift and evaluate a fixed deployment cost rule.
rows=[]
with tempfile.TemporaryDirectory() as tmp:
    f=Path(tmp)/"p.csv"
    f.write_text("Probability\n.1\n.4\n.9\n")
    b=Path(tmp)/"deployment.csv"
    for prior in [.1,.2,.5,.8]:
        _,a,_=run("methods-prior-adjustment",[.5,prior],f)
        b.write_text("Probability,Event\n"+"\n".join(f"{r[1]}, {y}" for r,y in zip(a,[0,1,1]))+"\n")
        _,c,_=run("methods-binary-cost-frontier",[2,1],b)
        rows.append([prior,*[r[1] for r in a],*c[0]])
    report("prior_cost",["TargetPrior","AdjustedP1","AdjustedP2","AdjustedP3","Threshold","TP","FP","TN","FN","TotalCost"],rows)

# 4. Sensitivity of exchange and forward-start options.
rows=[]
with tempfile.TemporaryDirectory() as tmp:
    f=Path(tmp)/"x.csv"
    for rho in [-1,-.5,0,.5,1]:
        f.write_text(f"S1,S2,Vol1,Vol2,Correlation\n100,100,.2,.2,{rho}\n")
        _,a,_=run("methods-margrabe-exchange",[1,0,0],f)
        _,b,_=run("methods-forward-start",[100,0,0,.3])
        close(a[0][0]**2,.08*(1-rho))
        rows.append([rho,a[0][0],a[0][1],b[0][0],b[0][1]])
    report("relative_volatility",["Correlation","ExchangeVol","ExchangeCall","ForwardStartVol","ForwardStartCall"],rows)

# 5. Compare full portfolio loss with a disjoint set of credit tranches.
rows=[]
total=run("methods-tranche-loss",[0,1])[1][0][1]
for a,d in [(0,.05),(.05,.25),(.25,1)]:
    _,v,_=run("methods-tranche-loss",[a,d])
    rows.append([a,d,v[0][0],v[0][1],v[0][2]])
close(sum(r[3] for r in rows),total)
report("credit_loss_partition",["Attachment","Detachment","PortfolioEL","TrancheEL","TrancheLossRate"],rows)

# 6. Budget and variance attribution under an increasingly concentrated portfolio.
rows=[]
with tempfile.TemporaryDirectory() as tmp:
    f=Path(tmp)/"w.csv"
    for w in [.5,.6,.7,.8,.9,1]:
        f.write_text(f"WeightOld,WeightNew\n.5,{w}\n.5,{1-w}\n")
        _,a,_=run("methods-variance-shapley",[1,2,0,2,1,0],f)
        r=a[0]
        close(r[1]-r[0],r[2]+r[3])
        rows.append([w,*r])
    report("risk_attribution_path",["NewWeight1","OldVariance","NewVariance","WeightEffect","CovarianceEffect","Residual"],rows)

# 7. Link depth walk with execution markout and cash accounting.
rows=[]
with tempfile.TemporaryDirectory() as tmp:
    f=Path(tmp)/"fills.csv"
    for q in [1,2,3,5,6,10,12]:
        _,a,_=run("methods-depth-walk",[q])
        filled=q-a[-1][2]
        price=a[-1][3]
        f.write_text(f"SignedQty,FillPrice,Fee,Mark\n{filled},{price},0,101\n")
        _,b,_=run("methods-position-cash-reconcile",[10000],f)
        close(b[0][3]-10000,filled*(101-price))
        rows.append([q,filled,a[-1][2],price,b[0][2],b[0][3],b[0][4]])
    report("execution_equity",["Target","Filled","Remaining","VWAP","Cash","Equity","MarkedTradeImpact"],rows)

# 8. Debt dynamics under different nominal growth scenarios.
rows=[]
with tempfile.TemporaryDirectory() as tmp:
    f=Path(tmp)/"macro.csv"
    for g in [-.02,0,.03,.05,.08]:
        f.write_text("Interest,Growth,PrimarySurplus,StockFlowAdjustment\n"+"\n".join(f".05,{g},.01,0" for _ in range(5))+"\n")
        _,a,_=run("methods-debt-dynamics",[.6],f)
        for r in a:
            close(r[5],r[1]+r[2]+r[3]+r[4])
            rows.append([g,*r])
    report("debt_growth_path",["Growth","Period","PreviousDebt","InterestGrowthEffect","PrimaryEffect","StockFlow","DebtRatio"],rows)

print(f"Completed {len(catalog)} research workflows and eight linked studies")
