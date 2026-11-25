"""Reproduce every analytical fixture, independent experiment and linked pipeline."""
import math
import sys
import tempfile
from pathlib import Path
from comprehensive_study_common import CATALOG,INDEX,verify,run,csvfile
from comprehensive_studies_numerics import studies as numerics
from comprehensive_studies_learning import studies as learning
from comprehensive_studies_dynamics import studies as dynamics
from comprehensive_studies_pricing import studies as pricing
from comprehensive_studies_portfolio import studies as portfolio
from comprehensive_studies_microstructure import studies as microstructure
from comprehensive_studies_inference import studies as inference
from comprehensive_studies_bayesian import studies as bayesian
from comprehensive_studies_economics import studies as economics
from comprehensive_studies_data import studies as data
from comprehensive_studies_linked import studies as linked

def main(binary,output):
    assert len(CATALOG)==len(INDEX)==80
    cases=output/"cases";studies=output/"studies"
    cases.mkdir(parents=True,exist_ok=True);studies.mkdir(parents=True,exist_ok=True)
    for c in CATALOG:
        _,raw=verify(binary,c)
        (cases/c["file"]).write_text(raw,encoding="utf-8")
    produced={}
    with tempfile.TemporaryDirectory() as tmp:
        input_file=Path(tmp)/"input.csv"
        def model(name,rows,params):
            header=INDEX[name]["columns"]
            if len(header)!=len(rows[0]):header=[f"V{i+1}" for i in range(len(rows[0]))]
            return run(binary,name,csvfile(input_file,header,rows),params)[1]
        def report(name,header,rows):
            assert name not in produced and rows,name
            assert all(len(r)==len(header) for r in rows),name
            assert all(math.isfinite(v) for r in rows for v in r),name
            csvfile(studies/(name+".csv"),header,rows)
            produced[name]=len(rows)
        for workflow in (numerics,learning,dynamics,pricing,portfolio,microstructure,inference,bayesian,economics,data,linked):workflow(model,report)
    assert len(produced)==92
    csvfile(output/"study-summary.csv",["Study","OutputRows"],sorted(produced.items()))
    print(f"PASS: 80 analytical model cases, 80 independent studies, 12 linked pipelines, {sum(produced.values())} output rows")

if __name__=="__main__":main(Path(sys.argv[1]).resolve(),Path(sys.argv[2]))
