"""Portfolio objective, covariance, posterior and trading-ledger verification."""
import math
from comprehensive_study_common import near,check_vector,dot,mv,mm,transpose,solve,inv

def studies(model,report):
    results=[]
    for s in (.5,1,2):
        f=[-2,-1,0,1,2];data=[[v,1+2*s*v,-1-s*v] for v in f]
        out=model("factor-covariance-estimate",data,[1,0,0,.01]);v=sum(z*z for z in f)/(len(f)-1)
        for i,r in enumerate(out):
            b=[2*s,-s];near(r[3],b[i]);near(r[2],.01)
            check_vector(r[4:],[b[i]*b[j]*v+.01*(i==j) for j in range(2)]);results.append([s,*r])
    report("factor_covariance_exact_loading",["Scale","Asset","Alpha","SpecificVariance","Loading","Covariance1","Covariance2"],results)

    results=[]
    for tau in (.1,.5,1):
        S=[[1,.2],[.2,2]];pi=[.1,.2];P=[[1,0],[0,1]];O=[[.3,.1],[.1,.4]];q=[.3,.1];delta=2
        data=[[0,*S[i],pi[i]] for i in range(2)]+[[1,*P[i],q[i]] for i in range(2)]+[[2,*O[i],0] for i in range(2)]
        C=inv([[tau*z for z in row] for row in S]);R=inv(O);prec=[[C[i][j]+R[i][j] for j in range(2)] for i in range(2)]
        V=inv(prec);rhs=[a+b for a,b in zip(mv(C,pi),mv(R,q))];m=mv(V,rhs)
        risk=[[S[i][j]+V[i][j] for j in range(2)] for i in range(2)];w=solve(risk,[z/delta for z in m])
        out=model("black-litterman-correlated-views",data,[2,2,tau,delta])
        for i,r in enumerate(out):near(r[2],m[i]);near(r[3],V[i][i]);near(r[5],w[i]);results.append([tau,*r])
    report("black_litterman_precision_identity",["Tau","Asset","PriorMean","PosteriorMean","MeanVariance","PredictiveVariance","Weight","MeanCov1","MeanCov2"],results)

    results=[]
    for te in (.01,.1,.3):
        S=[[1,.2],[.2,2]];a=[.5,-.1];bench=[.6,.4]
        w=te/math.sqrt(1+2-.4);out=model("active-tracking-frontier",[[*S[i],a[i],bench[i]] for i in range(2)],[te])
        check_vector([r[2] for r in out],[w,-w]);near(out[0][5],te);near(out[0][4],.6*w)
        results.extend([[te,*r] for r in out])
    report("tracking_frontier_closed_form",["TargetTE","Asset","Benchmark","ActiveWeight","TotalWeight","ActiveReturn","TrackingError","InformationRatio","BudgetResidual"],results)

    results=[]
    x=[[-.2,.1],[.1,-.2],[.2,.2],[-.1,.05]]
    for aversion in (1,5,20):
        obj=lambda w:aversion/2*sum(min(0,dot(r,[w,1-w]))**2 for r in x)/len(x)-sum(dot(r,[w,1-w]) for r in x)/len(x)
        out=model("mean-semivariance-simplex",x,[0,aversion,1e-9,20000]);w=out[0][1]
        near(out[0][4],obj(w));assert obj(w)<=min(obj(j/10000) for j in range(10001))+1e-7
        results.extend([[aversion,*r] for r in out])
    report("semivariance_grid_certificate",["Aversion","Asset","Weight","MeanReturn","DownsideSemivariance","Objective","ProjectedGradientResidual","BudgetResidual"],results)

    results=[]
    for rho in (-.5,0,.5,.9):
        S=[[1,2*rho],[2*rho,4]];out=model("maximum-diversification-kkt",S,[1e-10,10000])
        check_vector([r[1] for r in out],[2/3,1/3])
        near(out[0][3],math.sqrt(2/(1+rho)));results.extend([[rho,*r] for r in out])
    report("diversification_two_asset_closed_form",["Correlation","Asset","Weight","Volatility","Ratio","Variance","ScaledSolution","KKTResidual"],results)

    results=[]
    x=[[-.2,.1],[.1,-.2],[.2,.2],[-.1,.05]]
    for eta in (1,5,20):
        obj=lambda w:math.log(sum(math.exp(-eta*dot(r,[w,1-w])) for r in x)/len(x))/eta
        out=model("entropic-risk-allocation",x,[eta,1e-9,20000]);w=out[0][1]
        near(out[0][2],obj(w));assert obj(w)<=min(obj(j/10000) for j in range(10001))+1e-7
        results.extend([[eta,*r] for r in out])
    report("entropic_grid_certificate",["RiskAversion","Asset","Weight","EntropicRisk","MeanReturn","WorstTiltMass","ProjectedGradientResidual","BudgetResidual"],results)

    results=[]
    data=[[-1,.2],[.3,.4],[1.5,.1]]
    for t in (0,.2,1,3):
        out=model("empirical-bayes-alpha-grid",data,[t,t,2])
        w=[1/(r[1]+t) for r in data];m=sum(a[0]*b for a,b in zip(data,w))/sum(w)
        for a,r in zip(data,out):
            near(r[3],m);near(r[5],(t*a[0]+a[1]*m)/(a[1]+t));near(r[6],t*a[1]/(a[1]+t))
            results.append([t,*r])
    report("empirical_bayes_heterogeneous_variance",["Tau","Asset","Alpha","SamplingVariance","PriorMean","PriorVariance","PosteriorMean","PosteriorVariance","Shrinkage","NLL"],results)

    results=[]
    for fee in (0,.001,.01):
        data=[]
        for t in range(1,7):
            for j,s in enumerate((-1.,0.,1.),1):data.append([t,j,.03*s,s])
        out=model("rolling-panel-ridge-longshort",data,[2,0,fee,1]);nav=1;prev=[0,0,0]
        for t in range(4):
            rows=out[3*t:3*t+3];w=[r[3] for r in rows];ret=[r[4] for r in rows]
            check_vector(w,[-.5,0,.5]);near(sum(w),0);near(sum(map(abs,w)),1)
            c=rows[0][9];near(c,fee*sum(abs((1-c)*a-b) for a,b in zip(w,prev)))
            g=dot(w,ret);net=(1-c)*(1+g)-1;nav*=1+net
            near(rows[0][7],net);near(rows[0][10],nav);prev=[a*(1+b)/(1+g) for a,b in zip(w,ret)]
            results.extend([[fee,*r] for r in rows])
        changed=[r[:] for r in data]
        for r in changed[-3:]:r[2]+=.3
        more=model("rolling-panel-ridge-longshort",changed,[2,0,fee,1])
        check_vector([r[3] for r in out],[r[3] for r in more])
    report("rolling_panel_cash_and_time_isolation",["FeeRate","Date","Asset","Score","Weight","RealizedReturn","Contribution","GrossReturn","NetReturn","Turnover","Fee","NAV","Coefficient"],results)
