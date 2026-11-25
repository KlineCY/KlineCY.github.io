"""Predictive distributions, model uncertainty and sequential Bayesian decisions."""
import math
from functools import lru_cache
from comprehensive_study_common import near,check_vector,solve,dot

def ordinal_rank_alignment(model,report):
    out=[]
    for penalty in (.1,1,5):
        x=[[-2,1],[-1,1],[0,2],[1,3],[2,3]]
        ordinal=model("ordinal-logit-fixed-cuts",[[1,y,z] for z,y in x]+[[0,2,z] for z in (-3,-1,0,1,3)],[3,penalty,1e-9,100,-1,1])
        rank=model("ranknet-linear-fit",[[1,1,y,z] for z,y in x]+[[0,2,2,z] for z in (-3,-1,0,1,3)],[penalty,1e-9,100])
        beta=ordinal[0][-1];b=rank[0][-1];assert beta>0 and b>0
        grades=[]
        for r,s in zip(ordinal[-5:],rank[-5:]):
            q=r[4:7];near(sum(q),1);expected=sum((j+1)*v for j,v in enumerate(q));grades.append(expected)
            out.append([penalty,r[0],s[4],*q,expected,beta,b])
        assert all(a<b for a,b in zip(grades,grades[1:]))
    report("ordinal_probability_rank_alignment",["Penalty","Row","RankScore","P1","P2","P3","ExpectedGrade","OrdinalCoefficient","RankCoefficient"],out)

def mixture_state_risk(model,report):
    out=[]
    for penalty in (.1,.5,2):
        sample=[[-3,1],[-2,1],[-1,1],[1,1],[2,1],[3,1]]
        rows=model("mixture-regression-em",sample,[1,penalty,1e-8,20000,.5,-2,2])
        pi=rows[0][4];a,b=rows[0][-2:]
        mu=pi*a+(1-pi)*b;variance=1+pi*(a-mu)**2+(1-pi)*(b-mu)**2
        for r in rows:near(r[3],mu)
        # An explicit three-scenario portfolio illustration, not the continuous mixture distribution.
        scenarios=[[a/10,.02],[b/10,-.01],[mu/10,.01]]
        risk=model("entropic-risk-allocation",scenarios,[3,1e-9,30000])
        w=[r[1] for r in risk];near(sum(w),1)
        objective=math.log(sum(math.exp(-3*dot(w,r)) for r in scenarios)/3)/3
        near(risk[0][2],objective)
        out.append([penalty,pi,a,b,mu,variance,*w,objective])
    report("mixture_posterior_moments_scenario_risk",["Penalty","Prior1","Component1","Component2","MixtureMean","MixtureVariance","Weight1","Weight2","ScenarioEntropicRisk"],out)

def laplace_hierarchy(model,report):
    out=[]
    for precision in (.5,1,4):
        datasets=[[0,0,1,1],[0,1,1,1],[0,0,0,1]]
        estimates=[]
        for y in datasets:
            r=model("logistic-laplace-posterior",[[v,1] for v in y],[precision,1e-10,100])[0]
            beta=r[1];p=1/(1+math.exp(-beta));near(precision*beta+len(y)*p-sum(y),0,atol=1e-7)
            estimates.append([beta,r[2]**2])
        for tau in (0,.2,1):
            h=model("normal-hierarchical-grid",estimates,[0,1,tau,tau,1])
            V=1/(1+sum(1/(r[1]+tau) for r in estimates));M=V*sum(r[0]/(r[1]+tau) for r in estimates)
            for i,r in enumerate(h):
                beta,s2=estimates[i];w=tau/(tau+s2)
                near(r[1],w*beta+(1-w)*M);near(r[2],w*s2+(1-w)**2*V)
                out.append([precision,tau,i+1,beta,s2,r[1],r[2],M])
    report("logistic_laplace_hierarchical_pooling",["Precision","Tau","Group","LogisticMode","LaplaceVariance","PooledMean","PooledVariance","HyperMean"],out)

def matrix_normal_prediction_risk(model,report):
    out=[]
    for lam in (.2,1,5):
        S=[[.04,.01],[.01,.09]]
        data=[[1,.04,.02],[1,.05,.01],[1,.06,.03]]
        post=model("bayesian-multivariate-regression",data,[2,lam,.04,.01,.01,.09])
        m=[r[2] for r in post];V=1/(lam+3)
        check_vector(m,[.15/(lam+3),.06/(lam+3)])
        pred=[[(1+V)*v for v in r] for r in S]
        risk=model("active-tracking-frontier",[[*pred[i],m[i],.5] for i in range(2)],[.02])
        w=[r[2] for r in risk]
        near(w[0],.02/math.sqrt(pred[0][0]+pred[1][1]-2*pred[0][1]))
        near(risk[0][4],dot(w,m))
        out.append([lam,*m,V,pred[0][0],pred[0][1],pred[1][1],*w,risk[0][4]])
    report("matrix_normal_predictive_tracking_risk",["Precision","Mean1","Mean2","CoefficientVarianceScale","Risk11","Risk12","Risk22","Active1","Active2","ActiveReturn"],out)

def posterior_bandit_value(model,report):
    out=[]
    for counts in ((1,4,3,4),(2,4,2,4),(4,4,0,4)):
        k1,n1,k2,n2=counts
        post=model("beta-binomial-hyper-grid",[[k1,n1],[k2,n2]],[1,1,1,1,1,1])
        s=(k1+1,n1-k1+1,k2+1,n2-k2+1)
        check_vector([r[1] for r in post],[s[0]/(s[0]+s[1]),s[2]/(s[2]+s[3])])
        @lru_cache(None)
        def greedy(state,h):
            if h==0:return 0.
            a=[state[i]/(state[i]+state[i+1]) for i in (0,2)];j=0 if a[0]>=a[1] else 2;m=max(a)
            plus=list(state);minus=list(state);plus[j]+=1;minus[j+1]+=1
            return m+m*greedy(tuple(plus),h-1)+(1-m)*greedy(tuple(minus),h-1)
        for H in (1,3,6):
            r=model("beta-bandit-finite-horizon",[list(s)],[H,1])[0]
            g=greedy(s,H);assert r[3]>=g-1e-10
            if H==1:near(r[3],max(a[1] for a in post))
            out.append([*counts,H,r[2],r[3],g,r[3]-g])
    report("beta_posterior_exploration_value",["Success1","Trials1","Success2","Trials2","Horizon","OptimalAction","OptimalValue","GreedyValue","ExplorationValue"],out)

def pooling_uncertainty(model,report):
    out=[]
    data=[[.02,.001],[.05,.004],[-.01,.002]]
    for tau in (0,.001,.01,.1):
        eb=model("empirical-bayes-alpha-grid",data,[tau,tau,2])
        for v0 in (.01,1,100):
            h=model("normal-hierarchical-grid",data,[0,v0,tau,tau,1])
            V=1/(1/v0+sum(1/(v+tau) for _,v in data))
            for i,(a,b) in enumerate(zip(eb,h)):
                s=data[i][1];w=s/(s+tau)
                near(b[2]-a[6],w*w*V)
                out.append([tau,v0,i+1,a[5],b[1],a[6],b[2],b[2]-a[6]])
    report("empirical_bayes_vs_hypermean_uncertainty",["Tau","HyperPriorVariance","Group","EBMean","BayesMean","EBVariance","BayesVariance","AdditionalVariance"],out)

def studies(model,report):
    for fn in (ordinal_rank_alignment,mixture_state_risk,laplace_hierarchy,matrix_normal_prediction_risk,posterior_bandit_value,pooling_uncertainty):fn(model,report)
