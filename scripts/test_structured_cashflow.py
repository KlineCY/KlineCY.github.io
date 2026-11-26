import csv
import itertools
import math
import pathlib
import subprocess
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/"quant"/"research"))
from structured_cashflow import model

def close(a,b,tol=2e-8):
    assert math.isclose(a,b,rel_tol=tol,abs_tol=2e-6),(a,b)

def main():
    compiler = sys.argv[1] if len(sys.argv)>1 else "g++"
    with tempfile.TemporaryDirectory() as temp:
        binary=pathlib.Path(temp)/"structured"
        subprocess.run([compiler,"-std=c++17","-O2","-Wall","-Wextra","-Werror",
                        str(ROOT/"quant"/"cpp"/"structured_cashflow.cpp"),"-o",str(binary)],check=True)
        cases=[]
        with (ROOT/"quant"/"research"/"structured-cashflow-cases.csv").open() as f:
            for row in csv.DictReader(f):
                cases.append(tuple(float(row[k]) for k in
                     ("balance","gross","net","months","cpr","yield","bump","cpr_down","cpr_up","senior")))
        for gross,n,cpr,senior_fraction,shift in itertools.product(
                (0,.03,.09),(1,12,60,360),(0,.06,.35,1),(.2,.8),(False,True)):
            b=100000
            net=gross*.85
            cd=min(1,cpr+.1) if shift else cpr
            cu=max(0,cpr-.1) if shift else cpr
            cases.append((b,gross,net,n,cpr,net,.0025,cd,cu,b*senior_fraction))
        rows_checked=0
        coordinates=0
        for a in cases:
            output=subprocess.check_output([str(binary),*map(str,a)],text=True).splitlines()
            ref,ledger=model(a)
            assert output[0].startswith("S,")
            native=list(map(float,output[0].split(",")[1:]))
            assert len(native)==len(ref)==12
            for x,y in zip(native,ref):
                close(x,y)
                coordinates+=1
            assert len(output)==len(ledger)+1
            b=a[0]
            principal=0
            sp=0
            jp=0
            for line,row in zip(output[1:],ledger):
                parts=line.split(",")
                assert parts[0]=="M"
                vals=list(map(float,parts[1:]))
                assert len(vals)==len(row)==16
                for x,y in zip(vals,row):
                    close(x,y)
                    coordinates+=1
                t,begin,scheduled,prepay,end,i,p,d,sb,ps,si,jb,pj,ji,se,je=vals
                assert min(begin,scheduled,prepay,end,i,p,d,sb,ps,si,jb,pj,ji,se,je)>=-1e-7
                close(begin,scheduled+prepay+end)
                close(p,ps+pj)
                close(i,si+ji)
                close(begin,sb+jb)
                close(end,se+je)
                if pj>1e-6: close(se,0)
                principal+=p; sp+=ps; jp+=pj
                rows_checked+=1
            close(principal,b); close(sp,a[9]); close(jp,b-a[9])
            close(native[0],native[1]+native[2])
            close(native[0],native[4]+native[5])
            assert native[6]<=native[7]+1e-8
            if a[5]==a[2]: close(native[0],b)
            if a[4]==a[7]==a[8]:
                assert native[10]>0 and native[11]>0
        valid=list(cases[0])
        bad=[]
        for i,v in ((0,0),(1,-.01),(2,2),(3,2.5),(4,1.01),(5,-2),(6,0),(7,-.01),(8,1.01),(9,valid[0])):
            x=valid.copy(); x[i]=v; bad.append(x)
        for a in bad:
            assert subprocess.run([str(binary),*map(str,a)],capture_output=True).returncode!=0
            try: model(a)
            except ValueError: pass
            else: raise AssertionError("invalid input accepted")
        print(f"{len(cases)} parameter scenarios; {rows_checked} monthly ledgers; {coordinates} numerical coordinates; {len(bad)} invalid inputs")
if __name__=="__main__":
    main()
