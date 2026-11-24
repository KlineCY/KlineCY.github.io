import csv
import io
import json
import math
import subprocess
import sys
import tempfile
from pathlib import Path

root=Path(__file__).resolve().parents[1]/"research"/"applied-examples"
binary=str(Path(sys.argv[1]).resolve())
cat=json.loads((root/"catalog.json").read_text(encoding="utf-8"))

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
    # Independent leave-one-out least squares refits.
    x=[-2,0,1,4,7]
    y=[1,2,0,3,8]
    p.write_text("X,Y\n"+"\n".join(f"{a},{b}" for a,b in zip(x,y))+"\n")
    _,rows=run("applied-press",p,[])
    for i,r in enumerate(rows):
        xx=[v for j,v in enumerate(x) if j!=i]
        yy=[v for j,v in enumerate(y) if j!=i]
        a=sum(xx)/len(xx);b=sum(yy)/len(yy)
        slope=sum((u-a)*(v-b) for u,v in zip(xx,yy))/sum((u-a)**2 for u in xx)
        near(r[-1],y[i]-(b+slope*(x[i]-a)))
    p.write_text("K1,K2,K3\n1,2,3\n2,4,6\n3,6,9\n")
    _,rows=run("applied-gram-centering",p,[])
    for r in rows: near(sum(r[1:]),0)
    for j in range(1,4): near(sum(r[j] for r in rows),0)
    # Competing risk mass conservation under tied observations.
    p.write_text("Time,Event\n1,1\n1,2\n1,0\n2,1\n3,0\n")
    _,rows=run("applied-competing-incidence",p,[])
    for r in rows: near(sum(r[5:]),1)
    # Geometric Asian put-call parity.
    _,rows=run("applied-geometric-asian",root/"geometric-asian.csv",[100,100,.03,.01,.25,1])
    m,v,c,put=rows[0]
    near(c-put,math.exp(-.03)*(math.exp(m+v/2)-100))
    p.write_text("Portfolio,Benchmark,ComponentA,ComponentB\n.03,.01,.01,.01\n-.02,.04,-.04,-.02\n.07,.02,.02,.03\n")
    _,rows=run("applied-carino",p,[])
    near(sum(sum(r[3:]) for r in rows),1.03*.98*1.07-1.01*1.04*1.02)
    p.write_text("MarketVolume\n10\n0\n200\n")
    _,rows=run("applied-participation-cap",p,[20,.15])
    near(sum(r[2] for r in rows)+rows[-1][3],20)
    for r in rows: assert r[-1]<=.15+1e-12
    bad=[
        ("applied-vif","X1,X2\n1,1\n2,2\n3,3\n",[]),
        ("applied-gram-centering","K1,K2\n1,2\n3,4\n",[]),
        ("applied-soft-label","P,Q\n.4,.5\n.4,.5\n",[]),
        ("applied-option-box","K,C,P\n110,1,2\n90,2,1\n",[]),
        ("applied-daily-leverage","Return\n-.6\n",[2,0]),
        ("applied-netting","A,B,C\n1,2,-1\n",[]),
        ("applied-competing-incidence","Time,Event\n1,3\n",[]),
        ("applied-participation-cap","MarketVolume\n-1\n",[10,.1]),
        ("applied-ar-aggregation","Horizon\n3\n",[1,2]),
        ("applied-carino","P,B,A,C\n.1,.05,.1,.1\n",[]),
    ]
    for command,data,params in bad:
        p.write_text(data)
        run(command,p,params,False)
print(f"PASS: {len(cat)} analytic workflows, independent numerical invariants and invalid-input rejection")
