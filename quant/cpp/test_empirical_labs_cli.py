import csv
import math
import subprocess
import sys
import tempfile
from pathlib import Path

def require(condition, message):
    if not condition:
        raise AssertionError(message)

def run(binary, args, expected_success):
    completed = subprocess.run([binary, *map(str,args)],
                               text=True, capture_output=True, timeout=20)
    if expected_success:
        require(completed.returncode == 0,
                "Valid command failed: "+str(args)+"\n"+completed.stderr)
        require(completed.stdout.strip(), "Empty result: "+str(args))
    else:
        require(completed.returncode == 1,
                "Invalid input accepted: "+str(args)+"\n"+completed.stdout)
        require("Quant research input error" in completed.stderr,
                "Missing input error: "+str(args))
    return completed.stdout

binary = sys.argv[1]
with tempfile.TemporaryDirectory() as directory:
    normal = Path(directory)/"research.csv"
    invalid = Path(directory)/"invalid.csv"
    columns = ["Open","High","Low","Close","PreviousClose",
               "Target","Forecast","Score","Label","Mean","Sd",
               "P0","P1","P2","Ordinal","Running","Outcome",
               "Instrument","Treatment","Effect","SE",
               "Treated","Donor1","Donor2","Rate","Accrual",
               "SignedOrders","Price","Volume","Sign","Return","NavA","NavB"]
    rows = []
    for i in range(8):
        x = (i-4)/5
        rows.append(dict(Open=100,High=110,Low=90,Close=102,PreviousClose=99,
                         Target=i,Forecast=i+.2,Score=8-i,
                         Label=int(i%3==0),Mean=i,Sd=1,
                         P0=.2,P1=.5,P2=.3,Ordinal=i%3,
                         Running=x,Outcome=(2 if x<0 else 5)+3*x,
                         Instrument=int(i>=4),Treatment=1 if i<4 else 3,
                         Effect=1+i/10,SE=1,
                         Treated=i+1,Donor1=i+1,Donor2=8-i,
                         Rate=.05,Accrual=.01,SignedOrders=(-1)**i,
                         Price=100+i,Volume=1,Sign=(-1)**i,
                         Return=.01 if i%2==0 else -.01,
                         NavA=100-i,NavB=100-i/2))
    bad = [row.copy() for row in rows]
    for row in bad:
        row.update(High=1,PreviousClose=0,Sd=0,Label=0,
                   P0=.8,Running=2,Treatment=1,SE=0,Donor1=0,Donor2=0,
                   Rate=-200,Price=0,Volume=-1,Return=-.6,NavA=0)
    for path, data in ((normal, rows), (invalid, bad)):
        with path.open("w", newline="", encoding="utf8") as stream:
            writer=csv.DictWriter(stream,fieldnames=columns)
            writer.writeheader()
            writer.writerows(data)
    a = str(normal)
    b = str(invalid)
    cases = [
      (["range-parkinson",a,"High","Low"],["range-parkinson",b,"High","Low"]),
      (["range-gk",a,"Open","High","Low","Close"],["range-gk",b,"Open","High","Low","Close"]),
      (["range-rs",a,"Open","High","Low","Close"],["range-rs",b,"Open","High","Low","Close"]),
      (["overnight-gap",a,"Open","PreviousClose"],["overnight-gap",b,"Open","PreviousClose"]),
      (["pinball-loss",a,"Target","Forecast",.5],["pinball-loss",a,"Target","Forecast",0]),
      (["gaussian-crps",a,"Target","Mean","Sd"],["gaussian-crps",b,"Target","Mean","Sd"]),
      (["average-precision",a,"Score","Label"],["average-precision",b,"Score","Label"]),
      (["ranked-score3",a,"P0","P1","P2","Ordinal"],["ranked-score3",b,"P0","P1","P2","Ordinal"]),
      (["rd-local",a,"Running","Outcome",0,1],["rd-local",b,"Running","Outcome",0,1]),
      (["wald-iv",a,"Instrument","Treatment","Outcome"],["wald-iv",b,"Instrument","Treatment","Outcome"]),
      (["meta-fixed",a,"Effect","SE"],["meta-fixed",b,"Effect","SE"]),
      (["synthetic-simplex",a,"Treated","Donor1","Donor2"],["synthetic-simplex",b,"Treated","Donor1","Donor2"]),
      (["coupon-accrual",1000,.06,2,45,90],["coupon-accrual",1000,.06,2,91,90]),
      (["ois-discount",a,"Rate","Accrual"],["ois-discount",b,"Rate","Accrual"]),
      (["merton-default",100,100,.02,.2,1],["merton-default",100,100,0,0,1]),
      (["credit-zero",100,.02,.1,.4,1],["credit-zero",100,.02,.1,1.4,1]),
      (["queue-poisson",1,1,0,1],["queue-poisson",-1,1,0,1]),
      (["transient-impact",a,"SignedOrders",2,.5],["transient-impact",a,"SignedOrders",2,1.5]),
      (["rv-signature",a,"Price",2],["rv-signature",b,"Price",2]),
      (["volume-bucket",a,"Volume","Sign",2],["volume-bucket",b,"Volume","Sign",2]),
      (["leveraged-nav",a,"Return",2,0,252],["leveraged-nav",b,"Return",2,0,252]),
      (["drawdown-tail",a,"NavA","NavB",.8],["drawdown-tail",b,"NavA","NavB",.8]),
      (["margin-call",50,10,100,90,.1],["margin-call",50,10,100,-1,.1]),
      (["diversification-2",.5,1,0,1],["diversification-2",1.5,1,0,1]),
    ]
    for valid, broken in cases:
        stdout=run(binary,valid,True)
        require("nan" not in stdout.lower() and "inf" not in stdout.lower(),
                "Nonfinite successful output: "+str(valid))
        run(binary,broken,False)
    print(f"PASS: {len(cases)} empirical C++17 CSV commands and {len(cases)} invalid-input cases")
