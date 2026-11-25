"""Independent pricing formula, replication and calibration studies."""
import math,itertools,cmath
from comprehensive_study_common import near,check_vector

def norm(z):return .5*math.erfc(-z/math.sqrt(2))
def call(F,K,v):
    d1=(math.log(F/K)+v/2)/math.sqrt(v);return F*norm(d1)-K*norm(d1-math.sqrt(v))

def studies(model,report):
    results=[]
    for b in (.05,.1,.2):
        a=.1;sig=.8;rho=.2;m=.1
        data=[[k,a+b*(rho*(k-m)+math.sqrt((k-m)**2+sig*sig))] for k in (-1,-.5,0,.5,1)]
        out=model("svi-profile-calibration",data,[rho,rho,2,sig,sig,2,m,m,2])
        for r in out:
            near(r[4],a);near(r[5],b);near(r[2],r[1]);k=r[0]
            f=lambda z:a+b*(rho*(z-m)+math.sqrt((z-m)**2+sig*sig))
            h=1e-4;w=f(k);d=(f(k+h)-f(k-h))/(2*h);dd=(f(k+h)-2*w+f(k-h))/(h*h)
            g=(1-k*d/(2*w))**2-d*d/4*(1/w+.25)+dd/2
            near(r[9],g,atol=1e-6);results.append([b,*r])
    report("svi_recovery_and_density",["Slope","LogMoneyness","MarketVariance","FittedVariance","Residual","A","B","Rho","M","Sigma","DensityFactor","WingLeft","WingRight","TrainingSSE"],results)

    results=[]
    for excess in (1,5,20):
        data=[[80,30,1],[100,20+excess,1],[120,10,1]]
        # One convexity half-space is violated; its orthogonal projection is feasible.
        shift=excess/3
        expected=[30+shift,20+excess-2*shift,10+shift]
        out=model("call-curve-convex-projection",data,[100,1e-10,10000])
        check_vector([r[2] for r in out],expected,atol=1e-6)
        for r in out:assert r[5]<1e-7 and abs(r[7])<1e-7;results.append([excess,*r])
    report("call_convexity_single_halfspace",["Violation","Strike","RawCall","ProjectedCall","LowerBound","UpperBound","MaxViolation","Objective","DualGap","Iterations"],results)

    results=[]
    for sig in (0,.02,.1):
        D=[math.exp(-.03*t) for t in range(1,5)];out=model("ho-lee-curve-tree",[[d] for d in D],[1,sig,.02])
        rates={(int(r[0]),int(r[1])):r[3] for r in out}
        for t,d in enumerate(D):
            total=0
            for path in itertools.product((0,1),repeat=t):
                node=0;discount=1
                for step,up in enumerate(path):discount*=math.exp(-rates[step,node]);node+=up
                discount*=math.exp(-rates[t,node]);total+=discount/2**t
            near(total,d)
        results.extend([[sig,*r] for r in out])
    report("ho_lee_path_discount_repricing",["Volatility","Step","Node","Shift","ShortRate","StatePrice","TargetDiscount","ModelDiscount","CurveResidual","CapletContribution"],results)

    results=[]
    for s in (.01,.03,.06):
        f=[s,s+.01,s+.02,s+.03];A=B=0;data=[]
        for j,z in enumerate(f,1):
            d=math.exp(-.01*j);A+=.5*d;B+=.5*d*z;data.append([.5*j,.5,d,B/A])
        out=model("multicurve-forward-bootstrap",data,[])
        check_vector([r[4] for r in out],f)
        results.extend([[s,*r] for r in out])
    report("multicurve_exact_forward_recovery",["InitialForward","Maturity","Accrual","Discount","ParSwap","Forward","Annuity","FloatingPV","RepricedSwap","Residual"],results)

    results=[]
    for vol in (.1,.2,.3):
        for step in (1,.5):
            F=100;data=[]
            for j in range(int(10/step),int(300/step)+1):
                K=j*step;c=call(F,K,vol**2);p=c-F+K
                data.append([K,max(c,0),max(p,0)])
            out=model("variance-strip-quadrature",data,[F,1,1e-8])
            v=out[-1][5];assert abs(v-vol*vol)<.0002
            results.append([vol,step,v,vol*vol,v-vol*vol])
    report("variance_strip_black_scholes",["Volatility","StrikeStep","Estimate","Exact","Error"],results)

    results=[]
    for rho in (-.8,0,.8):
        S=100;K=105;rd=.02;rf=.03;q=.01;vol=.2;fx=.1;T=2;fixed=1.3
        r=model("quanto-lognormal-option",[[S,K,rd,rf,q,vol,fx,rho,T,fixed]],[])[0]
        F=S*math.exp((rf-q-rho*vol*fx)*T);c=fixed*math.exp(-rd*T)*call(F,K,vol*vol*T)
        near(r[0],c);near(r[1],c-fixed*math.exp(-rd*T)*(F-K));results.append([rho,*r])
    report("quanto_adjusted_forward_black",["Correlation","Call","Put","QuantoForward","AdjustedDrift","DiscountedUnderlying","ParityResidual"],results)

    results=[]
    # In the deterministic variance limit, integrate the known variance path independently.
    for v0 in (.01,.04,.09):
        S=100;K=105;r=.03;q=.01;T=1.5;k=1.3;theta=.04
        out=model("heston-fourier-inversion",[[S,K,r,q,T,k,theta,0,-.5,v0]],[100,2000])[0]
        n=20000;v=sum(theta+(v0-theta)*math.exp(-k*T*(j+.5)/n) for j in range(n))*T/n
        f=S*math.exp((r-q)*T);c=math.exp(-r*T)*call(f,K,v)
        near(out[0],c,atol=1e-6);results.append([v0,0,out[0],c,out[0]-c])
    # Independently solve the affine characteristic-function Riccati ODE by RK4.
    for xi in (.25,.5):
        S=K=100;r=.02;q=.01;T=1;k=2;theta=.04;rho=-.6;v0=.04;U=60;N=300
        def cf(u):
            D=C=0j;dt=T/300
            def f(z):return .5*xi*xi*z*z+(rho*xi*1j*u-k)*z-.5*(u*u+1j*u)
            for _ in range(300):
                a=f(D);b=f(D+dt*a/2);c=f(D+dt*b/2);d=f(D+dt*c)
                C+=k*theta*dt*(D+2*(D+dt*a/2)+2*(D+dt*b/2)+(D+dt*c))/6
                D+=dt*(a+2*b+2*c+d)/6
            return cmath.exp(C+D*v0)
        F=S*math.exp((r-q)*T);logk=math.log(K/F);p=[.5,.5]
        for j in range(N):
            u=(j+.5)*U/N;phase=cmath.exp(-1j*u*logk)/(1j*u)
            p[0]+=(phase*cf(u-1j)).real*U/N/math.pi;p[1]+=(phase*cf(u)).real*U/N/math.pi
        c=math.exp(-r*T)*(F*p[0]-K*p[1])
        out=model("heston-fourier-inversion",[[S,K,r,q,T,k,theta,xi,rho,v0]],[U,N])[0]
        near(out[0],c,atol=2e-4);results.append([v0,xi,out[0],c,out[0]-c])
    report("heston_independent_riccati_reference",["InitialVariance","VolOfVol","Price","Reference","Error"],results)

    results=[]
    for sigma in (0,.1,.3):
        r0=.03;k=1.2;theta=.04
        def discount(T):
            if sigma==0:return math.exp(-theta*T-(r0-theta)*(1-math.exp(-k*T))/k)
            g=math.sqrt(k*k+2*sigma*sigma);e=math.exp(g*T)-1;den=(g+k)*e+2*g
            B=2*e/den;A=(2*g*math.exp((k+g)*T/2)/den)**(2*k*theta/sigma**2)
            return A*math.exp(-B*r0)
        data=[[t,discount(t)] for t in (.5,1,2,5)]
        out=model("cir-zero-curve-grid",data,[r0,k,k,2,theta,theta,2,sigma,sigma,2])
        for r in out:near(r[1],r[2]);results.append([sigma,*r])
    report("cir_affine_reference",["Volatility","Maturity","MarketDiscount","ModelDiscount","LogResidual","Kappa","Theta","Sigma","LogSSE"],results)
