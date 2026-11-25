"""Independent inference and survival references."""
import math,itertools
from comprehensive_study_common import near,check_vector

def studies(model,report):
    results=[]
    for noise in (.1,.5,1):
        data=[[2*z+noise*u,z,z] for z,u in itertools.product((-1,1),repeat=2)]
        out=model("anderson-rubin-inversion",data,[3,0,1,2,3])
        for r in out:
            b=r[0];F=2*(2-b)**2/noise**2;near(r[1],F);near(r[2],float(F<=3));near(r[3],2*noise**2)
            results.append([noise,*r])
    report("ar_orthogonal_design_reference",["Noise","Candidate","Statistic","Accept","ResidualVariance"],results)

    results=[]
    for G in (2,3,5,8):
        data=[]
        for g in range(1,G+1):data.extend([[g,1,g],[-g,-1,g]])
        out=model("cluster-wild-sign-test",data,[0])[0]
        near(out[1],G*(G+1));near(out[-1],2/2**G)
        results.append(out)
    report("cluster_sign_exact_tail",["NullSlope","ObservedScore","Clusters","Assignments","PValue"],results)

    results=[]
    for m in (-.75,-.25,0,.25,.75):
        out=model("empirical-likelihood-mean",[[-1],[1]],[m])[0]
        w=(1+m)/2;ratio=-2*math.log(4*w*(1-w));near(out[2],ratio);near(out[3],1);near(out[4],0)
        results.append(out)
    report("empirical_likelihood_two_point_weights",["Candidate","Lambda","LR","WeightSum","MomentResidual","Feasible"],results)

    results=[]
    x=[[y,1] for y in range(1,10)]
    for tau in (.2,.5,.8):
        r=model("quantile-regression-admm",x,[tau,1,1e-8,100000])[0]
        ref=math.ceil(9*tau);near(r[1],ref,atol=1e-5)
        loss=lambda b:sum((a[0]-b)*(tau-(a[0]<b)) for a in x)
        near(r[2],loss(ref),atol=1e-6);assert r[3]<=1e-8 and r[4]<=1e-8;results.append([tau,*r])
    report("quantile_intercept_order_statistic",["Tau","Feature","Coefficient","Loss","PrimalResidual","DualResidual","Iterations"],results)

    results=[]
    for m in (1,3,5):
        data=[[1,1]]*m+[[2,2]]*m+[[3,0]]*m
        out=model("aalen-johansen-incidence",data,[])
        check_vector(out[-1][5:],[1/3,1/3,1/3]);assert all(abs(sum(r[5:])-1)<1e-10 for r in out)
        results.extend([[m,*r] for r in out])
    report("aalen_johansen_probability_mass",["Multiplicity","Time","AtRisk","Events1","Events2","Censored","Survival","CIF1","CIF2"],results)

    results=[]
    for k in (1,3,7):
        data=[[1,1]]*k+[[2,2]]*2+[[1,2]]*3
        out=model("interval-censored-discrete-em",data,[1e-11,10000,1,2]);w=k/(k+2)
        near(out[0][1],w,atol=1e-8);near(out[1][1],1-w,atol=1e-8);near(out[0][3],k*math.log(w)+2*math.log(1-w))
        results.extend([[k,*r] for r in out])
    report("interval_censoring_exact_mle",["FirstCount","Support","Mass","CDF","LogLikelihood","Iterations"],results)

    results=[]
    for shift in (-2,0,3):
        data=[[0,1,1],[0,3,1],[1,2+shift,1],[1,4+shift,1],[1,6+shift,1]]
        r=model("overlap-weighted-effects",data,[0,1e-9,100])[0]
        check_vector(r[:6],[2+shift,4+shift,2,3,2,0],atol=1e-7);results.append([shift,*r])
    report("overlap_intercept_hajek",["Shift","Effect","TreatedMean","ControlMean","TreatedESS","ControlESS","Balance","ScoreNorm"],results)

    results=[]
    for G in (1,1.5,2,5):
        data=[[1],[2],[-1],[-2],[3]]
        r=model("rosenbaum-pair-bounds",data,[G])[0]
        def tail(p):return sum(math.comb(5,k)*p**k*(1-p)**(5-k) for k in range(3,6))
        near(r[3],tail(1/(1+G)));near(r[4],tail(G/(1+G)));results.append(r)
    report("rosenbaum_binomial_tail",["Gamma","Pairs","PositivePairs","LowerP","UpperP"],results)
