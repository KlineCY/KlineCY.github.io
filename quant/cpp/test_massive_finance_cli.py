import csv
import math
import pathlib
import subprocess
import sys
import tempfile

exe=pathlib.Path(sys.argv[1])
assert exe.is_file()

def write(path,head,rows):
    with path.open("w",newline="",encoding="utf-8") as f:
        writer=csv.writer(f)
        writer.writerow(head)
        writer.writerows(rows)

def invoke(name,args,ok=True):
    p=subprocess.run([str(exe),name,*[str(v) for v in args]],
                     capture_output=True,text=True,timeout=45)
    if ok:
        assert p.returncode==0,(name,args,p.stderr)
        assert p.stdout.strip(),("No stdout",name)
    else:
        assert p.returncode!=0,("Invalid case succeeded",name,args)
    return p.stdout

with tempfile.TemporaryDirectory() as folder:
    root=pathlib.Path(folder)
    option=root/"option.csv"
    write(option,["Strike","Call"],[[90,14],[100,10],[110,7]])
    credit=root/"credit.csv"
    write(credit,["Tenor","Hazard"],[[1,0.02],[2,0.02],[3,0.02]])
    future=root/"futures.csv"
    write(future,["Settlement"],[[100],[101],[99],[103]])
    market=root/"market.csv"
    records=[]
    for i in range(50):
        label=i%2
        records.append([0.1*i,2,0.7 if label==0 else 0.3,
                        0.3 if label==0 else 0.7,label,
                        2.0,2.0-0.001*i,1.0,3.0,
                        100+i*0.1,1+(i%3),i+1,
                        0.95 if label else 0.05])
    columns=["Feature","Count","P0","P1","Label","Actual",
             "Forecast","Lower","Upper","TradePrice","Volume","Seconds","Risk"]
    write(market,columns,records)
    book=root/"book.csv"
    write(book,["IsBid","Price","Quantity"],
          [[1,99,5],[1,98,10],[0,101,3],[0,102,7]])
    good=[
       ("parity-forward",(100,100,0,0,1)),
       ("parity-error",(10,10,100,100,0,0,1)),
       ("american-crr",(100,100,0.05,0,0.2,1,100,"PUT")),
       ("black76-price",(100,100,0.02,0.2,1,"CALL")),
       ("black76-vega",(100,100,0.02,0.2,1,"CALL")),
       ("bachelier-price",(100,100,0.02,10,1,"CALL")),
       ("bachelier-delta",(100,100,0.02,10,1,"CALL")),
       ("barrier-down-bridge",(100,101,90,0.2,1)),
       ("barrier-up-bridge",(100,101,110,0.2,1)),
       ("implied-black-vol",(9,100,100,0.02,0,1,"CALL")),
       ("call-monotonicity",(option,"Strike","Call")),
       ("call-convexity",(option,"Strike","Call")),
       ("dupire-local-variance",(100,10,2,0,0.04,0,0)),
       ("cds-protection",(credit,"Tenor","Hazard",2,0.02,0.4,4)),
       ("fx-cross-rate",(2,3)),
       ("fx-triangular-error",(2,3,6)),
       ("futures-variation",(future,"Settlement",2,10,1000)),
       ("futures-margin-shortfall",(future,"Settlement",2,10,1000,800)),
       ("poisson-glm",(market,"Feature","Count",0,0.0001,20,0.01)),
       ("multiclass-loss",(market,"P0","P1","Label")),
       ("reliability-bins",(market,"Risk","Label",5)),
       ("interval-coverage",(market,"Actual","Lower","Upper")),
       ("online-moments",(market,"TradePrice")),
       ("regression-metrics",(market,"Actual","Forecast")),
       ("economic-threshold",(2,1,0.2)),
       ("economic-utility",(0.4,2,1,0.2)),
       ("book-market-sweep",(book,"IsBid","Price","Quantity","BUY",5)),
       ("market-vwap",(market,"TradePrice","Volume")),
       ("vwap-slippage",(101,100,"BUY")),
       ("trade-durations",(market,"Seconds")),
       ("volume-bars",(market,"TradePrice","Volume",20)),
       ("inventory-penalty",(10,0.2,100,1,0.1)),
       ("inventory-mark",(10,100,102)),
    ]
    assert len(good)==33
    result={}
    for name,args in good:
        result[name]=invoke(name,args)
        invoke(name,args[:-1],ok=False)
    assert abs(float(result["parity-forward"].split()[1]))<1e-9
    assert abs(float(result["parity-error"].split()[1]))<1e-9
    assert abs(float(result["fx-cross-rate"].split()[1])-6)<1e-8
    assert abs(float(result["fx-triangular-error"].split()[1]))<1e-8
    assert abs(float(result["book-market-sweep"].splitlines()[0].split()[1])-5)<1e-8
    assert abs(float(result["economic-threshold"].split()[1])-0.4)<1e-8
    assert "Time" not in result["cds-protection"]
    assert "PremiumAnnuity" in result["cds-protection"]
    assert "Index,Variation,Equity,Shortfall" in result["futures-variation"]
    assert "Bin,Count,MeanProbability,EventRate" in result["reliability-bins"]
    assert "Bar,Volume,VWAP" in result["volume-bars"]
    wrong=[
       ("parity-forward",(0,100,0,0,1)),
       ("parity-error",(-1,10,100,100,0,0,1)),
       ("american-crr",(100,100,0.05,0,0.2,1,0,"PUT")),
       ("black76-price",(100,100,0.02,0,1,"CALL")),
       ("black76-vega",(100,100,0.02,0.2,1,"WRONG")),
       ("bachelier-price",(100,100,0.02,-1,1,"CALL")),
       ("bachelier-delta",(100,100,0.02,10,1,"UNKNOWN")),
       ("barrier-down-bridge",(100,101,90,0,1)),
       ("barrier-up-bridge",(100,101,0,0.2,1)),
       ("implied-black-vol",(200,100,100,0.02,0,1,"CALL")),
       ("call-monotonicity",(option,"Missing","Call")),
       ("call-convexity",(option,"Strike","Missing")),
       ("dupire-local-variance",(100,10,2,0,0,0,0)),
       ("cds-protection",(credit,"Tenor","Hazard",2,0.02,1.2,4)),
       ("fx-cross-rate",(0,3)),
       ("fx-triangular-error",(2,3,-6)),
       ("futures-variation",(future,"Settlement",2,0,1000)),
       ("futures-margin-shortfall",(future,"Settlement",2,10,1000,-800)),
       ("poisson-glm",(market,"Feature","Count",0,0,20,0.01)),
       ("multiclass-loss",(market,"P0","P0","Label")),
       ("reliability-bins",(market,"Risk","Label",1)),
       ("interval-coverage",(market,"Actual","Upper","Lower")),
       ("online-moments",(market,"Missing")),
       ("regression-metrics",(market,"Actual","Missing")),
       ("economic-threshold",(2,1,2)),
       ("economic-utility",(-1,2,1,0.2)),
       ("book-market-sweep",(book,"IsBid","Price","Quantity","HOLD",5)),
       ("market-vwap",(market,"TradePrice","Missing")),
       ("vwap-slippage",(101,100,"UNKNOWN")),
       ("trade-durations",(market,"Label")),
       ("volume-bars",(market,"TradePrice","Volume",0)),
       ("inventory-penalty",(10,-0.2,100,1,0.1)),
       ("inventory-mark",(10,100,0)),
    ]
    assert len(wrong)==33
    for name,args in wrong:
        invoke(name,args,ok=False)
print("PASS: 33 new derivatives and execution CSV commands plus 66 rejected bad-input cases")
