"""Independent numerical references and CSV interfaces for the advanced curriculum."""
import csv
import io
import json
import math
import subprocess
from pathlib import Path
from model_study_common import near,csvfile,dot,solve,ridge,N,pdf,bs,sigmoid

ROOT=Path(__file__).resolve().parent/"advanced-examples"
GROUPS=("econometrics","learning","pricing","allocation","numerics","microstructure","inference","bayesian")
CATALOG=[c for g in GROUPS for c in json.loads((ROOT/f"catalog-{g}.json").read_text(encoding="utf-8"))]
INDEX={c["command"][9:]:c for c in CATALOG}

def run(binary,name,file=None,params=None,good=True):
    c=INDEX[name]
    args=[str(Path(binary).resolve()),c["command"],str(file or ROOT/c["file"]),*map(str,c["params"] if params is None else params)]
    proc=subprocess.run(args,capture_output=True,text=True,timeout=90)
    if not good:
        assert proc.returncode==1,(name,proc.returncode,proc.stderr)
        return None
    assert proc.returncode==0,(name,proc.returncode,proc.stderr)
    raw=list(csv.reader(io.StringIO(proc.stdout)))
    assert raw and len(raw)>1 and all(len(r)==len(raw[0]) for r in raw[1:]),name
    rows=[[float(v) for v in r] for r in raw[1:]]
    assert all(math.isfinite(v) for r in rows for v in r),name
    return raw[0],rows,proc.stdout

def verify(binary,c):
    h,rows,raw=run(binary,c["command"][9:])
    assert h==c["header"],(c["command"],h,c["header"])
    assert len(rows)==len(c["expected_prefix"]),c["command"]
    for r,s in zip(rows,c["expected_prefix"]):
        for a,b in zip(r,s):
            try:near(a,b,atol=2e-8)
            except AssertionError as e:raise AssertionError((c["command"],r,s)) from e
    return rows,raw

def inv(A):
    n=len(A)
    return list(map(list,zip(*(solve(A,[float(j==i) for j in range(n)]) for i in range(n)))))

def mv(A,x):return [dot(r,x) for r in A]
def transpose(A):return list(map(list,zip(*A)))
def mm(A,B):return [[dot(r,c) for c in transpose(B)] for r in A]
def grid(lo,hi,n,log=False):
    if log:return [math.exp(math.log(lo)+(math.log(hi)-math.log(lo))*j/(n-1)) for j in range(n)]
    return [lo+(hi-lo)*j/(n-1) for j in range(n)]
def normal_density(y,m,v):return math.exp(-.5*(y-m)**2/v)/math.sqrt(2*math.pi*v)
