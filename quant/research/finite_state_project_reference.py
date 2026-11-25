"""Independent reference calculations for eight connected finite-state projects.

All cases use synthetic observations or deliberately specified probabilities.
"""
import itertools as it
import math
import random
from functools import lru_cache
from finite_state_reference import bernoulli_loss, flatten, paths

def experiments(seed=20261125):
    rng=random.Random(seed+1)
    # Enumerate full segment durations at initiation, rather than using age hazards.
    for n in range(1,8):
        for _ in range(5):
            obs=[rng.randrange(2) for _ in range(n)]
            pi=[.4,.6];A=[[0,1],[1,0]]
            dur=[[.5,.3,.2],[.2,.5,.3]];emit=[[.8,.2],[.25,.75]]
            hist=[]
            def visit(t,previous,weight):
                for j in range(2):
                    statep=pi[j] if previous is None else A[previous][j]
                    if not statep:continue
                    for d in range(1,4):
                        w=weight*statep*dur[j][d-1]*math.prod(emit[j][o] for o in obs[t:min(n,t+d)])
                        if t+d>=n:hist.append((j,n-t,d,w))
                        else:visit(t+d,j,w)
            visit(0,None,1.)
            ll=sum(w for _,_,_,w in hist)
            f=[[0.]*3 for _ in range(2)];pred=[0.,0.];age=0
            for j,a,d,w in hist:
                w/=ll;f[j][a-1]+=w;age+=a*w
                if a<d:pred[j]+=w
                else:
                    for i in range(2):pred[i]+=w*A[j][i]
            nextobs=[sum(pred[j]*emit[j][v] for j in range(2)) for v in range(2)]
            yield 'duration_filter',[2,3,2,n,*pi,*flatten(A),*flatten(dur),*flatten(emit),*obs],[math.log(ll),*[sum(v) for v in f],age,*pred,*nextobs,*flatten(f)],'full-duration enumeration, including right-censored last segment'
    for n in range(2,7):
        for obs in range(n+1):
            p0=[.1+i*.025 for i in range(n)];p1=[.45+i*.04 for i in range(n)]
            w=[i%3+1 for i in range(n)];prior=[.65,.35];A=1;B=5
            f=[bernoulli_loss(p,[1]*n) for p in (p0,p1)]
            l=sum(prior[i]*f[i][obs] for i in range(2));post=[prior[i]*f[i][obs]/l for i in range(2)]
            ell=[sum(min(max(j-A,0),B-A)*v for j,v in enumerate(bernoulli_loss(p,w))) for p in (p0,p1)]
            for e in (.0,.1,.4):
                delta=min(e,post[0]);q=[post[0]-delta,post[1]+delta]
                dot=lambda a:sum(x*y for x,y in zip(a,ell))
                yield 'credit_posterior_stress',[2,n,obs,A,B,e,*w,prior[0],*p0,prior[1],*p1],[l,dot(prior),dot(post),dot(q),dot(q)-dot(post),delta,*post,*q,*ell],'enumerate calibration default sets and future loss sets separately'
    # Two-state ambiguity sets have an analytic interval for the first probability.
    for n in range(1,6):
        for e in (0,.1,.4,.8):
            reward=[[.4,0.],[1.2,.6]]
            p=[[[.8,.2],[.3,.7]],[[.3,.7],[.7,.3]]]
            discount=.9
            def f(t,i,robust):
                if t==n:return 0.
                child=[f(t+1,j,robust) for j in range(2)]
                vals=[]
                for a in range(2):
                    q=p[i][a][0]
                    if robust:
                        lo=max(0,q-e);hi=min(1,q+e)
                        exp=min(lo*child[0]+(1-lo)*child[1],hi*child[0]+(1-hi)*child[1])
                    else:exp=q*child[0]+(1-q)*child[1]
                    vals.append(reward[i][a]+discount*exp)
                return max(vals)
            v=f(0,0,True);base=f(0,0,False)
            arg=[e,n,2,2,0,discount,*flatten([reward[i][a],*p[i][a]] for i,a in it.product(range(2),repeat=2))]
            yield 'robust_mdp',arg,{0:v,1:base,3:v,4:0,6:1},'explicit two-endpoint worst-transition recursion'
    # Brute force finite decision tree, track true wealth and running peak instead of compressed drawdown.
    for n in range(1,5):
        for H in (1,3,5):
            for pen in (0,.1,.5):
                m=2;k=2;cost=1;step=.04;move=[[0,0],[-2,3]]
                p=[[.7,.3],[.25,.75]]
                def val(t,i,a,w,peak,breached):
                    if t==n:return 0.
                    vals=[]
                    for b in range(m):
                        c=cost*abs(a-b);pre=w-c;prepeak=peak
                        v=0
                        for j in range(k):
                            end=pre+move[b][j];endpeak=max(prepeak,end)
                            hit=breached or prepeak-pre>=H or endpeak-end>=H
                            v+=p[i][j]*(step*(end-w)-pen*(hit and not breached)+val(t+1,j,b,end,endpeak,hit))
                        vals.append(v)
                    return max(vals)
                v=val(0,0,0,0,0,False)
                yield 'drawdown_policy',[n,k,m,0,0,H,cost,step,pen,*flatten(move),*flatten(p)],{0:v,5:v,6:0,7:1},'uncompressed wealth and running-peak decision tree'
    for n in range(1,11):
        for cp in (-1,1):
            s=100;delta=4;strike=101
            asian=plain=prob=0.;endstates=set()
            for z in it.product((-1,1),repeat=n):
                a=[0]+list(it.accumulate(z));total=sum(a)
                avg=s+delta*total/(n+1);end=s+delta*a[-1]
                asian+=max(0,cp*(avg-strike))/2**n;plain+=max(0,cp*(end-strike))/2**n
                prob+=(cp*(avg-strike)>0)/2**n;endstates.add((a[-1],total))
            var=delta**2*sum(j*j for j in range(1,n+1))/(n+1)**2
            yield 'asian_additive',[s,delta,n,strike,cp],[asian,plain,s,var,prob,1,len(endstates)],'complete additive price path enumeration and closed-form average variance'
    # Independent POMDP reference keeps unnormalized state masses along every event history.
    for n in range(1,6):
        for q in (0,1,2):
            for belief in (.2,.7):
                wait=.03;fill=-.2;market=1;shock=.6;sw=.15
                p=[[.55,.1,.05,.3],[.15,.15,.25,.45]]
                def visit(t,j,mass):
                    total=sum(mass)
                    if t==0:return total*market
                    hold=total*wait
                    for e in range(4):
                        z=[mass[i]*p[i][e] for i in range(2)];a=sum(z)
                        if e==2:hold+=a*(market+shock)
                        elif e==0 and j==0:hold+=a*fill
                        else:hold+=visit(t-1,max(0,j-1) if e in (0,1) else j,[(1-sw)*z[0]+sw*z[1],sw*z[0]+(1-sw)*z[1]])
                    return min(total*market,hold)
                v=visit(n,q,[1-belief,belief])
                @lru_cache(None)
                def full(t,i,j):
                    if t==0:return market
                    hold=wait
                    for e in range(4):
                        if e==2:cost=market+shock
                        elif e==0 and j==0:cost=fill
                        else:
                            l=max(0,j-1) if e in (0,1) else j
                            cost=(1-sw)*full(t-1,i,l)+sw*full(t-1,1-i,l)
                        hold+=p[i][e]*cost
                    return min(market,hold)
                lower=(1-belief)*full(n,0,q)+belief*full(n,1,q)
                prob=(1-belief)*p[0][0]+belief*p[1][0];b=sw+(1-2*sw)*belief*p[1][0]/prob
                yield 'hidden_queue_choice',[n,q,belief,wait,fill,market,shock,*p[0][:3],*p[1][:3],sw],{0:v,2:lower,3:market,4:v-lower,5:prob,6:b},'unnormalized hidden-state event tree and fully observed bound'
    # Enumerate deterministic transport plans; a constrained optimum is a convex combination of at most two.
    for _ in range(20):
        p=[.3,.4,.3];loss=[rng.randint(-2,6) for _ in range(3)]
        c=[[abs(i-j)*rng.randint(1,4) for j in range(3)] for i in range(3)]
        plans=[(sum(p[i]*c[i][j] for i,j in enumerate(a)),sum(p[i]*loss[j] for i,j in enumerate(a))) for a in it.product(range(3),repeat=3)]
        for B in (0,.15,.5,2):
            best=max(v for cost,v in plans if cost<=B)
            for a,b in it.product(plans,repeat=2):
                if a[0]<B<b[0]:
                    u=(B-a[0])/(b[0]-a[0]);best=max(best,(1-u)*a[1]+u*b[1])
            yield 'transport_stress',[3,3,B,*p,*loss,*flatten(c)],{0:best,2:best,4:0},'all deterministic transport plans and two-plan convex mixtures'
    # Analytic derivatives and direct Gaussian fourth moments, rather than jet propagation.
    for _ in range(60):
        a,b,c,d,e,f=[rng.uniform(-1.5,1.5) for _ in range(6)]
        s,t=rng.uniform(.05,.5),rng.uniform(.05,.5);rho=rng.uniform(-.8,.8)
        z=c*a+d*b;p=1/(1+math.exp(-z))
        value=math.log1p(math.exp(z))+e*a*b+f*a*a
        gx=c*p+e*b+2*f*a;gy=d*p+e*a
        hxx=c*c*p*(1-p)+2*f;hxy=c*d*p*(1-p)+e;hyy=d*d*p*(1-p)
        v=s*s;w=t*t;u=rho*s*t
        eq=.5*(hxx*v+2*hxy*u+hyy*w)
        delta=gx*gx*v+2*gx*gy*u+gy*gy*w
        # Var(X²)=2v²; Var(XY)=vw+u²; Cov(X²,Y²)=2u².
        quad=(.5*hxx*hxx*v*v+hxy*hxy*(v*w+u*u)+.5*hyy*hyy*w*w
              +2*hxx*hxy*v*u+hxx*hyy*u*u+2*hxy*hyy*w*u)
        yield 'nonlinear_risk_jet',[a,b,c,d,e,f,s,t,rho],[value,gx,gy,hxx,hxy,hyy,value+eq,delta,quad,delta+quad],'closed-form derivatives and Isserlis fourth moments'

if __name__=='__main__':
    import json
    import sys
    for method,args,expected,design in experiments():
        if len(sys.argv)==1 or method==sys.argv[1]:
            print(json.dumps(dict(method=method,input=args,expected=expected,design=design),ensure_ascii=False))
