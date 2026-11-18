import csv
import math
import pathlib
import subprocess
import sys
import tempfile

binary=pathlib.Path(sys.argv[1])
assert binary.is_file()

def write(path,columns,rows):
    with open(path,"w",encoding="utf-8",newline="") as f:
        w=csv.writer(f);w.writerow(columns);w.writerows(rows)

def call(name,*args,success=True):
    result=subprocess.run([str(binary),name,*[str(a) for a in args]],
                          capture_output=True,text=True,timeout=35)
    if success:
        assert result.returncode==0,(name,args,result.stderr)
        assert result.stdout.strip(),("Empty command output",name)
    else:
        assert result.returncode!=0,("Invalid command accepted",name,args)
    return result.stdout

with tempfile.TemporaryDirectory() as d:
    root=pathlib.Path(d)
    data=root/"research.csv"
    rows=[]
    for t in range(90):
        x=(t-45)/25
        noise=0.15*math.sin(t/5)
        y=1+2*x+noise
        rows.append([x,y,math.sin(t/7)*0.02,100+t*0.1+math.sin(t/5),
                     int(t>=45),min(2,t//30),int(t>=40)*3,
                     y+0.1*math.sin(t/3),y+0.1,
                     0.2,0.0,0.1,0.0])
    write(data,["Factor","Target","Return","Price","Label","Class",
                "Shift","Actual","PredA","Radius","PredB","Probability",
                "Unused"],rows)
    knots=root/"knots.csv"
    write(knots,["X","Y"],[[0,1],[1,3],[2,5]])
    commands=[
        ("spd-solve",(4,2,3,6,5)),
        ("cg-solve",(4,2,3,6,5)),
        ("spd-logdet",(4,2,3)),
        ("normal-logpdf2",(0,0,0,0,1,0,1)),
        ("qr-ols",(data,"Target","Factor")),
        ("natural-spline",(knots,"X","Y",1.5)),
        ("root-bisect",(2,0,2)),
        ("root-newton",(2,0,2)),
        ("integrate-square",(0,1)),
        ("integrate-normal-density",(-1,1)),
        ("quad-gradient",(1,2,2,3)),
        ("quad-hessian",(2,3)),
        ("bfgs-quadratic",(0,0,2,-1)),
        ("logit-irls",(data,"Factor","Label",0,1)),
        ("platt-calibrate",(data,"Factor","Label",0,1)),
        ("softmax-train",(data,"Factor","Class",3,0,400)),
        ("ols-hac",(data,"Target","Factor",2)),
        ("ljung-box",(data,"Return",2)),
        ("arch-lm",(data,"Return",1)),
        ("jarque-bera",(data,"Return")),
        ("block-bootstrap-ci",(data,"Return",5,100,0.9,42)),
        ("rts-smooth",(data,"Return",0,1,0.1,0.5)),
        ("arima-one",(data,"Price",1)),
        ("mean-break",(data,"Shift",10)),
        ("multi-break",(data,"Shift",1,10,0.1)),
        ("kernel-predict",(data,"Factor","Target",0,0.1,1)),
        ("gp-predict",(data,"Factor","Target",0,1,0.1)),
        ("conformal-interval",(data,"Target","Actual",0.5,0.1)),
        ("conformal-coverage",(data,"Target","Radius","Actual")),
        ("forecast-dm",(data,"PredA","PredB","Actual",1)),
        ("feature-filter",(data,"Target",1,"Factor","Return","Shift")),
        ("adam-predict",(data,"Factor","Target",0.3,400,0.03)),
        ("mlp-predict",(data,"Factor","Target",0.3,6,300,0.01)),
        ("bagged-predict",(data,"Factor","Target",0.3,20,42)),
    ]
    assert len(commands)==34,len(commands)
    results={name:call(name,*args) for name,args in commands}
    assert abs(float(results["root-bisect"].split()[1])-math.sqrt(2))<1e-5
    assert abs(float(results["root-newton"].split()[1])-math.sqrt(2))<1e-5
    assert abs(float(results["integrate-square"].split()[1])-1/3)<1e-5
    assert abs(float(results["spd-logdet"].split()[1])-math.log(8))<1e-5
    assert abs(float(results["natural-spline"].split()[1])-4)<1e-5
    assert abs(float(results["mean-break"].splitlines()[0].split()[1])-40)<1e-5
    assert "Probability" in results["softmax-train"]
    assert "Variance" in results["gp-predict"]
    assert "HACStandardError" in results["ols-hac"]
    assert "TrainingMSE" in results["adam-predict"]
    assert "Index,State,Variance" in results["rts-smooth"]
    invalid=[
        ("spd-solve",(1,2,1,1,1)),
        ("cg-solve",(1,2,1,1,1)),
        ("spd-logdet",(0,0,1)),
        ("normal-logpdf2",(0,0,0,0,1,2,1)),
        ("qr-ols",(data,"Missing","Factor")),
        ("natural-spline",(knots,"X","Y",5)),
        ("root-bisect",(0,-1,1)),
        ("root-newton",(-1,-1,1)),
        ("integrate-square",(1,0)),
        ("quad-gradient",(1,2,2)),
        ("logit-irls",(data,"Factor","Label",0,-1)),
        ("softmax-train",(data,"Factor","Class",1,0,20)),
        ("ols-hac",(data,"Target","Factor",100)),
        ("ljung-box",(data,"Return",100)),
        ("arch-lm",(data,"Return",100)),
        ("jarque-bera",(data,"Missing")),
        ("block-bootstrap-ci",(data,"Return",0,100,0.9,42)),
        ("rts-smooth",(data,"Return",0,1,-1,0.5)),
        ("arima-one",(data,"Price",200)),
        ("mean-break",(data,"Shift",100)),
        ("kernel-predict",(data,"Factor","Target",0,0,1)),
        ("gp-predict",(data,"Factor","Target",0,1,0)),
        ("conformal-interval",(data,"Target","Actual",0.5,0.001)),
        ("forecast-dm",(data,"PredA","PredA","Actual",1)),
        ("feature-filter",(data,"Target",100,"Factor")),
        ("adam-predict",(data,"Factor","Target",0,0,0.1)),
        ("mlp-predict",(data,"Factor","Target",0,0,300,0.01)),
        ("bagged-predict",(data,"Factor","Target",0,0,42)),
    ]
    for name,args in invalid:
        call(name,*args,success=False)
print("PASS: 34 additional math/ML research commands and 28 rejected invalid inputs")
