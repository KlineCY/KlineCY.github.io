import csv
import math
import pathlib
import subprocess
import sys
import tempfile

exe=pathlib.Path(sys.argv[1])

def save(path, columns, rows):
    with open(path,"w",newline="",encoding="utf-8") as file:
        writer=csv.writer(file);writer.writerow(columns);writer.writerows(rows)

def run(cmd,args,good=True):
    process=subprocess.run([str(exe),cmd,*[str(x) for x in args]],
                           capture_output=True,text=True,timeout=40)
    if good:
        assert process.returncode==0,(cmd,args,process.stderr)
        assert process.stdout.strip(),(cmd,args)
    else:
        assert process.returncode!=0,("Invalid input accepted",cmd,args)
    return process.stdout

with tempfile.TemporaryDirectory() as tmp:
    base=pathlib.Path(tmp)
    price=base/"market.csv"
    columns=["Price","Return","Factor","Probability","Label","Equity",
             "Loss","VaR","ES","Weight","Target","ADV","Spread","Participation",
             "Exposure","DefaultProbability","LossGivenDefault","Cashflow",
             "Dividend","Split","Shares","StartWeight","EndWeight"]
    data=[]
    for t in range(60):
        p=100+t*0.2+3*math.sin(t/3)
        r=0.01*math.sin(t/4)
        f=0.01*math.cos(t/7)
        data.append([p,r,f,0.05+0.9*t/59,int(t>=30),
                     100+0.2*t+5*math.sin(t/5),
                     0.02+0.03*math.sin(t/6),0.02,0.04,
                     0.3+0.1*math.sin(t/7),0.25+0.1*math.cos(t/8),
                     2000+t*20,0.001+0.0001*t,0.05,100+t*10,
                     0.01+0.001*(t%8),0.4,2+0.1*math.sin(t/4),
                     0,1,1000+t*10,0.45,0.55])
    save(price,columns,data)
    curve=base/"curve.csv"
    save(curve,["Tenor","ZeroRate"],[[1,0.05],[2,0.05],[3,0.05]])
    payment=base/"payments.csv"
    save(payment,["PaymentTime"],[[1],[2]])
    cash=base/"bond.csv"
    save(cash,["Time","Cashflow"],[[1,5],[2,105]])
    credit=base/"credit.csv"
    save(credit,["Exposure","DefaultProbability","LossGivenDefault"],
         [[100,0.02,0.4],[200,0.04,0.5],[150,0.01,0.6]])
    events=base/"events.csv"
    save(events,["Start","End"],[[i*3,i*3+2] for i in range(18)])
    a=base/"published.csv"
    save(a,["Date","Value"],[[1,10],[3,30],[5,50]])
    b=base/"decisions.csv"
    save(b,["Date","Value"],[[2,20],[3,33],[5,55]])
    irr=base/"investor.csv"
    save(irr,["Cashflow"],[[-100],[110]])

    cases=[
        ("sabr-vol",[100,105,1,0.2,1,0,0.3]),
        ("sabr-atm",[100,1,0.2,1,0,0.3]),
        ("option-reprice",[100,100,0.02,0,0.2,1,1,0.01,0,"call"]),
        ("gamma-scalp",[0.1,0.2,0.04,1,100]),
        ("bond-risk",[cash,0.05]),
        ("bond-shock",[100,2,4,0.01]),
        ("swap-fixed",[curve,payment,0.05,100]),
        ("swap-payer",[curve,payment,0.05,100]),
        ("swap-float",[curve,2,100]),
        ("forward-discount",[curve,1,2]),
        ("nelson-siegel",[1,0.03,0.01,0.02,2]),
        ("curve-slope-ns",[1,3,0.03,0.01,0.02,2]),
        ("credit-vasicek",[0.02,0.2,0]),
        ("credit-simulate",[credit,0.2,100,2026]),
        ("margin-open",[1000,2,100,10,0.2,0.1]),
        ("margin-mark",[1000,2,100,101,10,0.2,0.1]),
        ("execution-optimal",[100,10,0.1,0.2,1]),
        ("execution-quadratic",[price,"Shares",0.1,1]),
        ("book-top",[99,10,101,20]),
        ("attribution-period",[price,"StartWeight","EndWeight","Return",0.001]),
        ("factor-return-contrib",[price,"Weight","Return",0]),
        ("triple-barrier",[price,"Price",1,10,0.04,0.05]),
        ("barrier-series",[price,"Price",10,0.04,0.05]),
        ("purged-folds",[events,3,1]),
        ("expanding-ends",[60,20,10,5]),
        ("calibration-ece",[price,"Probability","Label",10]),
        ("calibration-brier",[price,"Probability","Label"]),
        ("cashflow-account",[price,100,"Return","Cashflow"]),
        ("investor-irr",[irr,"Cashflow"]),
        ("twr-growth",[price,"Return"]),
        ("asof-numeric",[a,b]),
        ("align-numeric",[a,b]),
        ("stock-action",[100,50,0,2]),
        ("stock-action-series",[price,"Price","Dividend","Split"]),
        ("rights-exprice",[100,80,0.5]),
        ("underwater-episodes",[price,"Equity"]),
        ("underwater-duration",[price,"Equity"]),
        ("kupiec-backtest",[price,"Loss","VaR",0.95]),
        ("es-breach-residual",[price,"Loss","VaR","ES"]),
        ("rolling-beta",[price,"Return","Factor",10]),
        ("hedge-beta",[price,"Weight","Factor",0]),
        ("liquidation-days",[price,"Shares","ADV",0.2]),
        ("liquidation-cost",[price,"Shares","Spread","Participation",0.1]),
        ("quote-ofi",[99,99,10,12,101,101,8,7]),
        ("weighted-ofi",[price,"Return","Weight"]),
        ("kelly-binary",[0.6,1,1]),
        ("kelly-normal",[0.1,0.5,1]),
        ("vol-size",[0.2,0.1,-1,1]),
        ("jump-correction",[0.2,0,0.2]),
        ("jump-simulate",[100,0.05,0.2,0.1,0,0.2,0.1,12,42]),
        ("holdings-entropy",[price,"Weight"]),
        ("holdings-hhi",[price,"Weight"]),
        ("project-npv",[price,100,"Cashflow",0.05]),
        ("project-wacc",[0.5,0.1,0.06,0.25]),
        ("perpetuity",[10,0.02,0.07]),
    ]
    assert len(cases)==55,len(cases)
    result={}
    for command,args in cases:
        result[command]=run(command,args)
    assert abs(float(result["stock-action"].split()[1]))<1e-8
    assert abs(float(result["margin-mark"].splitlines()[0].split()[1])-1020)<1e-5
    assert abs(float(result["kelly-binary"].split()[1])-0.2)<1e-7
    assert abs(float(result["kelly-normal"].split()[1])-0.2)<1e-7
    assert abs(float(result["project-wacc"].split()[1])-0.0725)<1e-7
    assert abs(float(result["perpetuity"].split()[1])-200)<1e-6
    assert abs(float(result["investor-irr"].split()[1])-0.1)<1e-7
    assert abs(float(result["quote-ofi"].split()[1])-3)<1e-7
    assert abs(float(result["gamma-scalp"].split()[1]))<1e-7
    assert "MicroPrice" in result["book-top"]
    assert "Drawdown" in result["underwater-episodes"]
    assert "P95Loss" in result["credit-simulate"]
    assert len(result["expanding-ends"].splitlines())==5
    assert len(result["execution-optimal"].splitlines())==11
    assert len(result["barrier-series"].splitlines())==51

    invalid=[
        ("sabr-vol",[100,100,1,0.2,1,1,0.2]),
        ("sabr-atm",[100,1,0.2,1,1,0.2]),
        ("option-reprice",[0,100,0,0,0.2,1,1,0,0,"call"]),
        ("option-reprice",[100,100,0,0,0.2,1,1,0,0,"invalid"]),
        ("bond-risk",[cash,-99999]),
        ("swap-payer",[curve,payment,0.05,-1]),
        ("swap-float",[curve,-1,100]),
        ("forward-discount",[curve,2,1]),
        ("nelson-siegel",[-1,0.03,0.01,0.02,2]),
        ("curve-slope-ns",[3,1,0.03,0.01,0.02,2]),
        ("credit-vasicek",[0.02,1,0]),
        ("credit-simulate",[credit,1,100,42]),
        ("margin-open",[100,2,100,10,0.2,0.1]),
        ("margin-mark",[1000,2,100,-1,10,0.2,0.1]),
        ("execution-optimal",[100,10,0,0.2,1]),
        ("execution-quadratic",[price,"Shares",-1,1]),
        ("book-top",[101,10,100,20]),
        ("attribution-period",[price,"Missing","EndWeight","Return",0.001]),
        ("triple-barrier",[price,"Price",-1,5,0.04,0.05]),
        ("barrier-series",[price,"Price",100,0.04,0.05]),
        ("purged-folds",[events,100,1]),
        ("expanding-ends",[60,80,10,5]),
        ("calibration-ece",[price,"Probability","Label",1]),
        ("cashflow-account",[price,-100,"Return","Cashflow"]),
        ("investor-irr",[price,"Missing"]),
        ("asof-numeric",[b,a]),
        ("align-numeric",[price,b]),
        ("stock-action",[100,50,0,0]),
        ("stock-action-series",[price,"Price","Dividend","Missing"]),
        ("rights-exprice",[100,80,-1]),
        ("underwater-duration",[price,"Missing"]),
        ("kupiec-backtest",[price,"Loss","VaR",1]),
        ("es-breach-residual",[price,"Loss","Missing","ES"]),
        ("rolling-beta",[price,"Return","Factor",100]),
        ("hedge-beta",[price,"Weight","Factor",100]),
        ("liquidation-days",[price,"Shares","ADV",0]),
        ("liquidation-cost",[price,"Shares","Spread","Participation",-1]),
        ("quote-ofi",[101,101,10,10,100,100,10,10]),
        ("kelly-binary",[0.5,0,1]),
        ("kelly-normal",[0.1,0,1]),
        ("vol-size",[0.2,0,-1,1]),
        ("jump-correction",[-1,0,0.1]),
        ("jump-simulate",[100,0.05,0.2,0.1,0,0.2,0.1,0,42]),
        ("holdings-entropy",[price,"Missing"]),
        ("project-npv",[price,100,"Missing",0.05]),
        ("project-wacc",[1.2,0.1,0.06,0.25]),
        ("perpetuity",[10,0.07,0.07]),
    ]
    for command,args in invalid:
        run(command,args,good=False)
print("PASS: 55 expanded cross-asset C++ CLI workflows and 46 invalid-input cases")
