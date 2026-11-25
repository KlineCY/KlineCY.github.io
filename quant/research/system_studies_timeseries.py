"""Time-series research and independent risk-accounting studies."""
import math
from system_study_common import near,check_vector,bartlett,dot,solve,ridge,mm,mv,transpose

def studies(model,report):
    rows=[];x=[math.sin(.31*t)+.3*math.cos(1.3*t) for t in range(64)]
    for J in [1,2,3,4,6]:
        fit=model("haar-energy",[[v] for v in x],[J])
        for r in fit:
            j,k,typ=map(int,r[:3]);length=2**j;z=x[(k-1)*length:k*length]
            ref=(sum(z[:length//2])-sum(z[length//2:]))/math.sqrt(length) if typ else sum(z)/math.sqrt(length)
            near(r[3],ref);near(r[6],0);rows.append([J,*r,ref])
        near(sum(r[4] for r in fit),dot(x,x))
    report("haar_multiscale_orthogonality",["Depth","Level","Index","Type","Coefficient","Energy","TotalEnergy","ReconstructionError","ReferenceCoefficient"],rows)
    rows=[];x=[1+.1*t+math.sin(.5*t) for t in range(50)]
    for d in [0,.2,.5,.9,1]:
        for K in [2,5,10]:
            # Independent finite generalized-binomial product.
            w=[(-1)**k*math.prod((d-i)/(i+1) for i in range(k)) for k in range(K+1)]
            fit=model("fractional-filter",[[v] for v in x],[d,K])
            for r in fit:
                t=int(r[0])-1;ref=sum(w[k]*x[t-k] for k in range(K+1))
                check_vector(r[1:],[ref,sum(w),sum(map(abs,w))]);rows.append([d,K,*r,ref])
            changed=x[:30]+[v+100 for v in x[30:]]
            b=model("fractional-filter",[[v] for v in changed],[d,K])
            for r,s in zip(fit,b):
                if r[0]<=30:check_vector(r,s)
    report("fractional_causal_filter",["Order","Lag","Time","Filtered","WeightSum","WeightL1","Reference"],rows)
    rows=[];data=[[0.,.3]]
    for i in range(1,35):
        t=data[-1][0]+.1+.13*(i%5);a=math.exp(-.7*(t-data[-1][0]))
        data.append([t,a*data[-1][1]+(1-a)*1.2+.15*math.sin(1.5*i)])
    for lo,hi,g in [(.1,1,21),(.2,2,31),(.7,.7,2)]:
        ref=[]
        for k in [math.exp(math.log(lo)+(math.log(hi)-math.log(lo))*j/(g-1)) for j in range(g)]:
            a=[];b=[];z=[]
            for (t,x),(u,y) in zip(data,data[1:]):
                A=math.exp(-k*(u-t));a.append(1-A);b.append((1-A*A)/(2*k));z.append(y-A*x)
            mu=sum(a[i]*z[i]/b[i] for i in range(len(a)))/sum(a[i]**2/b[i] for i in range(len(a)))
            var=sum((z[i]-a[i]*mu)**2/b[i] for i in range(len(a)))/len(a)
            f=sum(.5*(math.log(2*math.pi*var*b[i])+(z[i]-a[i]*mu)**2/(var*b[i])) for i in range(len(a)))
            ref.append((f,k,mu,var))
        f,k,mu,var=min(ref)
        r=model("ou-irregular-profile",data,[lo,hi,g])[0]
        check_vector(r,[k,mu,var,f,0]);rows.append([lo,hi,g,*r])
    report("ou_exact_transition_likelihood",["KappaLow","KappaHigh","GridCount","Kappa","Mean","DiffusionVariance","NLL","MeanScore"],rows)
    rows=[];data=[[t,2+.4*math.cos(2*math.pi*t/12)-.2*math.sin(4*math.pi*t/12)+.1*math.cos(1.7*t),int(t<36)] for t in range(48)]
    for K in [1,2,3]:
        for lam in [0,.1,1]:
            design=lambda t:[1]+[f(2*math.pi*k*t/12) for k in range(1,K+1) for f in (math.cos,math.sin)]
            X=[design(r[0]) for r in data if r[2]];y=[r[1] for r in data if r[2]]
            beta=ridge(X,y,lam,True);fit=model("fourier-seasonal-fit",data,[12,K,lam])
            for r,z in zip(fit,data):
                near(r[3],dot(design(z[0]),beta));check_vector(r[5:],beta)
                rows.append([K,lam,*r[:5],dot(design(z[0]),beta)])
    report("fourier_frozen_seasonality",["Harmonics","Penalty","Time","Train","Actual","Prediction","Residual","Reference"],rows)
    rows=[]
    for a,b in [(.1,.7),(.2,.5),(.05,.94)]:
        fit=model("garch-horizon-risk",[[.04,.001,a,b,.2]],[40]);h=.001+a*.04+b*.04;m=.001/(1-a-b)
        for r in fit:
            k=int(r[1]);ref=m+(h-m)*(a+b)**(k-1);s=k*m+(h-m)*(1-(a+b)**k)/(1-a-b)
            check_vector(r[2:],[ref,s,s/k]);rows.append([a,b,*r])
    report("garch_integrated_variance",["Alpha","Beta","Scenario","Horizon","Variance","Cumulative","MeanPerPeriod"],rows)
    rows=[];data=[[math.sin(.2*t),.8*math.sin(.2*t)+.1*math.cos(t),.6*math.sin(.2*t)] for t in range(60)]
    a=[(y-u)**2 for y,u,v in data];b=[(y-v)**2 for y,u,v in data];d=[u-v for u,v in zip(a,b)]
    for L in [0,1,5,10]:
        r=model("rolling-forecast-comparison",data,[L])[0]
        m=sum(d)/len(d);v=bartlett(d,L)
        check_vector(r,[len(d),sum(a)/len(a),sum(b)/len(b),m,v,m*math.sqrt(len(d)/v),0]);rows.append([L,*r])
    report("paired_forecast_hac",["Lag","N","MSEA","MSEB","MeanDifference","LongRunVariance","DM","Degenerate"],rows)
    rows=[];S=[[.8,.2,.1],[.2,1.2,.3],[.1,.3,.9]];B=[[1,1,1],[.4,-.1,.8]]
    for f in [[1,0],[1,.5],[0,1]]:
        n=len(S);m=len(B);K=[S[i]+[B[j][i] for j in range(m)] for i in range(n)]+[r+[0]*m for r in B]
        ref=solve(K,[0]*n+f)[:n]
        fit=model("factor-replicating-gls",[S[i]+[B[j][i] for j in range(m)] for i in range(n)],f)
        for j,r in enumerate(fit):
            near(r[1],ref[j]);near(r[2],dot(ref,mv(S,ref)));check_vector(r[3:5],f);near(r[-1],0);rows.append([*f,*r])
    report("factor_replication_kkt",["TargetBudget","TargetExposure","Asset","Weight","Variance","Exposure1","Exposure2","Stationarity"],rows)
    rows=[];data=[[.012*math.sin(.2*t),.009*math.cos(.4*t)] for t in range(60)];base=[.6,.4]
    for L in [5,10]:
        for fee in [0,.001,.004]:
            fit=model("rolling-risk-scaling",data,[L,.008,1.5,fee,*base]);old=[0.,0.];nav=1
            for r in fit:
                t=int(r[0])-1;hist=[dot(base,z) for z in data[t-L:t]];m=sum(hist)/L;s=math.sqrt(sum((v-m)**2 for v in hist)/(L-1))
                a=min(1.5,.008/s) if s else 1.5;w=[a*v for v in base]
                F=r[6];turn=sum(abs((1-F)*v-u) for u,v in zip(old,w));gross=dot(w,data[t])
                near(r[1],a);near(r[2],s);near(r[5],turn);near(F,fee*turn)
                net=(1-F)*(1+gross)-1;nav*=1+net;near(r[4],net);near(r[7],nav)
                old=[v*(1+z)/(1+gross) for v,z in zip(w,data[t])];rows.append([L,fee,*r])
    report("risk_target_self_financing",["Lookback","FeeRate","Period","Scale","ForecastVol","Gross","Net","Turnover","Fee","NAV","Weight1","Weight2"],rows)
    rows=[];data=[]
    for t in range(45):
        x=math.sin(.3*t);z=math.cos(.7*t);y=.2+1.5*x-.8*z+.1*math.sin(1.7*t)
        data.append([y,x,z,int(t<30)])
    X=[r[1:3] for r in data[:30]];y=[r[0] for r in data[:30]]
    for lam in [0,.01,.1]:
        m=[sum(r[j] for r in X)/len(X) for j in range(2)];ym=sum(y)/len(y)
        ref=ridge([[a-b for a,b in zip(r,m)] for r in X],[v-ym for v in y],lam*(len(y)-1))
        fit=model("minimum-variance-hedge",data,[lam])
        for r,z in zip(fit,data):
            near(r[3],z[0]-dot(z[1:3],ref));check_vector(r[4:],ref);rows.append([lam,*r])
    report("hedge_frozen_covariance",["Ridge","Row","Train","Exposure","Hedged","Hedge1","Hedge2"],rows)
    rows=[];data=[[.15,1,0],[.25,2,1],[.2,0,3],[.3,5,2],[.1,-1,4]]
    for alpha in [.5,.75,.9]:
        for w in [[.6,.4],[1,-.3]]:
            fit=model("scenario-tail-attribution",data,[alpha,*w]);loss=[dot(r[1:],w) for r in data]
            # Integrate the upper quantile function on every probability interval.
            ordered=sorted(zip(loss,data));cum=0;es=0;c=[0.,0.];var=None
            for L,z in ordered:
                end=cum+z[0]
                if var is None and end+1e-12>=alpha:var=L
                mass=max(0.,end-max(alpha,cum))
                es+=mass*L/(1-alpha)
                for j in range(2):c[j]+=mass*w[j]*z[j+1]/(1-alpha)
                cum=end
            for j,r in enumerate(fit):
                near(r[2],var);near(r[3],es);near(r[4],c[j]);near(r[6],0);rows.append([alpha,*w,*r])
    report("scenario_es_integral",["Alpha","Weight1","Weight2","Asset","Weight","VaR","ES","Contribution","TailMass","Residual"],rows)
    rows=[]
    for initial in [100,1000]:
        for scale in [0,1,3]:
            data=[[scale*f,r] for f,r in [(10,.03),(-5,-.02),(20,.05),(0,-.1)]]
            fit=model("cashflow-portfolio-unitization",data,[initial,2]);nav=initial;wealth=1
            for r,z in zip(fit,data):
                nav=(nav+z[0])*(1+z[1]);wealth*=1+z[1]
                near(r[5],nav);near(r[7],wealth-1);near(r[8],0);rows.append([initial,scale,*r])
    report("unitized_cashflow_invariance",["InitialValue","FlowScale","Period","Opening","Flow","Units","UnitPrice","Closing","Return","TWR","Residual"],rows)
    rows=[]
    for shift in [0,.01,-.02]:
        data=[];P=B=1
        for t in range(1,13):
            wp=.4+.1*math.sin(t);wb=.5;rp=[.02*math.sin(t)+shift,.01*math.cos(t)];rb=[.013*math.sin(t),.008*math.cos(t)]
            data.extend([[t,1,wp,wb,rp[0],rb[0]],[t,2,1-wp,1-wb,rp[1],rb[1]]])
            P*=1+wp*rp[0]+(1-wp)*rp[1];B*=1+.5*(rb[0]+rb[1])
        fit=model("portfolio-brinson-linked",data,[]);near(sum(sum(r[6:9]) for r in fit),P-B)
        for r in fit:near(r[9],P-B);near(r[10],0);rows.append([shift,*r])
    report("carino_geometric_conservation",["ReturnShift","Period","Group","Allocation","Selection","Interaction","Link","LinkedAllocation","LinkedSelection","LinkedInteraction","Active","Residual"],rows)
