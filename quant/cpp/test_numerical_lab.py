"""Independent analytical, numerical and structural tests for the numerical laboratories."""
import csv
import json
import math
import statistics as st
import tempfile
from collections import defaultdict
from pathlib import Path
from test_researchflow import run, load, write, solve, dot, near, compare

DATA=Path(__file__).resolve().parents[1]/"research"/"numerical-examples"
CATALOG=json.loads((DATA/"catalog.json").read_text(encoding="utf-8"))
CHECKS={"pde_european":[{"row":2,"field":"AnalyticPrice","value":8.827321225352122,"tol":1e-8},{"row":2,"field":"GridPrice","value":8.827321225352122,"tol":0.015}],"pde_barrier":[{"row":3,"field":"KnockOutCall","value":0},{"row":4,"field":"KnockOutCall","value":0}],"american_psor":[{"row":0,"field":"AmericanPut","value":40,"tol":1e-7}],"svi_slice":[{"row":0,"field":"a","value":0.02},{"row":0,"field":"b","value":0.1},{"row":0,"field":"rho","value":-0.3},{"row":4,"field":"Residual","value":0}],"calendar_grid":[{"row":2,"field":"VarianceChange","value":-0.001},{"row":2,"field":"Violation","value":1},{"row":3,"field":"Violation","value":0}],"nelson_siegel":[{"row":0,"field":"Beta0","value":0.04},{"row":0,"field":"Beta1","value":-0.025},{"row":0,"field":"Beta2","value":0.015},{"row":0,"field":"Tau","value":1}],"keyrate_curve":[{"row":0,"field":"BasePV","value":106.60819714629602},{"row":0,"field":"CentralDV01","value":0.0008485983934937151},{"row":1,"field":"CentralDV01","value":0.019743942459379146}],"bond_cashflow_risk":[{"row":0,"field":"PV","value":101.77944403833929},{"row":0,"field":"ShockedPV","value":97.88279297780511}],"cashflow_hedge":[{"row":0,"field":"Hedge1","value":0.875},{"row":0,"field":"Hedge2","value":1.375},{"row":2,"field":"ResidualExposure","value":0.75}],"inflation_indexed":[{"row":0,"field":"NominalCash","value":3.15},{"row":1,"field":"NominalCash","value":113.3},{"row":1,"field":"CumulativePV","value":109.78954747601165}],"box_markowitz":[{"row":0,"field":"Weight","value":0.1},{"row":1,"field":"Weight","value":0.3},{"row":2,"field":"Weight","value":0.6}],"turnover_admm":[{"row":0,"field":"Weight","value":0.4,"tol":1e-7},{"row":1,"field":"Weight","value":0.6,"tol":1e-7},{"row":0,"field":"TurnoverL1","value":0.2,"tol":1e-7}],"equality_quadratic":[{"row":0,"field":"Weight","value":0.23333333333333334},{"row":1,"field":"Weight","value":0.3333333333333333},{"row":2,"field":"Weight","value":0.43333333333333335},{"row":0,"field":"FactorExposure","value":0.2}],"robust_mean_ellipsoid":[{"row":0,"field":"Weight","value":1.2},{"row":1,"field":"Weight","value":1.6},{"row":0,"field":"WorstCaseExpectedReturn","value":8}],"exposure_frontier":[{"row":0,"field":"Variance","value":0.68},{"row":1,"field":"Variance","value":0.5},{"row":2,"field":"Weight2","value":0.8}],"multivariate_hc3":[],"multivariate_hac":[],"multivariate_cluster":[],"wild_coefficient":[{"row":0,"field":"Replications","value":1000}],"joint_wald_hc0":[{"row":0,"field":"Restrictions","value":2}],"damped_logistic":[{"row":0,"field":"Estimate","value":0,"tol":1e-8}],"ridge_press":[],"recursive_least_squares":[{"row":0,"field":"Coefficient0","value":0.5},{"row":3,"field":"PriorPrediction","value":1.5},{"row":3,"field":"Coefficient0","value":2}],"gaussian_discriminant":[{"row":0,"field":"Probability","value":0.5},{"row":1,"field":"LogOdds","value":3},{"row":1,"field":"Probability","value":0.9525741268224334}],"pca_jacobi":[{"row":0,"field":"RetainedVarianceShare","value":0.8},{"row":0,"field":"ReconstructionSquaredError","value":4},{"row":1,"field":"ReconstructionSquaredError","value":1}],"antithetic_gbm":[{"row":0,"field":"Pairs","value":6},{"row":0,"field":"AnalyticPrice","value":8.827321225352122,"tol":1e-8}],"geometric_control":[{"row":0,"field":"PilotPaths","value":3},{"row":0,"field":"EvaluationPaths","value":4}],"gbm_path_audit":[{"row":0,"field":"TerminalSpot","value":100},{"row":1,"field":"PathMaximum","value":115.1909910168909},{"row":2,"field":"IntegratedVariance","value":0.04}],"bridge_survival":[{"row":1,"field":"ConditionalSurvival","value":0},{"row":1,"field":"BridgePayoff","value":0},{"row":0,"field":"DiscretePayoff","value":20}],"heston_paths":[{"row":0,"field":"TerminalSpot","value":100},{"row":1,"field":"TerminalVariance","value":0.04},{"row":2,"field":"IntegratedVariance","value":0.04},{"row":2,"field":"NegativeRawVarianceSteps","value":0}],"fama_macbeth_multi":[{"row":0,"field":"MeanPremium","value":0.025},{"row":1,"field":"MeanPremium","value":0.0225},{"row":2,"field":"MeanPremium","value":-0.005}],"weighted_neutralization":[],"event_market_model":[{"row":2,"field":"CAR","value":0.03},{"row":5,"field":"CAR","value":-0.02},{"row":0,"field":"Beta","value":1.5}],"independent_double_sort":[{"row":0,"field":"MeanReturn","value":0.01},{"row":3,"field":"MeanReturn","value":0.05},{"row":7,"field":"MeanReturn","value":0.02}],"horizon_rank_ic":[{"row":0,"field":"RankIC","value":1},{"row":1,"field":"RankIC","value":-1},{"row":2,"field":"RankIC","value":0.5}],"best_quote_ofi":[{"row":0,"field":"OFI","value":7},{"row":1,"field":"OFI","value":7},{"row":2,"field":"OFI","value":11},{"row":3,"field":"CumulativeOFI","value":17}],"trade_sign_quote_tick":[{"row":0,"field":"InferredSign","value":0},{"row":1,"field":"InferredSign","value":1},{"row":2,"field":"InferredSign","value":-1},{"row":3,"field":"SignedQuantity","value":-4},{"row":4,"field":"InferredSign","value":1}],"preaveraging_variation":[{"row":0,"field":"Returns","value":12},{"row":0,"field":"Window","value":3}],"time_markout":[{"row":0,"field":"MatchedTrades","value":2},{"row":0,"field":"WeightedMarkoutBps","value":-32.67326732673268},{"row":1,"field":"WeightedMarkoutBps","value":-65.34653465346535}],"depth_liquidity_sweep":[{"row":1,"field":"VWAP","value":100.05},{"row":1,"field":"Fee","value":0.3},{"row":3,"field":"FilledQuantity","value":60},{"row":3,"field":"UnfilledQuantity","value":20}]}
# JSON numeric literals are also Python numeric literals; only numeric checks occur.

