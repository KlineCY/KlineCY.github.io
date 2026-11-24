"""Independent nondegenerate studies of econometric estimation and online learning."""
import math
import random
from itertools import product
from advanced_study_common import near,dot,solve,ridge,inv,mv,mm,transpose,grid,N,pdf

def studies(model,report):
    # 1. Radial invariance and original Tyler map.
    data=[[1,.2],[2,1],[-1,2],[-3,-.4],[.5,-1],[1,3],[-2,1],[4,-1]]
    rows=[]
    for delta in [.15,.35,.7,1]:
        a=model("tyler-shape",data,[delta,1e-10,5000])
        scaled=[[v*(i+1)*(-1 if i%2 else 1) for v in r] for i,r in enumerate(data)]
        b=model("tyler-shape",scaled,[delta,1e-10,5000])
        S=[[a[2*i+j][2] for j in range(2)] for i in range(2)]
        W=inv(S);z=[[float(i==j)*delta for j in range(2)] for i in range(2)]
        for r in data:
            v=dot(r,mv(W,r))
            for i in range(2):
                for j in range(2):z[i][j]+=(1-delta)*2*r[i]*r[j]/(len(data)*v)
        tr=z[0][0]+z[1][1]
        for i,r in enumerate(a):
            near(r[2],b[i][2]);near(r[2],2*z[i//2][i%2]/tr)
            rows.append([delta,*r,b[i][2]])
    report("tyler_directional_invariance",["Shrinkage","Row","Column","Shape","Trace","FixedPointResidual","Iterations","RescaledShape"],rows)

    # 2. Scalar overidentified GMM with independent full matrix sandwich.
    rows=[]
    data=[]
    for i in range(36):
        z1=math.sin(.7*i);z2=math.cos(.23*i);x=.8*z1+.4*z2+.25*math.sin(1.7*i)
        y=1.3*x+.2*math.cos(1.3*i)+.12*z2
        data.append([y,x,z1,z2])
    n=len(data);A=[sum(r[j]*r[1] for r in data)/n for j in [2,3]];b=[sum(r[j]*r[0] for r in data)/n for j in [2,3]]
    beta1=dot(A,b)/dot(A,A)
    def S(beta):return [[sum((r[0]-beta*r[1])**2*r[i+2]*r[j+2] for r in data)/n for j in range(2)] for i in range(2)]
    for penalty in [.001,.01,.1,1]:
        S1=S(beta1)
        for j in range(2):S1[j][j]+=penalty
        W=inv(S1);wa=mv(W,A);den=dot(A,wa);ref=dot(wa,b)/den
        g=[b[j]-A[j]*ref for j in range(2)];J=n*dot(g,mv(W,g))
        var=dot(wa,mv(S(ref),wa))/(n*den*den)
        a=model("linear-gmm-two-step",data,[1,penalty])[0]
        near(a[1],ref);near(a[2],math.sqrt(var));near(a[3],J);near(a[5],math.hypot(*g));near(a[6],0)
        rows.append([penalty,beta1,ref,*a[1:]])
    report("gmm_weight_and_sandwich",["Ridge","FirstStep","ReferenceCoefficient","Coefficient","RobustSE","JStatistic","DF","MomentNorm","ScoreResidual"],rows)

    # 3. GLS profile, candidate likelihood and independently transformed design.
    rows=[];data=[];err=0
    for i in range(48):
        x=math.sin(.4*i);err=.65*err+.17*math.cos(1.7*i)+.09*math.sin(.13*i)
        data.append([1.2+.7*x+err,1,x])
    for low,high,g in [(-.8,.8,17),(-.2,.9,23),(.1,.7,13)]:
        candidates=[]
        for rho in grid(low,high,g):
            a=math.sqrt(1-rho*rho);X=[[a*v for v in data[0][1:]]];y=[a*data[0][0]]
            for i in range(1,len(data)):
                X.append([data[i][j]-rho*data[i-1][j] for j in [1,2]])
                y.append(data[i][0]-rho*data[i-1][0])
            w=ridge(X,y,0);v=sum((u-dot(z,w))**2 for z,u in zip(X,y))/len(y)
            f=.5*len(y)*(1+math.log(2*math.pi*v))-.5*math.log(1-rho*rho)
            candidates.append((f,rho,v,w))
        f,rho,v,w=min(candidates)
        a=model("ar1-gls-profile",data,[low,high,g])
        for j,r in enumerate(a):near(r[1],w[j]);near(r[2],rho);near(r[3],v);near(r[4],f);rows.append([low,high,g,*r])
    report("gls_profile_likelihood",["GridLow","GridHigh","GridCount","Feature","Coefficient","Rho","InnovationVariance","ProfileNLL","ScoreResidual"],rows)

    # 4. Unequal group sizes: full covariance inverse reference for REML.
    rows=[];data=[]
    for g,n in enumerate([3,4,5,7,8],1):
        for t in range(n):data.append([g,2+.6*(g-3)+.3*math.sin(1.1*t+g),1])
    y=[r[1] for r in data];n=len(y);ones=[1.]*n
    for hi in [1,4,10]:
        refs=[]
        for ratio in grid(0,hi,21):
            C=[[float(i==j)+ratio*float(data[i][0]==data[j][0]) for j in range(n)] for i in range(n)]
            v=solve(C,ones);w=solve(C,y);den=sum(v);mean=sum(w)/den
            e=[u-mean for u in y];s=dot(e,solve(C,e))/(n-1)
            ld=sum(math.log1p(ratio*sum(r[0]==g for r in data)) for g in range(1,6))
            f=.5*((n-1)*(1+math.log(2*math.pi*s))+ld+math.log(den));refs.append((f,ratio,mean,s))
        f,ratio,mean,s=min(refs)
        a=model("random-intercept-reml",data,[0,hi,21])
        for r in a:
            g=r[0];group=[u[1] for u in data if u[0]==g];ref=ratio*(sum(group)-len(group)*mean)/(1+ratio*len(group))
            near(r[1],mean);near(r[2],ref);near(r[3],s);near(r[4],ratio);near(r[5],f);rows.append([hi,*r])
    report("random_intercept_reml_profile",["GridHigh","Group","FixedIntercept","RandomIntercept","ResidualVariance","VarianceRatio","RestrictedNLL"],rows)

    # 5. SUR using independently assembled block GLS.
    rows=[];data=[]
    for i in range(32):
        x=math.sin(.4*i);z=math.cos(.19*i);e=.3*math.sin(1.2*i)
        data.append([1+2*x+e,.5-z+.7*e+.1*math.cos(1.8*i),1,x,1,z])
    X=[r[2:4] for r in data];Z=[r[4:] for r in data];y=[r[0] for r in data];z=[r[1] for r in data]
    a=ridge(X,y,0);b=ridge(Z,z,0);e=[[u-dot(v,a),w-dot(t,b)] for u,w,v,t in zip(y,z,X,Z)]
    for penalty in [.001,.01,.1]:
        S=[[sum(r[i]*r[j] for r in e)/len(e)+(penalty if i==j else 0) for j in range(2)] for i in range(2)]
        W=inv(S);H=[[0.]*4 for _ in range(4)];rhs=[0.]*4
        for i in range(len(data)):
            D=[X[i]+[0,0],[0,0]+Z[i]];Dt=transpose(D);H0=mm(mm(Dt,W),D);b0=mv(mm(Dt,W),[y[i],z[i]])
            for j in range(4):
                rhs[j]+=b0[j]
                for k in range(4):H[j][k]+=H0[j][k]
        ref=solve(H,rhs);fit=model("sur-feasible-gls",data,[2,2,penalty])
        for r,v in zip(fit,ref):near(r[2],v);near(r[3],S[0][0]);near(r[4],S[0][1]);near(r[5],S[1][1]);rows.append([penalty,*r,v])
    report("sur_joint_normal_equations",["Ridge","Equation","Feature","Coefficient","ResidualCov11","ResidualCov12","ResidualCov22","ScoreResidual","IndependentCoefficient"],rows)

    # 6. Monte Carlo joint squared loss of a fixed-target positive-part shrinkage estimator.
    rows=[];rng=random.Random(20261124)
    for d in [3,10,30]:
        for level in [0,.5,2]:
            for rep in range(24):
                theta=[level*math.sin(j+1) for j in range(d)]
                y=[u+rng.gauss(0,1) for u in theta];a=model("james-stein-means",[[v] for v in y],[1,0])
                dist=dot(y,y);factor=max(0,1-(d-2)/dist)
                for r,v in zip(a,y):near(r[2],factor*v);near(r[3],factor)
                rows.append([d,level,rep+1,sum((u-v)**2 for u,v in zip(y,theta)),sum((r[2]-v)**2 for r,v in zip(a,theta)),factor])
    report("james_stein_joint_risk",["Dimension","SignalScale","Replication","RawSquaredLoss","ShrinkSquaredLoss","Multiplier"],rows)

    # 7. Fully observed rank-one singular-value shrinkage, plus cold-start holdout.
    rows=[];u=[1,2,3];v=[2,1,4];sigma=math.sqrt(dot(u,u)*dot(v,v))
    for lam in [.1,.5,1,3]:
        data=[[i+1,j+1,1,u[i]*v[j]] for i in range(3) for j in range(3)]+[[4,j+1,0,10+j] for j in range(3)]
        a=model("masked-als",data,[1,lam,1e-9,10000])
        b=model("masked-als",[r if r[2] else [r[0],r[1],0,r[3]+100] for r in data],[1,lam,1e-9,10000])
        for r,s in zip(a,b):
            ref=r[3]*(1-lam/sigma) if r[2] else 0
            near(r[4],ref,atol=2e-7);near(r[4],s[4]);rows.append([lam,*r,ref])
    report("masked_als_singular_shrinkage",["Penalty","Row","Column","Train","Actual","Prediction","Objective","StationarityResidual","Iterations","ReferencePrediction"],rows)

    # 8. Probit original score and likelihood independent of Newton.
    rows=[];data=[[1,(i-15)/7,int((i%5)+.11*i>3)] for i in range(31)]
    for lam in [.05,.2,1,4]:
        a=model("probit-ridge-fit",data,[lam,1e-9,200]);w=[r[1] for r in a];g=[lam*v for v in w];f=.5*lam*dot(w,w)
        for *x,y in data:
            s=2*y-1;z=s*dot(x,w);cdf=N(z);f-=math.log(cdf)
            for j in range(2):g[j]-=s*x[j]*pdf(z)/cdf
        near(a[0][2],f);assert max(map(abs,g))<1e-7
        for j,r in enumerate(a):rows.append([lam,*r,g[j]])
    report("probit_likelihood_score",["Penalty","Feature","Coefficient","PenalizedNLL","ScoreResidual","Iterations","IndependentScore"],rows)

    # 9. Concave scalar mixture likelihood: independent root bracket.
    rows=[];y=[0,0,1,1,2,3,4,5,6,7,8,9]
    for low,high in [(1,4),(1,6),(2,7)]:
        a=[math.exp(-low+v*math.log(low)-math.lgamma(v+1)) for v in y]
        b=[math.exp(-high+v*math.log(high)-math.lgamma(v+1)) for v in y]
        def score(p):return sum((u-v)/(p*u+(1-p)*v) for u,v in zip(a,b))
        if score(0)<=0:ref=0.
        elif score(1)>=0:ref=1.
        else:
            lo,hi=0.,1.
            for _ in range(100):
                m=(lo+hi)/2
                if score(m)>0:lo=m
                else:hi=m
            ref=(lo+hi)/2
        r=model("poisson-mixture-weight",[[v] for v in y],[low,high,1e-9,10000])[0]
        ll=sum(math.log(ref*u+(1-ref)*v) for u,v in zip(a,b));near(r[0],ref);near(r[2],ll)
        rows.append([low,high,*r,ref])
    report("poisson_mixture_profile",["LowRate","HighRate","LowWeight","HighWeight","LogLikelihood","ProjectedScoreResidual","Iterations","ReferenceWeight"],rows)

    # 10. Independent enumeration of every hidden-state path.
    rows=[];obs=[1,0,1,1,0,0,1]
    for init,a,b,e0,e1 in [(.4,.1,.2,.2,.8),(.7,.3,.4,.1,.9),(.5,.2,.2,.35,.65)]:
        def enumerate_paths(k):
            out=[]
            for path in product([0,1],repeat=k):
                p=init if path[0] else 1-init
                for i,s in enumerate(path):
                    emit=e1 if s else e0;p*=emit if obs[i] else 1-emit
                    if i:p*=((1-a,a),(b,1-b))[path[i-1]][s]
                out.append((path,p))
            return out
        allpaths=enumerate_paths(len(obs));den=sum(v for _,v in allpaths)
        fit=model("bernoulli-hmm-filter",[[v] for v in obs],[init,a,b,e0,e1])
        prev=1
        for t,r in enumerate(fit):
            prefix=enumerate_paths(t+1);mass=sum(v for _,v in prefix)
            f=sum(v for path,v in prefix if path[-1])/mass
            s=sum(v for path,v in allpaths if path[t])/den
            near(r[3],f);near(r[4],s);near(r[5],math.log(mass/prev));near(r[6],math.log(den));prev=mass
            rows.append([init,a,b,e0,e1,*r,f,s])
    report("hmm_path_enumeration",["Initial","Transition01","Transition10","Emission0","Emission1","Time","Observation","PriorHigh","FilteredHigh","SmoothedHigh","LogPredictiveProbability","LogLikelihood","ReferenceFilter","ReferenceSmoother"],rows)

    # 11. Sequential RLS versus batch exponentially weighted normal equations.
    rows=[];data=[[1,math.sin(.17*t),.5+.8*math.sin(.17*t)+.2*math.cos(1.3*t)] for t in range(40)]
    for gamma in [.85,.95,1]:
        for lam in [.1,1]:
            fit=model("recursive-least-squares",data,[gamma,lam]);prev=[0.,0.]
            for t,r in enumerate(fit):
                wts=[gamma**(t-j) for j in range(t+1)]
                X=[[math.sqrt(w)*v for v in u[:2]] for w,u in zip(wts,data[:t+1])]
                y=[math.sqrt(w)*u[2] for w,u in zip(wts,data[:t+1])]
                ref=ridge(X,y,lam*gamma**(t+1))
                near(r[2],dot(data[t][:2],prev));near(r[4],dot(data[t][:2],ref))
                for a,b in zip(r[6:],ref):near(a,b)
                prev=ref;rows.append([gamma,lam,*r])
    report("rls_batch_equivalence",["Forgetting","PriorRidge","Time","Actual","Prediction","Innovation","PosteriorPrediction","InformationTrace","Beta1","Beta2"],rows)

    # 12. Dirichlet-multinomial profile and posterior normalization.
    rows=[];data=[[12,1,2],[1,9,1],[4,3,7],[5,5,5],[0,8,4],[10,2,1]];m=[.4,.35,.25]
    for low,high,g in [(.1,10,15),(.5,50,17),(1,100,13)]:
        candidates=[]
        for k in grid(low,high,g,True):
            ll=0
            for r in data:
                n=sum(r);ll+=math.lgamma(n+1)-sum(math.lgamma(v+1) for v in r)+math.lgamma(k)-math.lgamma(k+n)
                ll+=sum(math.lgamma(k*p+v)-math.lgamma(k*p) for p,v in zip(m,r))
            candidates.append((-ll,k))
        ll,k=min(candidates)
        fit=model("dirichlet-multinomial-grid",data,[low,high,g,*m])
        for r,x in zip(fit,data):
            near(r[1],k);near(r[2],ll);near(sum(r[3:]),1)
            for a,b,c in zip(r[3:],x,m):near(a,(k*c+b)/(k+sum(x)))
            rows.append([low,high,g,*r])
    report("dirichlet_multinomial_profile",["GridLow","GridHigh","GridCount","Group","Concentration","TrainingNLL","PosteriorMean1","PosteriorMean2","PosteriorMean3"],rows)
