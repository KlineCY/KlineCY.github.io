"""Linked pipelines with explicit inter-model contracts and independent conservation checks."""
import math
from system_study_common import near,check_vector,bartlett,dot

def studies(model,report):
    rows=[]
    # Corporate actions -> investable return series -> volatility scaling -> unitized account.
    assets=[]
    for j in [0,1]:
        data=[[100+20*j,1,0]]
        for t in range(1,41):
            r=.01*math.sin(.4*t+j)+.004*math.cos(.7*t);a=2 if t==12+j else 1;d=.2 if t%9==0 else 0
            P=data[-1][0]*(1+r)/a-d;data.append([P,a,d])
        a=model("corporate-action-ledger",data,[1]);assets.append([r[5] for r in a[1:]])
    ret=list(map(list,zip(*assets)))
    for cost in [0,.001,.003]:
        risk=model("rolling-risk-scaling",ret,[8,.008,1.4,cost,.6,.4])
        account=model("cashflow-portfolio-unitization",[[10 if i%7==0 else -3 if i%5==0 else 0,r[4]] for i,r in enumerate(risk)],[100,1])
        for a,b in zip(risk,account):
            near(b[7],a[7]-1);near(b[8],0);rows.append([cost,a[0],a[1],a[2],a[4],a[6],a[7],b[2],b[5],b[7]])
    report("linked_actions_risk_unitization",["Cost","Period","RiskScale","ForecastVol","NetReturn","Fee","StrategyNAV","ExternalFlow","AccountValue","TWR"],rows)

    # Exchange events -> bars -> frozen seasonal estimates -> out-of-sample forecast comparison.
    rows=[];trades=[]
    for t in range(72):
        for k in [0,1,2]:
            trades.append([t+k/4,100+.1*math.sin(.5*t+k),2+math.cos(2*math.pi*t/12)+.1*k])
    bars=model("session-ohlcv-bars",trades,[0,72,1]);near(sum(r[6] for r in bars),sum(r[2] for r in trades))
    data=[[r[0],r[6],int(r[0]<48)] for r in bars];mean=sum(r[1] for r in data[:48])/48
    for K in [1,2,3]:
        fit=model("fourier-seasonal-fit",data,[12,K,.1]);test=[[r[2],r[3],mean] for r in fit if not r[1]]
        dm=model("rolling-forecast-comparison",test,[3])[0]
        hac=model("newey-west-long-run",[[(y-a)**2-(y-b)**2] for y,a,b in test],[3])[0]
        near(dm[3],hac[0]);near(dm[4],hac[2]);near(dm[5],hac[0]/hac[3])
        rows.append([K,*dm,hac[3],hac[4]])
    report("linked_events_seasonality_forecast",["Harmonics","N","MSEA","MSEB","LossDiff","HACVariance","DM","Degenerate","MeanSE","EffectiveN"],rows)

    # Availability-time features -> same-time weighted standardization -> group rank loss.
    rows=[];data=[]
    for t in range(12):
        for a in range(1,5):
            data.append([1,a,t+.2,math.sin(.5*t+a)])
            data.append([0,a,t+1,0])
    for maxage in [1,2]:
        aligned=model("lagged-asof-panel",data,[maxage])
        signal=[[r[1],r[4],1+r[0]] for r in aligned if not r[5]]
        standardized=model("cross-sectional-weighted-standardize",signal,[])
        # The label is an independent, synthetic next-period cross-sectional outcome.
        ranked=[]
        for r in standardized:
            k=int(r[0])-1;label=math.sin(.6*k)+.15*math.cos(1.3*k)
            ranked.append([r[1],r[6],label])
        out=model("pairwise-rank-logistic",ranked,[])
        for r in out:
            g=r[0];z=[v for v in standardized if v[1]==g];near(sum(v[3]*v[6] for v in z),0)
            rows.append([maxage,*r])
    report("linked_asof_cross_section_ranking",["AgeLimit","Group","Pairs","Loss","Concordance","MeanAbsGradient","GradientSum"],rows)

    # Frozen classifier -> validation threshold -> independent conformal calibration -> testing.
    rows=[];data=[]
    for i in range(100):
        label=int((i%7)/7<.5+.2*math.sin(.3*i));x=.8*label+math.sin(.8*i)
        data.append([int(i<40),label,x])
    for penalty in [.05,.2,1]:
        qda=model("weighted-gaussian-discriminant",data,[penalty])
        # Separate train 0:40, validation 40:60, calibration 60:80, test 80:100.
        dec=[[r[3],r[2],int(40<=i<60)] for i,r in enumerate(qda) if i>=40]
        chosen=model("cost-sensitive-threshold",dec,[1,2])
        cal=[[int(60<=i<80),int(r[2])+1,1-r[3],r[3]] for i,r in enumerate(qda) if i>=60]
        conf=model("conformal-classification-set",cal,[.15])
        for j in range(20):
            a=chosen[40+j];b=conf[20+j]
            near(a[2],qda[80+j][3]);near(a[3]+1,b[2]);near(b[5],b[6+int(a[3])])
            rows.append([penalty,81+j,a[2],a[3],a[4],a[5],a[6],b[3],b[4],b[5]])
    report("linked_classifier_decision_conformal",["Ridge","TestRow","Probability","Label","DecisionThreshold","Decision","Cost","ConformalThreshold","SetSize","Covered"],rows)

    # Fit a hedge on a historical window -> weighted empirical tail loss on held-out rows.
    rows=[];data=[]
    for t in range(80):
        x=.02*math.sin(.4*t);z=.015*math.cos(.7*t);e=.003*math.sin(1.9*t)
        data.append([1.4*x-.8*z+e,x,z,int(t<50)])
    for lam in [0,.00001,.0001]:
        hedge=model("minimum-variance-hedge",data,[lam]);h=hedge[0][4:];test=data[50:]
        scenario=[[1/len(test),-r[0],r[1],r[2]] for r in test]
        for a in [.8,.9,.95]:
            risk=model("scenario-tail-attribution",scenario,[a,1,*h]);unhedged=model("scenario-tail-attribution",scenario,[a,1,0,0])
            near(sum(r[4] for r in risk),risk[0][3]);near(sum(r[4] for r in unhedged),unhedged[0][3])
            for j,r in enumerate(risk):rows.append([lam,a,unhedged[0][3],r[0],r[1],r[2],r[3],r[4]])
    report("linked_hedge_tail_risk",["Penalty","Confidence","UnhedgedES","Component","Weight","HedgedVaR","HedgedES","Contribution"],rows)

    # Multi-period GARCH forecast -> Gaussian probability scoring -> paired loss HAC.
    rows=[];actual=[.012*math.sin(.4*t)+.006*math.cos(1.2*t) for t in range(90)]
    for alpha,beta in [(.1,.8),(.2,.6)]:
        h=.0001;last=0;predict=[]
        for t,y in enumerate(actual):
            z=model("garch-horizon-risk",[[h,.00001,alpha,beta,last]],[1])[0][2]
            predict.append([y,0,math.sqrt(z)]);h=z;last=y
        score=model("gaussian-crps-score",predict,[])
        fixed=model("gaussian-crps-score",[[y,0,.015] for y in actual],[])
        delta=[a[1]-b[1] for a,b in zip(score,fixed)]
        hac=model("newey-west-long-run",[[v] for v in delta],[5])[0];near(hac[0],sum(delta)/len(delta));near(hac[2],bartlett(delta,5))
        for i,(a,b) in enumerate(zip(score,fixed)):rows.append([alpha,beta,i+1,predict[i][2],a[1],b[1],hac[0],hac[3]])
    report("linked_garch_distribution_scoring",["Alpha","Beta","Time","ForecastStd","GarchCRPS","FixedCRPS","MeanDiff","HACSE"],rows)

    # Exact geometric Asian value -> pilot-controlled arithmetic Asian -> variance reduction.
    rows=[]
    for m in [4,12,24]:
        for sig in [.1,.25,.5]:
            r=[100,100,.025,.01,sig,1]
            geo=model("asian-geometric-exact",[r],[m])[0]
            arith=model("asian-arithmetic-control",[r],[m,2000,10000,711])[0]
            near(geo[0],arith[5]);assert arith[3]<arith[2];assert arith[1]>=geo[0]-6*arith[3]-1e-8
            rows.append([m,sig,geo[0],arith[0],arith[1],arith[2],arith[3],arith[4],arith[6]])
    report("linked_asian_exact_control",["Monitoring","Volatility","GeometricCall","PlainArithmetic","ControlledArithmetic","PlainSE","ControlledSE","Beta","VarianceRatio"],rows)

    # Curve node sensitivity -> scenario losses -> ES component attribution.
    rows=[];curve=[[.5,math.exp(-.02*.5),.5],[1,math.exp(-.025),.5],[1.5,math.exp(-.03*1.5),.5],[2,math.exp(-.035*2),.5]]
    for rate in [.01,.04,.07]:
        risk=model("swap-curve-keyrate",curve,[1000,rate,1e-5]);g=[r[4] for r in risk]
        scenarios=[]
        for i in range(20):
            shock=[.002*math.sin(.5*i)+.001*j*math.cos(i) for j in range(4)]
            scenarios.append([.05,*[-a*b for a,b in zip(g,shock)]])
        es=model("scenario-tail-attribution",scenarios,[.9,1,1,1,1])
        near(sum(r[4] for r in es),es[0][3])
        for j,r in enumerate(es):rows.append([rate,j+1,g[j],r[2],r[3],r[4]])
    report("linked_swap_curve_tail_risk",["FixedRate","Node","Sensitivity","VaR","ES","ESContribution"],rows)

    # Integer allocation -> optimal venue route -> FIFO execution and liquidation accounting.
    rows=[];assets=[[1,10,37,10],[2,15,40,8],[3,20,22,6]]
    for B in [60,90,120]:
        allocation=model("integer-lot-allocation",assets,[B])
        for j,a in enumerate(allocation):
            q=a[1];price=assets[j][1];route=model("venue-convex-routing",[[1,.02,.01,10],[2,.01,.02,10]],[q])
            trades=[[r[1],price,price,r[2]] for r in route if r[1]>1e-12]
            if not trades:continue
            trades.append([-q,price*1.03,price*1.03,0])
            ledger=model("fifo-position-ledger",trades,[B])
            final=ledger[-1];near(final[1],0);near(final[6],B+q*price*.03-route[0][5]);near(final[-1],0)
            rows.append([B,a[0],q,a[2],route[0][5],final[3],final[6],final[-1]])
    report("linked_integer_route_execution",["Budget","Asset","Lots","Notional","RoutingCost","RealizedPnL","FinalEquity","Residual"],rows)

    # Futures external funding and investment performance are separate accounting quantities.
    rows=[]
    for mult in [5,10,20]:
        data=[[100,2,0],[108,2,1],[95,1,1],[90,-1,1],[85,0,1]]
        ledger=model("futures-margin-ledger",data,[100,100,mult,.12,.08])
        # Explicit accounting convention: treat the ledger's funding as opening capital
        # for the day's performance measurement. This is retrospective attribution.
        events=[];balance=100
        for r in ledger:
            net=r[3]-r[4];flow=r[9];invested=balance+flow
            assert invested>0 and invested+net>0
            events.append([flow,net/invested]);balance=invested+net
            near(balance,r[10])
        account=model("cashflow-portfolio-unitization",events,[100,1])
        for i,r in enumerate(account):
            near(r[8],0);near(r[5],ledger[i][10]);rows.append([mult,i+1,r[2],r[5],r[6],r[7]])
    report("linked_margin_and_external_flows",["Multiplier","Event","CapitalFlow","AccountValue","PeriodReturn","TWR"],rows)

    # Price shock -> Logit market demand -> IO final demand -> direct and embodied footprint.
    rows=[];A=[[.1,.2,.1],[.2,.1,.1],[.05,.1,.2]];intensity=[1,2,.5]
    for shock in [-.2,0,.2,.5]:
        choice=model("logit-market-share",[[.4,1+shock],[.3,1.1],[.1,.8]],[1.2])
        # Assumption: 100 consumers each buy at most one unit, unit monetary output price normalized.
        f=[100*r[1] for r in choice];io=model("input-output-carbon",[A[j]+[intensity[j],f[j]] for j in range(3)],[])
        near(sum(f),100*(1-choice[0][2]));near(sum(r[2] for r in io),sum(r[4] for r in io))
        for j,r in enumerate(io):rows.append([shock,j+1,choice[j][1],f[j],r[1],r[2],r[3],r[4],r[5]])
    report("linked_demand_network_footprint",["PriceShock","Sector","MarketShare","FinalDemand","Output","DirectFootprint","EmbodiedIntensity","DemandFootprint","TotalFootprint"],rows)

    # Label intervals -> purged training sets -> separate QDA fit for each test fold.
    rows=[];data=[]
    for i in range(60):
        fold=i//20;label=i%2;data.append([i,i+2,fold,label,.6*label+math.sin(.5*i)])
    for embargo in [0,2,4]:
        cut=model("label-interval-purge-folds",[r[:3] for r in data],[embargo])
        for fold in [0,1,2]:
            state={int(r[1])-1:int(r[2]) for r in cut if r[0]==fold}
            fit=model("weighted-gaussian-discriminant",[[int(state[i]==1),r[3],r[4]] for i,r in enumerate(data)],[.1])
            for i,r in enumerate(fit):
                if state[i]==2:
                    assert all(not (data[j][0]<=data[i][1] and data[i][0]<=data[j][1]) for j in state if state[j]==1)
                    p=r[3];y=r[2];loss=(p-y)**2
                    rows.append([embargo,fold,i+1,sum(v==1 for v in state.values()),p,y,loss])
    report("linked_purged_classifier_validation",["Embargo","Fold","Row","TrainingCount","Probability","Label","BrierLoss"],rows)