def close(a,b,tol=2e-8,label=""):
    assert abs(a-b)<=tol*max(1,abs(b)),(label,a,b,tol)


def phi(x):
    return .5*math.erfc(-x/math.sqrt(2))


def killed_call(S,K,r,q,v,T,B):
    if S>=B:return 0
    mu=r-q-v*v/2
    sd=v*math.sqrt(T)
    a,b=math.log(K),math.log(B)
    def interval(mean):
        prob=phi((b-mean)/sd)-phi((a-mean)/sd)
        asset=math.exp(mean+sd*sd/2)*(phi((b-mean-sd*sd)/sd)-phi((a-mean-sd*sd)/sd))
        return asset-K*prob
    x=math.log(S)
    return math.exp(-r*T)*(interval(x+mu*T)-math.exp(2*mu*(b-x)/(v*v))*interval(2*b-x+mu*T))


def crr_put(S,K,r,q,v,T,n=1200):
    dt=T/n;u=math.exp(v*math.sqrt(dt));d=1/u
    p=(math.exp((r-q)*dt)-d)/(u-d);disc=math.exp(-r*dt)
    values=[max(K-S*u**(2*j-n),0) for j in range(n+1)]
    for i in range(n-1,-1,-1):
        for j in range(i+1):
            values[j]=max(K-S*u**(2*j-i),disc*((1-p)*values[j]+p*values[j+1]))
    return values[0]


