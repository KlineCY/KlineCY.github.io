"""Probability forecasts, classification and inference: independent references."""
import math
import itertools
from system_study_common import near,check_vector,bartlett,dot,solve,ridge,mm,mv,transpose,N,pdf

def studies(model,report):
    rows=[];data=[[.02*math.sin(.5*t),.01*math.cos(.3*t),.015*math.sin(.9*t)] for t in range(64)]
    for p in [[.2,.3,.5],[1/3]*3,[.8,.1,.1]]:
        fit=model("expert-mixture-logwealth",data,p);wealth=[1.]*3;total=1.
        for r,z in zip(fit,data):
            a=[p[j]*wealth[j]/total for j in range(3)];wealth=[u*(1+v) for u,v in zip(wealth,z)]
            nxt=dot(p,wealth);check_vector(r[1:],[nxt/total-1,nxt,max(wealth),math.log(max(wealth)/nxt),*a])
            for j in range(3):assert math.log(wealth[j]/nxt)<=-math.log(p[j])+1e-10
            total=nxt;rows.append([*p,*r])
    report("expert_wealth_regret_bound",["Prior1","Prior2","Prior3","Period","Return","Wealth","BestWealth","LogRegret","Weight1","Weight2","Weight3"],rows)
    rows=[]
    for s in [.5,1,2]:
        for z in [-4,-2,-1,0,.4,2,4]:
            m=.3;y=m+s*z;fit=model("gaussian-crps-score",[[y,m,s]],[])[0]
            # Numerical quadrature of CDF discrepancy, independent of closed form.
            def integral(a,b,step):
                n=10000;h=(b-a)/n
                return h*sum((N(a+(i+.5)*h)-step)**2 for i in range(n))
            ref=s*(integral(-12,z,0)+integral(z,12,1))
            near(fit[1],ref,atol=5e-7);near(fit[2],-math.log(pdf(z)/s));near(fit[3],N(z));near(fit[4],z)
            rows.append([s,z,*fit,ref])
    report("gaussian_crps_quadrature",["Scale","Z","Row","CRPS","NLL","PIT","Standardized","NumericalIntegral"],rows)
    rows=[];data=[[((i*17)%31)/30,int(i%5>1),int(i<24)] for i in range(40)]
    for fp,fn in [(1,1),(1,4),(4,1)]:
        val=[r for r in data if r[2]];cand=[2]+sorted({r[0] for r in val},reverse=True)
        def cost(t):return sum(fp*(1-y) if p>=t else fn*y for p,y,_ in val)
        t=min(cand,key=lambda t:(cost(t),-t))
        fit=model("cost-sensitive-threshold",data,[fp,fn])
        changed=model("cost-sensitive-threshold",[[p,y if v else 1-y,v] for p,y,v in data],[fp,fn])
        for r,z,s in zip(fit,data,changed):
            near(r[4],t);near(r[5],float(z[0]>=t));near(r[4],s[4]);rows.append([fp,fn,*r])
    report("threshold_validation_only",["FalsePositiveCost","FalseNegativeCost","Row","Validation","Probability","Label","Threshold","Decision","Cost"],rows)
    rows=[];data=[]
    for i in range(40):
        a=1+.7*math.sin(i);b=1+.6*math.cos(.7*i);c=1.5;total=a+b+c
        data.append([int(i<25),1+i%3,a/total,b/total,c/total])
    for alpha in [.02,.1,.25,.5]:
        s=sorted(1-r[int(r[1])+1] for r in data if r[0]);k=math.ceil((len(s)+1)*(1-alpha));q=1 if k>len(s) else s[k-1]
        fit=model("conformal-classification-set",data,[alpha])
        for r,z in zip(fit,data):
            inc=[float(1-p<=q+1e-14) for p in z[2:]]
            check_vector(r[3:],[q,sum(inc),inc[int(z[1])-1],*inc]);rows.append([alpha,*r])
    report("conformal_order_statistic",["Miscoverage","Row","Calibration","Label","Threshold","Size","Covered","Include1","Include2","Include3"],rows)
    rows=[];data=[[g,math.sin(.7*i+g),i%4] for g in range(1,4) for i in range(8)]
    for shift in [-10,0,10]:
        z=[[g,s+shift,y] for g,s,y in data];fit=model("pairwise-rank-logistic",z,[])
        for r in fit:
            a=[u for u in z if u[0]==r[0]]
            pairs=[(i,j) for i in range(len(a)) for j in range(len(a)) if a[i][2]>a[j][2]]
            def loss(v):return sum(math.log1p(math.exp(v[j]-v[i])) for i,j in pairs)/len(pairs)
            v=[u[1] for u in a];g=[];h=1e-5
            for i in range(len(v)):
                u=v[:];w=v[:];u[i]+=h;w[i]-=h;g.append((loss(u)-loss(w))/(2*h))
            c=sum(float(v[i]>v[j])+.5*float(v[i]==v[j]) for i,j in pairs)/len(pairs)
            check_vector(r[1:],[len(pairs),loss(v),c,sum(map(abs,g))/len(g),0],atol=1e-7);rows.append([shift,*r])
    report("ranking_finite_difference_gradient",["ScoreShift","Group","Pairs","Loss","Concordance","MeanAbsGradient","GradientSum"],rows)
    rows=[];data=[[int(i<30),i%2,math.sin(.4*i)+.8*(i%2)] for i in range(40)]
    for lam in [.01,.1,1]:
        a=[[r[2] for r in data if r[0] and r[1]==k] for k in [0,1]]
        mean=[sum(v)/len(v) for v in a];var=[sum((x-mean[k])**2 for x in v)/len(v)+lam for k,v in enumerate(a)]
        fit=model("weighted-gaussian-discriminant",data,[lam])
        for r,z in zip(fit,data):
            g=[math.log(len(a[k]))-.5*(math.log(var[k])+(z[2]-mean[k])**2/var[k]) for k in [0,1]]
            d=g[1]-g[0];check_vector(r[3:],[1/(1+math.exp(-d)),float(d>0),d]);rows.append([lam,*r])
    report("qda_direct_density_ratio",["Ridge","Row","Train","Label","Probability","Decision","LogOdds"],rows)
    rows=[]
    for n in [12,30,60]:
        X=[[1,(i-(n-1)/2)/n] for i in range(n)]
        y=[.5+.7*r[1]+(.1+.4*abs(r[1]))*math.sin(1.7*i) for i,r in enumerate(X)]
        fit=model("ols-hc3-inference",[[v,*r] for v,r in zip(y,X)],[])
        beta=ridge(X,y,0);H=mm(transpose(X),X)
        press=0;var=[0.,0.]
        for i,r in enumerate(X):
            b=ridge(X[:i]+X[i+1:],y[:i]+y[i+1:],0)
            e=y[i]-dot(r,b);press+=e*e;z=solve(H,r)
            for j in range(2):var[j]+=e*e*z[j]*z[j]
        for j,r in enumerate(fit):
            check_vector(r[1:],[beta[j],var[j],math.sqrt(var[j]),2/n,press]);rows.append([n,*r])
    report("hc3_leave_one_out_reference",["N","Feature","Coefficient","HC3Variance","HC3SE","MeanLeverage","PRESS"],rows)
    rows=[];data=[[.1,1,.2,.5],[.7,-.5,.3,.2],[1,0,.5,1.5],[.4,.8,.4,-.2]]
    tau=[.1,.5,.9]
    for w in [[1,1,1],[1,3,2],[3,1,4]]:
        fit=model("quantile-crossing-projection",data,[*tau,*w])
        for r,z in zip(fit,data):
            q=z[1:];candidates=[]
            for split in itertools.product([0,1],repeat=2):
                blocks=[];begin=0
                for j,flag in enumerate(split+(1,),1):
                    if flag:blocks.append((begin,j));begin=j
                v=[]
                for i,j in blocks:
                    m=sum(w[k]*q[k] for k in range(i,j))/sum(w[i:j]);v.extend([m]*(j-i))
                if all(a<=b for a,b in zip(v,v[1:])):candidates.append((sum(a*(b-c)**2 for a,b,c in zip(w,q,v)),v))
            f,v=min(candidates)
            check_vector(r[6:],v);near(r[4],f)
            pin=lambda v:sum((z[0]-a)*(t-float(z[0]<a)) for a,t in zip(v,tau))
            near(r[2],pin(q));near(r[3],pin(v));rows.append([*w,*r])
    report("quantile_projection_partition_reference",["Weight1","Weight2","Weight3","Row","Actual","RawLoss","ProjectedLoss","Distance","Crossings","Q1","Q2","Q3"],rows)
    rows=[]
    for n,k in [(6,2),(8,4),(10,3)]:
        y=[math.sin(.7*i)+.1*i for i in range(n)];d=[int(i<k) for i in range(n)]
        vals=[]
        for a in itertools.combinations(range(n),k):
            s=sum(y[j] for j in a);vals.append(s/k-(sum(y)-s)/(n-k))
        obs=vals[0];m=sum(vals)/len(vals);var=sum((v-m)**2 for v in vals)/len(vals)
        ext=sum(abs(v)+1e-12*max(1,abs(obs))>=abs(obs) for v in vals)
        r=model("permutation-randomization",list(map(list,zip(y,d))),[])[0]
        check_vector(r,[obs,len(vals),ext,ext/len(vals),m,var]);rows.append([n,k,*r])
    report("randomization_combinatorial_reference",["N","Treated","Observed","Assignments","Extreme","PValue","NullMean","NullVariance"],rows)
    rows=[];data=[[1,1],[1,0],[2,1],[3,0],[4,1],[5,0],[6,1]]
    for tau in [2.5,4,6]:
        fit=model("kaplan-meier-rmst",data,[tau]);S=1;g=0;area=0;last=0
        for r in fit:
            t=r[0];n=sum(z[0]>=t for z in data);d=sum(u==t and e==1 for u,e in data);c=sum(u==t and e==0 for u,e in data)
            area+=(t-last)*S
            if d:S*=1-d/n
            if d and n>d:g+=d/(n*(n-d))
            check_vector(r[1:],[n,d,c,S,S*S*g,area]);last=t;rows.append([tau,*r])
    report("kaplan_meier_risk_set_reference",["Horizon","Time","AtRisk","Events","Censored","Survival","GreenwoodVariance","RMST"],rows)
    rows=[];data=[[i/4,(.4+.8*i/4 if i<0 else 1.3-.5*i/4)+.05*math.sin(i*1.7)] for i in range(-16,17)]
    for h in [1.5,2.5,4.5]:
        for lam in [0,.05]:
            fit=model("bunching-rd-local",data,[0,h,lam]);ref=[]
            for side in [0,1]:
                z=[(x,y,1-abs(x)/h) for x,y in data if int(x>=0)==side and abs(x)<h]
                X=[[math.sqrt(w),math.sqrt(w)*x] for x,y,w in z];y=[math.sqrt(w)*y for x,y,w in z]
                b=ridge(X,y,lam,True);ss=sum((u-dot(v,b))**2 for u,v in zip(y,X));w=[r[2] for r in z]
                ref.append([*b,ss,sum(w)**2/dot(w,w)])
            jump=ref[1][0]-ref[0][0]
            for r,z in zip(fit,ref):check_vector(r[1:],[*z,jump]);rows.append([h,lam,*r])
    report("rd_weighted_regression_reference",["Bandwidth","Penalty","Side","Intercept","Slope","WeightedSSE","EffectiveSample","Jump"],rows)
    rows=[];x=[.5*math.sin(.2*i)+.2*math.cos(1.6*i) for i in range(80)]
    for L in [0,1,4,10,20]:
        m=sum(x)/len(x);v=bartlett(x,0);s=bartlett(x,L);r=model("newey-west-long-run",[[v] for v in x],[L])[0]
        check_vector(r,[m,len(x),s,math.sqrt(s/len(x)),len(x)*v/s,0]);rows.append([L,*r])
    report("long_run_variance_quadratic_form",["Bandwidth","Mean","N","LongRunVariance","MeanSE","EffectiveN","Degenerate"],rows)
