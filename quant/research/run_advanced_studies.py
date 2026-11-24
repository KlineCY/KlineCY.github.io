"""Run every analytical case and all independent advanced-model studies."""
import math
import sys
import tempfile
from pathlib import Path
from advanced_study_common import CATALOG,INDEX,verify,run,csvfile
from advanced_studies_estimation import studies as estimation
from advanced_studies_finance import studies as finance
from advanced_studies_numerics import studies as numerics
from advanced_studies_inference import studies as inference

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
            assert name not in produced,name
            assert rows and all(len(row)==len(header) for row in rows),name
            assert all(math.isfinite(v) for row in rows for v in row),name
            csvfile(studies/(name+".csv"),header,rows)
            produced[name]=len(rows)
        for workflow in (estimation,finance,numerics,inference):workflow(model,report)
    assert len(produced)==48
    csvfile(output/"study-summary.csv",["Study","OutputRows"],sorted(produced.items()))
    print(f"PASS: {len(CATALOG)} complete model cases, {len(produced)} independent studies, {sum(produced.values())} study output rows")

if __name__=="__main__":main(Path(sys.argv[1]).resolve(),Path(sys.argv[2]))
