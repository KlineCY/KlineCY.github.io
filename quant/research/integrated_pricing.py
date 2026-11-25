"""Linked derivative price, curve and risk-neutral integration experiments."""
import math
from comprehensive_study_common import near,check_vector

def normal(z):return .5*math.erfc(-z/math.sqrt(2))
def forward_call(F,K,w):
    z=math.sqrt(w);d=(math.log(F/K)+.5*w)/z
    return F*normal(d)-K*normal(d-z)

def smile_to_strip(model,report):
    output=[]
    F=100
    for b in (.02,.04,.08):
        strikes=list(range(60,161,5))
        a=.02;rho=-.2;m=.0;s=.3
        data=[[math.log(K/F),a+b*(rho*math.log(K/F)+math.hypot(math.log(K/F),s))] for K in strikes]
        svi=model("svi-profile-calibration",data,[rho,rho,2,s,s,2,m,m,2])
        calls=[forward_call(F,K,r[2]) for K,r in zip(strikes,svi)]
        # Deliberately disturb a single interior quote; retain exact forward parity later.
        for shock in (0,1,3):
            raw=calls[:];raw[8]+=shock
            proj=model("call-curve-convex-projection",[[K,c,1] for K,c in zip(strikes,raw)],[F,1e-9,30000])
            C=[r[2] for r in proj];P=[max(0,c-F+K) for c,K in zip(C,strikes)]
            slopes=[(v-u)/5 for u,v in zip(C,C[1:])]
            assert all(-1-1e-7<=v<=1e-7 for v in slopes)
            assert all(y>=x-1e-7 for x,y in zip(slopes,slopes[1:]))
            strip=model("variance-strip-quadrature",[[K,max(0,c),p] for K,c,p in zip(strikes,C,P)],[F,1,1e-7])
            f=[(p if K<F else c if K>F else (c+p)/2)/K**2 for K,c,p in zip(strikes,C,P)]
            v=sum(5*(x+y) for x,y in zip(f,f[1:]))
            near(strip[-1][5],v)
            obj=.5*sum((c-r)**2 for c,r in zip(C,raw))
            near(proj[0][6],obj,atol=1e-7)
            output.append([b,shock,obj,proj[0][5],proj[0][7],v,min(r[9] for r in svi)])
    report("svi_quote_projection_variance_strip",["SVISlope","QuoteShock","ProjectionObjective","Violation","DualGap","TruncatedVariance","MinimumNodeDensityFactor"],output)

def curve_to_tree(model,report):
    output=[]
    for s in (0,.05,.2):
        r0=.025;k=1.1;theta=.03
        def D(T):
            if s==0:return math.exp(-theta*T-(r0-theta)*(-math.expm1(-k*T))/k)
            g=math.sqrt(k*k+2*s*s);z=math.exp(g*T)-1;den=(g+k)*z+2*g
            B=2*z/den;A=(2*g*math.exp((k+g)*T/2)/den)**(2*k*theta/(s*s))
            return A*math.exp(-B*r0)
        data=[[t,D(t)] for t in (1,2,3,4)]
        cir=model("cir-zero-curve-grid",data,[r0,k,k,2,theta,theta,2,s,s,2])
        discounts=[r[2] for r in cir]
        for vol in (0,.02,.08):
            tree=model("ho-lee-curve-tree",[[d] for d in discounts],[1,vol,.025])
            prev=1;annuity=0;swap=[]
            for t,d in enumerate(discounts,1):
                annuity+=d;swap.append([t,1,d,(1-d)/annuity])
                rows=[r for r in tree if r[0]==t-1]
                near(sum(r[4]*math.exp(-r[3]) for r in rows),d)
                cap=sum(r[8] for r in rows)
                assert cap>=0
                if vol==0:near(cap,max(0,prev-1.025*d))
                prev=d
            curve=model("multicurve-forward-bootstrap",swap,[])
            prev=1
            for t,(r,d) in enumerate(zip(curve,discounts)):
                near(r[4],prev/d-1)
                near(r[7],r[3])
                rows=[z for z in tree if z[0]==t]
                output.append([s,vol,t+1,d,r[4],r[3],sum(z[8] for z in rows),max(z[7] for z in rows)])
                prev=d
    report("cir_holee_swap_caplet_repricing",["CIRVolatility","TreeVolatility","Maturity","Discount","Forward","ParSwap","CapletPV","CurveResidual"],output)

