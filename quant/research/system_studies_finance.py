"""Independent option-pricing, cashflow, execution and position-accounting studies."""
import math
import itertools
from system_study_common import near,check_vector,N,bs,dot

def studies(model,report):
    rows=[]
    for S,K,r,q,sigma,T in [(100,100,.03,.01,.2,1),(90,100,.02,0,.25,.5),(110,100,.01,.02,.3,2)]:
        z=sigma*math.sqrt(T);d1=(math.log(S/K)+(r-q)*T)/z+z/2;d2=d1-z
        c=S*math.exp(-q*T)*N(d1)-K*math.exp(-r*T)*N(d2);p=c-S*math.exp(-q*T)+K*math.exp(-r*T)
        for M in [100,200,400]:
            a=model("tridiagonal-cn-option",[[S,K,r,q,sigma,T]],[M,M,400,1])[0]
            b=model("tridiagonal-cn-option",[[S,K,r,q,sigma,T]],[M,M,400,0])[0]
            assert abs(a[0]-c)<.1 and abs(b[0]-p)<.1
            near(a[0]-b[0],S*math.exp(-q*T)-K*math.exp(-r*T),atol=2e-5)
            assert max(a[4],b[4])<1e-8
            rows.append([S,K,r,q,sigma,T,M,a[0],b[0],c,p,a[4],b[4]])
    report("crank_nicolson_bs_benchmark",["Spot","Strike","Rate","Dividend","Vol","Time","Grid","Call","Put","ExactCall","ExactPut","CallLinearResidual","PutLinearResidual"],rows)
    rows=[]
    for m in [1,4,12,52]:
        for sig in [.1,.3,.6]:
            S=100;K=105;r=.02;q=.01;T=1.5
            times=[T*j/m for j in range(1,m+1)]
            a=math.log(S)+(r-q-.5*sig*sig)*sum(times)/m
            v=sig*sig*sum(min(t,u) for t in times for u in times)/(m*m)
            f=math.exp(a+v/2);d2=(a-math.log(K))/math.sqrt(v);d1=d2+math.sqrt(v);df=math.exp(-r*T)
            c=df*(f*N(d1)-K*N(d2));p=df*(K*N(-d2)-f*N(-d1))
            z=model("asian-geometric-exact",[[S,K,r,q,sig,T]],[m])[0]
            check_vector(z,[c,p,a,v,df*f,0]);rows.append([m,sig,*z])
    report("geometric_asian_covariance_sum",["Monitoring","Volatility","Call","Put","LogMean","LogVariance","DiscountedMean","Parity"],rows)
    rows=[]
    for m in [1,4,12,24]:
        for sig in [.15,.4]:
            S=100;K=100;r=.03;q=.01;T=1
            exact=model("asian-geometric-exact",[[S,K,r,q,sig,T]],[m])[0][0]
            for seed in [13,97,431]:
                z=model("asian-arithmetic-control",[[S,K,r,q,sig,T]],[m,1500,8192,seed])[0]
                near(z[5],exact)
                assert z[2]>=0 and z[3]>=0 and z[4]>=0 and z[6]>=0
                assert abs(z[0]-z[1])<=6*(z[2]+z[3])+1e-8
                if m==1:near(z[1],exact,atol=1e-8);assert z[3]<1e-8
                else:assert z[3]<z[2] and z[1]>=exact-6*z[3]-1e-8
                rows.append([m,sig,seed,*z])
    report("asian_control_variance_experiment",["Monitoring","Volatility","Seed","Plain","Controlled","PlainSE","ControlledSE","PilotBeta","ExactGeometric","VarianceRatio"],rows)
    rows=[];cf=[[t,3 if t<5 else 103,math.exp(-.025*t-.001*t*t)] for t in range(1,6)]
    for z in [-.03,0,.015,.08]:
        def price(s):return sum(c*d*math.exp(-s*t) for t,c,d in cf)
        P=price(z);h=1e-4
        dur=(price(z-h)-price(z+h))/(2*h*P);conv=(price(z-h)+price(z+h)-2*P)/(h*h*P)
        a=model("cashflow-zspread-fit",cf,[P,-.2,.2,1e-10])[0]
        near(a[0],z);near(a[3],dur,atol=3e-7);near(a[4],conv,atol=2e-6);near(a[5],0);rows.append([z,*a])
    report("zspread_finite_difference",["KnownSpread","Spread","ModelPrice","MarketPrice","Duration","Convexity","Residual"],rows)
    rows=[];data=[[t,math.exp(-.02*t-.001*t*t),.5] for t in [.5,1,1.5,2,2.5,3]]
    for K in [-.01,.02,.05]:
        for h in [1e-3,1e-4,1e-5]:
            fit=model("swap-curve-keyrate",data,[100,K,h]);A=sum(d*a for t,d,a in data);pv=100*(1-data[-1][1]-K*A)
            g=[100*t*d*(K*a+int(j==len(data)-1)) for j,(t,d,a) in enumerate(data)]
            for j,r in enumerate(fit):
                near(r[2],pv);near(r[3],(1-data[-1][1])/A)
                near(r[4],g[j],atol=2e-6 if h<.001 else 5e-4);near(r[5],sum(g),atol=5e-4)
                near(r[6],0,atol=1e-8);rows.append([K,h,*r,g[j]])
    report("swap_node_parallel_risk",["FixedRate","Bump","Node","Time","PV","ParRate","KeyDerivative","ParallelDerivative","Residual","AnalyticDerivative"],rows)
    rows=[];data=[[.15,0],[.25,.04],[.3,.15],[.2,.38],[.1,.9]]
    for b in [[0,.03,.1,1],[0,.1,.3,.6,1],[0,.25,.5,.75,1]]:
        fit=model("credit-tranche-loss",data,b);el=sum(p*l for p,l in data)
        for r in fit:
            a,d=r[1:3];ref=sum(p*(max(l-a,0)-max(l-d,0)) for p,l in data)
            near(r[3],ref);near(r[4],ref/(d-a));near(r[5],el);near(r[6],0);rows.append(r)
        near(sum(r[3] for r in fit),el)
    report("credit_waterfall_conservation",["Tranche","Attachment","Detachment","ExpectedLoss","LossRate","PortfolioEL","Residual"],rows)
    rows=[];data=[[1,.3,.7,5],[2,-.1,1.2,4],[3,.8,.3,6]]
    for Q in [0,2,5,10,15]:
        fit=model("venue-convex-routing",data,[Q]);w=[r[1] for r in fit]
        near(sum(w),Q);f=sum(r[1]*q+r[2]*q*q for r,q in zip(data,w))
        # Feasible integer-grid upper bound; convex solution cannot cost more.
        grid=[]
        for a in range(51):
            for b in range(41):
                u=a/10;v=b/10;z=Q-u-v
                if -1e-10<=z<=6+1e-10:grid.append(sum(r[1]*q+r[2]*q*q for r,q in zip(data,[u,v,z])))
        assert f<=min(grid)+1e-8
        for r in fit:near(r[5],f);near(r[6],0);near(r[7],0);rows.append([Q,*r,min(grid)])
    report("routing_convex_grid_bound",["Quantity","Venue","Allocation","Cost","Marginal","Multiplier","Total","BudgetResidual","KKTResidual","GridUpperBound"],rows)
    rows=[]
    for fees in [0,.02,.2]:
        data=[[3,10,10,fees],[-1,12,11,fees],[-4,9,10,fees],[1,8,8,fees],[3,11,12,fees],[-2,13,13,fees]]
        fit=model("fifo-position-ledger",data,[100]);lots=[];cash=100;real=0;fee=0
        for r,z in zip(fit,data):
            q,p,m,f=z;cash-=q*p+f;fee+=f
            # Independent integer unit ledger.
            side=1 if q>0 else -1
            for _ in range(abs(int(q))):
                if lots and lots[0][0]!=side:
                    a,b=lots.pop(0);real+=a*(p-b)
                else:lots.append((side,p))
            pos=sum(a for a,b in lots);u=sum(a*(m-b) for a,b in lots)
            check_vector(r[1:],[pos,cash,real,u,fee,cash+pos*m,0]);rows.append([fees,*r])
    report("fifo_unit_lot_reference",["PerTradeFee","Event","Position","Cash","Realized","Unrealized","Fees","Equity","Residual"],rows)
    rows=[]
    for multiplier in [1,10,50]:
        data=[[100,2,0],[105,2,1],[93,-1,2],[90,-2,1],[110,0,3]]
        fit=model("futures-margin-ledger",data,[100,100,multiplier,.12,.08]);cash=100;old=0;prev=100;sum_vm=0;fund=0;fee=0
        for r,z in zip(fit,data):
            S,q,f=z;vm=multiplier*old*(S-prev);c=cash+vm-f
            im=.12*multiplier*abs(q)*S;mm=max(0,.08*multiplier*abs(old)*S-c);F=max(0,im-c);cash=c+F
            check_vector(r[1:],[old,S,vm,f,c,q,im,mm,F,cash]);sum_vm+=vm;fund+=F;fee+=f
            near(cash,100+sum_vm+fund-fee);rows.append([multiplier,*r]);old=q;prev=S
    report("futures_settlement_cash_conservation",["Multiplier","Day","OldContracts","Settlement","VM","Fee","PreFundingCash","Target","InitialMargin","MaintenanceShortfall","ExternalFunding","ClosingCash"],rows)
    rows=[]
    for initial in [0,10,-5]:
        data=[[3,2.1,.01,.002,.1],[-1,2.2,.02,.003,.2],[2,1.9,-.01,.001,.05],[0,2.4,.03,.002,0]]
        fit=model("fx-cash-ledger",data,[100,initial,2]);B=100;F=initial;E=B+2*F;prev=2
        for r,z in zip(fit,data):
            x,S,rf,rb,f=z;oldB,oldF=B,F;B=oldB*(1+rb)-x*S-f;F=oldF*(1+rf)+x;e=B+S*F
            check_vector(r[1:],[B,F,S,oldF*(S-prev),oldB*rb+S*oldF*rf,f,e,0]);near(e-E,r[4]+r[5]-f)
            prev=S;E=e;rows.append([initial,*r])
    report("fx_dual_currency_ledger",["InitialForeign","Period","BaseCash","ForeignCash","FX","FXPnL","Interest","Fee","Equity","Residual"],rows)
    rows=[];data=[[1,3,7.1,5],[2,4,5.5,4],[3,5,9.2,4]]
    for B in [0,8,12,20,30]:
        fit=model("integer-lot-allocation",data,[B]);candidates=[]
        for ks in itertools.product(*(range(r[3]+1) for r in data)):
            c=sum(k*r[1] for k,r in zip(ks,data))
            if c<=B:candidates.append((sum((k*r[1]-r[2])**2 for k,r in zip(ks,data)),c))
        best=min(candidates)[0]
        near(sum(r[4] for r in fit),best);assert sum(r[2] for r in fit)<=B
        for r in fit:near(r[7],best);rows.append([B,*r,best])
    report("integer_allocation_exhaustive",["Budget","Asset","Lots","Value","Target","SquaredLoss","TotalCost","Unused","TotalLoss","ExhaustiveLoss"],rows)
    rows=[];data=[[1,10,5],[1,9,4],[1,11,2],[-1,8,3],[-1,10,4],[-1,11,5]]
    for ref in [8,9.5,11]:
        fit=model("batch-auction-clearing",data,[ref]);keys=[]
        for r in fit:
            p=r[0];b=sum(z[2] for z in data if z[0]==1 and z[1]>=p);s=sum(z[2] for z in data if z[0]==-1 and z[1]<=p)
            check_vector(r[1:5],[b,s,min(b,s),b-s]);keys.append((min(b,s),-abs(b-s),-abs(p-ref),-p))
        j=max(range(len(keys)),key=lambda k:keys[k]);assert fit[j][5]==1 and sum(r[5] for r in fit)==1
        for r in fit:rows.append([ref,*r])
    report("auction_clearing_lexicographic",["Reference","Price","Demand","Supply","Volume","Imbalance","Chosen"],rows)
