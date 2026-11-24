"""Structural economics and point-in-time data engineering reference studies."""
import math
from system_study_common import near,check_vector,dot,solve,mv,transpose

def studies(model,report):
    rows=[];data=[[.6,.2],[1.2,.3],[2.1,.5]]
    for sig in [.2,.9,1,1.000000001,2,5]:
        for Y in [1,10]:
            fit=model("ces-cost-demand",data,[sig,Y]);z=1-sig
            c=math.exp(sum(a*math.log(p) for p,a in data)) if abs(z)<1e-7 else sum(a*p**z for p,a in data)**(1/z)
            for r,(p,a) in zip(fit,data):
                near(r[2],Y*a*(c/p)**sig,atol=1e-7);near(r[4],c);near(r[6],0);rows.append([sig,Y,*r])
            near(sum(r[3] for r in fit),1);near(sum(r[1]*r[2] for r in fit),Y*c)
    report("ces_dual_expenditure",["Elasticity","Output","Input","Price","Quantity","Share","UnitCost","TotalCost","Residual"],rows)
    rows=[];data=[[.3,1],[.8,2],[-.2,.6]]
    for a in [.1,1,3]:
        fit=model("logit-market-share",data,[a])
        u=[math.exp(v-a*p) for v,p in data];den=1+sum(u);h=1e-5
        for j,r in enumerate(fit):
            p=data[j][1];s=u[j]/den
            def share(delta):
                v=u[j]*math.exp(-a*delta);return v/(den-u[j]+v)
            e=p*(share(h)-share(-h))/(2*h*s)
            check_vector(r[1:],[s,1/den,e,math.log(den)/a,0]);rows.append([a,*r])
    report("logit_elasticity_finite_difference",["PriceCoefficient","Product","Share","Outside","OwnElasticity","Surplus","Residual"],rows)
    rows=[]
    for s,d,n,g,a in [(.2,.1,0,0,.5),(.3,.05,.01,.02,.3),(.15,.08,.02,.01,.4)]:
        for initial in [.5,2,10]:
            fit=model("solow-transition",[[s,d,n,g,a]],[initial,40]);star=(s/((1+n)*(1+g)-1+d))**(1/(1-a));k=initial
            for r in fit:
                check_vector(r[2:],[k,k**a,(1-s)*k**a,star,k-star])
                nxt=((1-d)*k+s*k**a)/((1+n)*(1+g))
                assert abs(nxt-star)<=abs(k-star)+1e-10;k=nxt
                rows.append([s,d,n,g,a,initial,*r])
    report("solow_fixed_point_transition",["Savings","Depreciation","PopulationGrowth","TechnologyGrowth","Alpha","InitialCapital","Scenario","Period","Capital","Output","Consumption","SteadyCapital","Gap"],rows)
    rows=[]
    for gamma in [.5,1,2,4]:
        for lower in [-1,0,3,6]:
            y,z,r,b=12,8,.05,.95
            a=model("two-period-consumption",[[y,z,r,b,gamma,lower]],[])[0]
            c,d,s,e,bind,u=a
            near(c+s,y);near(d,z+(1+r)*s);assert s>=lower and c>0 and d>0
            if bind:assert e>=-1e-10
            else:near(e,0)
            def utility(s):
                def U(c):return math.log(c) if gamma==1 else (c**(1-gamma)-1)/(1-gamma)
                return U(y-s)+b*U(z+(1+r)*s)
            ref=max(utility(lower+(y-lower)*i/10001) for i in range(10001))
            assert u>=ref-1e-8
            rows.append([gamma,lower,*a,ref])
    report("consumption_euler_and_grid",["RiskAversion","MinimumSaving","Consumption1","Consumption2","Saving","EulerResidual","Binds","Utility","GridUtility"],rows)
    rows=[];A=[[.1,.2,.1],[.2,.1,.1],[.05,.1,.2]];e=[1,2,.5]
    for f in [[10,0,0],[5,3,1],[1,1,10]]:
        M=[[float(i==j)-A[i][j] for j in range(3)] for i in range(3)]
        x=solve(M,f);h=solve(transpose(M),e);total=dot(e,x)
        fit=model("input-output-carbon",[A[i]+[e[i],f[i]] for i in range(3)],[])
        for j,r in enumerate(fit):
            check_vector(r[1:],[x[j],e[j]*x[j],h[j],h[j]*f[j],total,0]);rows.append([*f,*r])
    report("input_output_dual_footprint",["Demand1","Demand2","Demand3","Sector","Output","DirectFootprint","EmbodiedIntensity","DemandFootprint","Total","Residual"],rows)
    rows=[];H=[[2,-.4,.1],[-.4,1.5,-.2],[.1,-.2,1.2]]
    for target in [[1,2,3],[.5,1,1.5],[3,1,2]]:
        b=mv(H,target);fit=model("market-clearing-tatonnement",[r+[v] for r,v in zip(H,b)],[])
        for j,r in enumerate(fit):check_vector(r[1:],[target[j],0,0]);rows.append([*target,*r])
    report("linear_market_clearing_reference",["Target1","Target2","Target3","Market","Price","ExcessDemand","Residual"],rows)
    rows=[]
    for initial in [1,5,100]:
        data=[[100,1,0],[101,1,0],[50,2,.5],[48,1,2],[96,.5,0],[95,1,1]]
        fit=model("corporate-action-ledger",data,[initial]);q=initial;V=initial*data[0][0];prev=data[0][0]
        for r,(p,a,d) in zip(fit,data):
            q*=a;cash=q*d;v=q*p+cash;ret=a*(p+d)/prev-1
            check_vector(r[1:],[p,q,cash,v,ret,a*p/prev-1,q+cash/p])
            near(v,V*(1+ret));q+=cash/p;V=v;prev=p;rows.append([initial,*r])
    report("corporate_action_total_return",["InitialShares","Time","Close","Shares","DividendCash","Value","TotalReturn","PriceReturn","ReinvestedShares"],rows)
    rows=[]
    for w in [.1,.3,.7]:
        for r in [-1,-.5,.1]:
            data=[[1,(1-w)*.6,.08,0],[2,(1-w)*.4,-.02,0],[3,w,r,1]]
            fit=model("survivorship-cohort",data,[]);full=sum(z[1]*z[2] for z in data);surv=.6*.08-.4*.02
            for a,z in zip(fit,data):
                check_vector(a[3:],[z[1]*z[2],full,surv,1-w]);near(surv-full,w*(surv-r));rows.append([w,r,*a])
    report("cohort_delisting_decomposition",["DelistedWeight","DelistedReturn","Asset","Weight","Return","Contribution","FullReturn","SurvivorReturn","SurvivorWeight"],rows)
    rows=[];data=[]
    for a in [1,2,3]:
        data.extend([[1,a,t,100*a+t] for t in range(a,18,4)])
        data.extend([[0,a,t,0] for t in range(20)])
    for age in [0,1,4,10]:
        fit=model("lagged-asof-panel",data,[age]);queries=[r for r in data if r[0]==0]
        for r,z in zip(fit,queries):
            _,a,t,_=z;valid=[u for u in data if u[0]==1 and u[1]==a and u[2]<=t]
            if not valid:check_vector(r,[a,t,0,0,0,1])
            else:
                v=max(valid,key=lambda u:u[2]);d=t-v[2];check_vector(r,[a,t,v[2],d,v[3] if d<=age else 0,float(d>age)])
            rows.append([age,*r])
    report("asof_no_future_visibility",["MaxAge","Asset","Decision","Available","Age","Value","Missing"],rows)
    rows=[];data=[[t/10,10+math.sin(t),1+t%3] for t in range(30) if t%4!=0]
    for dt in [.25,.5,1]:
        fit=model("session-ohlcv-bars",data,[0,3,dt]);near(sum(r[6] for r in fit),sum(z[2] for z in data))
        for r in fit:
            a,b=r[:2];z=[u for u in data if a<=u[0]<b]
            ref=[z[0][1],max(u[1] for u in z),min(u[1] for u in z),z[-1][1],sum(u[2] for u in z),sum(u[1]*u[2] for u in z)/sum(u[2] for u in z),len(z),0] if z else [0,0,0,0,0,0,0,1]
            check_vector(r[2:],ref);rows.append([dt,*r])
    report("ohlcv_bucket_mass_conservation",["Width","BarStart","BarEnd","Open","High","Low","Close","Volume","VWAP","Count","Empty"],rows)
    rows=[]
    for exponent in [0,1,2]:
        data=[[g,math.sin(i+g),float((i+1)**exponent)] for g in [1,2,3] for i in range(8)]
        fit=model("cross-sectional-weighted-standardize",data,[])
        for g in [1,2,3]:
            a=[r for r in fit if r[1]==g];w=sum(r[3] for r in a)
            near(sum(r[3]*r[6] for r in a),0);near(sum(r[3]*r[6]**2 for r in a),w)
        for r in fit:rows.append([exponent,*r])
    report("weighted_cross_section_moments",["WeightExponent","Row","Group","Value","Weight","Mean","Std","Z","Degenerate"],rows)
    rows=[];data=[[i,i+i%4+1,i%3] for i in range(18)]
    for h in [0,1,3]:
        fit=model("label-interval-purge-folds",data,[h]);near(len(fit),3*len(data))
        for r in fit:
            f,i,role,over,emb,s,e=r;z=data[int(i)-1];test=[a for a in data if a[2]==f]
            o=z[2]!=f and any(z[0]<=a[1] and a[0]<=z[1] for a in test)
            m=z[2]!=f and not o and any(a[1]<z[0]<=a[1]+h for a in test)
            expected=2 if z[2]==f else 0 if o or m else 1
            check_vector(r[2:],[expected,float(o),float(m),z[0],z[1]])
            rows.append([h,*r])
    report("purged_fold_interval_reference",["EmbargoLength","TestFold","Row","Role","Overlap","Embargo","Start","End"],rows)