def linear(x,penalty=0,skip=0):
    X=[[1]+r[skip:-1] for r in x];y=[r[-1] for r in x];d=len(X[0])
    cols=list(zip(*X))
    A=[[dot(a,b)+(penalty if i==j and i>0 else 0)
        for j,b in enumerate(cols)] for i,a in enumerate(cols)]
    inv=[solve(A,[float(i==j) for i in range(d)]) for j in range(d)]
    beta=solve(A,[dot(c,y) for c in cols]);res=[b-dot(a,beta) for a,b in zip(X,y)]
    h=[dot(v,[dot(a,v) for a in inv]) for v in X]
    return X,y,beta,res,h,inv


def meat_cov(X,res,h,inv,kind,p,groups=None):
    d=len(inv)
    score=[[a*u for a in row] for row,u in zip(X,res)]
    n=len(X)
    if kind=="multivariate_hc3":
        score=[[v/(1-h[i]) for v in s] for i,s in enumerate(score)]
    elif kind=="multivariate_cluster":
        pool=defaultdict(lambda:[0.0]*d)
        for g,v in zip(groups,score):
            pool[g]=[a+b for a,b in zip(pool[g],v)]
        score=list(pool.values())
    M=[[sum(v[i]*v[j] for v in score) for j in range(d)] for i in range(d)]
    if kind=="multivariate_hac":
        lag=int(p[0])
        for i in range(d):
            for j in range(d):
                M[i][j]+=sum((1-k/(lag+1))*sum(score[t][i]*score[t-k][j]+score[t-k][i]*score[t][j] for t in range(k,n)) for k in range(1,lag+1))
    scale=1
    if kind=="multivariate_cluster":
        G=len(score);scale=G/(G-1)*(n-1)/(n-d)
    return [[scale*dot(a,[dot(v,b) for v in M]) for b in inv] for a in inv]


