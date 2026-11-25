"""Joint statistical inference, posterior risk and identification diagnostics."""
import math,itertools
from comprehensive_study_common import near,check_vector

def weak_iv_to_sensitivity(model,report):
    output=[]
    for strength in (.1,.5,1):
        data=[]
        for z,u in itertools.product((-1,1),repeat=2):
            d=strength*z+.3*u
            data.append([2*d+.2*u,d,z])
        candidates=[0,1,2,3]
        rows=model("anderson-rubin-inversion",data,[4,*candidates])
        for r in rows:
            b=r[0];v=(2-b)*.3+.2
            F=2*((2-b)*strength)**2/v**2
            near(r[1],F);near(r[3],2*v*v)
            # Sensitivity of paired, sign-based inference is a separate identification diagnostic.
            pair=model("rosenbaum-pair-bounds",[[1],[2],[-1],[3]],[1,2])
            assert pair[1][4]>=pair[0][4]
            output.append([strength,b,F,r[2],pair[0][4],pair[1][4]])
    report("weak_iv_and_hidden_bias_diagnostics",["FirstStageStrength","Candidate","ARStatistic","Accepted","PairPAtGamma1","PairUpperAtGamma2"],output)

def conditional_quantile_to_tail(model,report):
    output=[]
    for scale in (.1,.5,1):
        # Symmetric, nine-point loss sample. Median and quantile are unambiguous.
        loss=[scale*i for i in range(-4,5)]
        for tau in (.2,.5,.8):
            qr=model("quantile-regression-admm",[[v,1] for v in loss],[tau,1,1e-8,100000])[0]
            q=loss[math.ceil(tau*len(loss))-1]
            near(qr[1],q,atol=1e-5)
            empirical=model("empirical-likelihood-mean",[[v] for v in loss],[0,.5*scale])
            near(empirical[0][2],0);assert empirical[1][2]>0
            # Gaussian tail estimate is a separate parametric benchmark, with sample variance.
            sd=math.sqrt(sum(v*v for v in loss)/(len(loss)-1))
            tail=model("gaussian-tail-importance",[[q]],[0,sd,30000,9])[0]
            exact=.5*math.erfc(q/(sd*math.sqrt(2)))
            near(tail[3],exact);assert abs(tail[1]-exact)<=6*tail[2]+1e-12
            output.append([scale,tau,q,tail[1],tail[2],exact,empirical[1][2]])
    report("quantile_empirical_likelihood_tail_comparison",["Scale","Tau","Quantile","GaussianTailEstimate","MCSE","ExactGaussianTail","MeanLR"],output)

def censored_credit_posterior(model,report):
    output=[]
    for k in (1,3,6):
        data=[[1,1]]*k+[[2,2]]*2+[[1,2]]*3
        em=model("interval-censored-discrete-em",data,[1e-11,10000,1,2]);mass=em[0][1]
        near(mass,k/(k+2),atol=1e-8)
        # Independent complete-cohort counts, not fractional imputed EM counts.
        posterior=model("beta-binomial-hyper-grid",[[k,k+2],[2,k+2]],[1,1,1,1,1,1])
        for i,r in enumerate(posterior):
            a=([k,2][i]+1);b=k+2-[k,2][i]+1
            near(r[1],a/(a+b));near(r[2],a*b/((a+b)**2*(a+b+1)))
            output.append([k,i+1,mass,r[1],r[2],r[5]])
    report("censoring_and_credit_rate_posterior",["FirstCount","Cohort","EMFirstMass","PosteriorMean","PosteriorVariance","LogEvidence"],output)

def overlap_cluster_invariance(model,report):
    output=[]
    for shift in (-1,0,2):
        rows=[]
        for g in (1,2,3,4):
            rows.extend([[0,g,1],[1,g+shift,1]])
        result=model("overlap-weighted-effects",rows,[0,1e-10,100])[0]
        near(result[0],shift)
        # Paired cluster score using the same outcomes and centered treatment indicators.
        test=[[r[1],r[0],g] for g in (1,2,3,4) for r in rows[2*(g-1):2*g]]
        for null in (0,shift):
            sign=model("cluster-wild-sign-test",test,[null])[0]
            near(sign[1],2*abs(shift-null))
            if null==shift:near(sign[-1],1)
            else:near(sign[-1],2/16)
            output.append([shift,null,result[0],result[5],sign[1],sign[-1]])
    report("overlap_estimation_cluster_score",["TrueShift","Null","OverlapEffect","BalanceError","ObservedScore","SignPValue"],output)

def hierarchical_mean_risk(model,report):
    output=[]
    data=[[.02,.001],[.06,.01],[-.01,.002]]
    for tau in (0,.001,.01):
        hier=model("normal-hierarchical-grid",data,[0,.01,tau,tau,1])
        m=[r[1] for r in hier];v=[r[2] for r in hier]
        # Bayesian mean uncertainty contributes to each marginal risk; this diagonal
        # approximation explicitly discards cross-group posterior covariance.
        S=[[((.02+v[i]) if i==j else 0) for j in range(3)] for i in range(3)]
        front=model("active-tracking-frontier",[[*S[i],m[i],1/3] for i in range(3)],[.01])
        a=[r[2] for r in front];near(sum(a),0)
        if tau==0:check_vector(a,[0,0,0])
        else:near(sum(S[i][i]*a[i]**2 for i in range(3)),.0001)
        for i,r in enumerate(front):output.append([tau,i+1,m[i],v[i],a[i],r[4],r[5]])
    report("hierarchical_posterior_marginal_risk_budget",["Tau","Asset","PosteriorMean","PosteriorVariance","ActiveWeight","ExpectedActiveReturn","TrackingError"],output)

def posterior_transition_reward(model,report):
    output=[]
    for prior in (.5,1,3):
        data=[[1,1,4],[1,2,1],[2,1,1],[2,2,4]]
        rows=model("dirichlet-transition-posterior",data,[2,prior,prior])
        P=[[rows[2*i+j][3] for j in range(2)] for i in range(2)]
        two=[[rows[2*i+j][-1] for j in range(2)] for i in range(2)]
        reward=[-1,2];variance=[.2,.3];pi=[1,0]
        fixed=model("markov-cumulative-reward",[[*P[i],reward[i],variance[i],pi[i]] for i in range(2)],[3])
        # Three-period expected reward integrates repeated row use through E[P^2].
        exact=reward[0]+sum(P[0][j]*reward[j] for j in range(2))+sum(two[0][j]*reward[j] for j in range(2))
        plugin=fixed[-1][1]
        delta=sum((two[0][j]-sum(P[0][k]*P[k][j] for k in range(2)))*reward[j] for j in range(2))
        near(exact-plugin,delta)
        output.append([prior,P[0][0],two[0][0],plugin,exact,delta,fixed[-1][2]])
    report("posterior_transition_cumulative_reward",["Prior","StayMean","TwoStepStay","PluginMean","IntegratedMean","Difference","PluginVariance"],output)

def studies(model,report):
    for fn in (weak_iv_to_sensitivity,conditional_quantile_to_tail,censored_credit_posterior,overlap_cluster_invariance,hierarchical_mean_risk,posterior_transition_reward):fn(model,report)
