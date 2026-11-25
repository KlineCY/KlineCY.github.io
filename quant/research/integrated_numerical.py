"""Cross-method numerical approximations, conditioning and dynamic forecasts."""
import math
from comprehensive_study_common import near,check_vector,solve,dot

def lanczos_inverse(model,report):
    out=[]
    for scale in (.5,1,5):
        A=[[2*scale,.2,0],[.2,4*scale,.3],[0,.3,6*scale]];b=[1.,2.,-1.]
        norm=math.sqrt(dot(b,b));rows=model("lanczos-krylov",[[*r,z] for r,z in zip(A,b)],[3,1e-12])
        assert len(rows)==3
        T=[[0.]*3 for _ in range(3)]
        for i,r in enumerate(rows):
            T[i][i]=r[1]
            if i<2:T[i][i+1]=T[i+1][i]=r[2]
        z=solve(T,[norm,0,0]);quad=norm*z[0]
        ref=dot(b,solve(A,b));near(quad,ref)
        cg=model("pcg-jacobi",[[*r,y] for r,y in zip(A,b)],[1e-10,100,1])
        near(cg[-1][3],-.5*quad)
        out.append([scale,quad,ref,cg[-1][3],cg[-1][2],max(r[3] for r in rows)])
    report("lanczos_tridiagonal_inverse_energy",["Scale","KrylovQuadratic","DenseQuadratic","PCGEnergy","PCGResidual","OrthogonalityError"],out)

def chebyshev_tail(model,report):
    out=[]
    for n in (8,16,32):
        # Approximate the central-to-moderate standard Gaussian tail on [0,5].
        nodes=[2.5*(1+math.cos(math.pi*(j+.5)/n)) for j in range(n)]
        y=[[.5*math.erfc(z/math.sqrt(2))] for z in nodes]
        grid=[-1,-.2,.2,.6,1]
        curve=model("chebyshev-interpolant",y,grid)
        for r in curve:
            a=2.5*(1+r[0]);exact=.5*math.erfc(a/math.sqrt(2))
            mc=model("gaussian-tail-importance",[[a]],[0,1,30000,7])[0]
            near(mc[3],exact);assert abs(mc[1]-exact)<=6*mc[2]+1e-12
            if n>=16:assert abs(r[1]-exact)<1e-6
            out.append([n,a,r[1],exact,mc[1],mc[2],abs(r[1]-exact)])
    report("chebyshev_gaussian_tail_importance",["Nodes","Threshold","ChebyshevProbability","ExactProbability","ImportanceProbability","MCSE","InterpolationError"],out)

def arma_innovations_garch(model,report):
    out=[]
    y=[1,.9,1.2,.8,1.4,1.1,.7,1.3,.9,1.5,1.2]
    for span in (.2,.5,.8):
        fit=model("arma11-profile-grid",[[v] for v in y[:8]],[-span,span,3,-span,span,3])[0]
        phi,theta,mu=fit[:3];e=[0.]
        for t in range(1,len(y)):
            forecast=mu+phi*(y[t-1]-mu)+theta*e[-1];e.append(y[t]-forecast)
        near(fit[3],sum(v*v for v in e[1:8])/7);near(fit[5],y[8]-e[8])
        g=model("garch-targeted-grid",[[int(t<8),e[t]] for t in range(1,len(y))],[.1,.1,2,.6,.6,2])
        for t,r in enumerate(g,1):
            if t>=8:
                near(r[3],g[t-2][5]+.1*e[t-1]**2+.6*g[t-2][3])
                out.append([span,t+1,phi,theta,mu,e[t],r[3],r[4]])
    report("arma_mean_garch_innovation_filter",["GridSpan","Period","AR","MA","Mean","Innovation","ForecastVariance","StandardizedInnovation"],out)

def chebyshev_curve_tree(model,report):
    out=[]
    r=.03;end=5
    for n in (4,8,16):
        nodes=[end/2*(1+math.cos(math.pi*(j+.5)/n)) for j in range(n)]
        exact=lambda t:math.exp(-r*t-.001*t*t)
        interp=model("chebyshev-interpolant",[[exact(t)] for t in nodes],[2*t/end-1 for t in range(1,6)])
        D=[z[1] for z in interp]
        tree=model("ho-lee-curve-tree",[[d] for d in D],[1,0,.02])
        for t,d in enumerate(D,1):
            q=[a for a in tree if a[0]==t-1];near(sum(z[4]*math.exp(-z[3]) for z in q),d)
            if n>=8:near(d,exact(t),atol=1e-8)
            out.append([n,t,d,exact(t),d-exact(t),q[0][3],q[0][7]])
    report("chebyshev_discount_curve_tree_repricing",["Nodes","Maturity","ApproxDiscount","ExactDiscount","InterpolationError","TreeRate","TreeResidual"],out)

def qr_risk_precision(model,report):
    out=[]
    for noise in (.001,.01,.05):
        x=[[1,z] for z in (-2,-1,0,1,2)]
        y=[.03+.4*r[1]+noise*v for r,v in zip(x,[1,-1,0,-1,1])]
        qr=model("rrqr-regression",[[z,*r] for z,r in zip(y,x)],[1e-11])
        b=[r[1] for r in qr]
        near(qr[0][3]**2,sum((z-dot(r,b))**2 for z,r in zip(y,x)))
        # Condition number of the regularized one-factor risk solve varies with noise.
        D=[noise**2+.001,.002];B=[[b[1]],[.3]];alpha=[.01,.02]
        wb=model("woodbury-risk-solve",[[D[i],B[i][0],alpha[i]] for i in range(2)],[])
        H=[[D[i]*(i==j)+B[i][0]*B[j][0] for j in range(2)] for i in range(2)]
        check_vector([r[1] for r in wb],solve(H,alpha))
        out.append([noise,*b,qr[0][3],wb[0][1],wb[1][1],wb[0][2]])
    report("rrqr_residual_to_factor_precision",["Noise","Intercept","Loading","ResidualNorm","RiskSolution1","RiskSolution2","LogDet"],out)

def state_observation_moments(model,report):
    out=[]
    for phi in (.2,.5,.9):
        x=[[0,1,1,2,1],[2,0,0,1,1],[1,2,1,0,0]]
        a=model("dynamic-factor-sequential",x,[phi,.1,0,1,1,1,.5,1])
        # Discrete approximation to Gaussian state uncertainty through a finite Markov model.
        for t,r in enumerate(a):
            p=[.5,.5];mu=[r[4]-math.sqrt(r[5]),r[4]+math.sqrt(r[5])]
            m=model("markov-cumulative-reward",[[1,0,mu[0],0,p[0]],[0,1,mu[1],0,p[1]]],[3])
            for h,v in enumerate(m,1):
                near(v[1],h*r[4]);near(v[2],h*h*r[5])
                out.append([phi,t+1,h,r[4],r[5],v[1],v[2]])
    report("filter_uncertainty_persistent_state_moments",["Phi","Period","Horizon","FilteredMean","FilteredVariance","CumulativeMean","CumulativeVariance"],out)

def studies(model,report):
    for fn in (lanczos_inverse,chebyshev_tail,arma_innovations_garch,chebyshev_curve_tree,qr_risk_precision,state_observation_moments):fn(model,report)
