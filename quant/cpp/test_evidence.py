"""Numerical references and independent invariants for evidence laboratories."""
import csv
import io
import itertools
import json
import math
import random
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "research" / "evidence-examples"
CAT = json.loads((ROOT / "catalog.json").read_text(encoding="utf-8"))
BIN = str(Path(sys.argv[1]).resolve())


def run(cmd, path, p, good=True):
    result = subprocess.run([BIN, cmd, str(path), *map(str, p)],
                            capture_output=True, text=True, timeout=30)
    if not good:
        assert result.returncode == 1 and "Quant research input error:" in result.stderr, (cmd, result.stderr)
        return None
    assert result.returncode == 0, (cmd, result.stderr)
    reader = csv.reader(io.StringIO(result.stdout))
    h = next(reader)
    rows = [list(map(float, r)) for r in reader]
    assert len(set(h)) == len(h) and rows
    assert all(len(r) == len(h) and all(math.isfinite(v) for v in r) for r in rows)
    return rows


def near(a, b):
    assert math.isclose(a, b, rel_tol=1e-8, abs_tol=1e-9), (a, b)


def compare(a, b, name):
    assert len(a) == len(b), (name, a, b)
    for row, ref in zip(a, b):
        assert len(row) == len(ref), (name, row, ref)
        for u, v in zip(row, ref):
            near(u, v)


def write(path, h, rows):
    with path.open("w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(h)
        w.writerows(rows)


def main():
    total = 0
    with tempfile.TemporaryDirectory() as d:
        temp = Path(d) / "case.csv"
        for c in CAT:
            path = ROOT / c["file"]
            rows = run(c["command"], path, c["params"])
            compare(rows, c["expected"], c["command"])
            total += len(rows)
            run(c["command"], path.with_suffix(".missing"), c["params"], False)
            run(c["command"], path, c["params"] + ["bad"], False)
            with path.open(newline="", encoding="utf-8") as f:
                data = list(csv.reader(f))
            data[1][0] = "nan"
            write(temp, data[0], data[1:])
            run(c["command"], temp, c["params"], False)
        rng = random.Random(2026)
        # Closed-form FDR and Holm references across 12 shuffled families.
        for m in range(2, 14):
            p = [rng.random() for _ in range(m)]
            write(temp, ["P"], [[v] for v in p])
            ids = sorted(range(m), key=lambda j: p[j])
            H = sum(1 / j for j in range(1, m + 1))
            bh = run("evidence-fdr-adjustment", temp, [.05])
            holm = run("evidence-holm-adjustment", temp, [.05])
            for i, j in enumerate(ids):
                q = min(1, min(m * p[ids[k]] / (k + 1) for k in range(i, m)))
                near(bh[j][2], q)
                near(bh[j][3], min(1, q * H))
                near(holm[j][2], min(1, max((m-k)*p[ids[k]] for k in range(i+1))))
        # Finite-state option reconstruction on an irregular support.
        states = [0, 1, 2.5, 4, 8]
        prob = [.1, .2, .15, .25, .3]
        D = .93
        prices = [D*sum(q*max(s-k, 0) for s,q in zip(states,prob)) for k in states]
        write(temp, ["K","C"], list(zip(states, prices)))
        result = run("evidence-call-implied-atoms", temp, [D])
        for r, p in zip(result, prob): near(r[2], p)
        near(sum(r[1] for r in result), D)
        # State price moment constraints and minimal norm under an orthogonal null perturbation.
        X = [[1,0],[1,1],[1,2]]
        write(temp, ["Cash","Payoff"], X)
        result = run("evidence-minimum-norm-sdf", temp, [1,.75])
        m = [r[1] for r in result]
        for j, q in enumerate([1,.75]): near(sum(v*x[j] for v,x in zip(m,X))/3,q)
        for t in [-1,.5,2]:
            assert sum(v*v for v in m) <= sum((v+t*z)**2 for v,z in zip(m,[1,-2,1]))+1e-10
        # Verify price and allocation conservation at multiple candidate limits.
        orders = [[1,103,4],[1,101,3],[1,101,2],[-1,99,3],[-1,101,3],[-1,101,6]]
        write(temp, ["Side","Limit","Qty"], orders)
        grid = run("evidence-call-auction-grid", temp, [101])
        chosen = next(r for r in grid if r[-1] == 1)
        near(chosen[3], max(r[3] for r in grid))
        fills = run("evidence-auction-pro-rata", temp, [chosen[0]])
        for s in [-1,1]: near(sum(r[4] for r in fills if r[1]==s),chosen[3])
        for r in fills: assert 0 <= r[4] <= r[3]
        near(fills[4][4]/fills[4][3],fills[5][4]/fills[5][3])
        # Self-financing with existing long and short positions.
        w = [[.2,.6,.01],[-.4,-.1,.02],[.5,.5,.005]]
        write(temp, ["Old","Target","Fee"], w)
        out = run("evidence-heterogeneous-cost-rebalance", temp, [])
        f = out[0][5]
        near(f, sum(r[4] for r in out))
        for r, inp in zip(out,w): near(r[3]+inp[0],(1-f)*inp[1])
        # Joint permutation reference, retaining cross-strategy dependence.
        x = [[1,-2],[3,1],[-1,2],[2,4]]
        write(temp, ["A","B"], x)
        out = run("evidence-max-signflip", temp, [])
        stats = [abs(sum(r[j] for r in x)/len(x)) for j in range(2)]
        null = [[abs(sum(s*r[j] for s,r in zip(sign,x))/len(x)) for j in range(2)]
                for sign in itertools.product([-1,1],repeat=len(x))]
        for j in range(2):
            near(out[j][2],sum(r[j]>=stats[j] for r in null)/len(null))
            near(out[j][3],sum(max(r)>=stats[j] for r in null)/len(null))
        # Domain failures exercise economic, statistical and temporal constraints.
        bad_cases = [
            ("evidence-fdr-adjustment",["P"],[[1.1]],[.05]),
            ("evidence-call-implied-atoms",["K","C"],[[0,2],[1,1.9],[2,0]],[1]),
            ("evidence-complete-replication",["Cash","Asset","Claim"],[[1,1,0],[1,3,1]],[1,4]),
            ("evidence-discrete-shift-weight",["Category","Domain","Loss"],[[0,0,1],[1,1,0]],[]),
            ("evidence-corporate-action-cashbook",["Price","Split","Dividend","Trade","Fee"],[[100,1,0,2,1]],[100,0]),
            ("evidence-cohort-att",["Unit","Time","G","Y"],[[1,0,0,1],[1,1,0,2],[2,0,-1,1],[2,1,-1,2]],[]),
            ("evidence-adaptive-prediction-set",["Split","Label","P0","P1"],[[1,0,.5,.5],[0,0,.5,.5]],[.2]),
            ("evidence-local-linear-rd",["X","Y"],[[1,1],[2,2],[3,3],[4,4]],[0,5]),
            ("evidence-delayed-entry-survival",["Enter","Exit","Event"],[[2,1,1]],[]),
            ("evidence-cscv-mean-pbo",["A","B"],[[1,2],[2,1],[3,1],[4,2]],[3]),
        ]
        for cmd,h,x,p in bad_cases:
            write(temp,h,x)
            run(cmd,temp,p,False)
    print(f"PASS: {len(CAT)} analytic reference workflows, {total} verified output rows, 96 malformed-input checks, 10 domain failures and independent randomized and structural invariants")


if __name__ == "__main__":
    main()
