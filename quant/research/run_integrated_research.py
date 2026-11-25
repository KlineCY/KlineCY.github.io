"""Run synthetic, cross-model research pipelines and persist every output."""
import math
import sys
import tempfile
from pathlib import Path
from comprehensive_study_common import INDEX,csvfile,run
from integrated_portfolio import studies as portfolio

def main(binary,output):
    output.mkdir(parents=True,exist_ok=True)
    produced={}
    with tempfile.TemporaryDirectory() as tmp:
        path=Path(tmp)/"input.csv"
        def model(name,rows,params):
            assert rows and all(len(r)==len(rows[0]) for r in rows),name
            header=INDEX[name]["columns"]
            if len(header)!=len(rows[0]):header=[f"V{i+1}" for i in range(len(rows[0]))]
            return run(binary,name,csvfile(path,header,rows),params)[1]
        def report(name,header,rows):
            assert name not in produced and rows,name
            assert all(len(r)==len(header) for r in rows),name
            assert all(math.isfinite(v) for r in rows for v in r),name
            csvfile(output/(name+".csv"),header,rows)
            produced[name]=len(rows)
        for workflow in (portfolio,):workflow(model,report)
    assert len(produced)==6
    csvfile(output/"summary.csv",["Pipeline","Rows"],sorted(produced.items()))
    print(f"PASS: {len(produced)} cross-model research pipelines, {sum(produced.values())} output rows")

if __name__=="__main__":main(Path(sys.argv[1]).resolve(),Path(sys.argv[2]))
