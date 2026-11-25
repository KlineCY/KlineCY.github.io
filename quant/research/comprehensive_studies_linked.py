"""Cross-model studies: verify interfaces, independent accounting and causal information sets."""
import math,itertools
from comprehensive_study_common import near,check_vector,dot,mv,mm,transpose,solve

def studies(model,report):
    out=[]
    # Forecast coefficient stability, covariance construction and constrained risk decision.
    for scale in (.5,1,2):
        f=[-2,-1,0,1,2]
        data=[[z,scale*z,-.5*scale*z] for z in f]
        cov=model("factor-covariance-estimate",data,[1,0,0,.1])
        S=[r[4:] for r in cov]
        mu=[.03,.01]
        te=.1
        w=model("active-tracking-frontier",[[*S[i],mu[i],.5] for i in range(2)],[te])
        a=[r[2] for r in w]
        near(sum(a),0);near(dot(a,mv(S,a)),te**2)
        expected=te/math.sqrt(S[0][0]+S[1][1]-2*S[0][1])
        near(a[0],expected)
        out.append([scale,S[0][0],S[0][1],S[1][1],a[0],a[1],dot(mu,a),te])
    report("factor_to_tracking_risk",["LoadingScale","Cov11","Cov12","Cov22","Active1","Active2","ActiveAlpha","TrackingError"],out)
    out=[]
    for tau in (.1,.5,1):
        S=[[1,.2],[.2,2]];pi=[.1,.2];q=[.3,.1];O=[[.3,.1],[.1,.4]]
        rows=model("black-litterman-correlated-views",[[0,*S[i],pi[i]] for i in range(2)]+[[1,float(i==0),float(i==1),q[i]] for i in range(2)]+[[2,*O[i],0] for i in range(2)],[2,2,tau,2])
        m=[r[2] for r in rows];V=[r[6:] for r in rows]
        risk=[[S[i][j]+V[i][j] for j in range(2)] for i in range(2)]
        # Separate relative active allocation from the unconstrained absolute allocation.
        act=model("active-tracking-frontier",[[*risk[i],m[i],.5] for i in range(2)],[.15])
        a=[r[2] for r in act];near(dot(a,mv(risk,a)),.15**2)
        near(sum(a),0);assert dot(m,a)>=-1e-10
        out.append([tau,*m,*a,dot(m,a),math.sqrt(dot(a,mv(risk,a)))])
    report("posterior_to_active_allocation",["Tau","Mean1","Mean2","Active1","Active2","ActiveReturn","TrackingError"],out)
    out=[]
    for shape in (.5,1,3):
        # Positive-volume GLM predicts a scale for order-splitting temporary costs.
        pred=model("gamma-log-glm",[[1,1,1],[1,2,1],[1,4,1],[0,2,1]],[shape,1,1e-9,100])
        volume=pred[-1][3];temp=[1/volume,2/volume];Q=2
        plan=model("transient-impact-schedule",[[0,temp[0],.2],[1,temp[1],0]],[Q,.3,1,.5])
        H=[[temp[0]+.3,.3*math.exp(-1)],[.3*math.exp(-1),temp[1]+.3+.1]]
        q=Q*(H[1][1]-H[0][1])/(H[0][0]+H[1][1]-2*H[0][1])
        q=max(0,min(Q,q))
        near(plan[0][1],q);near(sum(r[1] for r in plan),Q)
        near(plan[-1][2],0)
        out.append([shape,volume,*temp,plan[0][1],plan[1][1],plan[0][-1]])
    report("volume_prediction_to_execution",["Shape","ExpectedVolume","Temp1","Temp2","Order1","Order2","Objective"],out)
    out=[]
    for decay in (.3,.7,1.2):
        data=[[t,1+2*math.exp(-decay*t),1] for t in (0,1,2,3,5)]
        fit=model("spread-resiliency-profile",data,[decay/2,1.5*decay,11])[0]
        plan=model("transient-impact-schedule",[[0,1,.1],[1,2,.1],[3,1,0]],[3,.5,fit[0],1])
        w=[r[1] for r in plan]
        near(sum(w),3);near(fit[0],decay)
        temp=sum(.5*r[1]**2*a for r,a in zip(plan,[1,2,1]))
        trans=.25*sum(w[i]*w[j]*math.exp(-decay*abs(t-s)) for i,t in enumerate([0,1,3]) for j,s in enumerate([0,1,3]))
        risk=.05*sum(r[2]**2 for r in plan[:2])
        near(plan[0][-1],temp+trans+risk)
        out.append([decay,fit[3],*w,temp,trans,risk,plan[0][-1]])
    report("resiliency_to_transient_execution",["Decay","HalfLife","Order1","Order2","Order3","TemporaryCost","TransientCost","RiskCost","Total"],out)
    out=[]
    for lag in (0,.5,1):
        # Sequence reconciliation commits unique event values; quote sign remains causal.
        replay=model("sequence-gap-replay",[[1,1,2],[1,3,4],[1,2,3],[1,2,3]],[1])
        near(replay[-1][-1],9);near(replay[-1][5],3)
        rows=model("trade-signing-asof",[[0,0,99,101],[1,1,101,2],[0,1,109,111],[1,2,109,3],[0,3,100,102],[1,4,101,4],[0,5,101,103]],[lag,1,10])
        volume=sum(r[2] for r in rows);near(volume,replay[-1][-1])
        eligible=[r for r in rows if r[-1]]
        spread=sum(r[2]*r[5] for r in eligible)/sum(r[2] for r in eligible)
        markout=sum(r[2]*r[7] for r in eligible)/sum(r[2] for r in eligible)
        out.append([lag,volume,sum(r[2] for r in eligible),spread,markout])
    report("event_replay_to_trade_cost",["QuoteLag","CommittedVolume","ValidVolume","EffectiveSpread","Markout"],out)
    out=[]
    for shape in (1,2,3):
        # Competing risks: compare parametric cumulative probabilities with the same survival mass.
        data=[[1,1,1]]*shape+[[1,2,1]]*shape+[[1,0,1]]*shape
        param=model("queue-competing-hazard-fit",data,[1,0,1e-9,100])
        empirical=model("aalen-johansen-incidence",[[r[0],r[1]] for r in data],[])[-1]
        near(sum(r[3] for r in param)+param[0][4],1)
        near(sum(empirical[5:]),1)
        near(param[0][3],param[1][3]);near(empirical[6],empirical[7])
        out.append([shape,param[0][3],param[1][3],param[0][4],*empirical[5:]])
    report("competing_risk_parametric_nonparametric",["Multiplicity","ParamCIF1","ParamCIF2","ParamSurvival","AJSurvival","AJCIF1","AJCIF2"],out)
    out=[]
    for vol in (.1,.2,.3):
        # A constant-variance Heston surface generates OTM prices and the variance strip.
        data=[]
        for K in range(20,301,2):
            row=model("heston-fourier-inversion",[[100,K,0,0,1,1,vol**2,0,0,vol**2]],[100,100])
            data.append([K,max(0,row[0][0]),max(0,row[0][1])])
        strip=model("variance-strip-quadrature",data,[100,1,1e-8]);v=strip[-1][5]
        assert abs(v-vol**2)<.0005
        out.append([vol,v,vol**2,v-vol**2])
    report("heston_surface_to_variance_strip",["Volatility","ReplicatedVariance","AnalyticVariance","Error"],out)
    out=[]
    for sigma in (0,.02,.05):
        # Calibrate a rate tree, use the repriced discount factors to recover a projection curve.
        D=[math.exp(-.02*t) for t in (1,2,3)]
        tree=model("ho-lee-curve-tree",[[d] for d in D],[1,sigma,.01])
        inferred=[next(r[6] for r in tree if r[0]==t) for t in range(3)]
        near(max(abs(a-b) for a,b in zip(inferred,D)),0)
        forwards=[.02,.03,.04];A=B=0;data=[]
        for i,(d,f) in enumerate(zip(inferred,forwards),1):
            A+=d;B+=d*f;data.append([i,1,d,B/A])
        boot=model("multicurve-forward-bootstrap",data,[])
        check_vector([r[4] for r in boot],forwards)
        out.extend([[sigma,*r] for r in boot])
    report("rate_tree_to_forward_bootstrap",["Volatility","Maturity","Accrual","Discount","ParSwap","Forward","Annuity","FloatingPV","RepricedSwap","Residual"],out)
    out=[]
    for prior in (.5,1,2):
        # Mean-transition Markov forecasts are conditional plug-ins, distinct from integrated two-step prediction.
        rows=model("dirichlet-transition-posterior",[[1,1,4],[1,2,1],[2,1,2],[2,2,3]],[2,prior,prior])
        P=[[rows[2*i+j][3] for j in range(2)] for i in range(2)]
        mu=[-1,2];initial=[1,0]
        reward=model("markov-cumulative-reward",[[*P[i],mu[i],0,initial[i]] for i in range(2)],[2])
        near(reward[1][1],mu[0]+dot(P[0],mu))
        plug=mm(P,P)[0][0];integrated=rows[0][-1]
        out.append([prior,plug,integrated,integrated-plug,reward[1][1],reward[1][2]])
    report("markov_parameter_risk_and_reward",["Prior","PluginTwoStep","BayesianTwoStep","Difference","MeanReward","RewardVariance"],out)
    out=[]
    for alpha in (.2,.5,1):
        # Align point-in-time revisions, train only on available labels, then freeze feature statistics.
        history=[[0,1,1,1,1],[0,1,2,2,2],[0,1,3,3,3],[0,1,1,4,100],[1,1,1,1,0],[1,1,2,2,0],[1,1,3,3,0]]
        v=model("bitemporal-vintage-join",history,[10]);check_vector([r[5] for r in v],[1,2,3])
        prices=[[0,1,1,100],[0,1,2,110],[0,1,3,121],[0,1,4,133.1]]+[[1,1,t,3] for t in (1,2,3)]
        labels=model("label-window-availability",prices,[1,0]);check_vector([r[5] for r in labels],[1,1,0])
        z=model("train-only-winsor-standardize",[[r[5],v[i][5]] for i,r in enumerate(labels)],[0,1,1e-12])
        near(z[0][7],1.5);near(z[-1][-1],1)
        # Empirical Bayes uses only the historical eligible returns.
        est=model("empirical-bayes-alpha-grid",[[r[3],.1] for r in labels if r[5]],[alpha,alpha,2])
        near(est[0][5],.1)
        out.append([alpha,z[0][7],z[-1][-1],sum(r[5] for r in labels),est[0][5]])
    report("vintage_label_feature_pipeline",["PriorVariance","TrainMean","TestStandardized","EligibleLabels","PosteriorAlpha"],out)
    out=[]
    for split in (.5,2,4):
        prices=[[1,100,1,0],[2,100/split,split,0],[3,100/split-1,1,1]]
        index=model("corporate-action-return-ledger",prices,[])
        account=[[r[0],r[-1],0,1,0] for r in index]
        cash=model("multi-currency-cash-reconciliation",account,[])
        for r in cash:near(r[6],0);near(r[-1],100)
        out.append([split,index[-1][-1],cash[-1][-1],sum(r[5] for r in cash)])
    report("corporate_actions_to_wealth_account",["Split","TotalReturnIndex","AccountIndex","TotalPnL"],out)
    out=[]
    for ice in (1,1.5,2):
        # Local trade expenditure reallocation transmitted as a fixed final-demand vector.
        trade=model("ces-armington-trade-counterfactual",[[1,1,.5,1,1],[2,1,.5,1,ice]],[2,100])
        f=[r[-1] for r in trade];A=[[.2,.1],[.1,.2]];v=[.5,.5]
        q=model("leontief-dual-price-system",[[*A[i],f[i],v[i]] for i in range(2)],[])
        outputs=[r[1] for r in q];p=[r[2] for r in q]
        near(sum(f),100);near(dot(p,f),dot(v,outputs))
        out.append([ice,*f,*outputs,dot(p,f),dot(v,outputs)])
    report("ces_expenditure_to_production_network",["Iceberg","Demand1","Demand2","Output1","Output2","FinalDemandValue","ValueAdded"],out)
