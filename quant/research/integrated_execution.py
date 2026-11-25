"""Market event, liquidity estimation, dynamic control and accounting pipelines."""
import math,itertools
from comprehensive_study_common import near,check_vector

def replay_to_signs(model,report):
    output=[]
    for size in (1,2,4):
        orders=[[1,1,1,99,10*size],[1,2,-1,101,5*size],[3,3,1,0,2*size],[1,4,-1,102,3*size],[3,5,-1,0,4*size],[3,6,1,0,size]]
        book=model("price-time-matching-replay",orders,[])
        events=[]
        for t,r in enumerate(book):
            if r[2]>0:events.append([1,t,r[3]/r[2],r[2]])
            if r[6]>0 and r[8]>0:events.append([0,t,r[6],r[8]])
        signs=model("trade-signing-asof",events,[0,1,100])
        trades=[r for r in book if r[2]>0]
        assert len(signs)==len(trades)
        for i,(a,b) in enumerate(zip(signs,trades)):
            near(a[1]*a[2],b[3])
            assert a[3] in (-1,1)
            output.append([size,i+1,a[1],a[2],a[3],a[4],a[5],a[7],a[8]])
    report("matching_replay_to_trade_costs",["SizeScale","Trade","Price","Quantity","Sign","PretradeMid","EffectiveSpread","Markout","Valid"],output)

def competing_survival(model,report):
    output=[]
    for k in (1,2,4):
        data=[[1,1,1]]*k+[[2,2,1]]*k+[[3,0,1]]*k
        nonparam=model("aalen-johansen-incidence",[[a,b] for a,b,_ in data],[])
        for h in (1,2,3):
            param=model("queue-competing-hazard-fit",data,[h,0,1e-10,100])
            a,b=param[0][2],param[1][2]
            near(a,1/6);near(b,1/6)
            p=[r[3] for r in param];S=param[0][4]
            near(sum(p)+S,1);near(S,math.exp(-h/3))
            q=nonparam[h-1];near(sum(q[5:]),1)
            output.append([k,h,a,b,S,*p,q[5],q[6],q[7]])
    report("queue_parametric_nonparametric_incidence",["Multiplicity","Horizon","FillHazard","CancelHazard","ParametricSurvival","FillCIF","CancelCIF","EmpiricalSurvival","EmpiricalFill","EmpiricalCancel"],output)

def recovery_execution(model,report):
    output=[]
    for decay in (.25,.5,1.):
        data=[[t,.01+.02*math.exp(-decay*t),1] for t in (0,1,2,3,4)]
        fit=model("spread-resiliency-profile",data,[decay/2,1.5*decay,11])[0]
        near(fit[0],decay)
        for gamma in (0,.2,1):
            Q=5;eta=.8;a=[1.,2.,1.5];times=[0,1,3];v=[.2,.1,0]
            rows=model("transient-impact-schedule",[[t,c,z] for t,c,z in zip(times,a,v)],[Q,eta,fit[0],gamma])
            q=[r[1] for r in rows];near(sum(q),Q);near(rows[-1][2],0)
            def objective(z):
                risk=sum(v[i]*(Q-sum(z[:i+1]))**2 for i in range(3))
                return .5*(sum(a[i]*z[i]**2 for i in range(3))+eta*sum(z[i]*z[j]*math.exp(-decay*abs(times[i]-times[j])) for i in range(3) for j in range(3))+gamma*risk)
            ref=min(objective([Q*i/40,Q*j/40,Q*(40-i-j)/40]) for i in range(41) for j in range(41-i))
            near(rows[0][-1],objective(q));assert objective(q)<=ref+1e-7
            output.append([decay,gamma,*q,rows[0][-1],ref,fit[3]])
    report("liquidity_recovery_to_execution_schedule",["Decay","RiskAversion","Quantity1","Quantity2","Quantity3","Objective","GridUpperBound","HalfLife"],output)

