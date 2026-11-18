import csv
import subprocess
import sys
import tempfile
from pathlib import Path

def execute(binary, args, valid):
    process = subprocess.run([binary,*map(str,args)],capture_output=True,text=True,timeout=20)
    if valid:
        assert process.returncode==0,(args,process.stderr)
        assert process.stdout.strip(),args
        tokens=process.stdout.replace(","," ").split()
        assert all(t.lower() not in ("nan","+nan","-nan","inf","+inf","-inf")
                   for t in tokens),args
    else:
        assert process.returncode==1,(args,process.stdout,process.stderr)
        assert "Quant research input error:" in process.stderr,args
    return process.stdout

binary = sys.argv[1]
with tempfile.TemporaryDirectory() as temp:
    file=Path(temp)/"quant.csv"
    columns=("StartA EndA RetA StartB EndB RetB R Feature Treatment Outcome "
             "Propensity Mu0 Mu1 TradePrice DollarVolume SignedVolume PriceChange "
             "Sign Time Cash ActualBinary PredictedBinary ActualClass PredictedClass "
             "P0 P1 P2 Loss1 Loss2 Initial Revised Reference Released Timestamp "
             "Bid Ask ScenarioProbability").split()
    records=[]
    for i in range(8):
        r=dict(StartA=i,EndA=i+1,RetA=.01*(i+1),
               StartB=i,EndB=i+1,RetB=.005*(i+1),
               R=.01 if i%2==0 else -.01,
               Feature=i,Treatment=int(i>=4),Outcome=i+2,
               Propensity=.5,Mu0=i,Mu1=i+1,
               TradePrice=99 if i%2==0 else 101,
               DollarVolume=100+i,SignedVolume=i-3.5,PriceChange=2*i-7,
               Sign=1 if i%2==0 else -1,Time=i+1,Cash=10,
               ActualBinary=i%2,PredictedBinary=i%2,
               ActualClass=i%3,PredictedClass=i%3,
               P0=1 if i%3==0 else 0,P1=1 if i%3==1 else 0,
               P2=1 if i%3==2 else 0,
               Loss1=i,Loss2=8-i,Initial=i,Revised=i+.5,
               Reference=i,Released=i+1,Timestamp=2*i,
               Bid=100,Ask=101,ScenarioProbability=1/8)
        records.append(r)
    with file.open("w",newline="",encoding="utf-8") as handle:
        writer=csv.DictWriter(handle,fieldnames=columns)
        writer.writeheader()
        writer.writerows(records)
    path=str(file)
    good=[
      ["hy-cov",path,"StartA","EndA","RetA","StartB","EndB","RetB"],
      ["realized-quarticity",path,"R"],
      ["kernel-1",path,"R"],
      ["truncated-rv",path,"R",.02],
      ["ht-ate",path,"Treatment","Outcome","Propensity"],
      ["aipw-ate",path,"Treatment","Outcome","Propensity","Mu0","Mu1"],
      ["smd-covariate",path,"Feature","Treatment"],
      ["ipw-ess",path,"Treatment","Propensity"],
      ["quoted-spread",path,"Bid","Ask"],
      ["replenishment",path,"DollarVolume","DollarVolume"],
      ["kyle-lambda",path,"SignedVolume","PriceChange"],
      ["sign-autocorr",path,"Sign"],
      ["forward-zero",1,.95,0,1],
      ["zero-dv01",100,.03,2],
      ["cash-duration",path,"Time","Cash",.03],
      ["cash-convexity",path,"Time","Cash",.03],
      ["mcc",path,"ActualBinary","PredictedBinary"],
      ["macro-f1",path,"ActualClass","PredictedClass",3],
      ["balanced-acc",path,"ActualClass","PredictedClass",3],
      ["multiclass-brier3",path,"P0","P1","P2","ActualClass"],
      ["euler-vol2",.5,.5,1,0,1],
      ["tail-es2",path,"Loss1","Loss2",.5,.5,.75],
      ["factor-stress2",1,2,.1,.2,1,0,1],
      ["scenario-expectation",path,"Loss1","ScenarioProbability"],
      ["vintage-rmse",path,"Initial","Revised"],
      ["publish-delay",path,"Reference","Released"],
      ["quote-stale",path,"Timestamp",1],
      ["crossed-quotes",path,"Bid","Ask"],
      ["digital-call",100,100,0,0,.2,1],
      ["vanna-call",100,100,0,0,.2,1],
      ["volga-call",100,100,0,0,.2,1],
      ["forward-variance",1,.2,2,.3]]
    bad=[
      ["hy-cov",path,"EndA","StartA","RetA","StartB","EndB","RetB"],
      ["realized-quarticity",path,"Missing"],
      ["kernel-1",path,"Missing"],
      ["truncated-rv",path,"R",0],
      ["ht-ate",path,"Feature","Outcome","Propensity"],
      ["aipw-ate",path,"Treatment","Outcome","Propensity","Mu0","Missing"],
      ["smd-covariate",path,"Feature","Missing"],
      ["ipw-ess",path,"Treatment","Feature"],
      ["quoted-spread",path,"Ask","Bid"],
      ["replenishment",path,"DollarVolume","SignedVolume"],
      ["kyle-lambda",path,"Missing","PriceChange"],
      ["sign-autocorr",path,"Feature"],
      ["forward-zero",1,.95,2,1],
      ["zero-dv01",-100,.03,2],
      ["cash-duration",path,"Cash","Time",.03],
      ["cash-convexity",path,"Cash","Time",.03],
      ["mcc",path,"ActualClass","PredictedBinary"],
      ["macro-f1",path,"ActualClass","PredictedClass",2],
      ["balanced-acc",path,"ActualClass","PredictedClass",2],
      ["multiclass-brier3",path,"P0","P1","P2","Missing"],
      ["euler-vol2",.5,.5,0,0,0],
      ["tail-es2",path,"Loss1","Loss2",.5,.5,1],
      ["factor-stress2",1,2,.1,.2,1,0,"not-a-number"],
      ["scenario-expectation",path,"Loss1","Missing"],
      ["vintage-rmse",path,"Initial","Missing"],
      ["publish-delay",path,"Released","Reference"],
      ["quote-stale",path,"Timestamp",-1],
      ["crossed-quotes",path,"Missing","Ask"],
      ["digital-call",100,100,0,0,0,1],
      ["vanna-call",0,100,0,0,.2,1],
      ["volga-call",100,100,0,0,.2,0],
      ["forward-variance",1,.3,2,.1]]
    assert len(good)==len(bad)==32
    for command in good: execute(binary,command,True)
    for command in bad: execute(binary,command,False)
    print("PASS: 32 domain CLI workflows and 32 invalid-input safeguards")
