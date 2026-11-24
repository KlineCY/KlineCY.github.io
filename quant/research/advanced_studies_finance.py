"""Independent pricing, robust allocation, and execution experiments."""
import math
from functools import lru_cache
from advanced_study_common import near,dot,solve,mv,N,bs

def studies(model,report):
    # 13. Independent high-cutoff mixture and rigorous omitted-payoff bounds.
    rows=[]
    for rate in [0,.03]:
        for intensity in [.2,1,3]:
            for strike in [80,100,120]:
                S=100;q=.01;vol=.2;m=-.08;sv=.25;T=1
                a=intensity*T;ej=math.exp(m+.5*sv*sv)
                def price(n):
                    prob=math.exp(-a+n*math.log(a)-math.lgamma(n+1))
                    v=vol*vol*T+n*sv*sv
                    f=S*math.exp((rate-q-intensity*(ej-1))*T+n*(m+.5*sv*sv))
                    d1=(math.log(f/strike)+v/2)/math.sqrt(v);d2=d1-math.sqrt(v)
                    return prob*math.exp(-rate*T)*(f*N(d1)-strike*N(d2)),prob*math.exp(-rate*T)*(strike*N(-d2)-f*N(-d1))
                exact=[sum(price(n)[j] for n in range(100)) for j in range(2)]
                for cutoff in [2,5,12,40]:
                    fit=model("merton-jump-mixture",[[S,strike,rate,q,vol,intensity,m,sv,T]],[cutoff])[0]
                    for j in range(2):
                        near(fit[j],sum(price(n)[j] for n in range(cutoff+1)))
                        assert -1e-9<=exact[j]-fit[j]<=fit[3+j]+1e-8
                    assert abs(fit[5])<=fit[3]+fit[4]+1e-8
                    rows.append([rate,intensity,strike,cutoff,*fit,*exact])
    report("jump_mixture_error_bound",["Rate","Intensity","Strike","Cutoff","Call","Put","PoissonMass","CallTailBound","PutTailBound","ParityResidual","ReferenceCall","ReferencePut"],rows)

    # 14. Vasicek discount factor from Gaussian integrated short rate.
    rows=[]
    for k in [.2,.7,1.5]:
        for sig in [.01,.04,.08]:
            for expiry,maturity in [(1,2),(2,5),(3,10)]:
                r0=.03;theta=.045;strike=.9
                def B(t):return -math.expm1(-k*t)/k
                def disc(t):
                    mean=theta*t+(r0-theta)*B(t)
                    var=sig*sig/(k*k)*(t-2*B(t)-math.expm1(-2*k*t)/(2*k))
                    return math.exp(-mean+var/2)
                ps,pt=disc(expiry),disc(maturity)
                v=sig*sig*B(maturity-expiry)**2*(-math.expm1(-2*k*expiry))/(2*k)
                f=pt/ps;d1=(math.log(f/strike)+v/2)/math.sqrt(v);d2=d1-math.sqrt(v)
                c=ps*(f*N(d1)-strike*N(d2));p=ps*(strike*N(-d2)-f*N(-d1))
                fit=model("vasicek-bond-option",[[r0,k,theta,sig,expiry,maturity,strike]],[])[0]
                for a,b in zip(fit,[ps,pt,f,v,c,p,0]):near(a,b)
                rows.append([k,sig,expiry,maturity,*fit])
    report("vasicek_integrated_gaussian",["Kappa","Sigma","Expiry","Maturity","DiscountExpiry","DiscountMaturity","BondForward","LogForwardVariance","Call","Put","ParityResidual"],rows)

    # 15. Callable tree: recursive cash flows with explicit exercise dates.
    rows=[]
    for vol in [.0,.005,.02]:
        for call in [0,100,103,110]:
            data=[[1,.02,4,0],[2,.025,4,call],[3,.03,4,call],[4,.035,4,0]]
            @lru_cache(None)
            def value(t,j,callable):
                if t==len(data):return 100.
                df=math.exp(-(data[t][1]+vol*(2*j-t)))
                x=df*(data[t][2]+.5*(value(t+1,j,callable)+value(t+1,j+1,callable)))
                cap=data[t-1][3] if t else 0
                return min(x,cap) if callable and cap>0 else x
            fit=model("callable-rate-lattice",data,[100,vol,1])
            for r in fit:
                t,j=int(r[0]),int(r[1]);s,c=value(t,j,False),value(t,j,True)
                near(r[3],s);near(r[4],c);near(r[5],s-c);assert r[4]<=r[3]+1e-10
                rows.append([vol,call,*r])
    report("callable_tree_recursion",["RateSpacing","CallPrice","Period","Node","ShortRate","StraightValue","CallableValue","CallOptionValue","ExerciseFlag"],rows)

    # 16. Recover known asset values and volatilities from generated equity observables.
    rows=[]
    for V in [90,120,180]:
        for vol in [.15,.35,.65]:
            for T in [.5,2]:
                D=100;r=.02;z=vol*math.sqrt(T);d1=(math.log(V/D)+r*T+z*z/2)/z;d2=d1-z
                E=V*N(d1)-D*math.exp(-r*T)*N(d2);ev=V*N(d1)*vol/E
                fit=model("structural-credit-fit",[[E,ev,D,r,T]],[.02,2,1e-9])[0]
                near(fit[0],V);near(fit[1],vol);near(fit[4],N(-d2));near(fit[6],0);near(fit[7],0)
                rows.append([V,vol,T,E,ev,*fit])
    report("structural_credit_recovery",["TrueAssetValue","TrueAssetVol","Time","Equity","EquityVol","AssetValue","AssetVol","EquityModel","EquityVolModel","DefaultProbability","RiskNeutralDistance","PriceResidual","VolResidual"],rows)

    # 17. Kirk implied correlation from independently generated approximate quotes.
    rows=[]
    for strike in [0,10,30]:
        for rho in [-.7,-.3,0,.4,.8]:
            f,g=120,100;v,w=.25,.2;r=.03;T=1.5;b=g/(g+strike)
            eff=math.sqrt(v*v+b*b*w*w-2*b*rho*v*w);z=eff*math.sqrt(T)
            d1=math.log(f/(g+strike))/z+z/2;d2=d1-z;c=math.exp(-r*T)*(f*N(d1)-(g+strike)*N(d2))
            fit=model("kirk-spread",[[f,g,strike,v,w,r,T,c]],[1e-9])[0]
            near(fit[0],rho);near(fit[1],eff);near(fit[2],c);near(fit[4],0);near(fit[5],0)
            rows.append([strike,rho,c,*fit])
    report("kirk_correlation_recovery",["Strike","TrueCorrelation","Quote","ImpliedCorrelation","EffectiveVol","Call","Put","PriceResidual","ParityResidual"],rows)

    # 18. Constant-variance Black-Scholes benchmark, then stochastic-variance martingale checks.
    rows=[]
    for k,xi,rho in [(0,0,0),(1.5,.3,-.6),(1.5,.6,-.6),(1.5,.3,.5)]:
        for steps in [12,48,120]:
            S=100;K=105;r=.03;q=.01;v=.04;T=1;paths=8192
            fit=model("heston-full-truncation",[[S,K,r,q,v,k,v,xi,rho,T]],[steps,paths,157])[0]
            ref=bs(S,K,r,q,math.sqrt(v),T)
            assert abs(fit[6])<=6*fit[5]+1e-8
            near(fit[0]-fit[1],fit[4]-K*math.exp(-r*T))
            assert 0<=fit[7]<=1
            if xi==0:assert abs(fit[0]-ref)<=6*fit[2]+1e-8
            rows.append([k,xi,rho,steps,paths,ref,*fit])
    report("heston_martingale_grid",["Kappa","VolOfVol","Rho","Steps","Paths","ConstantVarianceBS","Call","Put","CallSE","PutSE","DiscountedSpot","SpotSE","MartingaleResidual","NegativeVarianceFraction"],rows)

    # 19. Ellipsoid objective versus independent scalar derivative.
    rows=[]
    for corr in [-.3,.2,.7]:
        s11=.03;s22=.08;s12=corr*math.sqrt(s11*s22);m1,m2=.02,.035
        for delta in [0,.1,.3]:
            for cap in [.65,1]:
                gamma=3;lo=1-cap;hi=cap
                def var(w):return s11*w*w+2*s12*w*(1-w)+s22*(1-w)**2
                def grad(w):
                    dv=2*(s11+s22-2*s12)*w+2*(s12-s22)
                    return .5*(gamma+delta/math.sqrt(var(w)))*dv-(m1-m2)
                l,h=lo,hi
                if grad(l)>=0:ref=l
                elif grad(h)<=0:ref=h
                else:
                    for _ in range(100):
                        m=(l+h)/2
                        if grad(m)<0:l=m
                        else:h=m
                    ref=(l+h)/2
                fit=model("ellipsoid-robust-qp",[[s11,s12,m1,0,cap],[s12,s22,m2,0,cap]],[1,gamma,delta,1e-9,10000])
                near(fit[0][1],ref,atol=1e-7);near(fit[1][1],1-ref,atol=1e-7)
                w=fit[0][1];m=m1*w+m2*(1-w);f=.5*gamma*var(w)-m+delta*math.sqrt(var(w))
                near(fit[0][3],m-delta*math.sqrt(var(w)));near(fit[0][5],f)
                rows.append([corr,delta,cap,ref,*fit[0][1:]])
    report("ellipsoid_robust_frontier",["Correlation","Uncertainty","Cap","ReferenceWeight","Weight","NominalMean","RobustMean","Variance","Objective","ProjectedGradient","Iterations"],rows)

    # 20. Factor neutrality and capacity: exact one-dimensional feasible family.
    rows=[]
    S=[[2,.3,.2],[.3,1.5,.1],[.2,.1,1]]
    direction=[1,1,-2];mu=[.5,.3,-.7];v=dot(direction,mv(S,direction))
    for gamma in [.5,1,3]:
        for cap in [.3,.6,1]:
            ref=min(cap/2,max(-cap/2,dot(mu,direction)/(gamma*v)))
            data=[r+[mu[j],-cap,cap,1,[1,-1,0][j]] for j,r in enumerate(S)]
            fit=model("affine-box-admm",data,[2,gamma,1,1e-8,10000,0,0])
            for j,r in enumerate(fit):
                near(r[1],ref*direction[j],atol=1e-7);assert max(r[3:7])<=1e-8
                rows.append([gamma,cap,j+1,ref*direction[j],*r[1:]])
    report("affine_neutrality_exact",["RiskAversion","Cap","Asset","ReferenceWeight","Weight","Objective","AffineResidual","PrimalResidual","DualResidual","KKTResidual","Iterations"],rows)

    # 21. Exact two-scenario minimax by quadratic stationary points and intersections.
    rows=[]
    for cap in [.65,.8,1]:
        for diag in [2.,3.,4.]:
            # Scenario 1 has variance w^2+diag(1-w)^2; scenario 2 reverses it.
            data=[[1,1,0],[1,0,diag],[2,diag,0],[2,0,1]]
            fit=model("scenario-minimax-variance",data,[cap,1e-8,1000])
            ref=(1+diag)/4
            for r in fit:near(r[1],.5);near(r[2],ref);near(r[3],ref);rows.append([cap,diag,*r,ref])
    # Asymmetric scenarios, independent dense interval reference (lower/upper certificate).
    for cap in [.75,1]:
        data=[[1,1,0],[1,0,3],[2,4,0],[2,0,1]]
        fit=model("scenario-minimax-variance",data,[cap,2e-3,1000])
        f=lambda w:max(w*w+3*(1-w)**2,4*w*w+(1-w)**2)
        ref=min(f(1-cap+(2*cap-1)*j/20000) for j in range(20001))
        assert fit[0][3]<=ref+1e-7 and fit[0][2]<=ref+2e-3 and fit[0][4]<=2e-3
        for r in fit:rows.append([cap,4,*r,ref])
    report("scenario_minimax_certificate",["Cap","ScenarioVariance","Asset","Weight","WorstVariance","DualLowerBound","Gap","Iterations","ReferenceObjective"],rows)

    # 22. Multiasset execution: independently solve a full inventory linear system.
    rows=[]
    H=[[1,.25],[.25,1.5]];S=[[.8,.2],[.2,.6]];q=[5,3]
    for T in [3,5,10]:
        for risk in [0,.1,1,4]:
            m=2*(T-1);M=[[0.]*m for _ in range(m)];b=[0.]*m
            for t in range(T-1):
                for i in range(2):
                    u=2*t+i
                    if t==0:b[u]=dot(H[i],q)
                    for j in range(2):
                        M[u][2*t+j]=2*H[i][j]+risk*S[i][j]
                        if t:M[u][2*(t-1)+j]=-H[i][j]
                        if t+1<T-1:M[u][2*(t+1)+j]=-H[i][j]
            z=solve(M,b);path=[q]+[z[2*t:2*t+2] for t in range(T-1)]+[[0,0]]
            fit=model("multiasset-execution-lq",[H[i]+S[i]+[q[i]] for i in range(2)],[T,risk])
            total=0
            for t in range(T):
                trade=[a-b for a,b in zip(path[t],path[t+1])]
                total+=dot(trade,mv(H,trade))+risk*dot(path[t+1],mv(S,path[t+1]))
                for j in range(2):
                    r=fit[2*t+j];near(r[2],trade[j]);near(r[3],path[t+1][j]);near(r[-1],0)
            for r in fit:near(r[6],total);rows.append([T,risk,*r])
    report("multiasset_execution_block_reference",["Periods","InventoryPenalty","Period","Asset","Trade","Remaining","TemporaryCost","InventoryRisk","TotalObjective","StationarityResidual"],rows)

    # 23. Log-growth optimum versus exact one-dimensional derivative.
    rows=[];data=[[.15,-.08,.2],[-.2,.04,.2],[.05,.03,.3],[.01,.02,.3]]
    for cap in [.55,.7,1]:
        for scale in [.5,1,2]:
            z=[[r[0]*scale,r[1]*scale,r[2]] for r in data]
            def grad(w):return sum(p*(a-b)/(1+a*w+b*(1-w)) for a,b,p in z)
            lo,hi=1-cap,cap
            if grad(lo)<=0:w=lo
            elif grad(hi)>=0:w=hi
            else:
                for _ in range(100):
                    m=(lo+hi)/2
                    if grad(m)>0:lo=m
                    else:hi=m
                w=(lo+hi)/2
            a=model("log-optimal-box",z,[cap,1e-11,10000])
            near(a[0][1],w,atol=1e-7)
            f=sum(p*math.log(1+x*w+y*(1-w)) for x,y,p in z)
            near(a[0][2],f)
            rows.append([cap,scale,w,*a[0][1:]])
    report("log_optimal_scenario_reference",["Cap","ReturnScale","ReferenceWeight","Weight","ExpectedLogGrowth","MinimumWealthFactor","ProjectedGradient","Iterations"],rows)

    # 24. Stochastic execution: independent recursive enumeration of every action.
    rows=[]
    for Q in [4,8,12]:
        for phi in [0,.5,5,30]:
            data=[[.65,4,1,.05],[.8,5,.6,.1],[.5,6,1.2,.2]]
            @lru_cache(None)
            def value(t,q):
                if t==len(data):return phi*q*q
                p,cap,eta,risk=data[t]
                return min(p*(eta*a*a+risk*(q-a)**2+value(t+1,q-a))+(1-p)*(risk*q*q+value(t+1,q)) for a in range(min(q,cap)+1))
            fit=model("stochastic-execution-dp",data,[Q,phi])
            for r in fit:
                t,q=int(r[0])-1,int(r[1]);near(r[5],value(t,q));near(r[4],r[2]*data[t][0]);assert 0<=r[2]<=min(q,data[t][1]);rows.append([Q,phi,*r,value(t,q)])
    report("stochastic_execution_enumeration",["InitialInventoryBound","TerminalPenalty","Period","Inventory","Order","FillProbability","ExpectedTrade","Value","BellmanResidual","ReferenceValue"],rows)