def structural(fn,x,p,names,rows,command,source):
    r=[dict(zip(names,v)) for v in rows]
    if fn=="pde_european":
        assert r[-1]["AbsError"]<r[0]["AbsError"] and r[-1]["AbsError"]<.015
        for v in r:close(v["AbsError"],abs(v["GridPrice"]-v["AnalyticPrice"]))
    elif fn=="pde_barrier":
        K,rate,q,v,T,B=p[:6]
        for z in r:
            close(z["KnockOutCall"],killed_call(z["Spot"],K,rate,q,v,T,B),.004)
            assert 0<=z["KnockOutCall"]<=z["VanillaCall"]+1e-9
    elif fn=="american_psor":
        K,rate,q,v,T=p[:5]
        for z in r:
            close(z["AmericanPut"],crr_put(z["Spot"],K,rate,q,v,T),.015)
            assert z["AmericanPut"]>=z["IntrinsicValue"]-1e-8
            assert z["MaxComplementarityResidual"]<=1e-7
    elif fn=="svi_slice":
        for z in r:
            close(z["ObservedVariance"],z["FittedVariance"])
            assert z["DensityFactor"]>0
    elif fn=="calendar_grid":
        assert sum(z["Violation"] for z in r)==1
    elif fn=="nelson_siegel":
        for z in r:
            close(z["FittedYield"],z["ObservedYield"])
            close(z["Discount"],math.exp(-z["Maturity"]*z["FittedYield"]))
    elif fn=="keyrate_curve":
        for z in r:
            assert z["UpPV"]<z["BasePV"]<z["DownPV"] and z["SecondDerivative"]>0
    elif fn=="bond_cashflow_risk":
        z=r[0];h=p[1]
        close(z["QuadraticPV"],z["PV"]*(1-z["SpreadDuration"]*h+.5*z["SpreadConvexity"]*h*h))
    elif fn=="cashflow_hedge":
        w=[r[0]["Hedge1"],r[0]["Hedge2"]]
        for j in range(2):close(sum(a[j]*z["ResidualExposure"] for a,z in zip(x,r)),p[0]*w[j])
    elif fn=="inflation_indexed":
        close(sum(z["DiscountedCash"] for z in r),r[-1]["CumulativePV"])
    elif fn=="box_markowitz":
        w=[z["Weight"] for z in r];close(sum(w),p[1])
        assert all(a[1]-1e-8<=v<=a[2]+1e-8 for v,a in zip(w,x))
        assert r[0]["GradientMappingNorm"]<1e-7
    elif fn=="turnover_admm":
        w=[z["Weight"] for z in r];close(sum(w),1)
        close(sum(abs(z["Trade"]) for z in r),r[0]["TurnoverL1"])
        assert r[0]["PrimalResidual"]<1e-7 and r[0]["DualResidual"]<1e-7
    elif fn=="equality_quadratic":
        w=[z["Weight"] for z in r]
        close(sum(w),p[0]);close(dot(w,[z[1] for z in x]),p[1])
        assert r[0]["StationarityResidual"]<1e-8
    elif fn=="robust_mean_ellipsoid":
        z=r[0];close(z["WorstCaseExpectedReturn"],z["NominalExpectedReturn"]-p[1]*z["Volatility"])
    elif fn=="exposure_frontier":
        for z in r:
            close(z["Weight1"]+z["Weight2"],1);close(z["ExpectedReturn"],z["TargetReturn"])
    elif fn in {"multivariate_hc3","multivariate_hac","multivariate_cluster","joint_wald_hc0","wild_coefficient"}:
        skip=int(fn=="multivariate_cluster");X,y,beta,res,h,inv=linear(x,skip=skip)
        if fn=="wild_coefficient":
            z=r[0];j=int(p[0]);close(z["Estimate"],beta[j]);close(z["MonteCarloP"],(z["Exceedances"]+1)/(p[2]+1))
            assert run(command,source,p)[1]==rows
        else:
            V=meat_cov(X,res,h,inv,fn,p,[a[0] for a in x] if skip else None)
            if fn=="joint_wald_hc0":
                ix=[int(j) for j in p[::2]];delta=[beta[j]-v for j,v in zip(ix,p[1::2])]
                cov=[[V[i][j] for j in ix] for i in ix]
                close(r[0]["WaldChiSquareStatistic"],dot(delta,solve(cov,delta)))
            else:
                for j,z in enumerate(r):
                    close(z["Estimate"],beta[j]);close(z["StandardError"],math.sqrt(V[j][j]))
    elif fn=="damped_logistic":
        w=[v["Estimate"] for v in r];g=[0.0]*len(w)
        for row in x:
            z=[1]+row[:-1];q=1/(1+math.exp(-dot(z,w)))
            g=[a+b*(q-row[-1])/len(x) for a,b in zip(g,z)]
        for j in range(1,len(w)):g[j]+=p[0]*w[j]
        assert math.sqrt(dot(g,g))<1e-7
    elif fn=="ridge_press":
        for i,z in enumerate(r):
            _,_,w,_,_,_=linear(x[:i]+x[i+1:],p[0])
            close(z["LeaveOneOutPrediction"],dot([1]+x[i][:-1],w))
    elif fn=="recursive_least_squares":
        for t,z in enumerate(r):close(z["Coefficient0"],sum(a[-1] for a in x[:t+1])/(t+2))
    elif fn=="gaussian_discriminant":
        for z,a in zip(r,x[int(p[0]):]):close(z["Probability"],1/(1+math.exp(-3*a[0])))
    elif fn=="pca_jacobi":
        for z,a in zip(r,x[int(p[0]):]):close(z["ReconstructionSquaredError"],a[1]**2)
    elif fn=="antithetic_gbm":
        S,K,rate,q,v,T=p
        payoff=lambda z:math.exp(-rate*T)*max(0,S*math.exp((rate-q-v*v/2)*T+v*math.sqrt(T)*z)-K)
        a=[(payoff(z[0])+payoff(-z[0]))/2 for z in x]
        close(r[0]["AntitheticPrice"],st.mean(a));close(r[0]["PairSE"],st.stdev(a)/math.sqrt(len(a)))
    elif fn=="geometric_control":
        S,K,rate,q,v,T,pilot=p;pilot=int(pilot);d=len(x[0]);dt=T/d
        Y=[];C=[]
        for row in x:
            path=[S*math.exp((rate-q-v*v/2)*dt*(j+1)+v*math.sqrt(dt)*sum(row[:j+1])) for j in range(d)]
            Y.append(math.exp(-rate*T)*max(0,st.mean(path)-K))
            C.append(math.exp(-rate*T)*max(0,math.prod(path)**(1/d)-K))
        beta=st.covariance(Y[:pilot],C[:pilot])/st.variance(C[:pilot])
        var=v*v*sum(min(i,j)*dt for i in range(1,d+1) for j in range(1,d+1))/d**2
        m=math.log(S)+(rate-q-v*v/2)*dt*(d+1)/2
        a=(m-math.log(K)+var)/math.sqrt(var)
        control=math.exp(-rate*T)*(math.exp(m+var/2)*phi(a)-K*phi(a-math.sqrt(var)))
        close(r[0]["ControlMean"],control);close(r[0]["ControlCoefficient"],beta)
        adjusted=[a-beta*(b-control) for a,b in zip(Y[pilot:],C[pilot:])]
        close(r[0]["AdjustedPrice"],st.mean(adjusted));close(r[0]["AdjustedSE"],st.stdev(adjusted)/math.sqrt(len(adjusted)))
    elif fn=="gbm_path_audit":
        for z in r:assert z["PathMaximum"]>=z["TerminalSpot"]-1e-8
    elif fn=="bridge_survival":
        initial,B,sigma,T,K=p;dt=T/len(x[0])
        for row,z in zip(x,r):
            path=[initial]+row;prob=0 if max(path)>=B else math.prod(1-math.exp(-2*math.log(B/a)*math.log(B/b)/(sigma*sigma*dt)) for a,b in zip(path,path[1:]))
            close(z["ConditionalSurvival"],prob);close(z["BridgePayoff"],prob*max(row[-1]-K,0))
    elif fn=="heston_paths":
        for z in r:close(z["TerminalSpot"],100);close(z["TerminalVariance"],.04)
    elif fn=="fama_macbeth_multi":
        for z in r:assert z["HACStandardError"]>0 and z["Dates"]==4
    elif fn=="weighted_neutralization":
        groups=defaultdict(list)
        for a,z in zip(x,r):groups[a[0]].append((a,z))
        for group in groups.values():
            for j in range(3):
                close(sum(a[1]*(1 if j==0 else a[j+1])*z["Residual"] for a,z in group),0)
    elif fn=="event_market_model":
        for z in r:close(z["ActualReturn"]-z["ExpectedReturn"],z["AbnormalReturn"])
    elif fn=="independent_double_sort":
        for z in r:assert z["Count"]==1
    elif fn=="horizon_rank_ic":
        for z in r:assert abs(z["RankIC"])<=1+1e-12
    elif fn=="best_quote_ofi":
        for z in r:close(z["OFI"],z["BidContribution"]+z["AskContribution"])
    elif fn=="trade_sign_quote_tick":
        for a,z in zip(x,r):close(z["SignedQuantity"],a[-1]*z["InferredSign"])
    elif fn=="preaveraging_variation":
        ret=[math.log(b[0]/a[0]) for a,b in zip(x,x[1:])];k=int(p[0])
        g=[min(j/k,1-j/k) for j in range(k+1)]
        p2=sum(z*z for z in g)/k;p1=k*sum((a-b)**2 for a,b in zip(g,g[1:]))
        pre=sum(sum(g[j]*ret[i+j-1] for j in range(1,k))**2 for i in range(len(ret)-k+1))/(k*p2)
        noise=p1*dot(ret,ret)/(2*k*k*p2)
        close(r[0]["IntegratedVarianceEstimate"],pre-noise)
    elif fn=="time_markout":
        for z in r:assert z["MatchedTrades"]+z["UnmatchedTrades"]==2
    elif fn=="depth_liquidity_sweep":
        for z in r:close(z["FilledQuantity"]+z["UnfilledQuantity"],z["RequestedQuantity"])
    else:raise AssertionError(fn)


