"""Independent causal, panel, survival and Bayesian predictive research."""
import math
from itertools import product
from advanced_study_common import near,dot,solve,ridge,inv,mv,mm,transpose,grid,N,pdf,sigmoid,normal_density

def studies(model,report):
    # 37. Local projection with independent score-based HAC variance.
    rows=[];data=[]
    for t in range(80):
        shock=math.sin(.83*t);control=math.cos(.23*t);y=.5*shock+.7*control+.2*math.sin(1.4*t)
        data.append([shock,y,control])
    for H in [3,6]:
        for L in [0,2,5]:
            fit=model("local-projection-hac",data,[H,L])
            for h,r in enumerate(fit):
                n=len(data)-h;D=[[1,v[0],v[2]] for v in data[:n]];Y=[v[1] for v in data[h:]]
                w=ridge(D,Y,0);G=mm(transpose(D),D);v=solve(G,[0,1,0])
                e=[y-dot(z,w) for y,z in zip(Y,D)];s=[dot(v,z)*u for z,u in zip(D,e)]
                var=sum(s[i]*s[j]*(1-abs(i-j)/(L+1)) for i in range(n) for j in range(n) if abs(i-j)<=L)
                near(r[2],w[1]);near(r[3],var);near(r[5],w[0]);near(r[6],dot(e,e));rows.append([H,L,*r])
    report("local_projection_hac_reference",["MaxHorizon","HACLag","Horizon","Observations","Response","HACVariance","HACSE","Intercept","ResidualSSE"],rows)

    # 38. Fixed effects via an explicit dummy regression, cluster score decomposition.
    rows=[]
    for units,times in [(4,5),(7,6),(10,8)]:
        data=[];D=[];Y=[]
        for i in range(units):
            for t in range(times):
                x=math.sin((i+1)*(t+1)*.43);y=.2*i-.1*t+1.5*x+.2*math.cos(i*.3+t*.7)
                data.append([i,t,x,y]);D.append([1,x]+[float(i==j) for j in range(1,units)]+[float(t==j) for j in range(1,times)]);Y.append(y)
        w=ridge(D,Y,0);fit=model("twfe-two-way-cluster",data,[])[0];near(fit[0],w[1])
        gram=mm(transpose(D),D);v=solve(gram,[float(j==1) for j in range(len(D[0]))])
        scores=[dot(v,z)*(y-dot(z,w)) for z,y in zip(D,Y)]
        e=sum(sum(s for s,r in zip(scores,data) if r[0]==i)**2 for i in range(units))
        t=sum(sum(s for s,r in zip(scores,data) if r[1]==j)**2 for j in range(times))
        c=dot(scores,scores)
        for a,b in zip(fit[1:5],[e,t,c,e+t-c]):near(a,b)
        rows.append([units,times,*fit,w[1]])
    report("twfe_cluster_inclusion_exclusion",["Entities","Times","Coefficient","VarianceEntity","VarianceTime","VarianceCell","TwoWayVariance","StandardError","NegativeFlag","Observations","DummyReference"],rows)

    # 39. AIPW score recomputation and holdout-fold nuisance invariance.
    rows=[];data=[]
    for i in range(90):
        x=math.sin(.41*i);z=math.cos(.21*i);d=int((i%7)/7<.5+.25*x)
        y=1.2+1.5*d+.6*x-.3*z+.15*math.cos(1.6*i);data.append([i%3,d,y,x,z])
    for lam in [.05,.2,1]:
        fit=model("crossfit-aipw",data,[lam,lam,.05,1e-9,200]);score=[]
        for r in fit:
            d,y,e,a,b=r[2:7];s=b-a+d*(y-b)/e-(1-d)*(y-a)/(1-e);near(s,r[7]);score.append(s)
        mean=sum(score)/len(score);se=math.sqrt(sum((s-mean)**2 for s in score)/(len(score)*(len(score)-1)))
        for r in fit:near(r[8],mean);near(r[9],se);rows.append([lam,*r])
        changed=[[r[0],r[1],r[2]+10 if r[0]==0 else r[2],*r[3:]] for r in data]
        b=model("crossfit-aipw",changed,[lam,lam,.05,1e-9,200])
        for r,s in zip(fit,b):
            if r[1]==0:
                for u,v in zip(r[4:7],s[4:7]):near(u,v)
        for fold in range(3):
            for arm in [0,1]:
                train=[r for r in data if r[0]!=fold and r[1]==arm]
                w=ridge([[1,*r[3:]] for r in train],[r[2] for r in train],lam,True)
                for r,source in zip(fit,data):
                    if source[0]==fold:near(r[5+arm],dot([1,*source[3:]],w))
    report("aipw_crossfit_scores",["Penalty","Row","Fold","Treatment","Outcome","Propensity","Outcome0","Outcome1","Score","ATE","IIDSE","Clipped"],rows)

    # 40. Weibull profile from direct right-censored likelihood.
    rows=[];data=[[.2,1],[.4,0],[.7,1],[1,1],[1.2,0],[1.5,1],[2.1,0],[3,1],[3.4,1],[4,0]]
    for lo,hi,g in [(.3,3,21),(.5,5,31),(1,4,15)]:
        events=sum(r[1] for r in data);ref=[]
        for k in grid(lo,hi,g,True):
            eta=(sum(t**k for t,_ in data)/events)**(1/k)
            ll=sum(e*(math.log(k)-k*math.log(eta)+(k-1)*math.log(t))-(t/eta)**k for t,e in data)
            ref.append((-ll,k,eta))
        f,k,eta=min(ref);fit=model("weibull-survival-profile",data,[lo,hi,g])
        for r in fit:
            t=r[0];near(r[2],k);near(r[3],eta);near(r[4],math.exp(-(t/eta)**k));near(r[5],k*t**(k-1)/eta**k);near(r[6],f);rows.append([lo,hi,g,*r])
    report("weibull_censored_profile",["GridLow","GridHigh","GridCount","Time","Event","Shape","Scale","Survival","Hazard","NegativeLogLikelihood"],rows)

    # 41. Dynamic panel moment and first-stage diagnostics under a constructed process.
    rows=[]
    for rho in [.2,.5,.8]:
        for noise in [0,.1,.3]:
            data=[];z=[];a=[];b=[]
            for i in range(8):
                v=[1+.3*i]
                for t in range(1,9):v.append(.2*i+rho*v[-1]+noise*math.sin(1.2*t+.7*i))
                for t,y in enumerate(v):data.append([i,t,y])
                for t in range(2,len(v)):z.append(v[t-2]);a.append(v[t-1]-v[t-2]);b.append(v[t]-v[t-1])
            d=dot(z,a);ref=dot(z,b)/d;r=model("dynamic-panel-diff-iv",data,[])[0]
            near(r[0],ref);near(r[1],0);near(r[2],d/dot(z,z));near(r[3],d*d/(dot(z,z)*dot(a,a)))
            if noise==0:near(r[0],rho)
            rows.append([rho,noise,*r])
    report("dynamic_panel_moment_reference",["TrueRho","Noise","Coefficient","InstrumentMoment","FirstStageCoefficient","FirstStageR2","Observations","Entities"],rows)

    # 42. SupF scan: independent two-segment regression over all eligible cuts.
    rows=[];data=[]
    for t in range(48):
        x=math.sin(.3*t);y=(.5+.8*x if t<25 else 1.3-.5*x)+.15*math.cos(1.2*t)
        data.append([y,1,x])
    def fit(a,b):
        X=[r[1:] for r in data[a:b]];y=[r[0] for r in data[a:b]];w=ridge(X,y,0);return w,sum((v-dot(z,w))**2 for z,v in zip(X,y))
    pooled=fit(0,len(data))[1]
    for minimum in [6,10,15]:
        a=model("supf-break-grid",data,[minimum]);all_f=[]
        for r in a:
            k=int(r[0]);w,l=fit(0,k);v,s=fit(k,len(data));f=(pooled-l-s)/2/((l+s)/(len(data)-4))
            near(r[1],pooled);near(r[2],l+s);near(r[3],f)
            for u,z in zip(r[5:],w+v):near(u,z)
            all_f.append(f);rows.append([minimum,*r])
        selected=max(range(len(all_f)),key=lambda j:all_f[j]);assert a[selected][4]==1 and sum(r[4] for r in a)==1
    report("supf_structural_break_scan",["MinimumSegment","BreakAfter","PooledSSE","SegmentSSE","FStatistic","Selected","LeftIntercept","LeftSlope","RightIntercept","RightSlope"],rows)

    # 43. Bayesian regression conjugate posterior and held-out label invariance.
    rows=[];data=[[int(i<20),1,math.sin(.3*i),.4+.7*math.sin(.3*i)+.2*math.cos(1.4*i)] for i in range(28)]
    X=[r[1:-1] for r in data[:20]];y=[r[-1] for r in data[:20]]
    for lam in [.1,1,5]:
        for alpha,beta in [(2,1),(4,.5)]:
            H=mm(transpose(X),X)
            for j in range(2):H[j][j]+=lam
            m=solve(H,mv(transpose(X),y));a=alpha+10;b=beta+.5*(dot(y,y)-dot(m,mv(H,m)))
            fit=model("bayesian-linear-nig",data,[lam,alpha,beta])
            changed=model("bayesian-linear-nig",[r if r[0] else [*r[:-1],r[-1]+100] for r in data],[lam,alpha,beta])
            for r,z,s in zip(fit,data,changed):
                u=z[1:-1];near(r[3],dot(u,m));near(r[4],math.sqrt(b/a*(1+dot(u,solve(H,u)))));near(r[5],2*a);near(r[6],a);near(r[7],b)
                for v,w in zip(r[3:],s[3:]):near(v,w)
                rows.append([lam,alpha,beta,*r])
    report("bayesian_linear_predictive_reference",["PriorPrecision","PriorAlpha","PriorBeta","Row","Train","Actual","PosteriorMean","PredictiveScale","PredictiveDF","Alpha","Beta"],rows)

    # 44. GP likelihood by Gaussian elimination determinant and predictive covariance.
    rows=[];data=[[1,-1,.3],[1,-.3,-.2],[1,.5,.8],[1,1,1.1],[0,0,.5],[0,1.5,.7]]
    X=[r[1] for r in data if r[0]];y=[r[2] for r in data if r[0]]
    def logdet(A):
        m=[r[:] for r in A];ld=0
        for i in range(len(m)):
            z=m[i][i];assert z>0;ld+=math.log(z)
            for j in range(i+1,len(m)):
                a=m[j][i]/z
                for k in range(i+1,len(m)):m[j][k]-=a*m[i][k]
        return ld
    for llo,lhi,vlo,vhi in [(.2,2,.01,.5),(.5,4,.05,1),(.1,1,.1,2)]:
        candidates=[]
        for l in grid(llo,lhi,5,True):
            for v in grid(vlo,vhi,5,True):
                K=[[math.exp(-.5*((a-b)/l)**2)+(v if i==j else 0) for j,b in enumerate(X)] for i,a in enumerate(X)]
                w=solve(K,y);f=.5*(dot(y,w)+logdet(K)+len(y)*math.log(2*math.pi));candidates.append((f,l,v,w,K))
        f,l,v,w,K=min(candidates)
        fit=model("gaussian-process-grid",data,[llo,lhi,5,vlo,vhi,5])
        for r,z in zip(fit,data):
            k=[math.exp(-.5*((z[1]-u)/l)**2) for u in X];var=1-dot(k,solve(K,k))
            near(r[3],dot(k,w));near(r[4],var);near(r[5],var+v);near(r[6],l);near(r[7],v);near(r[8],f)
            rows.append([llo,lhi,vlo,vhi,*r])
    report("gaussian_process_marginal_likelihood",["LengthLow","LengthHigh","NoiseLow","NoiseHigh","Row","Train","Actual","PosteriorMean","LatentVariance","ObservationVariance","LengthScale","NoiseVariance","TrainingNLL"],rows)

    # 45. Robust Student t likelihood and independent stationarity.
    rows=[];data=[[1,math.sin(.17*i),.4+.8*math.sin(.17*i)+.12*math.cos(1.7*i)+(2 if i%11==0 else 0)] for i in range(45)]
    for nu in [2,4,10]:
        for s in [.3,1]:
            lam=.1;fit=model("student-t-regression-em",data,[nu,s,lam,1e-8,10000]);w=[r[1] for r in fit];g=[lam*v for v in w];obj=.5*lam*dot(w,w)
            for r in data:
                x=r[:-1];e=r[-1]-dot(x,w);obj+=.5*(nu+1)*math.log1p(e*e/(nu*s*s))
                for j in range(2):g[j]-=(nu+1)*e*x[j]/(nu*s*s+e*e)
            near(fit[0][2],obj);assert max(map(abs,g))<=1e-7
            for j,r in enumerate(fit):rows.append([nu,s,*r,g[j]])
    report("student_t_original_score",["DF","Scale","Feature","Coefficient","Objective","ScoreResidual","Iterations","IndependentScore"],rows)

    # 46. Baum-Welch MAP score from explicit enumeration of hidden paths.
    rows=[];obs=[1,1,0,0,1,0,1];init=.4;e0=.2;e1=.8
    for lam in [.5,1,3]:
        a=model("hmm-transition-em",[[v] for v in obs],[init,e0,e1,lam,1e-9,10000])[0];u,v=a[:2];P=[[1-u,u],[v,1-v]]
        mass=0;counts=[[0.,0.],[0.,0.]]
        for path in product([0,1],repeat=len(obs)):
            p=init if path[0] else 1-init
            for i,s in enumerate(path):
                e=e1 if s else e0;p*=e if obs[i] else 1-e
                if i:p*=P[path[i-1]][s]
            mass+=p
            for i in range(1,len(path)):counts[path[i-1]][path[i]]+=p
        counts=[[z/mass for z in row] for row in counts]
        g1=(counts[0][1]+lam)/u-(counts[0][0]+lam)/(1-u)
        g2=(counts[1][0]+lam)/v-(counts[1][1]+lam)/(1-v)
        near(a[2],math.log(mass));near(a[3],math.log(mass)+lam*math.log(u*(1-u)*v*(1-v)));assert max(abs(g1),abs(g2))<=1e-7
        rows.append([lam,*a,g1,g2])
    report("hmm_transition_map_enumeration",["PseudoCount","Transition01","Transition10","LogLikelihood","PenalizedObjective","ScoreResidual","Iterations","IndependentScore01","IndependentScore10"],rows)

    # 47. Online change posterior by enumeration of every segmentation.
    rows=[];y=[0,.1,-.2,.05,2.1,1.9,2.3,2.05];m0=0;v0=1;s=.2
    def segment(a):
        m=m0;v=v0;ll=1.
        for z in a:
            ll*=normal_density(z,m,s+v);vn=1/(1/v+1/s);m=vn*(m/v+z/s);v=vn
        return ll
    for h in [.05,.2,.5]:
        fit=model("bayesian-online-change",[[v] for v in y],[h,m0,v0,s]);previous=1
        for t,r in enumerate(fit,1):
            weights=[0.]*t
            for breaks in product([0,1],repeat=t-1):
                start=0;p=1.
                for i,b in enumerate(breaks,1):
                    p*=h if b else 1-h
                    if b:p*=segment(y[start:i]);start=i
                p*=segment(y[start:t]);weights[t-start-1]+=p
            den=sum(weights);prob=[v/den for v in weights]
            near(r[2],prob[0]);near(r[3],sum((i+1)*v for i,v in enumerate(prob)));near(r[4],max(range(t),key=lambda j:prob[j])+1)
            near(r[5],math.log(den/previous));previous=den
            rows.append([h,*r,prob[0]])
    report("online_change_segmentation_reference",["Hazard","Time","Observation","ChangeProbability","ExpectedRunLength","MAPRunLength","LogPredictiveProbability","MassResidual","IndependentChangeProbability"],rows)

    # 48. Dynamic regression using independent Joseph matrix multiplication.
    rows=[];data=[[1,math.sin(.3*i),.2+.7*math.sin(.3*i)+.15*math.cos(1.3*i)] for i in range(45)]
    for q in [0,.01,.1]:
        for R in [.1,1]:
            m=[0.,0.];P=[[1.,0.],[0.,1.]];fit=model("dynamic-regression-kalman",data,[q,R,1])
            for t,r in enumerate(fit):
                x=data[t][:-1];y=data[t][-1];prior=[[P[i][j]+(q if i==j else 0) for j in range(2)] for i in range(2)]
                v=mv(prior,x);S=dot(x,v)+R;K=[z/S for z in v];pred=dot(m,x);m=[a+b*(y-pred) for a,b in zip(m,K)]
                A=[[float(i==j)-K[i]*x[j] for j in range(2)] for i in range(2)]
                P=mm(mm(A,prior),transpose(A))
                for i in range(2):
                    for j in range(2):P[i][j]+=R*K[i]*K[j]
                near(r[2],pred);near(r[4],S);near(r[5],dot(m,x));near(r[6],P[0][0]+P[1][1])
                for a,b in zip(r[7:],m):near(a,b)
                if q==0:
                    ref=ridge([v[:-1] for v in data[:t+1]],[v[-1] for v in data[:t+1]],R)
                    for a,b in zip(ref,m):near(a,b)
                rows.append([q,R,*r])
    report("dynamic_regression_joseph_reference",["ProcessVariance","ObservationVariance","Time","Actual","Prediction","Innovation","InnovationVariance","FilteredPrediction","PosteriorTrace","Beta1","Beta2"],rows)
