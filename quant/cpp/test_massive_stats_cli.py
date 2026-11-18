import csv
import math
import pathlib
import subprocess
import sys
import tempfile

exe=pathlib.Path(sys.argv[1])
assert exe.is_file()

def invoke(command,arguments,success=True):
    completed=subprocess.run([str(exe),command,*[str(v) for v in arguments]],
                             capture_output=True,text=True,timeout=40)
    if success:
        assert completed.returncode==0,(command,arguments,completed.stderr)
        assert completed.stdout.strip(),("Empty response",command)
    else:
        assert completed.returncode!=0,("Bad input accepted",command,arguments)
    return completed.stdout

with tempfile.TemporaryDirectory() as folder:
    sample=pathlib.Path(folder)/"research.csv"
    n=80
    columns=["Increment","Residual","Value","Probability","Probability2",
             "ReturnB","Equity","State","Duration","Event","Start",
             "End","Flow","WeightTime","Portfolio","Benchmark","Weight",
             "Shares","Cost","AcquiredDay","Actual","ForecastA","ForecastB"]
    values=[]
    for i in range(n):
        r=0.002*math.sin(i*1.11)+0.001*math.cos(i*0.31)
        b=0.003*math.sin(i*0.8)
        p=1/n
        q=(i+1)/(n*(n+1)/2)
        values.append([r,r+0.0001,1+i/10,p,q,b,1000+i*5,i%2,
                       i+1,int(i%3==0),100,101+r*100,1,i/n,2*b,b,
                       1/n,1,95+i/10,i,0.1*i,0.1*i,0.1*i+0.1])
    with sample.open("w",newline="",encoding="utf-8") as file:
        writer=csv.writer(file)
        writer.writerow(columns)
        writer.writerows(values)
    good=[
       ("variance-ratio",(sample,"Increment",3)),
       ("ljung-box",(sample,"Residual",4)),
       ("jarque-bera",(sample,"Residual")),
       ("discrete-entropy",(sample,"Probability")),
       ("kl-divergence",(sample,"Probability","Probability2")),
       ("js-divergence",(sample,"Probability","Probability2")),
       ("tail-coincide",(sample,"Increment","ReturnB",0.2,"LOWER")),
       ("hac-mean",(sample,"Increment",3)),
       ("kendall-tau",(sample,"Increment","ReturnB")),
       ("forecast-loss",(sample,"Actual","ForecastA","ForecastB",2)),
       ("drawdown-duration",(sample,"Equity")),
       ("markov-transitions",(sample,"State",2,0.1)),
       ("kaplan-meier",(sample,"Duration","Event")),
       ("time-weighted-return",(sample,"Start","End")),
       ("modified-dietz",(sample,"Flow","WeightTime",100,200)),
       ("capacity-net-profit",(100,0.1,1,0.001)),
       ("capacity-optimum",(0.1,1,0.001)),
       ("cov-shrink2",(1,0.4,2,0.3)),
       ("risk-parity2",(1,0,4)),
       ("rebalance-band2",(0.5,0.5,0.55,0.45,0.1)),
       ("weight-drift2",(0.5,0.5,0.1,-0.1,0)),
       ("kelly-binary",(0.6,1,1)),
       ("fractional-kelly",(0.6,1,1,0.5)),
       ("fund-watermark-fees",(100,0.2,100,0.01,0.2)),
       ("hhi",(sample,"Weight")),
       ("effective-holdings",(sample,"Weight")),
       ("loan-amortization",(1000,0.01,12)),
       ("information-ratio",(sample,"Increment",252)),
       ("market-capture",(sample,"Portfolio","Benchmark","UP")),
       ("tax-unrealized",(sample,"Shares","Cost",100)),
       ("tax-held",(sample,"AcquiredDay","Shares",100,30)),
    ]
    assert len(good)==31
    output={}
    for name,args in good:
        output[name]=invoke(name,args)
        invoke(name,args[:-1],success=False)
    assert abs(float(output["hhi"].split()[1])-1/n)<1e-6
    assert abs(float(output["effective-holdings"].split()[1])-n)<1e-5
    assert abs(float(output["market-capture"].split()[1])-2)<1e-5
    assert abs(float(output["risk-parity2"].splitlines()[1].split(",")[1])-2/3)<1e-5
    assert abs(float(output["fund-watermark-fees"].splitlines()[3].split()[1])-115.04)<1e-5
    assert "LjungBoxQ" in output["ljung-box"]
    assert "MeanLossDifference" in output["forecast-loss"]
    assert "Index,Weight" in output["weight-drift2"]
    assert "Time,AtRisk,Events,Survival" in output["kaplan-meier"]
    assert "Period,Interest,Principal,Balance" in output["loan-amortization"]
    fail=[
       ("variance-ratio",(sample,"Increment",50)),
       ("ljung-box",(sample,"Residual",80)),
       ("jarque-bera",(sample,"Absent")),
       ("discrete-entropy",(sample,"Increment")),
       ("kl-divergence",(sample,"Increment","Probability")),
       ("js-divergence",(sample,"Probability","Increment")),
       ("tail-coincide",(sample,"Increment","ReturnB",0.8,"LOWER")),
       ("hac-mean",(sample,"Increment",90)),
       ("kendall-tau",(sample,"Increment","Missing")),
       ("forecast-loss",(sample,"Actual","Missing","ForecastB",2)),
       ("drawdown-duration",(sample,"Residual")),
       ("markov-transitions",(sample,"State",1,0)),
       ("kaplan-meier",(sample,"Duration","WeightTime")),
       ("time-weighted-return",(sample,"Residual","End")),
       ("modified-dietz",(sample,"Flow","WeightTime",0,200)),
       ("capacity-net-profit",(-100,0.1,1,0.001)),
       ("capacity-optimum",(0.1,0,0.001)),
       ("cov-shrink2",(1,0.4,2,1.5)),
       ("risk-parity2",(1,-0.2,4)),
       ("rebalance-band2",(0.5,0.5,0.55,0.45,-1)),
       ("weight-drift2",(0.5,0.5,-2,0.1,0)),
       ("kelly-binary",(2,1,1)),
       ("fractional-kelly",(0.6,1,1,2)),
       ("fund-watermark-fees",(100,0.2,100,-0.01,0.2)),
       ("hhi",(sample,"Increment")),
       ("effective-holdings",(sample,"Increment")),
       ("loan-amortization",(1000,-1,12)),
       ("information-ratio",(sample,"Increment",0)),
       ("market-capture",(sample,"Portfolio","Benchmark","SIDEWAYS")),
       ("tax-unrealized",(sample,"Missing","Cost",100)),
       ("tax-held",(sample,"AcquiredDay","Shares",5,30)),
    ]
    assert len(fail)==31
    for name,args in fail:
        invoke(name,args,success=False)
print("PASS: all 31 advanced fund and econometrics CSV commands plus 62 invalid-input tests")
