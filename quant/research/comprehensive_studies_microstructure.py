"""Market event, intensity and execution references."""
import math,itertools
from comprehensive_study_common import near,check_vector

def studies(model,report):
    results=[]
    for h in (.1,1,4):
        data=[[1,1,1],[2,1,1],[3,2,1],[4,0,1]]
        out=model("queue-competing-hazard-fit",data,[h,0,1e-9,100])
        a,b=.2,.1;S=math.exp(-(a+b)*h)
        for j,r in enumerate(out):
            z=[a,b][j];near(r[1],math.log(z));near(r[2],z);near(r[3],z/(a+b)*(1-S));near(r[4],S);near(r[5],0,atol=1e-7)
            results.append([h,*r])
    report("competing_exponential_mle",["Horizon","Cause","Coefficient","Hazard","Probability","Survival","ScoreNorm"],results)

    results=[]
    for lag in (0,.5,1):
        data=[[0,0,99,101],[1,1,101,2],[0,1,109,111],[1,2,109,1],[0,3,100,102],[1,4,101,1],[0,5,101,103]]
        out=model("trade-signing-asof",data,[lag,1,10])
        quotes=[];trades=[];last=0;tick=0
        for typ,t,a,b in data:
            if typ==0:quotes.append((t,(a+b)/2));continue
            if last and a!=last:tick=1 if a>last else -1
            last=a;eligible=[q for q in quotes if q[0]<=t-lag]
            mid=eligible[-1][1];sign=1 if a>mid else -1 if a<mid else tick
            trades.append((t,a,b,sign,mid))
        for r,z in zip(out,trades):
            check_vector(r[:5],z);near(r[5],2*z[3]*(z[1]-z[4]));results.append([lag,*r])
        changed=[r[:] for r in data];changed[2][2:]=[199,201]
        new=model("trade-signing-asof",changed,[lag,1,10])
        check_vector(new[0][:6],out[0][:6])
    report("trade_signing_causal_event_order",["Lag","Time","Price","Quantity","Sign","Mid","EffectiveSpread","FutureMid","Markout","Valid"],results)

    results=[]
    for scale in (1,2,5):
        x=[[1,1,-1,101,2*scale],[1,2,-1,101,3*scale],[1,3,-1,102,2*scale],[3,4,1,0,4*scale],[2,2,-1,0,scale],[3,5,1,0,3*scale]]
        out=model("price-time-matching-replay",x,[])
        near(out[3][2],4*scale);near(out[3][3],404*scale);near(out[3][9],scale)
        near(out[4][8],102);near(out[4][9],2*scale);near(out[5][2],2*scale);near(out[5][5],scale)
        assert all(abs(r[-1])<1e-8 for r in out)
        results.extend([[scale,*r] for r in out])
    report("matching_fifo_quantity_conservation",["Scale","Event","ID","Filled","Notional","Rested","Cancelled","BestBid","BidDepth","BestAsk","AskDepth","ConservationError"],results)

    results=[]
    for A in (.1,.3,.5):
        gamma=.7;sig=.2;phi=.1
        out=model("market-maker-cara-dp",[[A,1,sig,phi]],[1,1,gamma,1,0,1,3])
        for r in out:
            q=int(r[2]);values=[]
            for db,da in itertools.product((-1,0,.5,1),repeat=2):
                if q==1 and db>=0 or q==-1 and da>=0:continue
                pb=0 if db<0 else A*math.exp(-db);pa=0 if da<0 else A*math.exp(-da)
                if pb+pa>1:continue
                J=0
                for e,p,d in [(0,1-pb-pa,0),(1,pb,db),(-1,pa,da)]:
                    if p:J+=p*math.exp(-gamma*d+.5*gamma**2*sig**2*(q+e)**2+gamma*phi*(q+e)**2)
                values.append(-math.log(J)/gamma)
            near(r[5],max(values));results.append([A,*r])
    report("market_maker_one_step_enumeration",["Intensity","Scenario","Remaining","Inventory","BidDistance","AskDistance","CertaintyEquivalent","BuyProbability","SellProbability"],results)

    results=[]
    for eta in (0,.2,2):
        for gamma in (0,.5,1):
            Q=2;a=[1,3];v=.4;k=.7
            H=[[a[0]+eta,eta*math.exp(-k)],[eta*math.exp(-k),a[1]+eta+gamma*v]]
            q=Q*(H[1][1]-H[0][1])/(H[0][0]+H[1][1]-2*H[0][1]);q=max(0,min(Q,q))
            out=model("transient-impact-schedule",[[0,1,v],[1,3,0]],[Q,eta,k,gamma])
            check_vector([r[1] for r in out],[q,Q-q]);near(out[-1][2],0)
            obj=.5*(H[0][0]*q*q+2*H[0][1]*q*(Q-q)+H[1][1]*(Q-q)**2)
            near(out[0][-1],obj);results.extend([[eta,gamma,*r] for r in out])
    report("transient_impact_closed_form",["Eta","Gamma","Time","Quantity","Remaining","TemporaryCost","TransientCost","RiskCost","Objective"],results)

    results=[]
    for t in (.2,.5,.9):
        for rate in (.5,1,2):
            out=model("hawkes-network-em",[[t,1]],[1,1,rate,.4,.2,1,1,1e-9,1000])[0]
            e=1-math.exp(-rate*(1-t));a=1/(1+e);m=1
            ll=math.log(m)+math.log(m)-2*m+math.log(a)-(e+1)*a
            check_vector(out[2:6],[m,a,ll,0],atol=1e-7);results.append([t,rate,*out])
    report("hawkes_single_event_map",["EventTime","Decay","Target","Source","Baseline","Excitation","LogPosterior","ScoreNorm","RowSum"],results)

    results=[]
    for k in (.2,.5,1.5):
        data=[[t,1.2+2*math.exp(-k*t),1+t] for t in (0,1,2,4,6)]
        out=model("spread-resiliency-profile",data,[k/2,1.5*k,11])[0]
        check_vector(out,[k,1.2,2,math.log(2)/k,0],atol=1e-7);results.append([k,*out])
    report("resiliency_exact_profile",["TrueDecay","Decay","Baseline","Amplitude","HalfLife","WeightedSSE"],results)

    results=[]
    for slope in (.1,.5,1):
        n=20;data=[[slope*i] for i in range(n+1)]
        out=model("two-scale-realized-variance",data,[2,3,5,10])
        for r in out:
            K=int(r[0]);fast=n*slope*slope;slow=(n-K+1)*K*slope*slope;ratio=(n-K+1)/(K*n)
            check_vector(r[1:],[fast,slow,(n-K+1)/K,ratio,(slow-ratio*fast)/(1-ratio)])
            results.append([slope,*r])
    report("two_scale_exact_increment_counts",["Slope","Stride","FastRV","SlowRV","Count","NoiseRatio","AdjustedRV"],results)
