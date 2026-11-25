"""End-to-end synthetic portfolio investigations using compiled C++17 stages."""
import math
from comprehensive_study_common import near,check_vector,dot,solve,mv

def vintage_signal(model,report):
    # Final revisions arrive too late for historical decisions.
    output=[]
    for revision in (0,10,100):
        data=[]
        signals={}
        for t in range(1,9):
            for i in range(1,4):
                a=(i-2)*(1+.1*t)
                data.extend([[0,i,t,t,a],[0,i,t,t+10,a+revision*(i-2)],[1,i,t,t,0]])
                signals[t,i]=a
        vintage=model("bitemporal-vintage-join",data,[0])
        panel=[]
        for i,t,target,effective,available,value,valid in vintage:
            assert valid==1 and available<=t and effective==target
            near(value,signals[t,i])
            panel.append([t,i,.02*signals[t,i],value])
        rows=model("rolling-panel-ridge-longshort",panel,[3,.01,.001,1])
        wealth=1;old=[0.]*3
        for t in range(4,9):
            group=rows[(t-4)*3:(t-3)*3]
            train=[r for r in panel if t-3<=r[0]<t]
            b=sum(r[2]*r[3] for r in train)/(.01+sum(r[3]**2 for r in train))
            w=[r[3] for r in group];ret=[r[4] for r in group]
            check_vector(w,[-.5,0,.5])
            for r in group:near(r[-1],b)
            fee=group[0][9];near(fee,.001*sum(abs((1-fee)*a-z) for a,z in zip(w,old)))
            gross=dot(w,ret);wealth*=(1-fee)*(1+gross)
            near(group[0][10],wealth)
            old=[a*(1+b)/(1+gross) for a,b in zip(w,ret)]
            output.append([revision,t,b,gross,fee,wealth])
    report("vintage_signal_to_net_wealth",["RevisionShock","Date","RidgeCoefficient","GrossReturn","Fee","NAV"],output)

def actions_factor_risk(model,report):
    output=[]
    f=[-.04,.02,.01,-.02,.03,.01,-.01,.04]
    e=[.01,-.01,.01,-.01,-.01,.01,-.01,.01]
    for split in (.5,1,2,4):
        prices=[]
        for beta in (1,2,3):
            prices0=[[0,100,1,0]];price=100
            for t,(a,b) in enumerate(zip(f,e),1):
                ratio=split if t==4 else 1
                dividend=1 if t==6 else 0
                value=price*(1+beta*a+.2*beta*b)
                price=(value-dividend)/ratio
                prices0.append([t,price,ratio,dividend])
            ret=model("corporate-action-return-ledger",prices0,[])
            prices.append([r[4]-1 for r in ret[1:]])
        data=[[f[t],*[prices[i][t] for i in range(3)]] for t in range(len(f))]
        rows=model("factor-covariance-estimate",data,[1,0,0,.001])
        S=[r[4:] for r in rows]
        wrows=model("maximum-diversification-kkt",S,[1e-9,10000])
        w=[r[1] for r in wrows]
        near(sum(w),1)
        var=dot(w,mv(S,w));sig=[math.sqrt(S[i][i]) for i in range(3)]
        near(wrows[0][3],dot(w,sig)/math.sqrt(var))
        for i,r in enumerate(rows):
            y=prices[i];mf=sum(f)/len(f);my=sum(y)/len(y)
            beta=sum((a-mf)*(b-my) for a,b in zip(f,y))/sum((a-mf)**2 for a in f)
            near(r[3],beta)
            output.append([split,i+1,r[3],r[2],w[i],var,wrows[0][3]])
    report("corporate_actions_factor_diversification",["Split","Asset","Loading","SpecificVariance","Weight","PortfolioVariance","DiversificationRatio"],output)

def alpha_views_allocation(model,report):
    output=[]
    S=[[.04,.012],[.012,.09]]
    for t in (0,.0025,.01,.04):
        raw=[[.05,.01],[.15,.04]]
        eb=model("empirical-bayes-alpha-grid",raw,[t,t,2])
        prior=[r[5] for r in eb]
        for tau in (.1,.5,1):
            O=.02
            x=[[0,*S[i],prior[i]] for i in range(2)]+[[1,1,-1,.03],[2,O,0,0]]
            bl=model("black-litterman-correlated-views",x,[2,1,tau,3])
            cov=[r[6:] for r in bl]
            mu=[r[2] for r in bl]
            risk=[[S[i][j]+cov[i][j] for j in range(2)] for i in range(2)]
            # Independently recover conditional mean and unrestricted weights.
            z=[tau*(S[i][0]-S[i][1]) for i in range(2)]
            den=O+z[0]-z[1];diff=.03-(prior[0]-prior[1])
            check_vector(mu,[prior[i]+z[i]*diff/den for i in range(2)])
            check_vector([r[5] for r in bl],solve(risk,[m/3 for m in mu]))
            active=model("active-tracking-frontier",[[*risk[i],mu[i],.5] for i in range(2)],[.03])
            w=[r[2] for r in active]
            near(sum(w),0)
            near(dot(w,mv(risk,w)),.03**2)
            output.append([t,tau,*prior,*mu,*w,active[0][4],active[0][5]])
    report("hierarchical_alpha_views_tracking",["PriorVariance","Tau","Prior1","Prior2","Posterior1","Posterior2","Active1","Active2","ExpectedActiveReturn","TrackingError"],output)

