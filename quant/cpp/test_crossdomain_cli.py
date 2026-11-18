import csv
import math
import pathlib
import subprocess
import sys
import tempfile

exe=pathlib.Path(sys.argv[1])
assert exe.is_file()

def save(path,header,rows):
    with open(path,"w",newline="",encoding="utf-8") as file:
        writer=csv.writer(file);writer.writerow(header);writer.writerows(rows)

def execute(command,*args,valid=True):
    result=subprocess.run([str(exe),command,*[str(arg) for arg in args]],
                          capture_output=True,text=True,timeout=30)
    if valid:
        assert result.returncode==0,(command,args,result.stderr)
        assert result.stdout.strip(),("Empty output",command)
    else:
        assert result.returncode!=0,("Invalid command accepted",command,args)
    return result.stdout

with tempfile.TemporaryDirectory() as temporary:
    folder=pathlib.Path(temporary)
    sample=folder/"samples.csv"
    observations=[]
    for i in range(60):
        x=(i-30)*0.1
        signal=math.sin(i*0.7)+0.3*math.cos(i*0.29)
        predict=1+2*x+0.03*math.sin(i*0.4)
        observation=signal+0.1*math.cos(i*0.1)
        observations.append([
            x,predict,-1+0.5*x,signal,observation,0.01*math.sin(i),
            int(i>=30),min(0.98,max(0.02,0.5+0.15*x)),
            predict+0.1,1,0.5+0.1*math.sin(i/3),
            int(i%2==0),int(i%3==0),2*x+1+0.03*math.sin(i/3)
        ])
    save(sample,["X","Y","Y2","Signal","Observation","Return",
                 "Label","Probability","ForecastA","Weight","Beta",
                 "Treated","Post","PairY"],observations)
    did=folder/"did.csv"
    save(did,["Outcome","Treated","Post"],
         [[10,0,0],[10,0,0],[11,0,1],[11,0,1],
          [12,1,0],[12,1,0],[15,1,1],[15,1,1]])
    states=folder/"state.csv"
    save(states,["Observation"],[[0],[0.2],[0.1],[4.9],[5],[5.1]])
    mixture=folder/"mixture.csv"
    save(mixture,["Value"],[[-6],[-5.5],[-5],[-4.5],[-4],
                           [4],[4.5],[5],[5.5],[6]])
    events=folder/"events.csv"
    save(events,["Time"],[[1],[2],[3]])
    daily=folder/"daily.csv"
    save(daily,["MarketVolume","ReturnA","ReturnB"],
         [[100+i,0.01*math.sin(i/4),0.01*math.cos(i/5)] for i in range(30)])
    paths=folder/"paths.csv"
    save(paths,["Spot0","Spot1","Spot2"],[[90,90,90] for i in range(8)])

    cases=[
        ("bayes-linear",(sample,"X","Y",0.01,1000,1)),
        ("adf-augmented",(sample,"Signal",2)),
        ("kpss-level",(sample,"Signal",3)),
        ("granger-cause",(sample,"Observation","Signal",2)),
        ("did-2x2",(did,"Outcome","Treated","Post")),
        ("rolling-pair",(sample,"PairY","X",10)),
        ("beta-posterior",(3,1,1,1)),
        ("weighted-qr",(sample,"X","Y","Weight")),
        ("hmm-two",(states,"Observation",0,5,1,1,0.9,0.9)),
        ("gmm-two",(mixture,"Value",75)),
        ("ridge-nested",(sample,"X","Y",20,30,0.001,0.1,1)),
        ("isotonic-real",(sample,"X","Y",1)),
        ("multi-ridge",(sample,"X","Y","Y2",0.001,1)),
        ("exp-ols",(sample,"X","Y",10)),
        ("ensemble-weights",(sample,"Y","ForecastA","Y")),
        ("conformal-class",(sample,"Probability","Label",0.1,0.95)),
        ("ewma-cov2",(daily,"ReturnA","ReturnB",0.9)),
        ("factor-neutral2",(sample,"Weight","Beta",0)),
        ("liquidity-var",(1000,0,0.1,0.975,100,0.01)),
        ("hawkes-intensity",(events,"Time",5,1,0.5,2)),
        ("hawkes-likelihood",(events,"Time",5,1,0.5,2)),
        ("pov-schedule",(daily,"MarketVolume",100,0.2)),
        ("lsm-put",(paths,100,0,1,"Spot0","Spot1","Spot2")),
        ("option-density",(12,10,9,10,1)),
        ("option-digital",(12,10,100,110,1)),
        ("bond-rolldown",(100,2,0.05,1)),
    ]
    assert len(cases)==26
    results={}
    for command,args in cases:
        results[command]=execute(command,*args)
    assert abs(float(results["beta-posterior"].splitlines()[0].split()[1])-2/3)<1e-6
    assert abs(float(results["did-2x2"].splitlines()[0].split()[1])-2)<1e-7
    assert abs(float(results["bond-rolldown"].split()[1])-
               (100*math.exp(-0.05)-100*math.exp(-0.1)))<1e-4
    assert abs(float(results["option-density"].split()[1])-0.01)<1e-7
    assert abs(float(results["option-digital"].split()[1])-0.2)<1e-7
    assert abs(float(results["lsm-put"].split()[1])-10)<1e-7
    assert abs(float(results["conformal-class"].splitlines()[0].split()[1]))<=1
    assert "PosteriorMean" in results["beta-posterior"]
    assert "HedgeRatio" in results["rolling-pair"]
    assert "LogLikelihood" in results["hmm-two"]
    assert results["pov-schedule"].startswith("Index,Executed")
    assert "Index,NeutralWeight" in results["factor-neutral2"]

    bad=[
        ("bayes-linear",(sample,"X","Y",0,1000,1)),
        ("adf-augmented",(sample,"Signal",100)),
        ("kpss-level",(sample,"Signal",100)),
        ("granger-cause",(sample,"Observation","Signal",100)),
        ("did-2x2",(sample,"Y","X","Post")),
        ("rolling-pair",(sample,"PairY","X",200)),
        ("beta-posterior",(-1,1,1,1)),
        ("weighted-qr",(sample,"X","Y","Label")),
        ("hmm-two",(states,"Observation",0,5,1,1,2,0.9)),
        ("gmm-two",(mixture,"Value",0)),
        ("ridge-nested",(sample,"X","Y",2,3,0.001,0.1,1)),
        ("isotonic-real",(sample,"Missing","Y",1)),
        ("multi-ridge",(sample,"X","Y","Y2",-1,1)),
        ("exp-ols",(sample,"X","Y",0)),
        ("ensemble-weights",(sample,"Y","Missing","Y")),
        ("conformal-class",(sample,"Probability","Label",1,0.95)),
        ("ewma-cov2",(daily,"ReturnA","ReturnB",1)),
        ("factor-neutral2",(sample,"Weight","Missing",0)),
        ("liquidity-var",(1000,0,0.1,1,100,0.01)),
        ("hawkes-intensity",(events,"Time",2,1,0.5,2)),
        ("hawkes-likelihood",(events,"Time",2,1,0.5,2)),
        ("pov-schedule",(daily,"MarketVolume",100,2)),
        ("lsm-put",(paths,-100,0,1,"Spot0","Spot1","Spot2")),
        ("option-density",(1,4,1,1,1)),
        ("option-digital",(1,4,100,110,1)),
        ("bond-rolldown",(100,2,0.05,3)),
    ]
    for name,args in bad:
        execute(name,*args,valid=False)
print("PASS: 26 crossdomain CSV commands and 26 invalid-input cases")