def main():
    total=0;assertions=0
    with tempfile.TemporaryDirectory() as directory:
        tmp=Path(directory)/"sample.csv"
        for e in CATALOG:
            source=DATA/e["fixture"];header,x=load(source);p=e["params"];cmd=e["command"]
            names,rows=run(cmd,source,p);total+=len(rows)
            for v in CHECKS[e["fn"]]:
                close(rows[v["row"]][names.index(v["field"])],v["value"],v.get("tol",2e-8),e["fn"])
                assertions+=1
            structural(e["fn"],x,p,names,rows,cmd,source)
            run(cmd,source,p+["invalid-number"],False)
            run(cmd,source.with_suffix(".missing"),p,False)
            bad=[v[:] for v in x];bad[0][0]=float("nan");write(tmp,header,bad)
            run(cmd,tmp,p,False)
        # Domain-specific guards and off-diagonal PCA verification.
        write(tmp,["X","Y"],[[-2,-2],[2,2],[-1,-1],[1,1],[3,3],[3,-3]])
        names,rows=run("num-pca-jacobi",tmp,[4,1])
        close(rows[0][names.index("ReconstructionSquaredError")],0)
        close(rows[1][names.index("ReconstructionSquaredError")],18)
        run("num-pca-jacobi",tmp,[4,3],False)
        write(tmp,["A","B","C"],[[0,1,0],[1,0,-1]])
        run("num-exposure-frontier",tmp,[.5],False)
        write(tmp,["T","C","I","Z"],[[1,2,-1,.03]])
        run("num-inflation-indexed",tmp,[.25,100],False)
        write(tmp,["Breach"],[[0],[1]])
        run("num-bridge-survival",tmp,[100,130,.2,1,100],False)
        write(tmp,["Mean","Lower","Upper","CovA","CovB","CovC"],[[0,0,.1,1,0,0],[0,0,.1,0,1,0],[0,0,.1,0,0,1]])
        run("num-box-markowitz",tmp,[1,1,100],False)
        write(tmp,["Time","Bid","Ask","BidSize","AskSize"],[[0,101,100,1,1],[1,99,100,1,1]])
        run("num-best-quote-ofi",tmp,[],False)
    print(f"PASS: {len(CATALOG)} numerical laboratories, {total} output rows, {assertions} analytical field checks, 40 independent structural/reference cases and 126 malformed/domain checks")


if __name__=="__main__":
    main()
