"""Analytical fixture regression tests and model-domain input guards."""
import sys
import tempfile
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"research"))
from comprehensive_study_common import CATALOG,INDEX,ROOT,csvfile,run,verify

binary=Path(sys.argv[1]).resolve()
assert len(CATALOG)==len(INDEX)==80
with tempfile.TemporaryDirectory() as tmp:
    f=Path(tmp)/"input.csv"
    for c in CATALOG:
        name=c["command"][14:]
        rows,_=verify(binary,c)
        for j,h in enumerate(c["header"]):
            if h in {"ConservationResidual","ReconstructionError","StationarityResidual","BudgetResidual","KKTResidual","LinearResidual","PriceResidual","ProbabilityResidual"}:
                for r in rows:assert abs(r[j])<=1e-6,(name,h,r[j])
        if name=="pcg-jacobi":
            assert rows[-1][-1]==1 and rows[-1][2]<=1e-10,(name,rows[-1])
        original=ROOT/c["file"]
        run(binary,name,original.with_suffix(".absent"),good=False)
        run(binary,name,params=c["params"]+["bad"],good=False)
        lines=original.read_text(encoding="utf-8").splitlines()
        row=lines[1].split(",");row[0]="nan"
        f.write_text("\n".join([lines[0],",".join(row),*lines[2:]])+"\n",encoding="utf-8")
        run(binary,name,f,good=False)

print("PASS: 80 analytical reference cases and 240 malformed-input guards")
