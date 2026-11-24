"""Execute complete reference models and independent integrated experiments."""
import math
import random
import sys
import tempfile
from pathlib import Path
from model_study_common import CATALOG,INDEX,ROOT,near,csvfile,run,verify,dot,ridge,sigmoid,N,pdf,bs

binary=Path(sys.argv[1]).resolve()
out=Path(sys.argv[2])
cases=out/"cases"
studies=out/"studies"
cases.mkdir(parents=True,exist_ok=True)
studies.mkdir(parents=True,exist_ok=True)
assert len(CATALOG)==len(INDEX)==36

for c in CATALOG:
    _,raw=verify(binary,c)
    (cases/c["file"]).write_text(raw,encoding="utf-8")

with tempfile.TemporaryDirectory() as tmp:
    input_file=Path(tmp)/"input.csv"
    def model(name,rows,params,header=None):
        width=len(rows[0])
        names=header or INDEX[name]["columns"]
        if len(names)!=width:names=[f"V{i+1}" for i in range(width)]
        _,result,_=run(binary,name,csvfile(input_file,names,rows),params)
        return result
    def report(name,header,rows):
        assert rows and all(len(row)==len(header) for row in rows),name
        assert all(math.isfinite(v) for row in rows for v in row),name
        csvfile(studies/(name+".csv"),header,rows)

    # 1. Exact orthogonal design and correlated-design coordinate reference.
    rows=[]
    for correlated in [0,1]:
        def features(i):
            a=[1. if i&(1<<j) else -1. for j in range(4)]
            if correlated:a[1]=.65*a[0]+.35*a[1]
            return a
        truth=[.8,-.4,.15,0]
        X=[features(i) for i in range(32)]
        y=[dot(a,truth)+(.08 if i&16 else -.08) for i,a in enumerate(X)]
        test=[(features(i),dot(features(i),truth)+(.12 if i&16 else -.12)) for i in range(32,64)]
        for lam in [0,.1,.5,1,2,4,8,16]:
            for l2 in [.1,1,5]:
                fit=model("fista-elastic",[a+[b] for a,b in zip(X,y)],[lam,l2,1e-8,5000])
                beta=[r[1] for r in fit]
                if not correlated:
                    ref=[math.copysign(max(0,abs(sum(a[j]*b for a,b in zip(X,y)))-lam),sum(a[j]*b for a,b in zip(X,y)))/(32+l2) for j in range(4)]
                else:
                    ref=[0.]*4
                    for _ in range(10000):
                        old=ref[:]
                        for j in range(4):
                            z=sum(a[j]*(b-sum(a[k]*ref[k] for k in range(4) if k!=j)) for a,b in zip(X,y))
                            ref[j]=math.copysign(max(0,abs(z)-lam),z)/(sum(a[j]**2 for a in X)+l2)
                        if max(abs(a-b) for a,b in zip(ref,old))<1e-13:break
                for a,b in zip(beta,ref):near(a,b,atol=1e-7)
                mse=sum((b-dot(a,beta))**2 for a,b in test)/len(test)
                obj=.5*sum((b-dot(a,beta))**2 for a,b in zip(X,y))+lam*sum(abs(v) for v in beta)+.5*l2*dot(beta,beta)
                near(obj,fit[0][2])
                assert fit[0][3]<=1e-8
                for j,r in enumerate(fit):rows.append([correlated,lam,l2,j+1,r[1],ref[j],obj,r[3],r[4],mse])
    report("elastic_path",["Correlated","L1","L2","Feature","Coefficient","IndependentCoefficient","Objective","KKT","Iterations","HeldoutMSE"],rows)

    # 2. Total variation path with an actual noisy change-point signal.
    rows=[];last_tv=float("inf")
    truth=[0. if t<20 else 2.5 if t<40 else -.5 for t in range(60)]
    y=[v+.3*math.sin(1.7*t)+.15*math.cos(.37*t) for t,v in enumerate(truth)]
    for lam in [0,.1,.25,.5,1,2,4,8,16]:
        fit=model("fused-admm",[[v] for v in y],[lam,2,1e-7,20000])
        z=[r[2] for r in fit]
        tv=sum(abs(z[i]-z[i-1]) for i in range(1,len(z)))
        assert tv<=last_tv+1e-4;last_tv=tv
        near(sum(z),sum(y),atol=1e-5)
        obj=.5*sum((a-b)**2 for a,b in zip(z,y))+lam*tv
        near(obj,fit[0][4],atol=1e-6)
        assert obj<=.5*sum((v-sum(y)/len(y))**2 for v in y)+1e-5
        assert max(fit[0][5:8])<=1e-7
        if lam==0:
            for a,b in zip(z,y):near(a,b,atol=2e-7)
        for t,r in enumerate(fit):rows.append([lam,t+1,truth[t],y[t],r[2],r[3],tv,obj,r[5],r[6],r[7],r[8]])
    report("total_variation_path",["Penalty","Time","LatentLevel","Observed","Fitted","Difference","TotalVariation","Objective","PrimalResidual","DualResidual","KKT","Iterations"],rows)

    # 3. Full covariance allocation versus independent two-asset analytic QP.
    rows=[]
    for rho in [-.7,-.3,0,.3,.7,.9]:
        a,b,c=.01,.02*rho,.04
        rb=model("risk-budget-newton",[[a,b,.35],[b,c,.65]],[1e-9,100])
        near(sum(r[1] for r in rb),1)
        near(rb[0][3],.35);near(rb[1][3],.65)
        for gamma in [1,3,10,30]:
            for cap in [.6,.8,1]:
                fit=model("box-covariance-qp",[[a,b,.03,0,cap],[b,c,.05,0,cap]],[1,gamma,1e-9,5000])
                w=min(cap,max(1-cap,((.03-.05)/gamma+c-b)/(a+c-2*b)))
                near(fit[0][1],w,atol=1e-6);near(fit[1][1],1-w,atol=1e-6)
                risk=a*w*w+2*b*w*(1-w)+c*(1-w)**2
                near(fit[0][3],.5*gamma*risk-(.03*w+.05*(1-w)))
                rows.append([rho,gamma,cap,w,1-w,risk,fit[0][3],fit[0][5],rb[0][1],rb[1][1],rb[0][3],rb[1][3]])
    report("full_covariance_allocation",["Correlation","RiskAversion","Cap","Weight1","Weight2","Variance","Objective","ProjectedGradient","RiskBudgetWeight1","RiskBudgetWeight2","RiskShare1","RiskShare2"],rows)

    # 4. Generate arbitrage-consistent option quotes from a known tilted distribution.
    rows=[];states=[70,85,100,115,130];prior=[1,2,3,2,1];D=.97
    for theta1 in [-3,-1,0,1,3]:
        for theta2 in [-2,0,2]:
            raw=[p*math.exp(theta1*s/130+theta2*max(s-90,0)/130+.4*max(s-110,0)/130) for s,p in zip(states,prior)]
            prob=[v/sum(raw) for v in raw]
            F=dot(states,prob)
            c1=D*sum(p*max(s-90,0) for s,p in zip(states,prob))
            c2=D*sum(p*max(s-110,0) for s,p in zip(states,prob))
            fit=model("risk-neutral-entropy-fit",list(zip(states,prior)),[D,F,90,c1,110,c2,1e-11,200])
            near(sum(r[1] for r in fit),1)
            for j,r in enumerate(fit):
                near(r[1],prob[j]);near(r[2],D*prob[j]);near(r[3],F);near(r[6],c1);near(r[7],c2)
                rows.append([theta1,theta2,r[0],prob[j],r[1],r[2],F,c1,c2,r[4],r[5]])
    report("risk_neutral_calibration",["TiltUnderlying","TiltCall","State","TrueProbability","FittedProbability","StatePrice","Forward","Call90","Call110","KL","MomentResidual"],rows)

    # 5. Independent original likelihood and score checks on nontrivial samples.
    rows=[]
    sample=[[1,(i-8)/4,1+(i%3),[0,1,3,2,5,1,6,3,4,7,2,8,4,9,5,10][i]] for i in range(16)]
    for alpha in [.1,.5,1,2]:
        for lam in [.1,1]:
            a=model("nb2-exposure",sample,[alpha,lam,1e-8,200]);beta=[r[1] for r in a]
            g=[lam*v for v in beta];ll=.5*lam*dot(beta,beta);k=1/alpha
            for row in sample:
                z=row[:2];mu=row[2]*math.exp(dot(z,beta));y=row[3];p=alpha*mu/(1+alpha*mu)
                ll+=math.lgamma(k)+math.lgamma(y+1)-math.lgamma(k+y)-k*math.log1p(-p)-(y*math.log(p) if y else 0)
                for j in range(2):g[j]+=z[j]*(mu-y)/(1+alpha*mu)
            near(ll,a[0][2]);assert max(map(abs,g))<1e-7
            rows.append([1,alpha,lam,*beta,ll,max(map(abs,g)),a[0][-1]])
    for zeros in [6,8,12]:
        data=[[0]]*zeros+[[1],[2],[3],[4]]
        a=model("zip-em",data,[1e-8,20000])[0];rate,pi=a[:2]
        near(rate/(-math.expm1(-rate)),2.5)
        near(pi+(1-pi)*math.exp(-rate),zeros/(zeros+4))
        rows.append([2,zeros,0,rate,pi,-a[2],a[3],a[4]])
    survival=[[1,1,.9,-.4],[2,0,-.5,.6],[2,1,1.3,.3],[3,1,.2,-.6],[4,0,-.8,.1],[5,1,.7,.4],[5,1,-.1,1.2],[6,0,.5,-.2],[7,1,-.7,.8],[8,0,-.2,-.3]]
    for lam in [.1,1,10]:
        a=model("cox-breslow-ridge",survival,[lam,1e-8,200]);beta=[r[1] for r in a]
        g=[lam*v for v in beta];ll=.5*lam*dot(beta,beta)
        for t,event,*z in survival:
            if event:
                risk=[row[2:] for row in survival if row[0]>=t];p=[math.exp(dot(v,beta)) for v in risk];den=sum(p)
                ll+=math.log(den)-dot(z,beta)
                for j in range(2):g[j]+=sum(w*v[j] for w,v in zip(p,risk))/den-z[j]
        near(ll,a[0][3]);assert max(map(abs,g))<1e-7
        rows.append([3,lam,0,*beta,ll,max(map(abs,g)),a[0][-1]])
    censored=[[1,(i-5)/3,0,max(0,v),int(v<=0)] for i,v in enumerate([-.4,-.1,.4,-.2,.7,1.2,.2,1.6,.8,2,1.8,2.4])]
    for sigma in [.5,1,1.5]:
        a=model("tobit-known-scale",censored,[sigma,.1,1e-8,20000]);beta=[r[1] for r in a]
        g=[.1*v for v in beta];ll=.05*dot(beta,beta)
        for *z,c,y,flag in censored:
            mu=dot(z,beta)
            if flag:
                v=(c-mu)/sigma;ll-=math.log(N(v));der=pdf(v)/(sigma*N(v))
            else:
                v=(mu-y)/sigma;ll+=math.log(sigma)+.5*math.log(2*math.pi)+.5*v*v;der=v/sigma
            for j in range(2):g[j]+=der*z[j]
        near(ll,a[0][2]);assert max(map(abs,g))<1e-7
        rows.append([4,sigma,.1,*beta,ll,max(map(abs,g)),a[0][-1]])
    report("likelihood_score_checks",["ModelCode","Parameter1","Parameter2","Coefficient1","Coefficient2","NegativeLogLikelihood","IndependentScoreResidual","Iterations"],rows)

    # 6. Holdout nonlinear classification: no test-time tuning.
    rows=[];data=[]
    for a in [-1,1]:
        for b in [-1,1]:
            for u in [.75,1.25]:
                for v in [.75,1.25]:data.append([1,int(a*b>0),a*u,b*v])
    for a in [-1,1]:
        for b in [-1,1]:
            for u,v in [(.55,.85),(.95,1.05),(1.45,1.25)]:data.append([0,int(a*b>0),a*u,b*v])
    configurations=[(0,a,.1,[a,.1]) for a in [0,.5,1]]+[(1,g,l,[g,l,.05,1e-8,200]) for g in [.25,.5,1,2,4] for l in [.1,.5,2]]
    for kind,a,b,params in configurations:
        name="shrinkage-lda" if kind==0 else "kernel-logistic-fit"
        fit=model(name,data,params);test=[r for r in fit if r[1]==0]
        loss=-sum(y*math.log(max(p,1e-15))+(1-y)*math.log(max(1-p,1e-15)) for _,_,y,p,*_ in test)/len(test)
        brier=sum((r[2]-r[3])**2 for r in test)/len(test)
        extra=model(name,data+[[0,1,100,-100]],params)
        for r,s in zip(fit,extra):near(r[3],s[3])
        for r in test:rows.append([kind,a,b,r[0],r[2],r[3],loss,brier])
    report("nonlinear_classification_holdout",["ModelCode","Parameter1","Parameter2","Row","Label","Probability","HoldoutLogLoss","HoldoutBrier"],rows)

    # 7. Independent scalar filter and bivariate DCC grid likelihoods.
    rows=[]
    values=[.03*t+math.sin(.35*t)+.2*math.cos(1.7*t) for t in range(100)]
    nt=70;ql,qh,rl,rh,g=.01,.4,.01,.4,5
    def grid(a,b,k):return [math.exp(math.log(a)+(math.log(b)-math.log(a))*i/(k-1)) for i in range(k)]
    def kfilter(Q,R,n):
        mean=0;P=1;out=[];ll=0
        for y in values[:n]:
            P+=Q;S=P+R;v=y-mean;l=.5*(math.log(2*math.pi*S)+v*v/S);out.append((mean,S,l));ll+=l
            mean+=P/S*v;P=P*R/S
        return ll,out
    best=min((kfilter(Q,R,nt)[0],Q,R) for Q in grid(ql,qh,g) for R in grid(rl,rh,g))
    a=model("kalman-grid-fit",[[v] for v in values],[nt,ql,qh,rl,rh,g,0,1])
    near(a[0][11],best[0]);near(a[0][9],best[1]);near(a[0][10],best[2])
    ref=kfilter(best[1],best[2],len(values))[1]
    for i,r in enumerate(a):
        near(r[3],ref[i][0]);near(r[4],ref[i][1]);rows.append([1,r[0],r[1],r[2],0,r[3],ref[i][2],r[9],r[10]])
    z=[[math.sin(t*1.31)*1.3,math.cos(t*.73)*.9+(.6 if t>=40 else -.4)*math.sin(t*1.31)] for t in range(100)]
    def dcc(A,B,n):
        u=sum(r[0]**2 for r in z[:nt])/nt;v=sum(r[1]**2 for r in z[:nt])/nt;c=sum(r[0]*r[1] for r in z[:nt])/nt
        p,q,k=u,v,c;ll=0;result=[]
        for x,y in z[:n]:
            rho=k/math.sqrt(p*q);den=1-rho*rho;l=.5*(math.log(den)+(x*x-2*rho*x*y+y*y)/den);ll+=l;result.append((rho,l))
            p=(1-A-B)*u+A*x*x+B*p;q=(1-A-B)*v+A*y*y+B*q;k=(1-A-B)*c+A*x*y+B*k
        return ll,result
    gridab=[(a,b) for a in [0,.1,.2,.3,.4] for b in [0,.2,.4,.6,.8] if a+b<1]
    best=min((dcc(a,b,nt)[0],a,b) for a,b in gridab)
    a=model("dcc-twoasset-grid",z,[nt,.4,.8,5])
    near(a[0][8],best[0])
    ref=dcc(a[0][6],a[0][7],len(z))[1]
    for i,r in enumerate(a):near(r[4],ref[i][0]);near(r[5],ref[i][1]);rows.append([2,r[0],r[1],r[2],r[3],r[4],r[5],r[6],r[7]])
    report("dynamic_state_calibration",["ModelCode","Time","Train","Observed1","Observed2","Prediction","ConditionalNLL","Parameter1","Parameter2"],rows)

    # 8. Three frozen forecast systems with independent linear algebra.
    rows=[];rng=random.Random(613)
    Y=[[0.,0.]]
    for _ in range(149):
        a,b=Y[-1];Y.append([.65*a+.15*b+rng.gauss(0,.15),-.2*a+.55*b+rng.gauss(0,.13)])
    nt=110
    for lag in [1,2,3]:
        for lam in [.1,1,10]:
            a=model("var-ridge-forecast",Y,[nt,lag,lam])
            design=lambda t:[v for k in range(1,lag+1) for v in Y[t-k]]
            X=[design(t) for t in range(lag,nt)];coef=[ridge(X,[Y[t][j] for t in range(lag,nt)],lam) for j in range(2)]
            for r in a:
                t,j=int(r[0])-1,int(r[1])-1;pred=dot(design(t),coef[j]);near(r[3],pred)
                rows.append([1,lag,lam,t+1,j+1,r[2],r[3],Y[t-1][j],r[4]])
    rv=[.04]
    for t in range(1,150):rv.append(max(.002,.01+.65*rv[-1]+.004*math.sin(t*.8)+.003*math.cos(t*.09)))
    z=lambda t:[1,rv[t-1],sum(rv[t-5:t])/5,sum(rv[t-22:t])/22]
    for lam in [.001,.01,.1,1]:
        a=model("har-rv-forecast",[[v] for v in rv],[nt,lam])
        w=ridge([z(t) for t in range(22,nt)],rv[22:nt],lam,True)
        for r in a:
            t=int(r[0])-1;near(r[2],dot(w,z(t)))
            rows.append([2,22,lam,t+1,1,r[1],r[2],rv[t-1],r[3]])
    ou=[0.]
    for _ in range(149):ou.append(.2+.75*ou[-1]+rng.gauss(0,.15))
    a=model("ou-conditional-fit",[[v] for v in ou],[nt,1])
    w=ridge([[1,v] for v in ou[:nt-1]],ou[1:nt],0)
    for r in a:
        t=int(r[0])-1;near(r[2],dot(w,[1,ou[t-1]]));near(r[4],w[1])
        rows.append([3,1,0,t+1,1,r[1],r[2],ou[t-1],r[3]])
    report("frozen_forecast_comparison",["ModelCode","Lag","Penalty","Time","Series","Actual","Forecast","PersistenceBenchmark","Error"],rows)

    # 9. Grid convergence: European Black-Scholes reference and American obstacle.
    rows=[];K=100;r=.03;q=.01;sig=.2;T=1;Smax=400
    for M in [40,80,160,320]:
        data=[[Smax*i/M,sig] for i in range(M+1)]
        a=model("localvol-call-pde",data,[K,r,q,T,M])
        for s in [80,100,120]:
            idx=round(s/(Smax/M));v=a[idx];ref=bs(s,K,r,q,sig,T)
            near(v[0],s)
            if M==320:assert abs(v[2]-ref)<.35
            rows.append([1,M,M,s,v[2],ref,v[2]-ref,0,v[-1]])
    for M in [40,80,120]:
        # Exact spot-grid nodes at 80,100,120 are not required by the query interface.
        a=model("american-put-pde",[[80],[100],[120]],[K,r,q,sig,T,Smax,M,M,1e-7,3000])
        for v in a:
            s,am,eu,prem,gap,_=v;ref=bs(s,K,r,q,sig,T,False)
            assert am>=max(K-s,0)-1e-6 and am>=eu-1e-6 and gap<=1e-7
            rows.append([2,M,M,s,eu,ref,eu-ref,prem,gap])
    # A genuine state-dependent diffusion, not a Black-Scholes reference case.
    for slope in [-.15,0,.15]:
        M=160
        data=[[Smax*i/M,max(.05,.2+slope*(Smax*i/M/100-1))] for i in range(M+1)]
        a=model("localvol-call-pde",data,[K,r,q,T,M])
        for i in [32,40,48]:
            v=a[i];ref=bs(v[0],K,r,q,sig,T)
            assert v[2]>=-1e-8
            rows.append([3,M,M,v[0],v[2],ref,v[2]-ref,slope,v[-1]])
    report("pde_convergence",["ModelCode","SpaceSteps","TimeSteps","Spot","EuropeanPrice","BSReference","Difference","ExercisePremiumOrLocalVolSlope","SolverResidual"],rows)

    # 10. Independent-pilot control variate across monitoring grids and volatility.
    rows=[]
    for m in [1,4,12,52]:
        for vol in [.1,.2,.4]:
            for paths in [2048,8192]:
                a=model("asian-independent-control",[[100,100,.02,.01,vol,1]],[m,2048,paths,42])[0]
                mu=math.log(100)+(.01-.5*vol*vol)*(m+1)/(2*m)
                v=vol*vol*(m+1)*(2*m+1)/(6*m*m)
                d2=(mu-math.log(100))/math.sqrt(v);exact=math.exp(-.02)*(math.exp(mu+v/2)*N(d2+math.sqrt(v))-100*N(d2))
                near(a[5],exact)
                assert a[2]>=0 and a[3]>=0 and a[3]<=1.05*a[2]+1e-8
                if m==1:near(a[1],exact);assert a[3]<1e-8
                rows.append([m,vol,paths,*a[:6],a[3]/a[2] if a[2]>0 else 0])
    report("asian_variance_reduction",["MonitoringCount","Volatility","Paths","PlainPrice","ControlPrice","PlainSE","ControlSE","PilotBeta","GeometricAnalytic","SERatio"],rows)

    # 11. Generate CDS cashflows independently and recover piecewise hazards.
    rows=[];dt=.25;maturities=[1,2,3,5];hazards=[.02,.05,.1,.04]
    for rec in [.2,.4,.6]:
        for rate in [0,.02,.05]:
            quotes=[];s=1;ann=0;prot=0;prior=0
            for T,h in zip(maturities,hazards):
                end=int(T/dt)
                for j in range(prior+1,end+1):
                    nxt=s*math.exp(-h*dt);df=math.exp(-rate*j*dt);ann+=dt*df*nxt;prot+=(1-rec)*df*(s-nxt);s=nxt
                quotes.append([T,prot/ann]);prior=end
            a=model("cds-hazard-bootstrap",quotes,[rec,rate,dt,1e-10])
            for h,v in zip(hazards,a):near(v[1],h);assert abs(v[5])<=1e-10;rows.append([rec,rate,v[0],h,v[1],v[2],v[3],v[4],v[5],v[6],v[7]])
    report("cds_curve_recovery",["Recovery","Rate","Maturity","TrueHazard","FittedHazard","Survival","MarketSpread","ModelSpread","Residual","PremiumAnnuity","ProtectionPV"],rows)

    # 12. Curve profile grid, true factor recovery, and weighted noisy fit.
    rows=[];times=[.25,.5,1,2,3,5,7,10,20,30]
    def factors(t,tau):
        z=t/tau;a=-math.expm1(-z)/z
        return [1,a,a-math.exp(-z)]
    for tau in [.5,1,2,4]:
        for noise in [0,.0005,.001]:
            data=[[t,dot([.035,-.025,.018],factors(t,tau))+noise*math.sin(2.2*i),1+1/(1+t)] for i,t in enumerate(times)]
            a=model("nelson-siegel-calibration",data,[.5,4,4,0])
            beta=a[0][4:7];fit_tau=a[0][7];sse=sum(w*(y-dot(beta,factors(t,fit_tau)))**2 for t,y,w in data)
            near(sse,a[0][-1])
            if noise==0:near(fit_tau,tau);assert sse<1e-20
            for v in a:rows.append([tau,noise,v[0],v[1],v[2],v[3],*v[4:]])
    report("yield_curve_profile",["TrueTau","Noise","Maturity","Observed","Fitted","Residual","Level","Slope","Curvature","SelectedTau","WeightedSSE"],rows)

    # 13. Rolling signal and covariance portfolios with a proper fee/wealth ledger.
    rows=[];rng=random.Random(20261124);panel=[];ret=[]
    for t in range(90):
        r=[]
        for j in range(4):
            z=math.sin(.19*t+.8*j);w=math.cos(.31*t+.6*j)
            y=.004*z-.003*w+rng.gauss(0,.006);r.append(y);panel.append([t+1,j+1,y,z,w])
        ret.append(r)
    look=30
    for cost in [0,.0005,.001,.002]:
        for penalty in [0,.01,.05]:
            a=model("rolling-elastic-portfolio",panel,[look,.25,penalty,.1,cost,1e-9,2000])
            nav=1;old=[0.]*4
            for r in a:
                t=int(r[0])-1;w=r[10:14];gross=dot(w,ret[t]);near(r[1],gross);near(r[3],cost*r[4])
                near(r[4],sum(abs((1-r[3])*v-u) for v,u in zip(w,old)))
                near(r[2],(1-r[3])*(1+gross)-1);nav*=1+r[2];near(r[5],nav);near(sum(w),0)
                old=[v*(1+y)/(1+gross) for v,y in zip(w,ret[t])]
                rows.append([1,cost,penalty,t+1,gross,r[2],r[3],r[4],nav,sum(abs(v) for v in w),sum(w)])
        for gamma in [2,10,30]:
            a=model("rolling-covariance-qp",ret,[look,.5,.001,gamma,.6,cost,1e-8,2000])
            nav=1;old=[0.]*4
            for r in a:
                t=int(r[0])-1;w=r[9:13];gross=dot(w,ret[t]);near(r[1],gross);near(r[4],cost*r[5])
                near(r[5],sum(abs((1-r[4])*v-u) for v,u in zip(w,old)))
                near(r[2],(1-r[4])*(1+gross)-1);nav*=1+r[2];near(r[3],nav);near(sum(w),1);assert max(w)<=.6+1e-9
                old=[v*(1+y)/(1+gross) for v,y in zip(w,ret[t])]
                rows.append([2,cost,gamma,t+1,gross,r[2],r[4],r[5],nav,sum(abs(v) for v in w),sum(w)])
    report("rolling_portfolio_ledger",["ModelCode","Cost","PenaltyOrRiskAversion","Time","GrossReturn","NetReturn","Fee","Turnover","NAV","GrossExposure","NetExposure"],rows)

    # 14. Synthetic control and DML: nuisance fitting, leakage checks, distinct estimands.
    rows=[];rng=random.Random(716)
    donors=[[math.sin(t*.2)+.05*t,math.cos(t*.37)-.03*t,math.sin(t*.4)+.1*math.cos(t)] for t in range(60)]
    pre=40;true=[.2,.5,.3];sample=[[int(t>=pre),dot(v,true)+(0 if t<pre else .7),*v] for t,v in enumerate(donors)]
    for lam in [.00001,.001,.1,1]:
        a=model("synthetic-control-fit",sample,[lam,1e-8,5000]);w=a[0][7:]
        near(sum(w),1);assert all(v>=0 for v in w)
        perturbed=[r[:] for r in sample]
        for r in perturbed[pre:]:r[1]+=100
        b=model("synthetic-control-fit",perturbed,[lam,1e-8,5000])
        for v,u in zip(w,b[0][7:]):near(v,u)
        for r in a:rows.append([1,lam,r[0],r[1],r[2],r[3],r[4],.7 if r[1] else 0,r[6]])
    sample=[]
    for i in range(300):
        x=rng.gauss(0,1);z=rng.gauss(0,1);d=.8*x-.3*z+rng.gauss(0,.7);y=1.5*d+.6*x+.2*z+rng.gauss(0,.5)
        sample.append([i%5,d,y,x,z])
    for lam in [.01,.1,1,10]:
        a=model("crossfit-dml-ridge",sample,[lam])
        den=sum(r[6]**2 for r in a);theta=sum(r[6]*r[7] for r in a)/den;near(theta,a[0][9]);near(sum(r[8] for r in a),0,atol=1e-7)
        near(math.sqrt(sum(r[8]**2 for r in a))/den,a[0][10])
        for fold in range(5):
            train=[r for r in sample if r[0]!=fold];X=[[1,*r[3:]] for r in train]
            d=ridge(X,[r[1] for r in train],lam,True);y=ridge(X,[r[2] for r in train],lam,True)
            for r,s in zip(a,sample):
                if s[0]==fold:near(r[4],dot([1,*s[3:]],d));near(r[5],dot([1,*s[3:]],y))
        for r in a:rows.append([2,lam,r[0],r[1],r[2],r[4],r[8],theta,r[10]])
    report("causal_research_design",["ModelCode","Penalty","Row","PostOrFold","ObservedTreatmentOrOutcome","SyntheticOrNuisance","GapOrOrthogonalScore","EffectOrEstimate","SolverGapOrIIDSE"],rows)

    # 15. Rolling conformal during a known noise-regime change.
    rows=[];rng=random.Random(99)
    data=[]
    for t in range(180):
        x=math.sin(t*.17);y=.4+1.2*x+rng.gauss(0,.15 if t<100 else .5);data.append([x,y])
    for n,m in [(40,20),(60,20),(60,40)]:
        for alpha in [.1,.2,.3]:
            a=model("rolling-conformal-ridge",data,[n,m,.1,alpha])
            k=math.ceil((m+1)*(1-alpha))
            for r in a:
                t=int(r[0])-1
                X=[[1,v[0]] for v in data[t-m-n:t-m]];w=ridge(X,[v[1] for v in data[t-m-n:t-m]],.1,True)
                scores=sorted(abs(v[1]-dot([1,v[0]],w)) for v in data[t-m:t])
                near(r[2],dot([1,data[t][0]],w));near(r[3],scores[k-1]);near(r[5]-r[4],2*r[3])
                assert r[8]<r[9]<=r[10]<r[0]
                rows.append([n,m,alpha,t+1,int(t>=100),r[1],r[2],r[3],r[4],r[5],r[6]])
    report("rolling_conformal_regimes",["TrainingLength","CalibrationLength","Alpha","Time","HighNoiseRegime","Actual","Prediction","Threshold","Lower","Upper","Covered"],rows)

    # 16. Event-time likelihood, network smoothing and NNLS active set.
    rows=[]
    event=[.2,.25,.31,.42,1.7,1.8,1.91,2.9,4.5,4.55,4.66,4.8,7,7.1,7.2,8.9]
    T=10
    for decay in [.3,.5,1,2,4]:
        r=model("hawkes-fixed-decay-fit",[[v] for v in event],[T,decay,.01,10,.95,1e-8,10000])[0]
        mu,a=r[:2];h=[sum(decay*math.exp(-decay*(t-u)) for u in event[:i]) for i,t in enumerate(event)]
        C=sum(-math.expm1(-decay*(T-t)) for t in event)
        nll=mu*T+a*C-sum(math.log(mu+a*v) for v in h)
        near(nll,r[2]);near(mu*T+a*C,r[5])
        rows.append([1,decay,0,mu,a,nll,r[3]])
    graph=[[0,1,0,0,0],[1,0,2,0,0],[0,2,0,1,0],[0,0,1,0,1],[0,0,0,1,0]]
    y=[1,2,0,5,4];weight=[1,1,0,1,1]
    for lam in [.01,.1,1,5,20,100]:
        a=model("graph-smoother",[r+[v,w] for r,v,w in zip(graph,y,weight)],[lam]);z=[r[1] for r in a]
        near(sum(w*(v-u) for w,v,u in zip(weight,z,y)),0,atol=1e-7)
        for i,r in enumerate(a):
            g=weight[i]*(z[i]-y[i])+lam*sum(graph[i][j]*(z[i]-z[j]) for j in range(5))
            near(g,0,atol=1e-7);rows.append([2,lam,i+1,r[1],r[2],r[-1],abs(r[4])])
    X=[[1,.3,-.2],[.2,1,.1],[1,1,.5],[0,.2,1],[.6,.8,-.3]]
    y=[1,-1,.3,.2,-.2]
    for lam in [.01,.1,1,10]:
        a=model("nnls-coordinate",[r+[v] for r,v in zip(X,y)],[lam,1e-9,20000]);w=[r[1] for r in a]
        for j,r in enumerate(a):
            g=lam*w[j]+sum(v[j]*(dot(v,w)-b) for v,b in zip(X,y))
            near(g,r[2]);assert g>=-1e-8 and w[j]>=0;near(w[j]*g,0)
            rows.append([3,lam,j+1,w[j],g,r[3],r[4]])
    report("event_network_active_set",["ModelCode","Parameter","Index","Estimate1","Estimate2","Objective","Residual"],rows)

    # 17. Nested model selection versus an independent scalar penalized solution.
    rows=[]
    panel=[]
    for t in range(55):
        for j in range(4):
            z=math.sin(.41*t+.9*j)+.2*j
            y=(.014 if t<30 else -.007)*z+.008*math.cos(.73*t+1.3*j)
            panel.append([t+1,j+1,y,z])
    candidates=[(0,.1),(.01,.1),(.05,.1),(.01,1)]
    def coefficient(t,L,l1,l2):
        hist=panel[4*(t-L):4*t]
        xy=sum(r[2]*r[3] for r in hist)
        return math.copysign(max(abs(xy)-l1,0),xy)/(sum(r[3]**2 for r in hist)+l2)
    for L in [15,20]:
        V=5
        for cost in [0,.001]:
            p=[L,V,.25,cost,1e-9,2000,*[v for pair in candidates for v in pair]]
            a=model("nested-elastic-selection",panel,p)
            nav=1
            for t,r in enumerate(a,L+V):
                losses=[]
                for l1,l2 in candidates:
                    losses.append(sum((panel[4*u+j][2]-coefficient(u,L,l1,l2)*panel[4*u+j][3])**2 for u in range(t-V,t) for j in range(4))/(4*V))
                selected=min(range(len(losses)),key=lambda j:losses[j])
                near(r[9],losses[selected],atol=1e-10)
                chosen=int(r[6])-1
                near(losses[chosen],losses[selected],atol=1e-10)
                b=coefficient(t,L,*candidates[chosen])
                for j in range(4):near(r[17+j],b*panel[4*t+j][3],atol=1e-8)
                near(sum(r[13:17]),0);near(r[1],sum(r[13+j]*panel[4*t+j][2] for j in range(4)))
                near(r[4],cost*r[5]);nav*=1+r[2];near(r[3],nav)
                assert r[12]<r[0] and r[10]<=1e-9
                rows.append([L,V,cost,*r[:13],selected+1,losses[selected]])
    report("nested_walkforward",["Train","Validation","Cost","Date","Gross","Net","NAV","Fee","Turnover","Selected","L1","L2","InnerMSE","OuterKKT","ValidationStart","ValidationEnd","IndependentSelected","IndependentMSE"],rows)

    # 18. Exact two-asset CVaR: partition the feasible interval at loss crossings.
    rows=[]
    scenarios=[[-.08,.01],[.04,-.03],[-.025,-.02],[.03,.015],[-.06,.025],[.025,-.08],[.01,.02],[.015,-.01]]
    for alpha in [.5,.75,.9]:
        pc=min(1,1/((1-alpha)*len(scenarios)))
        lines=[(-r[1],r[1]-r[0]) for r in scenarios]
        def tail(w):
            rem=1;v=0
            for loss in sorted([a+b*w for a,b in lines],reverse=True):
                mass=min(rem,pc);v+=mass*loss;rem-=mass
            return v
        for lam in [.05,.1,.2]:
            for cap in [.6,.8,1]:
                lo,hi=1-cap,cap
                breaks=[lo,hi]
                for i,(a,b) in enumerate(lines):
                    for c,d in lines[i+1:]:
                        if b!=d:
                            w=(c-a)/(b-d)
                            if lo<w<hi:breaks.append(w)
                breaks=sorted(set(breaks));points=breaks[:]
                for l,h in zip(breaks,breaks[1:]):
                    m=(l+h)/2;rem=1;slope=0
                    for a,b in sorted(lines,key=lambda z:z[0]+z[1]*m,reverse=True):
                        mass=min(rem,pc);slope+=mass*b;rem-=mass
                    points.append(min(h,max(l,.5-slope/(2*lam))))
                obj=lambda w:tail(w)+.5*lam*(w*w+(1-w)**2)
                w=min(points,key=obj);exact=obj(w)
                a=model("cvar-primal-dual",scenarios,[alpha,lam,cap,1e-6,20000])[0]
                near(a[2],tail(a[1]));near(a[3],obj(a[1]))
                assert a[4]<=exact+1e-8 and exact<=a[3]+1e-8 and a[5]<=1e-6
                rows.append([alpha,lam,cap,w,a[1],a[2],exact,a[3],a[4],a[5],a[6]])
    report("cvar_exact_interval",["Alpha","Ridge","Cap","ExactWeight","CertifiedWeight","CVaR","ExactObjective","Primal","Dual","Gap","Iterations"],rows)

    # 19. Transaction-penalized allocation versus the exact piecewise quadratic.
    rows=[]
    for cap in [.6,.8,1]:
        lo,hi=1-cap,cap;old=.8
        for cost in [0,.01,.05,.1,.2,.5,1]:
            def obj(w):return .5*(w*w+.8*w*(1-w)+2*(1-w)**2)-(.1*w+.3*(1-w))+2*cost*abs(w-old)
            points=[lo,hi,min(hi,max(lo,old))]
            for sign in [-1,1]:
                l,h=(lo,min(hi,old)) if sign<0 else (max(lo,old),hi)
                if l<=h:points.append(min(h,max(l,(1.4-2*cost*sign)/2.2)))
            exact=min(points,key=obj)
            a=model("turnover-admm-qp",[[1,.4,.1,0,cap,old],[.4,2,.3,0,cap,1-old]],[1,1,cost,1,1e-8,5000])
            near(a[0][1],exact,atol=1e-6);near(a[0][4],obj(exact),atol=1e-7)
            assert max(a[0][5:9])<=1e-8
            rows.append([cap,cost,old,exact,a[0][1],a[1][1],obj(exact),a[0][4],*a[0][5:]])
    report("turnover_exact_path",["Cap","Cost","Previous","ExactWeight","Weight1","Weight2","ExactObjective","Objective","BudgetGap","PrimalResidual","DualResidual","KKT","Iterations"],rows)

    # 20. Multi-factor estimation and exact risk attribution on rolling windows.
    rows=[];data=[]
    for t in range(80):
        f=[.012*math.sin(.31*t),.009*math.cos(.27*t)]
        ret=[.0003*(j+1)+(1-.2*j)*f[0]+(.3*j-.4)*f[1]+.003*math.sin(.61*t+j) for j in range(4)]
        data.append(f+ret)
    floor=1e-5
    for L in [20,30]:
        for lam in [.001,.01,.1]:
            a=model("rolling-factor-risk-qp",data,[2,L,lam,floor,.6,.001,1e-8,2000])
            nav=1
            for t,r in enumerate(a,L):
                hist=data[t-L:t];X=[[1,*v[:2]] for v in hist]
                B=[];specific=[]
                for j in range(4):
                    y=[v[2+j] for v in hist];b=ridge(X,y,lam,intercept=True)
                    B.append(b[1:]);specific.append(floor+sum((v-dot(z,b))**2 for z,v in zip(X,y))/L)
                    for h in range(2):near(r[14+2*j+h],b[1+h])
                w=r[10:14];beta=[sum(w[j]*B[j][h] for j in range(4)) for h in range(2)]
                fmean=[sum(v[j] for v in hist)/L for j in range(2)]
                S=[[sum((v[j]-fmean[j])*(v[k]-fmean[k]) for v in hist)/(L-1)+(floor if j==k else 0) for k in range(2)] for j in range(2)]
                fv=sum(beta[j]*S[j][k]*beta[k] for j in range(2) for k in range(2))
                sv=sum(w[j]**2*specific[j] for j in range(4))
                near(r[6],fv);near(r[7],sv);near(r[8],fv+sv)
                near(sum(w),1);near(r[1],dot(w,data[t][2:]));near(r[4],.001*r[5])
                nav*=1+r[2];near(r[3],nav)
                rows.append([L,lam,*r[:14],fv,sv])
    report("factor_risk_attribution",["Lookback","Ridge","Time","Gross","Net","NAV","Fee","Turnover","FactorVariance","SpecificVariance","TotalVariance","ProjectedGradient","W1","W2","W3","W4","IndependentFactorVariance","IndependentSpecificVariance"],rows)

    # 21. Integer execution: exhaustive feasible schedules as the reference.
    from itertools import product
    rows=[]
    profiles=[([1,1,1,1],[4,4,4,4]),([1,3,2,1],[2,5,5,4]),([4,2,1,3],[6,3,2,4])]
    Q=12
    for idx,(liquidity,caps) in enumerate(profiles):
        for risk in [0,.1,.5,1,2]:
            data=[[l,1,risk,c] for l,c in zip(liquidity,caps)]
            def objective(q):
                rem=Q;v=0
                for z,l in zip(q,liquidity):rem-=z;v+=z*z/l+risk*rem*rem
                return v
            exact=min(objective(q) for q in product(*(range(c+1) for c in caps)) if sum(q)==Q)
            a=model("integer-execution-dp",data,[Q])
            near(a[-1][5],exact);near(a[-1][2],0)
            assert sum(r[1] for r in a)==Q
            for t,r in enumerate(a):
                assert 0<=r[1]<=caps[t] and r[1]==int(r[1]);near(r[-1],0)
                rows.append([idx+1,risk,liquidity[t],caps[t],*r,exact])
    report("integer_execution_exhaustive",["Profile","Risk","Liquidity","Capacity","Period","Trade","Remaining","TemporaryCost","RiskCost","CumulativeCost","OptimalCost","BellmanResidual","IndependentCost"],rows)

    # 22. Inventory control: recursively enumerate feasible quote actions.
    from functools import lru_cache
    rows=[];quotes=[.1,.25,.5,1.]
    for Q in [2,3]:
        for terminal in [0,.1,.5,1,2]:
            for scale in [0,.1,.5]:
                data=[[.65-.03*t,.45+.04*t,.7+.2*t,scale*(1+.1*t)] for t in range(5)]
                fee=.02
                @lru_cache(None)
                def val(t,q):
                    if t==len(data):return -terminal*q*q
                    buy,sell,decay,risk=data[t]
                    vals=[-risk*q*q+val(t+1,q)]
                    for side,base in [(1,buy),(-1,sell)]:
                        if abs(q+side)>Q:continue
                        for delta in quotes:
                            p=base*math.exp(-decay*delta)
                            vals.append(p*(delta-fee-risk*(q+side)**2+val(t+1,q+side))+(1-p)*(-risk*q*q+val(t+1,q)))
                    return max(vals)
                a=model("inventory-quote-dp",data,[Q,terminal,fee,*quotes])
                for r in a:
                    t,q,side=int(r[0])-1,int(r[1]),int(r[2])
                    assert abs(q+side)<=Q
                    near(r[5],val(t,q));near(r[-1],0)
                    rows.append([Q,terminal,scale,*r,val(t,q)])
    report("inventory_policy_exact",["Bound","TerminalRisk","RunningRisk","Period","Inventory","Action","Spread","Probability","Value","ExpectedNextInventory","BellmanResidual","IndependentValue"],rows)

print("PASS: 36 complete model reference cases and 22 integrated model studies")
