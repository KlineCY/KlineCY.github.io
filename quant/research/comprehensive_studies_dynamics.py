"""Recursive prediction and state-space reference calculations."""
import itertools,math
from comprehensive_study_common import near,check_vector,solve,mv,dot,transpose,mm

def studies(model,report):
    results=[]
    for rho in (-.7,-.2,.2,.9):
        g=[rho**i for i in range(6)]
        out=model("levinson-durbin-path",[[v] for v in g],[5])
        for r in out:
            p=int(r[0]);j=int(r[1])-1;A=[[g[abs(a-b)] for b in range(p)] for a in range(p)]
            ref=solve(A,g[1:p+1]);near(r[2],ref[j]);near(r[4],1-dot(ref,g[1:p+1]));results.append([rho,*r])
    report("levinson_dense_toeplitz",["Rho","Order","Lag","Coefficient","Reflection","InnovationVariance","NormalResidual"],results)

    results=[]
    for phi in (.2,.5,.9):
        y=[1.];z=[0,1,0,2,0,1,3,-1,2]
        for v in z[1:]:y.append(.3+phi*y[-1]+.7*v)
        data=[[int(i<6),y[i],z[i]] for i in range(len(y))]
        out=model("arx-recursive-fit",data,[1,0])
        for i,r in enumerate(out):
            near(r[2],y[6+i]);check_vector(r[5:],[.3,phi,.7]);results.append([phi,*r])
        changed=[r[:] for r in data]
        for r in changed[6:]:r[1]+=100
        new=model("arx-recursive-fit",changed,[1,0])
        check_vector([r[2] for r in new],[r[2] for r in out])
    report("arx_recursive_no_future_label",["AR","Horizon","Actual","Prediction","Error","TrainingRSS","Intercept","AR1","Exog1"],results)

    results=[]
    y=[1,.8,1.4,.9,1.7,2.1,1.4]
    for span in (.2,.5,.8):
        grid=[-span,0,span]
        def residual(phi,theta,mu):
            e=[0.]
            for i in range(1,len(y)):e.append(y[i]-mu-phi*(y[i-1]-mu)-theta*e[-1])
            return e[1:]
        options=[]
        for a,b in itertools.product(grid,repeat=2):
            r=residual(a,b,0);s=[v-w for v,w in zip(r,residual(a,b,1))]
            m=dot(r,s)/dot(s,s);e=residual(a,b,m);v=dot(e,e)/len(e)
            options.append((len(e)/2*(math.log(2*math.pi*v)+1),a,b,m,v,m+a*(y[-1]-m)+b*e[-1]))
        ll,a,b,m,v,f=min(options)
        r=model("arma11-profile-grid",[[v] for v in y],[-span,span,3,-span,span,3])[0]
        check_vector(r[:6],[a,b,m,v,ll,f],atol=1e-7);results.append([span,*r])
    report("arma_profile_explicit_residual",["Span","AR","MA","Mean","InnovationVariance","NLL","Forecast","Observations"],results)

    results=[]
    x=[.1,-.15,.3,.2,-.4,.1,.5,-.2]
    for a,b in ((0,0),(.1,.6),(.2,.7)):
        data=[[int(i<6),r] for i,r in enumerate(x)]
        out=model("garch-targeted-grid",data,[a,a,2,b,b,2])
        v=sum(r*r for r in x[:6])/6;omega=(1-a-b)*v
        ll=0
        for i,r in enumerate(out):
            near(r[3],v);near(r[5],omega);near(r[4],x[i]/math.sqrt(v))
            if i<6:ll+=.5*(math.log(2*math.pi*v)+x[i]**2/v)
            v=omega+a*x[i]**2+b*v;results.append([a,b,*r])
        near(out[0][8],ll)
    report("garch_explicit_variance_recursion",["Alpha","Beta","Period","Train","Return","ForecastVariance","StandardizedShock","Omega","FittedAlpha","FittedBeta","NLL"],results)

    results=[]
    for s in (.5,1,2):
        y=[-2,-1,2,1,-2,-1,2,1,-2]
        data=[[int(i<8),s*v] for i,v in enumerate(y)]
        out=model("threshold-ar-fit",data,[0,0,2,0,2])
        for r in out:
            check_vector(r[7:],[5*s,3,-5*s,3]);near(r[4],r[3]);results.append([s,*r])
    report("threshold_ar_exact_regimes",["Scale","Period","Train","Regime","Actual","Forecast","Threshold","RSS","LowIntercept","LowAR","HighIntercept","HighAR"],results)

    results=[]
    for phi in (.4,.8,1):
        x=[[0,1,1,3,1],[1,2,0,4,1],[2,1,1,0,0]]
        q=.2;m=0;v=1;load=[1,2];noise=[.5,1.]
        out=model("dynamic-factor-sequential",x,[phi,q,m,v,*load,*noise])
        for a,r in zip(x,out):
            for _ in range(int(a[0])):m=phi*m;v=phi*phi*v+q
            near(r[2],m);near(r[3],v)
            prec=1/v;rhs=m/v
            for j in range(2):
                if a[2+2*j]:prec+=load[j]**2/noise[j];rhs+=load[j]*a[1+2*j]/noise[j]
            v=1/prec;m=v*rhs;near(r[4],m);near(r[5],v);results.append([phi,*r])
    report("dynamic_factor_batch_information",["Phi","Period","Gap","PriorMean","PriorVariance","PosteriorMean","PosteriorVariance","ObservedCount","InnovationNLL","CumulativeNLL"],results)

    results=[]
    for q in (0,.1,.5):
        data=[[1,1,1],[1,0,0],[1,1,2],[0,1,4]]
        out=model("local-trend-filter-grid",data,[q,q,2,q,q,2,.5,0,0,1,1])
        m=[0.,0.];V=[[1.,0.],[0.,1.]];F=[[1.,1.],[0.,1.]]
        for i,(a,r) in enumerate(zip(data,out)):
            if i:
                m=mv(F,m);V=mm(mm(F,V),transpose(F));V[0][0]+=q;V[1][1]+=q
            near(r[3],m[0]);near(r[4],m[1]);s=V[0][0]+.5;near(r[5],s)
            if a[1]:
                k=[V[0][0]/s,V[1][0]/s];e=a[2]-m[0];m=[m[j]+k[j]*e for j in range(2)]
                V=[[V[j][l]-k[j]*V[0][l] for l in range(2)] for j in range(2)]
            check_vector(r[6:11],[m[0],m[1],V[0][0],V[1][1],V[0][1]]);results.append([q,*r])
    report("local_trend_dense_kalman",["ProcessVariance","Period","Train","Observed","PriorLevel","PriorSlope","PredictionVariance","FilteredLevel","FilteredSlope","LevelVariance","SlopeVariance","Covariance","InnovationNLL","QLevel","QSlope","TrainingNLL"],results)

    results=[]
    for stay in (.3,.6,.9):
        P=[[stay,1-stay],[1-stay,stay]];mu=[-1,2];var=[.5,1.];pi=[.6,.4]
        out=model("markov-cumulative-reward",[[*P[i],mu[i],var[i],pi[i]] for i in range(2)],[5])
        for h,r in enumerate(out,1):
            m=s=0
            for path in itertools.product(range(2),repeat=h):
                p=pi[path[0]]
                for a,b in zip(path,path[1:]):p*=P[a][b]
                a=sum(mu[j] for j in path);v=sum(var[j] for j in path);m+=p*a;s+=p*(v+a*a)
            near(r[1],m);near(r[2],s-m*m);results.append([stay,*r])
    report("markov_reward_path_enumeration",["StayProbability","Horizon","MeanCumulative","VarianceCumulative","MeanNextReward","StateProbability1","StateProbability2"],results)
