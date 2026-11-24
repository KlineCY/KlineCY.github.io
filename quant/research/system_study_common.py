"""Independent reference calculations and executable research-model interface."""
import csv
import io
import json
import math
import subprocess
from pathlib import Path
from advanced_study_common import near,csvfile,dot,solve,ridge,inv,mv,mm,transpose,grid,N,pdf,bs,sigmoid

ROOT=Path(__file__).resolve().parent/"system-examples"
GROUPS=("timeseries","portfolio","derivatives","execution","learning","inference","economics","engineering")
CATALOG=[c for g in GROUPS for c in json.loads((ROOT/f"catalog-{g}.json").read_text(encoding="utf-8"))]
INDEX={c["command"][7:]:c for c in CATALOG}

def run(binary,name,file=None,params=None,good=True):
    c=INDEX[name]
    args=[str(Path(binary).resolve()),c["command"],str(file or ROOT/c["file"]),*map(str,c["params"] if params is None else params)]
    proc=subprocess.run(args,capture_output=True,text=True,timeout=90)
    if not good:
        assert proc.returncode==1,(name,proc.returncode,proc.stderr)
        return None
    assert proc.returncode==0,(name,proc.returncode,proc.stderr)
    raw=list(csv.reader(io.StringIO(proc.stdout)))
    assert len(raw)>1 and all(len(r)==len(raw[0]) for r in raw[1:]),name
    rows=[[float(v) for v in r] for r in raw[1:]]
    assert all(math.isfinite(v) for r in rows for v in r),name
    return raw[0],rows,proc.stdout

def verify(binary,c):
    h,rows,raw=run(binary,c["command"][7:])
    assert h==c["header"],(c["command"],h,c["header"])
    assert len(rows)==len(c["expected_prefix"]),(c["command"],len(rows),len(c["expected_prefix"]))
    for r,s in zip(rows,c["expected_prefix"]):
        for a,b in zip(r,s):
            try:near(a,b,atol=2e-8)
            except AssertionError as e:raise AssertionError((c["command"],r,s)) from e
    return rows,raw

def check_vector(actual,expected,atol=2e-8):
    assert len(actual)==len(expected),(actual,expected)
    for a,b in zip(actual,expected):near(a,b,atol=atol)

def bartlett(values,L):
    n=len(values);m=sum(values)/n
    z=[v-m for v in values]
    return sum(z[i]*z[j]*(1-abs(i-j)/(L+1)) for i in range(n) for j in range(n) if abs(i-j)<=L)/n
