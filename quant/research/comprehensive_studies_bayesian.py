"""Independent conjugacy, posterior mixtures, stochastic diagnostics and dynamic decisions."""
import math,itertools
from functools import lru_cache
from comprehensive_study_common import near,check_vector

def studies(model,report):
    out=[]
    for t in (0,.2,1,4):
        data=[[1,.5],[3,1],[2,2]]
        V=1/(1+sum(1/(r[1]+t) for r in data));M=V*sum(r[0]/(r[1]+t) for r in data)
        rows=model("normal-hierarchical-grid",data,[0,1,t,t,1])
        for a,r in zip(data,rows):
            w=t/(t+a[1]);check_vector(r[1:5],[w*a[0]+(1-w)*M,w*a[1]+(1-w)**2*V,M,t])
            out.append([t,*r])
    report("normal_hierarchy_conditional_moments",["Tau","Group","PosteriorMean","PosteriorVariance","HyperMean","HyperTau","Evidence"],out)
    out=[]
    for lam in (.1,.5,1,4):
        r=model("logistic-laplace-posterior",[[0,1],[1,1]],[lam,1e-10,100])[0]
        check_vector(r,[1,0,1/math.sqrt(lam+.5),-2*math.log(2)+.5*math.log(lam/(lam+.5)),0])
        out.append([lam,*r])
    report("laplace_symmetric_curvature",["Precision","Feature","Mode","PosteriorSD","EvidenceApprox","ScoreNorm"],out)
    out=[]
    for prior in (.5,1,3):
        a=[[prior+4,prior+1],[prior+2,prior+3]]
        data=[[1,1,4],[1,2,1],[2,1,2],[2,2,3]]
        rows=model("dirichlet-transition-posterior",data,[2,prior,prior])
        for r in rows:
            i,j=int(r[0])-1,int(r[1])-1;s=sum(a[i]);m=a[i][j]/s
            v=m*(1-m)/(s+1)
            two=0
            for k in range(2):
                two+=a[i][k]*(a[i][j]+(j==k))/(s*(s+1)) if k==i else a[i][k]/s*a[k][j]/sum(a[k])
            check_vector(r[2:],[a[i][j],m,v,two]);out.append([prior,*r])
        near(sum(r[-1] for r in rows[:2]),1);near(sum(r[-1] for r in rows[2:]),1)
    report("dirichlet_two_step_moment",["Prior","From","To","Alpha","Mean","Variance","TwoStep"],out)
    out=[]
    for a,b in ((1,1),(2,3),(.5,.5)):
        data=[[0,5],[2,5],[5,5]];rows=model("beta-binomial-hyper-grid",data,[a,a,1,b,b,1])
        for (k,n),r in zip(data,rows):
            A=a+k;B=b+n-k;m=A/(A+B)
            check_vector(r[1:5],[m,A*B/((A+B)**2*(A+B+1)),a,b]);out.append([a,b,*r])
    report("beta_binomial_exact_conjugacy",["Alpha","Beta","Group","PosteriorMean","PosteriorVariance","HyperAlpha","HyperBeta","Evidence"],out)
    out=[]
    for mu in (-2,0,1):
        data=[[.1],[-.3],[1]]
        rows=model("particle-sv-filter",data,[mu,.8,0,0,200,1,.5])
        for a,r in zip(data,rows):
            check_vector(r[1:],[mu,math.exp(mu),200,-.5*(math.log(2*math.pi)+mu+a[0]**2*math.exp(-mu)),0],atol=1e-7);out.append([mu,*r])
    report("particle_filter_deterministic_limit",["Mu","Time","LogVariance","Variance","ESS","LogPredictiveDensity","Resampled"],out)
    out=[]
    for seed in (3,19,51):
        rows=model("gaussian-tail-importance",[[2],[4],[6]],[0,1,30000,seed])
        for r in rows:
            exact=.5*math.erfc(r[0]/math.sqrt(2));near(r[3],exact)
            assert abs(r[1]-exact)<=6*r[2]+1e-15 and 0<r[5]<=30000
            out.append([seed,*r])
    report("importance_tail_sampling_error",["Seed","Threshold","Estimate","SE","Exact","Tilt","ESS"],out)
    out=[]
    for lam in (.5,1,3):
        rows=model("bayesian-multivariate-regression",[[1,1,2],[1,3,4]],[2,lam,2,.3,.3,1])
        for j,r in enumerate(rows):
            check_vector(r[2:5],[[4,6][j]/(lam+2),[2,1][j]/(lam+2),1/(lam+2)])
            out.append([lam,*r])
    report("matrix_normal_closed_form",["Precision","Feature","Response","Mean","Variance","FeatureCovariance","Evidence"],out)
    out=[]
    for h in (1,2,4,6):
        for s in ((1,1,1,1),(2,1,1,2),(2,3,3,2)):
            @lru_cache(None)
            def v(state,k):
                if k==0:return 0.
                return max(q(state,k,j) for j in (0,1))
            def q(state,k,j):
                j*=2;m=state[j]/(state[j]+state[j+1]);a=list(state);b=list(state);a[j]+=1;b[j+1]+=1
                return m+m*v(tuple(a),k-1)+(1-m)*v(tuple(b),k-1)
            r=model("beta-bandit-finite-horizon",[list(s)],[h,1])[0]
            a,b=q(s,h,0),q(s,h,1);check_vector(r[3:],[max(a,b),a,b]);out.append([*s,*r])
    report("bandit_bellman_tree_reference",["Alpha1","Beta1","Alpha2","Beta2","Scenario","Horizon","Action","Value","FirstValue","SecondValue"],out)
