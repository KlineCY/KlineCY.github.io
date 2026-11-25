"""Data availability, multi-currency wealth and risk-accounting research."""
import math
from comprehensive_study_common import near,check_vector,dot

def split_currency_wealth(model,report):
    out=[]
    for ratio in (.5,2,4):
        data=[[0,100,1,0],[1,110/ratio,ratio,0],[2,108/ratio,1,2/ratio]]
        action=model("corporate-action-return-ledger",data,[])
        for fx in (.8,1,1.2):
            records=[[0,100,0,1,0],[1,action[1][5],0,fx,0],[2,action[2][5]+5,0,fx,5*fx]]
            wealth=model("multi-currency-cash-reconciliation",records,[])
            near(wealth[1][3],110*fx);near(wealth[2][5],0);near(wealth[2][7],110*fx)
            out.append([ratio,fx,action[1][5],action[2][5],wealth[1][3],wealth[2][3],wealth[2][4],wealth[2][7]])
    report("split_dividend_currency_cash_wealth",["Split","FX","LocalIndex1","LocalIndex2","Wealth1","Wealth2","ExternalFlow","PerformanceIndex"],out)

def historical_fx_vintage(model,report):
    out=[]
    for revision in (1.1,1.5,2):
        x=[[0,1,1,1,1],[0,1,2,2,1.1],[0,1,2,4,revision],[0,1,3,3,1.2]]
        q=[[1,1,t,t,0] for t in (1,2,3)]
        vint=model("bitemporal-vintage-join",x+q,[0])
        fx=[r[5] for r in vint];check_vector(fx,[1,1.1,1.2])
        rows=model("multi-currency-cash-reconciliation",[[t,100,0,z,0] for t,z in zip((1,2,3),fx)],[])
        for r,z in zip(rows,fx):near(r[3],100*z);near(r[7],100*z);out.append([revision,*r])
    report("historical_fx_vintage_wealth_isolation",["Revision","Time","AssetBase","CashBase","Wealth","ExternalFlow","PnL","Return","Index"],out)

def currency_risk_allocation(model,report):
    out=[]
    local=[[.02,-.01],[-.01,.03],[.01,.015],[-.03,-.02],[.025,-.005]]
    for fxscale in (0,.01,.03):
        fx=[fxscale*v for v in (1,-1,0,2,-2)]
        gross=[]
        for i in range(2):
            localvalue=100;FX=1;rows=[[0,100,0,1,0]]
            for t,r in enumerate(local,1):
                localvalue*=1+r[i];FX*=1+fx[t-1]*(i==1)
                rows.append([t,localvalue,0,FX,0])
            ledger=model("multi-currency-cash-reconciliation",rows,[])
            gross.append([r[6] for r in ledger[1:]])
            for t,v in enumerate(gross[-1]):near(v,(1+local[t][i])*(1+fx[t]*(i==1))-1)
        scen=[[gross[i][t] for i in range(2)] for t in range(5)]
        alloc=model("entropic-risk-allocation",scen,[5,1e-9,20000]);w=[r[1] for r in alloc]
        near(sum(w),1);near(alloc[0][2],math.log(sum(math.exp(-5*dot(w,r)) for r in scen)/5)/5)
        out.append([fxscale,*w,alloc[0][2],alloc[0][3],alloc[0][4]])
    report("base_currency_returns_entropic_allocation",["FXScale","Weight1","Weight2","EntropicRisk","ExpectedReturn","MaximumTiltMass"],out)

def label_preprocess_rank(model,report):
    out=[]
    for delay in (0,1,2):
        prices=[]
        for i in (1,2,3):
            for t in range(1,9):prices.append([0,i,t,100*(1+.01*i)**t])
        queries=[[1,i,t,6] for t in range(1,6) for i in (1,2,3)]
        labels=model("label-window-availability",prices+queries,[1,delay])
        eligible=[r for r in labels if r[5]]
        raw=[[int(r[5]),r[0]] for r in labels]
        z=model("train-only-winsor-standardize",raw,[0,1,1e-12])
        train=[[int(r[5]),r[1],r[3],a[-1]] for r,a in zip(labels,z)]
        # Each date has three assets with identical label availability.
        rank=model("ranknet-linear-fit",train,[1,1e-9,100])
        assert rank[0][-1]>0
        for r,a in zip(labels,rank):
            near(r[3],.01*r[0]);out.append([delay,r[0],r[1],r[5],a[4],a[-1],len(eligible)])
    report("label_maturity_transform_rank_training",["Delay","Asset","Start","Eligible","Score","RankCoefficient","TrainingRows"],out)

def stream_vintage_join(model,report):
    out=[]
    for order in ((1,2,3),(3,1,2),(2,3,1)):
        stream=[[1,i,10*i] for i in order]+[[1,2,20]]
        replay=model("sequence-gap-replay",stream,[1])
        available={}
        committed=0
        for t,r in enumerate(replay,1):
            end=int(r[3])
            for j in range(committed+1,end+1):available[j]=t
            committed=end
        # Sequence index is the effective time, and the explicit economic clock starts
        # later than the three source timestamps so availability cannot precede effective time.
        x=[[0,1,j,available[j]+3,10*j] for j in (1,2,3)]
        queries=[[1,1,3,t] for t in range(4,8)]
        result=model("bitemporal-vintage-join",x+queries,[3])
        for r in result:
            visible=[j for j in available if available[j]+3<=r[1]]
            if visible:near(r[5],10*max(visible));near(r[6],1)
            else:near(r[6],0)
            out.append([order[0],*r])
    report("sequence_commit_to_point_in_time_vintage",["FirstSequence","Entity","Decision","Target","Effective","Available","Value","Valid"],out)

def watermark_label_eligibility(model,report):
    out=[]
    for lag in (0,1,3):
        arrivals=[[1,1,100],[3,2,121],[2,3,110],[4,4,133.1],[5,5,146.41]]
        w=model("watermark-window-ledger",arrivals,[1,lag])
        prices=[[0,1,r[0],r[3]] for r in w if r[2]==1]
        queries=[[1,1,t,6] for t in (1,2,3,4)]
        labels=model("label-window-availability",prices+queries,[1,0])
        available={r[2] for r in prices}
        for r in labels:
            ok=r[1] in available and r[2] in available
            near(r[6],int(ok));near(r[5],int(ok))
            if ok:near(r[3],.1)
            out.append([lag,*r,sum(z[4] for z in w)])
    report("late_event_windows_to_label_availability",["Lateness","Asset","Start","End","Label","Available","Eligible","Valid","LateCount"],out)

def studies(model,report):
    for fn in (split_currency_wealth,historical_fx_vintage,currency_risk_allocation,label_preprocess_rank,stream_vintage_join,watermark_label_eligibility):fn(model,report)
