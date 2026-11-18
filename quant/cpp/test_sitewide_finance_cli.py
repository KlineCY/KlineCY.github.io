import csv
import math
import pathlib
import subprocess
import sys
import tempfile

exe=pathlib.Path(sys.argv[1])
assert exe.is_file()

def write(path,columns,rows):
    with open(path,"w",encoding="utf-8",newline="") as file:
        writer=csv.writer(file)
        writer.writerow(columns)
        writer.writerows(rows)

def run(command,*args,valid=True):
    completed=subprocess.run([str(exe),command,*[str(a) for a in args]],
                             capture_output=True,text=True,timeout=30)
    if valid:
        assert completed.returncode==0,(command,args,completed.stderr)
        assert completed.stdout.strip(),("Missing output",command)
    else:
        assert completed.returncode!=0,("Invalid command accepted",command,args)
    return completed.stdout

with tempfile.TemporaryDirectory() as temporary:
    directory=pathlib.Path(temporary)
    option=directory/"options.csv"
    write(option,["Spot","Option","Delta"],
          [[100,10,0.5],[104,8,0.5],[105,7,0.6]])
    swap=directory/"swap.csv"
    write(swap,["Tenor","Rate","Accrual"],[[1,0.05,1],[2,0.05,1]])
    credit=directory/"credit.csv"
    write(credit,["Exposure","PD","LGD"],[[100,0.1,0.5],[200,0.2,0.4]])
    bad_credit=directory/"bad_credit.csv"
    write(bad_credit,["Exposure","PD","LGD"],[[100,1.2,0.5]])
    hazard=directory/"hazard.csv"
    write(hazard,["Tenor","Hazard"],[[1,0.1],[2,0.1]])
    bad_hazard=directory/"bad_hazard.csv"
    write(bad_hazard,["Tenor","Hazard"],[[1,-0.1],[2,0.1]])
    returns=directory/"returns.csv"
    write(returns,["Return"],[[math.sin(i/4)*0.01] for i in range(30)])
    book=directory/"book.csv"
    write(book,["Side","Price","Quantity"],[[1,99,3],[1,100,4],
                                             [0,101,2],[0,102,5]])
    crossed_book=directory/"crossed_book.csv"
    write(crossed_book,["Side","Price","Quantity"],
          [[1,102,1],[0,101,1]])
    trades=directory/"trades.csv"
    write(trades,["Price","Sign","Signed","Fee","Mid"],
          [[100,1,10,2,99.5],[101,1,-5,1,100.5],
           [102,-1,2,1,101.5]])
    buckets=directory/"buckets.csv"
    write(buckets,["Buy","Sell"],[[70,30],[20,80]])
    schedule=directory/"schedule.csv"
    write(schedule,["Desired","Capacity"],[[30,20],[50,40],[50,20]])
    valid=[
        ("delta-hedge",(option,"Spot","Option","Delta",0,0)),
        ("delta-gamma-hedge",(10,4,0.5,2)),
        ("greek-pnl",(2,3,4,1,0.1)),
        ("payer-swap",(swap,"Tenor","Rate","Accrual",0.05,100)),
        ("survival",(hazard,"Tenor","Hazard",3)),
        ("hazard-loss",(hazard,"Tenor","Hazard",0.4,0.02,100,2)),
        ("credit-el",(credit,"Exposure","PD","LGD")),
        ("credit-variance",(credit,"Exposure","PD","LGD")),
        ("fx-hedged",(100,110,6,7,1)),
        ("keyrate-hedge2",(1,0,0,2,3,4)),
        ("tracking-error2",(1,0,1,1,-1)),
        ("target-vol",(returns,"Return",5,0.1,1.5)),
        ("short-margin",(200,10,100,120,0,0.25)),
        ("book-replay",(book,"Side","Price","Quantity")),
        ("microprice",(100,102,3,1)),
        ("depth-imbalance",(3,1)),
        ("tick-sign",(trades,"Price")),
        ("signed-response",(trades,"Price","Sign",1)),
        ("effective-spread",(101,100,1)),
        ("market-impact",(0.02,2500,10000,1)),
        ("fifo-queue",(10,5,12)),
        ("tca-decompose",(trades,"Signed","Price","Fee","Mid",100)),
        ("vpin-proxy",(buckets,"Buy","Sell")),
        ("ofi-books",(100,102,3,2,100,102,5,3)),
        ("child-schedule",(schedule,"Desired","Capacity",100)),
        ("market-maker-quote",(100,2,0.1,0.2,1,2)),
        ("trade-book",(1000,0,5,100,1,100)),
    ]
    assert len(valid)==27
    output={name:run(name,*args) for name,args in valid}
    assert abs(float(output["microprice"].split()[1])-101.5)<1e-6
    assert abs(float(output["depth-imbalance"].split()[1])-0.5)<1e-6
    assert abs(float(output["delta-gamma-hedge"].splitlines()[0].split()[1])+9)<1e-6
    assert abs(float(output["fx-hedged"].split()[1])-760)<1e-6
    assert abs(float(output["keyrate-hedge2"].splitlines()[1].split(",")[1])+3)<1e-6
    assert abs(float(output["market-impact"].split()[1])-0.01)<1e-6
    assert abs(float(output["vpin-proxy"].split()[1])-0.5)<1e-6
    assert abs(float(output["trade-book"].splitlines()[2].split()[1])-999)<1e-6
    assert "Index,Cash,HedgeUnits" in output["delta-hedge"]
    assert "SwapPV" in output["payer-swap"]
    assert "MarginCall" in output["short-margin"]
    assert "Index,TargetGross" in output["target-vol"]
    assert "OrderFlowImbalance" in output["ofi-books"]
    invalid=[
        ("delta-hedge",(option,"Spot","Option","Delta",0,-1)),
        ("delta-gamma-hedge",(10,4,0.5,0)),
        ("greek-pnl",(2,3,4,1,"nan")),
        ("payer-swap",(swap,"Tenor","Missing","Accrual",0.05,100)),
        ("survival",(bad_hazard,"Tenor","Hazard",1)),
        ("hazard-loss",(hazard,"Tenor","Hazard",1.2,0.02,100,2)),
        ("credit-el",(bad_credit,"Exposure","PD","LGD")),
        ("credit-variance",(bad_credit,"Exposure","PD","LGD")),
        ("fx-hedged",(100,110,6,7,2)),
        ("keyrate-hedge2",(1,2,2,4,3,4)),
        ("tracking-error2",(-1,0,1,1,-1)),
        ("target-vol",(returns,"Return",100,0.1,1.5)),
        ("short-margin",(200,0,100,120,0,0.25)),
        ("book-replay",(crossed_book,"Side","Price","Quantity")),
        ("microprice",(102,100,3,1)),
        ("depth-imbalance",(0,0)),
        ("tick-sign",(trades,"Missing")),
        ("signed-response",(trades,"Price","Sign",100)),
        ("effective-spread",(101,100,0)),
        ("market-impact",(0.02,2500,0,1)),
        ("fifo-queue",(-10,5,12)),
        ("tca-decompose",(trades,"Signed","Price","Fee","Missing",100)),
        ("vpin-proxy",(buckets,"Buy","Missing")),
        ("ofi-books",(102,100,3,2,100,102,5,3)),
        ("child-schedule",(schedule,"Desired","Capacity",-100)),
        ("market-maker-quote",(100,2,0,0.2,1,2)),
        ("trade-book",(1000,0,5,100,1,0)),
    ]
    assert len(invalid)==27
    for name,args in invalid:
        run(name,*args,valid=False)
print("PASS: 27 financial, derivatives and microstructure CLI workflows and 27 invalid-input cases")
