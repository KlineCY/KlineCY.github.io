"""Dynamic model integration, missing observations and information-set diagnostics."""
import math
from comprehensive_study_common import near,check_vector,dot,solve,mv

def autoregressive_crosscheck(model,report):
    output=[]
    for phi in (.2,.5,.8):
        gamma=[phi**i for i in range(5)]
        lev=model("levinson-durbin-path",[[g] for g in gamma],[4])
        near(lev[-1][4],1-phi**2)
        y=[1.]
        for _ in range(9):y.append(.2+phi*y[-1])
        rows=model("arx-recursive-fit",[[int(t<6),z] for t,z in enumerate(y)],[1,0])
        for r in rows:
            near(r[2],r[1]);near(r[5],.2);near(r[6],phi)
            output.append([phi,r[0],r[1],r[2],lev[-1][4],r[6]])
    report("toeplitz_arx_prediction_consistency",["AR","Horizon","Actual","Prediction","InnovationVariance","EstimatedAR"],output)

def garch_particle_risk(model,report):
    output=[]
    for level in (.01,.04,.09):
        # Equal magnitude training returns identify the deterministic GARCH risk.
        r=math.sqrt(level)
        data=[[1,r],[1,-r],[1,r],[0,.5*r],[0,-2*r],[0,r]]
        g=model("garch-targeted-grid",data,[0,0,2,0,0,2])
        sv=model("particle-sv-filter",[[z[1]] for z in data],[math.log(level),.8,0,0,100,7,.5])
        for j,(a,b) in enumerate(zip(g,sv)):
            near(a[3],b[2]);near(b[3],100)
            likelihood=-.5*(math.log(2*math.pi*level)+data[j][1]**2/level)
            near(b[4],likelihood)
            output.append([level,j+1,a[3],b[2],b[3],b[4],likelihood])
    report("garch_and_particle_constant_risk_limit",["Variance","Period","GARCHVariance","ParticleVariance","ESS","PredictiveLogDensity","ExactLogDensity"],output)

def factor_missingness_to_wealth(model,report):
    output=[]
    for missing in (0,1,2):
        data=[]
        for t in range(6):
            f=.02*(t-2)
            data.append([0 if t==0 else 1,f,1,2*f,int(t%3!=missing)])
        modelrows=model("dynamic-factor-sequential",data,[.8,.01,0,.1,1,2,.02,.03])
        wealth=1
        # Use the preceding filtered mean, never the current return or measurement.
        for t in range(1,6):
            score=modelrows[t-1][4]
            r=.01*(t-2)
            # Two opposite exposures, with no transaction cost in this diagnostic.
            w=.5 if score>=0 else -.5
            gross=2*w*r;wealth*=1+gross
            output.append([missing,t+1,score,modelrows[t-1][5],w,r,gross,wealth])
        changed=[r[:] for r in data];changed[-1][1]+=10
        more=model("dynamic-factor-sequential",changed,[.8,.01,0,.1,1,2,.02,.03])
        for a,b in zip(modelrows[:-1],more[:-1]):check_vector(a,b)
    report("asynchronous_factor_to_lagged_decision",["MissingPhase","Period","LaggedState","LaggedVariance","Weight1","Return1","GrossReturn","NAV"],output)

def smooth_curve_trend(model,report):
    output=[]
    for q in (0,.01,.1):
        data=[[1,1,1],[1,1,2],[1,0,0],[1,1,4],[0,1,5],[0,1,6]]
        filt=model("local-trend-filter-grid",data,[q,q,2,q,q,2,.2,0,0,1,1])
        knots=[[r[0],r[6]] for r in filt]
        query=[1+j/4 for j in range(21)]
        interpolated=model("pchip-monotone",knots,query)
        for j,r in enumerate(interpolated):
            if j%4==0:near(r[1],knots[j//4][1])
            output.append([q,r[0],r[1],r[2],r[3],r[4]])
        # Retrospective interpolation is never used for the preceding predictions.
        changed=[r[:] for r in data];changed[-1][2]+=100
        new=model("local-trend-filter-grid",changed,[q,q,2,q,q,2,.2,0,0,1,1])
        for a,b in zip(filt[:-1],new[:-1]):check_vector(a,b)
    report("local_trend_retrospective_interpolation",["ProcessVariance","Time","FilteredInterpolation","Derivative","LowerKnot","UpperKnot"],output)

def threshold_state_risk(model,report):
    output=[]
    for scale in (.1,.5,1):
        y=[scale*v for v in (-2,-1,2,1,-2,-1,2,1,-2)]
        rows=model("threshold-ar-fit",[[int(i<8),v] for i,v in enumerate(y)],[0,0,2,0,2])
        states=[int(r[2]) for r in rows if r[1]]
        counts=[[0,0],[0,0]]
        for a,b in zip(states,states[1:]):counts[a][b]+=1
        post=model("dirichlet-transition-posterior",[[i+1,j+1,counts[i][j]] for i in range(2) for j in range(2)],[2,1,1])
        P=[[post[2*i+j][3] for j in range(2)] for i in range(2)]
        ret=[-.01,.02];var=[.0001,.0002]
        future=model("markov-cumulative-reward",[[*P[i],ret[i],var[i],float(i==states[-1])] for i in range(2)],[5])
        for r in future:
            near(sum(r[4:]),1);assert r[2]>=0
            output.append([scale,r[0],P[0][0],P[1][1],r[1],r[2],r[3]])
    report("observable_threshold_to_markov_risk",["Scale","Horizon","StayLow","StayHigh","CumulativeMean","CumulativeVariance","NextMean"],output)

def numerical_risk_geometry(model,report):
    output=[]
    for c in (-.5,0,.5):
        S=[[1,c],[c,2]]
        # Barrier center of a simple interval of active risk decisions.
        center=model("polytope-analytic-center",[[1,.5],[-1,.5]],[1e-10,100,.1])[0]
        near(center[1],0)
        for linear in (-1,0,1):
            q=model("simplex-active-set",[[*S[0],linear],[*S[1],0]],[])
            w=[r[1] for r in q];near(sum(w),1)
            a=[w[0]-.5,w[1]-.5]
            te=math.sqrt(dot(a,mv(S,a)))
            mu=[-linear,0]
            active=model("active-tracking-frontier",[[*S[i],mu[i],.5] for i in range(2)],[te])
            if linear!=0:near(active[0][5],te)
            output.append([c,linear,center[1],*w,te,q[0][3],active[0][4]])
    report("barrier_simplex_tracking_geometry",["Covariance","LinearTerm","Center","Weight1","Weight2","TrackingError","QuadraticObjective","MaxActiveReturn"],output)

def studies(model,report):
    for fn in (autoregressive_crosscheck,garch_particle_risk,factor_missingness_to_wealth,smooth_curve_trend,threshold_state_risk,numerical_risk_geometry):fn(model,report)
