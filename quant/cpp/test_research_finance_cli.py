import csv
import math
import pathlib
import subprocess
import sys
import tempfile

exe=pathlib.Path(sys.argv[1])
assert exe.is_file()

def write(path,cols,rows):
    with open(path,"w",encoding="utf-8",newline="") as file:
        writer=csv.writer(file);writer.writerow(cols);writer.writerows(rows)

def run(name,*args,valid=True):
    result=subprocess.run([str(exe),name,*[str(x) for x in args]],
                          capture_output=True,text=True,timeout=35)
    if valid:
        assert result.returncode==0,(name,args,result.stderr)
        assert result.stdout.strip(),(name,"No command output")
    else:
        assert result.returncode!=0,("Unexpectedly accepted",name,args)
    return result.stdout

with tempfile.TemporaryDirectory() as tmp:
    base=pathlib.Path(tmp)
    daily=base/"daily.csv"
    write(daily,["R1","R2","Equity","Signal","Return","Volume",
                 "OldPrice","NewPrice","Notional"],
        [[-0.04+0.001*i,0.005+0.001*(i%3),
          100+0.2*i+3*math.sin(i/8),
          math.sin(i/7)*0.5,math.sin(i/6)*0.01,
          1000+i*10,100+i,101+i,100+i]
         for i in range(65)])
    coupons=base/"par.csv"
    write(coupons,["Coupon"],[[0.05],[0.05],[0.05]])
    discounts=base/"discount.csv"
    write(discounts,["D"],[[pow(1.05,-i)] for i in (1,2,3)])
    bond=base/"bond.csv"
    write(bond,["Time","Cashflow"],[[1,100]])
    prices=base/"prices.csv"
    write(prices,["Price"],[[100],[101],[110],[111],[112]])
    roll=base/"rolls.csv"
    write(roll,["RollIndex","Old","New"],[[2,102,110]])
    offers=base/"offers.csv"
    write(offers,["Price","Quantity"],[[101,2],[102,5]])
    vwap=base/"vwap.csv"
    write(vwap,["Volume","Price","Filled"],[[1,100,1],[3,102,3],[2,101,2]])
    liquidity=base/"liquidity.csv"
    write(liquidity,["Shares","ADV"],[[200,1000],[-100,1000]])
    fills=base/"fills.csv"
    write(fills,["Signed","Price","Fee"],[[10,100,1],[-5,110,1]])
    buy_fills=base/"buy_fills.csv"
    write(buy_fills,["Signed","Price","Fee"],[[6,101,0]])
    strategy=base/"strategy.csv"
    write(strategy,["Signal","Return"],[[1,0],[1,0.1],[-1,0.1],[0,0]])
    commands=[
        ("cvar-evaluate",(daily,"R1","R2",0.5,0.8)),
        ("cvar-optimize",(daily,"R1","R2",0.5,0.8)),
        ("risk-budget2",(1,0,4,0.5,0.5,3000,0.15)),
        ("black-litterman-full",(0,1,0.1,1,1)),
        ("turnover-opt2",(1,0,4,0.01,0.02,0.5,0.5,1,0.001,300,0.1)),
        ("turnover-objective2",(0.5,0.5,0.01,0.02,1,0,4,0.5,0.5,1,0.001)),
        ("factor-cov2",(0.04,1,2,0.01,0.02)),
        ("factor-exposure2",(1,2,0.3,0.7)),
        ("online-cov2",(daily,"R1","R2")),
        ("fifo-trade",(10,10,2,5,12,12,15,3)),
        ("integer-allocation2",(100,0.5,0.5,10,20,0)),
        ("drawdown-gross",(daily,"Equity",0.2,1)),
        ("factor-stress2",(1,0,0.5,1,-0.1,0.02,0.01,0,0.5,0.5)),
        ("par-bond-boot",(coupons,"Coupon")),
        ("par-coupon",(discounts,"D")),
        ("credit-migration2",(1,0.8,0.2,0,1,2)),
        ("credit-migration-loss",(0.64,0.36,0,100)),
        ("credit-dv01",(bond,0,0)),
        ("credit-duration",(bond,0,0)),
        ("american-binomial-greeks",(100,100,0.02,0,0.2,1,60,"call")),
        ("barrier-downout-mc",(100,90,80,0,0,1,12,100,42)),
        ("realized-variance-swap",(daily,"Return",252)),
        ("variance-swap-settle",(0.05,0.04,100)),
        ("variance-notional",(20,0.2)),
        ("continuous-future",(prices,"Price",roll)),
        ("futures-held-return",(daily,"OldPrice","NewPrice","Notional")),
        ("fx-covered",(7,0.05,0.02,1)),
        ("fx-covered-basis",(7*math.exp(0.03),7,0.05,0.02,1)),
        ("market-sweep",(offers,"Price","Quantity",4,"buy",100)),
        ("market-sweep-slip",(offers,"Price","Quantity",4,"buy",100)),
        ("vwap-target",(vwap,"Volume",100)),
        ("vwap-actual",(vwap,"Price","Filled")),
        ("borrow-fee",(100,10,0.36,360)),
        ("short-dividend",(100,0.5)),
        ("short-collateral",(100,10,0.2)),
        ("liquidity-clip",(liquidity,"Shares","ADV",0.1)),
        ("liquidity-capacity-days",(liquidity,"Shares","ADV",0.1)),
        ("fills-reconcile",(fills,"Signed","Price","Fee")),
        ("implementation-shortfall",(buy_fills,"Signed","Price","Fee",10,100,103)),
        ("lagged-trading",(strategy,"Signal","Return",0.01,1)),
        ("fill-intensity",(2,1,0)),
        ("fill-probability",(2,1)),
        ("queue-probability",(3,10)),
    ]
    assert len(commands)==43,len(commands)
    results={name:run(name,*args) for name,args in commands}
    assert abs(float(results["black-litterman-full"].splitlines()[0].split()[1])-0.05)<1e-8
    assert abs(float(results["fifo-trade"].splitlines()[0].split()[1])-51)<1e-7
    assert abs(float(results["integer-allocation2"].splitlines()[2].split()[1])-10)<1e-7
    assert abs(float(results["variance-swap-settle"].split()[1])-1)<1e-7
    assert abs(float(results["fx-covered-basis"].split()[1]))<1e-6
    assert abs(float(results["market-sweep-slip"].splitlines()[2].split()[1])-101.5)<1e-6
    assert abs(float(results["liquidity-capacity-days"].split()[1])-2)<1e-6
    assert abs(float(results["implementation-shortfall"].splitlines()[-1].split()[1])-18)<1e-7
    assert results["cvar-optimize"].startswith("Index,Weight")
    assert results["continuous-future"].startswith("Index,BackAdjusted")
    assert results["lagged-trading"].startswith("Index,Weight,Turnover")
    invalid=[
        ("cvar-evaluate",(daily,"R1","R2",0.5,1)),
        ("risk-budget2",(1,2,1,0.5,0.5,200,0.1)),
        ("black-litterman-full",(0,1,0.1,1,0)),
        ("turnover-opt2",(1,0,4,0.01,0.02,0.5,0.5,1,-0.001,100,0.1)),
        ("factor-cov2",(-1,1,2,0.01,0.02)),
        ("online-cov2",(daily,"R1","Missing")),
        ("fifo-trade",(10,10,2,5,12,20,15,3)),
        ("integer-allocation2",(100,1,1,10,20,0)),
        ("drawdown-gross",(daily,"Equity",-1,1)),
        ("factor-stress2",(1,0,0.5,1,-0.1,0.02,0.01,0.5,0.5)),
        ("par-coupon",(daily,"Missing")),
        ("credit-migration2",(1,0.8,0.3,0,1,2)),
        ("credit-migration-loss",(-1,2,0,100)),
        ("credit-dv01",(bond,0)),
        ("american-binomial-greeks",(100,100,0.02,0,0.2,1,60,"bad")),
        ("barrier-downout-mc",(100,90,110,0,0.2,1,12,100,42)),
        ("variance-swap-settle",(-1,0.04,100)),
        ("continuous-future",(prices,"Missing",roll)),
        ("fx-covered",(0,0.05,0.02,1)),
        ("market-sweep",(offers,"Price","Quantity",-4,"buy",100)),
        ("vwap-target",(vwap,"Volume",-1)),
        ("borrow-fee",(100,10,-1,360)),
        ("liquidity-clip",(liquidity,"Shares","ADV",0)),
        ("fills-reconcile",(fills,"Signed","Price","Missing")),
        ("implementation-shortfall",(fills,"Signed","Price","Fee",10,100,103)),
        ("lagged-trading",(strategy,"Signal","Return",-1,1)),
        ("fill-probability",(-2,1)),
        ("queue-probability",(1,-1)),
    ]
    for name,args in invalid:
        run(name,*args,valid=False)
print("PASS: 43 portfolio, derivatives and trading CLI commands with 28 invalid-input cases")
