import csv
import io
import json
import math
import subprocess
import sys
import tempfile
from pathlib import Path

root = Path(__file__).resolve().parents[1] / "research" / "inference-examples"
binary = str(Path(sys.argv[1]).resolve())
cases = json.loads((root / "catalog.json").read_text(encoding="utf-8"))

def run(command, file, params, good=True):
    p = subprocess.run([binary, command, str(file), *map(str, params)], capture_output=True, text=True, timeout=30)
    if not good:
        assert p.returncode == 1, (command, p.stderr)
        return None
    assert p.returncode == 0, (command, p.stderr)
    reader = csv.reader(io.StringIO(p.stdout))
    return next(reader), [list(map(float, r)) for r in reader]

def close(a,b):
    assert math.isclose(a,b,rel_tol=1e-9,abs_tol=1e-9), (a,b)

def check(rows, expected):
    assert len(rows)==len(expected)
    for a,b in zip(rows,expected):
        assert len(a)==len(b)
        for v,w in zip(a,b):
            close(v,w)

with tempfile.TemporaryDirectory() as d:
    temp=Path(d)/"sample.csv"
    for c in cases:
        file=root/c["file"]
        h,r=run(c["command"],file,c["params"])
        assert h==c["header"]
        check(r,c["expected"])
        run(c["command"],file,c["params"]+["bad"],False)
        run(c["command"],file.with_suffix(".missing"),c["params"],False)
        lines=file.read_text().splitlines()
        row=lines[1].split(",")
        row[0]="nan"
        temp.write_text("\n".join([lines[0],",".join(row),*lines[2:]])+"\n")
        run(c["command"],temp,c["params"],False)
    # Uniform beta mixture: exact expectation under every binary path.
    import itertools
    for n in range(1,7):
        total=0
        q=.3
        for path in itertools.product([0,1],repeat=n):
            temp.write_text("Event\n"+"\n".join(map(str,path))+"\n")
            _,rows=run("inference-bernoulli-mixture",temp,[q,.1])
            total+=math.exp(rows[-1][2])*q**sum(path)*(1-q)**(n-sum(path))
        close(total,1)
    # Future observations cannot affect the already observed monitoring prefix.
    for command,params in [("inference-bernoulli-mixture",[.4,.1]),("inference-bounded-betting",[.4,1,.1]),("inference-alpha-spending",[.1]),("inference-likelihood-stopping",[.2,.8,.1,.1])]:
        temp.write_text("Event\n1\n0\n1\n")
        _,a=run(command,temp,params)
        temp.write_text("Event\n1\n0\n1\n0\n0\n")
        _,b=run(command,temp,params)
        check(a,b[:len(a)])
    # Fieller quadratic equals directly evaluated sample-mean contrast.
    temp.write_text("A,B\n1,2\n3,1\n5,4\n2,3\n")
    _,rows=run("inference-fieller-grid",temp,[2,-2,2,9])
    for row in rows:
        z=row[0]
        v=[a-z*b for a,b in [(1,2),(3,1),(5,4),(2,3)]]
        m=sum(v)/4
        close(row[1],m*m-4*sum((x-m)**2 for x in v)/12)
    # The finite-grid score difference integrates to the pinball loss difference.
    temp.write_text("Y,Q1,Q2\n0,2,-1\n3,1,4\n")
    _,rows=run("inference-murphy-quantile",temp,[.5,-1,4,6])
    close(sum(r[-1] for r in rows[:-1]),.5)
    # Economic and statistical input failures.
    invalid=[
        ("inference-bernoulli-mixture","Event\n2\n",[.5,.1]),
        ("inference-bounded-betting","X\n-1\n",[.5,1,.1]),
        ("inference-alpha-spending","P\n1.1\n",[.1]),
        ("inference-likelihood-stopping","Event\n1\n",[.8,.2,.1,.1]),
        ("inference-fieller-grid","A,B\n1,2\n2,2\n",[2,0,2,3]),
        ("inference-anderson-rubin-grid","Y,X,Z\n1,1,1\n2,1,1\n3,2,1\n4,3,1\n",[4,0,2,3]),
        ("inference-jackknife-ratio","A,B\n1,1\n2,-1\n3,0\n",[]),
        ("inference-exact-sign","D\n0\n0\n",[0]),
        ("inference-brier-groups","P,Y\n.5,2\n.5,0\n",[]),
        ("inference-encompassing","Y,F1,F2\n1,0,1\n2,0,1\n3,0,1\n",[]),
        ("inference-murphy-quantile","Y,Q1,Q2\n1,0,1\n",[1,0,2,3]),
    ]
    for command,data,params in invalid:
        temp.write_text(data)
        run(command,temp,params,False)
print(f"PASS: {len(cases)} reference workflows, {sum(len(c['expected']) for c in cases)} analytic rows, malformed inputs, sequential prefix invariance and independent mathematical checks")
