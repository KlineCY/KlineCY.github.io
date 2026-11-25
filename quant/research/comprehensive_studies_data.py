"""Point-in-time isolation, data conservation and wealth identities."""
import math
from comprehensive_study_common import near,check_vector

def studies(model,report):
    out=[]
    for new in (12,50,-3):
        rows=model("bitemporal-vintage-join",[[0,1,1,2,10],[0,1,1,4,new],[1,1,1,3,0],[1,1,1,5,0]],[10])
        near(rows[0][5],10);near(rows[1][5],new);out.extend([[new,*r] for r in rows])
    report("vintage_future_revision_isolation",["Revision","Entity","Decision","Target","Effective","Available","Value","Valid"],out)
    out=[]
    for lag in (0,1,3,10):
        data=[[1,1,2],[6,2,5],[2,3,4],[5,4,3],[8,5,1]]
        rows=model("watermark-window-ledger",data,[2,lag])
        near(sum(r[2]+r[4] for r in rows),len(data));near(sum(r[3]+r[5] for r in rows),sum(r[2] for r in data))
        for r in rows:out.append([lag,*r])
    report("watermark_count_value_conservation",["Lag","Start","End","Accepted","Sum","Late","LateSum"],out)
    out=[]
    for order in ((1,2,3,4),(4,3,2,1),(2,4,1,3)):
        data=[[1,i,10*i] for i in order]+[[1,2,20]]
        rows=model("sequence-gap-replay",data,[1]);near(rows[-1][3],4);near(rows[-1][5],4);near(rows[-1][6],100);near(rows[-1][2],0)
        out.extend([[order[0],*r] for r in rows])
    report("sequence_permutation_idempotence",["FirstSequence","Partition","Sequence","Status","ContiguousThrough","Buffered","Committed","Sum"],out)
    out=[]
    for delay in (0,.5,2):
        data=[[0,1,1,100],[0,1,2,110],[0,1,3,121]]+[[1,1,1,t] for t in (1.5,2,2.5,4)]
        rows=model("label-window-availability",data,[1,delay])
        for t,r in zip((1.5,2,2.5,4),rows):near(r[3],.1);near(r[5],float(t>=2+delay));out.append([delay,t,*r])
    report("label_delay_eligibility",["Delay","DecisionTime","Asset","Start","End","Label","Available","Eligible","Valid"],out)
    out=[]
    for k in (1,2,4):
        data=[[i,i+k] for i in range(1,7)];rows=model("overlapping-label-uniqueness",data,[])
        counts={t:sum(a<=t<=b for a,b in data) for t in range(1,7+k)}
        u=[sum(1/counts[t] for t in range(a,b+1))/(b-a+1) for a,b in data]
        check_vector([r[4] for r in rows],u);near(sum(r[5] for r in rows),len(data))
        out.extend([[k,*r] for r in rows])
    report("uniqueness_explicit_concurrency",["Horizon","Event","Start","End","Length","Uniqueness","Weight"],out)
    out=[]
    for test in (-1e4,0,1e4):
        rows=model("train-only-winsor-standardize",[[1,1],[1,2],[1,3],[0,test]],[0,1,1e-12])
        for r in rows:check_vector(r[4:6],[1,3]);near(r[7],2);near(r[8],math.sqrt(2/3));out.append([test,*r])
    report("winsor_holdout_mutation",["TestValue","Row","Train","Feature","Original","Lower","Upper","Clipped","Mean","SD","Z"],out)
    out=[]
    for split in (.5,2,4):
        data=[[1,100,1,0],[2,100/split,split,0],[3,100/split-1,1,1]]
        rows=model("corporate-action-return-ledger",data,[])
        for r in rows:near(r[4],1);near(r[5],100);out.append([split,*r])
    report("corporate_action_wealth_invariance",["SplitRatio","Time","Close","Split","Dividend","GrossReturn","Index"],out)
    out=[]
    for fx in (.5,1,2):
        data=[[1,80,20,1,0],[2,90,20,fx,0],[3,100,20,fx,10*fx]]
        rows=model("multi-currency-cash-reconciliation",data,[])
        near(rows[1][6],110*fx/100-1);near(rows[2][5],0);near(rows[2][7],110*fx)
        out.extend([[fx,*r] for r in rows])
    report("currency_cashflow_identity",["FX","Time","AssetBase","CashBase","Wealth","Flow","PnL","Return","Index"],out)