def asymmetric_risk(model,report):
    output=[]
    data=[[-.16,.04],[.05,-.10],[.09,.03],[.025,.025],[-.015,.01],[.03,-.015]]
    for gamma in (1,5,20):
        sem=model("mean-semivariance-simplex",data,[0,gamma,1e-9,50000])
        for eta in (1,5,20):
            ent=model("entropic-risk-allocation",data,[eta,1e-9,50000])
            for mode,rows in ((0,sem),(1,ent)):
                w=[r[1] for r in rows];near(sum(w),1)
                def metrics(q):
                    R=[dot(r,[q,1-q]) for r in data]
                    mean=sum(R)/len(R);sv=sum(min(r,0)**2 for r in R)/len(R)
                    risk=math.log(sum(math.exp(-eta*r) for r in R)/len(R))/eta
                    return mean,sv,risk
                mean,sv,risk=metrics(w[0])
                obj=lambda q:gamma/2*metrics(q)[1]-metrics(q)[0] if mode==0 else metrics(q)[2]
                ref=min(obj(j/2000) for j in range(2001))
                assert obj(w[0])<=ref+1e-7
                near(rows[0][4] if mode==0 else rows[0][2],obj(w[0]))
                output.append([gamma,eta,mode,*w,mean,sv,risk,obj(w[0]),ref])
    report("asymmetric_scenario_risk_frontiers",["Gamma","Eta","Mode","Weight1","Weight2","Mean","Semivariance","EntropicRisk","Objective","GridUpperBound"],output)

def volatility_budget(model,report):
    output=[]
    data=[[1,.02],[1,-.04],[1,.01],[1,-.03],[0,.06],[0,-.05],[0,.015]]
    for alpha,beta in ((0,0),(.1,.7),(.25,.65)):
        g=model("garch-targeted-grid",data,[alpha,alpha,2,beta,beta,2])
        for t in range(4,len(data)):
            v=g[t][3];S=[[v,.1*v],[.1*v,2*v]]
            rows=model("active-tracking-frontier",[[*S[i],[.02,.005][i],.5] for i in range(2)],[.005])
            w=[r[2] for r in rows];near(w[0],.005/math.sqrt(2.8*v));near(w[1],-w[0])
            near(rows[0][5],.005)
            output.append([alpha,beta,t+1,v,*w,rows[0][4],rows[0][5]])
        changed=[r[:] for r in data];changed[4][1]*=10
        future=model("garch-targeted-grid",changed,[alpha,alpha,2,beta,beta,2])
        near(future[4][3],g[4][3])
        if alpha>0:assert future[5][3]>g[5][3]
    report("filtered_volatility_to_tracking_budget",["Alpha","Beta","Date","ForecastVariance","Active1","Active2","ExpectedActiveReturn","TrackingError"],output)

def factor_inverse_certificate(model,report):
    output=[]
    for scale in (.1,.5,1,2):
        f=[-.03,-.01,.01,.04,.02,-.02]
        data=[[z,scale*z,-.5*scale*z,2*scale*z] for z in f]
        factor=model("factor-covariance-estimate",data,[1,0,0,.001])
        D=[r[2] for r in factor]
        var=sum((z-sum(f)/len(f))**2 for z in f)/(len(f)-1)
        B=[r[3]*math.sqrt(var) for r in factor];S=[r[4:] for r in factor];alpha=[.02,.01,.04]
        wb=model("woodbury-risk-solve",[[D[i],B[i],alpha[i]] for i in range(3)],[])
        weights=[r[1] for r in wb]
        check_vector(weights,solve(S,alpha))
        pcg=model("pcg-jacobi",[[*S[i],alpha[i]] for i in range(3)],[1e-10,100,1])
        assert pcg[-1][-1]==1
        near(pcg[-1][3],-.5*dot(alpha,weights))
        for i,r in enumerate(wb):output.append([scale,i+1,r[1],r[2],r[3],r[4],pcg[-1][2]])
    report("factor_covariance_woodbury_pcg",["LoadingScale","Asset","Solution","LogDet","QuadraticForm","LinearResidual","PCGRelativeResidual"],output)

def studies(model,report):
    for workflow in (vintage_signal,actions_factor_risk,alpha_views_allocation,asymmetric_risk,volatility_budget,factor_inverse_certificate):
        workflow(model,report)