def volatility_market_making(model,report):
    output=[]
    for noise in (0,.001,.003):
        price=[[.01*t+noise*(-1)**t] for t in range(21)]
        rv=model("two-scale-realized-variance",price,[2,3,4])
        for r in rv:
            var=max(0,r[-1]/20);sigma=math.sqrt(var);A=.25;gamma=1
            rows=model("market-maker-cara-dp",[[A,1,sigma,.05]],[2,2,gamma,1,0,1,3])
            for h in (1,2):
                layer=[z for z in rows if z[1]==h]
                near(layer[0][6],A*math.exp(-layer[0][3]) if layer[0][3]>=0 else 0)
                assert layer[0][4]==-1 and layer[-1][3]==-1
                near(layer[2][3],layer[2][4])
                for z in layer:assert z[6]+z[7]<=1+1e-12
                output.append([noise,r[0],var,h,layer[2][3],layer[2][5],layer[0][5],layer[-1][5]])
    report("two_scale_risk_to_inventory_policy",["NoiseAmplitude","Stride","VariancePerStep","Horizon","CentralBidDistance","CentralCE","ShortBoundaryCE","LongBoundaryCE"],output)

def point_process_quote_sensitivity(model,report):
    output=[]
    for t in (.2,.5,.8):
        for rate in (.5,1,2):
            fit=model("hawkes-network-em",[[t,1]],[1,1,rate,.4,.2,1,2,1e-9,1000])[0]
            mu,exc=fit[2],fit[3];near(mu,2/3)
            near(exc,1/(2+1-math.exp(-rate*(1-t))))
            intensity=mu+exc*rate*math.exp(-rate*(1-t))
            dt=.2
            rows=model("market-maker-cara-dp",[[intensity,1,.1,.05]],[1,1,1,dt,0,1,3])
            central=rows[1]
            assert central[6]+central[7]<=1
            ref=[]
            for db,da in itertools.product((-1,0,.5,1),repeat=2):
                pb=0 if db<0 else intensity*dt*math.exp(-db)
                pa=0 if da<0 else intensity*dt*math.exp(-da)
                if pb+pa>1:continue
                J=1-pb-pa
                if pb:J+=pb*math.exp(-db+.5*.1**2*dt+.05)
                if pa:J+=pa*math.exp(-da+.5*.1**2*dt+.05)
                ref.append(-math.log(J))
            near(central[5],max(ref))
            output.append([t,rate,mu,exc,intensity,central[3],central[4],central[5]])
    report("hawkes_intensity_to_quote_control",["EventTime","Decay","Baseline","Excitation","TerminalIntensity","BidDistance","AskDistance","CertaintyEquivalent"],output)

def stream_execution_audit(model,report):
    output=[]
    for order in ((1,2,3,4),(3,1,4,2),(4,2,1,3)):
        events=[[1,i,10*i] for i in order]+[[1,2,20]]
        ledger=model("sequence-gap-replay",events,[1])
        near(ledger[-1][5],4);near(ledger[-1][6],100)
        # Reconstructed unique sequence, then event-time window audit.
        data=[[i,t+1,10*i] for t,i in enumerate(order)]
        for lag in (0,2,10):
            bins=model("watermark-window-ledger",data,[2,lag])
            accepted=sum(r[2] for r in bins);late=sum(r[4] for r in bins)
            near(accepted+late,ledger[-1][5])
            near(sum(r[3]+r[5] for r in bins),ledger[-1][6])
            output.append([order[0],lag,accepted,late,sum(r[3] for r in bins),sum(r[5] for r in bins),ledger[-1][6]])
    report("sequence_watermark_execution_conservation",["FirstSequence","Lateness","Accepted","Late","AcceptedValue","LateValue","UniqueValue"],output)

def studies(model,report):
    for fn in (replay_to_signs,competing_survival,recovery_execution,volatility_market_making,point_process_quote_sensitivity,stream_execution_audit):fn(model,report)
