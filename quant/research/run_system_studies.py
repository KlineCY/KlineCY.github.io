"""Reproduce every analytical fixture, independent experiment and linked pipeline."""
import math
import sys
import tempfile
from pathlib import Path
from system_study_common import CATALOG,INDEX,verify,run,csvfile
from system_studies_timeseries import studies as timeseries
from system_studies_finance import studies as finance
from system_studies_learning import studies as learning
from system_studies_economics import studies as economics
from system_studies_linked import studies as linked

def main(binary,output):
    assert len(CATALOG)==len(INDEX)==48
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
        for workflow in (timeseries,finance,learning,economics,linked):workflow(model,report)
    assert len(produced)==60
    csvfile(output/"study-summary.csv",["Study","OutputRows"],sorted(produced.items()))
    print(f"PASS: 48 analytical model cases, 48 independent studies, 12 linked pipelines, {sum(produced.values())} output rows")

if __name__=="__main__":main(Path(sys.argv[1]).resolve(),Path(sys.argv[2]))
