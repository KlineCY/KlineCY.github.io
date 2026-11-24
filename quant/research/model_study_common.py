"""CSV execution and independent numerical references for the model research suite."""
import csv
import io
import json
import math
import subprocess
from pathlib import Path

ROOT=Path(__file__).resolve().parent/"model-examples"
GROUPS=("optimization","likelihood","dynamics","pricing","portfolio","systems")
CATALOG=[c for group in GROUPS for c in json.loads((ROOT/f"catalog-{group}.json").read_text(encoding="utf-8"))]
INDEX={c["command"][6:]:c for c in CATALOG}

def near(a,b,rtol=1e-7,atol=1e-8):
    if not math.isclose(a,b,rel_tol=rtol,abs_tol=atol):
        raise AssertionError((a,b,rtol,atol))

def csvfile(path,header,rows):
    with Path(path).open("w",encoding="utf-8",newline="") as f:
        w=csv.writer(f)
        w.writerow(header)
        w.writerows(rows)
    return Path(path)

def run(binary,name,file=None,params=None,good=True):
    c=INDEX[name]
    a=[str(Path(binary).resolve()),c["command"],str(file or ROOT/c["file"]),*map(str,c["params"] if params is None else params)]
    p=subprocess.run(a,capture_output=True,text=True,timeout=90)
    if not good:
        assert p.returncode==1,(name,p.returncode,p.stderr)
        return None
    assert p.returncode==0,(name,p.returncode,p.stderr)
    r=list(csv.reader(io.StringIO(p.stdout)))
    assert r and len(r)>1 and all(len(row)==len(r[0]) for row in r[1:]),name
    rows=[[float(v) for v in row] for row in r[1:]]
    assert all(math.isfinite(v) for row in rows for v in row),name
    return r[0],rows,p.stdout

def verify(binary,c):
    header,rows,raw=run(binary,c["command"][6:])
    assert header==c["header"],(c["command"],header,c["header"])
    assert len(rows)==len(c["expected_prefix"]),c["command"]
    for row,expected in zip(rows,c["expected_prefix"]):
        for a,b in zip(row,expected):
            try: near(a,b)
            except AssertionError as e: raise AssertionError((c["command"],row,expected)) from e
    return rows,raw

def dot(a,b):
    assert len(a)==len(b)
    return sum(x*y for x,y in zip(a,b))

def solve(A,b):
    """Independent pivoted Gaussian elimination, not the C++ Cholesky routine."""
    n=len(b)
    M=[list(row)+[v] for row,v in zip(A,b)]
    for k in range(n):
        j=max(range(k,n),key=lambda i:abs(M[i][k]))
        assert abs(M[j][k])>1e-15,"Singular reference system"
        M[k],M[j]=M[j],M[k]
        for i in range(k+1,n):
            f=M[i][k]/M[k][k]
            for l in range(k,n+1): M[i][l]-=f*M[k][l]
    x=[0.]*n
    for i in range(n-1,-1,-1):
        x[i]=(M[i][-1]-sum(M[i][j]*x[j] for j in range(i+1,n)))/M[i][i]
    return x

def ridge(X,y,lam,intercept=False):
    d=len(X[0])
    A=[[sum(r[i]*r[j] for r in X)+(lam if i==j and (not intercept or i!=0) else 0) for j in range(d)] for i in range(d)]
    b=[sum(r[i]*z for r,z in zip(X,y)) for i in range(d)]
    return solve(A,b)

def sigmoid(z):
    if z>=0:return 1/(1+math.exp(-z))
    a=math.exp(z)
    return a/(1+a)

def N(z):return .5*math.erfc(-z/math.sqrt(2))
def pdf(z):return math.exp(-.5*z*z)/math.sqrt(2*math.pi)
def bs(s,k,r,q,v,t,call=True):
    d1=(math.log(s/k)+(r-q+.5*v*v)*t)/(v*math.sqrt(t))
    d2=d1-v*math.sqrt(t)
    c=s*math.exp(-q*t)*N(d1)-k*math.exp(-r*t)*N(d2)
    return c if call else c-s*math.exp(-q*t)+k*math.exp(-r*t)