def quanto_heston_equivalence(model,report):
    output=[]
    for rho in (-.7,0,.7):
        for fx in (.05,.2):
            S=100;K=105;rd=.025;rf=.04;q=.01;v=.04;T=2;fixed=1.2
            adjusted=rf-q-rho*math.sqrt(v)*fx
            quanto=model("quanto-lognormal-option",[[S,K,rd,rf,q,math.sqrt(v),fx,rho,T,fixed]],[])[0]
            h=model("heston-fourier-inversion",[[S,K,rd,rd-adjusted,T,1,v,0,0,v]],[80,1000])[0]
            near(quanto[0],fixed*h[0]);near(quanto[1],fixed*h[1])
            near(quanto[2],h[4]);near(quanto[0]-quanto[1],fixed*math.exp(-rd*T)*(h[4]-K))
            output.append([rho,fx,adjusted,quanto[0],fixed*h[0],quanto[1],fixed*h[1],quanto[0]-fixed*h[0]])
    report("quanto_deterministic_heston_equivalence",["Correlation","FXVolatility","AdjustedDrift","QuantoCall","HestonCall","QuantoPut","HestonPut","CallDifference"],output)

def stochastic_variance_limit_strip(model,report):
    output=[]
    F=100;k=.8;theta=.04;T=1.5
    for v0 in (.01,.04,.09):
        variance=theta*T+(v0-theta)*(-math.expm1(-k*T))/k
        for step in (4,2,1):
            strikes=list(range(20,301,step))
            rows=model("heston-fourier-inversion",[[F,K,0,0,T,k,theta,0,0,v0] for K in strikes],[80,100])
            data=[]
            for K,r in zip(strikes,rows):
                c=forward_call(F,K,variance);near(r[0],c,atol=1e-7)
                near(r[0]-r[1],F-K)
                data.append([K,max(0,r[0]),max(0,r[1])])
            strip=model("variance-strip-quadrature",data,[F,T,1e-8])
            estimate=strip[-1][5]
            assert abs(estimate-variance/T)<.002
            output.append([v0,step,variance/T,estimate,estimate-variance/T])
    report("integrated_variance_heston_option_strip",["InitialVariance","StrikeSpacing","AnalyticVarianceRate","StripVarianceRate","Difference"],output)

def tree_forward_stress(model,report):
    output=[]
    for level in (-.02,0,.02,.06):
        D=[math.exp(-level*t) for t in range(1,5)]
        for shock in (-.01,0,.01):
            discount=[D[t-1]*math.exp(-shock*t) for t in range(1,5)]
            tree=model("ho-lee-curve-tree",[[z] for z in discount],[1,0,.01])
            A=0;data=[]
            for t,d in enumerate(discount,1):
                A+=d;data.append([t,1,d,(1-d)/A])
            curve=model("multicurve-forward-bootstrap",data,[])
            for t,r in enumerate(curve):
                near(r[4],math.expm1(level+shock))
                near(r[3],math.expm1(level+shock))
                q=[z for z in tree if z[0]==t]
                near(sum(z[4]*math.exp(-z[3]) for z in q),discount[t])
                near(q[0][3],level+shock)
                output.append([level,shock,t+1,r[4],q[0][3],sum(z[8] for z in q)])
    report("negative_rate_curve_tree_stress",["BaseRate","ParallelShock","Maturity","Forward","ShortRate","CapletPV"],output)

def replication_parity(model,report):
    output=[]
    for vol in (.1,.2,.4):
        for K in (80,100,120):
            S=100;rd=.03;rf=.03;q=.01;T=1.2
            rows=model("quanto-lognormal-option",[[S,K,rd,rf,q,vol,0,0,T,1]],[])[0]
            h=model("heston-fourier-inversion",[[S,K,rd,q,T,1,vol**2,0,0,vol**2]],[80,100])[0]
            near(rows[0],h[0]);near(rows[1],h[1])
            # Finite difference delta and gamma, compared to forward Black Greeks.
            eps=.01
            prices=[]
            for spot in (S-eps,S,S+eps):
                prices.append(model("quanto-lognormal-option",[[spot,K,rd,rf,q,vol,0,0,T,1]],[])[0][0])
            d1=(math.log(S/K)+(rd-q+.5*vol**2)*T)/(vol*math.sqrt(T))
            delta=math.exp(-q*T)*normal(d1)
            gamma=math.exp(-q*T)*math.exp(-.5*d1*d1)/(math.sqrt(2*math.pi)*S*vol*math.sqrt(T))
            fd=(prices[2]-prices[0])/(2*eps);gd=(prices[2]-2*prices[1]+prices[0])/eps**2
            near(fd,delta,atol=2e-6);near(gd,gamma,atol=2e-6)
            output.append([vol,K,rows[0],rows[1],fd,delta,gd,gamma])
    report("option_replication_greeks_and_parity",["Volatility","Strike","Call","Put","FDDeltа","AnalyticDelta","FDGamma","AnalyticGamma"],output)

def studies(model,report):
    for fn in (smile_to_strip,curve_to_tree,quanto_heston_equivalence,stochastic_variance_limit_strip,tree_forward_stress,replication_parity):fn(model,report)
