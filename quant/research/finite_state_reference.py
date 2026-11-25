"""Independent finite-state reference experiments using enumeration and direct equations.

The small reference problems deliberately avoid the state-merging algorithms used
by the C++ implementation. All observations and probabilities are synthetic.
"""
import itertools as it
import math
import random
import statistics as st
from collections import defaultdict
from functools import lru_cache

def product_probability(bits, p):
    return math.prod(pi if b else 1-pi for b,pi in zip(bits,p))

def bernoulli_loss(p, w):
    f=[0.]*(sum(w)+1)
    for bits in it.product((0,1),repeat=len(p)):
        f[sum(a*b for a,b in zip(bits,w))]+=product_probability(bits,p)
    return f

def loss_stats(f, alpha):
    mu=sum(i*p for i,p in enumerate(f))
    var=sum((i-mu)**2*p for i,p in enumerate(f))
    q=next(i for i in range(len(f)) if sum(f[:i+1])>=alpha-1e-13)
    es=min(z+sum(max(i-z,0)*p for i,p in enumerate(f))/(1-alpha) for z in range(len(f)))
    return mu,var,q,es

def tail_summary(values, observed):
    lo=sum(v<=observed for v in values)/len(values)
    hi=sum(v>=observed for v in values)/len(values)
    return [st.mean(values),st.pvariance(values),lo,hi,min(1,2*min(lo,hi)),values.count(observed)/len(values)]

def midranks(values):
    return [(1+sum(z<v for z in values)+sum(z<=v for z in values))/2 for v in values]

def paths(p, initial, n):
    for js in it.product(range(len(p)),repeat=n):
        pr=math.prod(p[i][j] for i,j in zip((initial,)+js,js))
        if pr: yield js,pr

def solve(a,b):
    """Independent elimination with explicit back substitution."""
    z=[list(row)+[rhs] for row,rhs in zip(a,b)]
    n=len(b)
    for i in range(n):
        k=max(range(i,n),key=lambda j:abs(z[j][i]))
        z[i],z[k]=z[k],z[i]
        if abs(z[i][i])<1e-13: raise ValueError('singular reference')
        for j in range(i+1,n):
            v=z[j][i]/z[i][i]
            z[j]=[a-v*b for a,b in zip(z[j],z[i])]
    out=[0.]*n
    for i in reversed(range(n)):
        out[i]=(z[i][-1]-sum(z[i][j]*out[j] for j in range(i+1,n)))/z[i][i]
    return out

def flatten(rows):
    return [x for row in rows for x in row]

def experiments(seed=20261125):
    """Yield (method, input, selected reference coordinates, description)."""
    rng=random.Random(seed)
    # HSMM: enumerate every complete segment partition and state sequence.
    for n in range(3,9):
        for _ in range(4):
            obs=[rng.randrange(2) for _ in range(n)]
            pi=[.4,.6]; A=[[0,1],[1,0]]
            dur=[[.5,.3,.2],[.2,.5,.3]]
            emission=[[.8,.2],[.25,.75]]
            weighted=[]
            def visit(t,prev,seq,prob,segments):
                if t==n:
                    weighted.append((seq,prob,segments));return
                for j in range(2):
                    w=pi[j] if prev is None else A[prev][j]
                    if not w: continue
                    for d in range(1,min(3,n-t)+1):
                        z=w*dur[j][d-1]*math.prod(emission[j][o] for o in obs[t:t+d])
                        visit(t+d,j,seq+[j]*d,prob*z,segments+1)
            visit(0,None,[],1.,0)
            l=sum(w for _,w,_ in weighted)
            best=max(w for _,w,_ in weighted)
            es=sum(w*s for _,w,s in weighted)/l
            last=[sum(w for seq,w,_ in weighted if seq[-1]==j)/l for j in range(2)]
            occ=[sum(w*seq.count(j) for seq,w,_ in weighted)/l for j in range(2)]
            yield 'duration_study',[2,3,2,n]+pi+flatten(A)+flatten(dur)+flatten(emission)+obs,[math.log(l),math.log(best),es]+last+occ,'all complete latent segment paths'
    for n in range(4,10):
        for m in (1,2,3):
            y=[rng.randrange(-3,4) for _ in range(n)]
            lam=.7; best=math.inf
            for cut in it.product((0,1),repeat=n-1):
                ends=[0]+[i+1 for i,b in enumerate(cut) if b]+[n]
                if any(b-a<m for a,b in zip(ends,ends[1:])):continue
                sse=sum(sum((z-st.mean(y[a:b]))**2 for z in y[a:b]) for a,b in zip(ends,ends[1:]))
                best=min(best,sse+lam*(len(ends)-2))
            yield 'segmentation',[m,lam]+y,[best],'all contiguous partitions'
    for n in range(3,9):
        for _ in range(4):
            y=[rng.randrange(2) for _ in range(n)]
            # Uniform beta prior: probability of one particular ordered sequence.
            def marg(a):return 1/((len(a)+1)*math.comb(len(a),sum(a)))
            null=marg(y); terms=[marg(y[:t])*marg(y[t:]) for t in range(1,n)]
            alt=st.mean(terms); post=[v/sum(terms) for v in terms]
            # The mode can be tied in exact arithmetic. Compare all non-mode coordinates.
            expected={0:math.log(null),1:math.log(alt),2:math.log(alt/null),3:alt/(alt+null),5:sum((i+1)*v for i,v in enumerate(post))}
            expected.update({6+i:v for i,v in enumerate(post)})
            yield 'bayes_break',[1,1]+y,expected,'integrated uniform-beta factorial probability'
    for _ in range(20):
        rows=[]
        for i,t in it.product(range(4),range(4)):
            if i==t and i in (1,2):continue
            x=rng.uniform(-2,2);y=1.4*x+2*i-.8*t+rng.uniform(-.2,.2)
            rows.append((i,t,x,y))
        # Full dummy regression: one time dummy omitted for identification.
        X=[[x]+[float(i==a) for a in range(4)]+[float(t==b) for b in range(1,4)] for i,t,x,y in rows]
        Y=[r[3] for r in rows]; cols=list(zip(*X)); k=len(cols)
        normal=[[sum(a*b for a,b in zip(c,d)) for d in cols] for c in cols]
        rhs=[sum(a*b for a,b in zip(c,Y)) for c in cols]
        coef=solve(normal,rhs);res=[y-sum(a*b for a,b in zip(row,coef)) for row,y in zip(X,Y)]
        rss=sum(z*z for z in res);df=len(rows)-k
        covcol=solve(normal,[1]+[0]*(k-1))
        yield 'two_way_within',[4,4]+flatten(rows),[coef[0],rss,df,math.sqrt(rss/df*covcol[0])],'unbalanced full dummy regression'
    # Credit: enumerate individual default indicators.
    for n in range(2,9):
        for _ in range(5):
            p=[rng.randrange(1,10)/10 for _ in range(n)]
            w=[rng.randint(1,4) for _ in p];alpha=.9
            f=bernoulli_loss(p,[1]*n)
            yield 'count_credit',[alpha]+p,[*loss_stats(f,alpha),1,*f],'enumerate heterogeneous default indicators'
            f=bernoulli_loss(p,w);A=2;B=6
            stop=sum(max(i-A,0)*z for i,z in enumerate(f))
            yield 'loss_distribution',[alpha,A]+flatten(zip(w,p)),[*loss_stats(f,alpha),stop,1,*f],'enumerate weighted credit losses'
            payoff=lambda k:min(max(k-A,0),B-A)
            value=sum(payoff(i)*z for i,z in enumerate(f))
            sens=[]
            for i in range(n):
                # Finite differences at p_i=0 and p_i=1 are exact because the expectation is affine.
                up=list(p);dn=list(p);up[i]=1;dn[i]=0
                sens.append(sum(payoff(j)*(a-b) for j,(a,b) in enumerate(zip(bernoulli_loss(up,w),bernoulli_loss(dn,w)))))
            yield 'tranche_sensitivity',[A,B]+flatten(zip(w,p)),[value,value/(B-A),*sens],'endpoint probability sensitivities'
            p2=[min(.95,x+.15) for x in p];f0=bernoulli_loss(p,[1]*n);f1=bernoulli_loss(p2,[1]*n)
            f=[.7*a+.3*b for a,b in zip(f0,f1)];obs=n//2
            mu=sum(i*v for i,v in enumerate(f));var=sum((i-mu)**2*v for i,v in enumerate(f))
            within=.7*sum(x*(1-x) for x in p)+.3*sum(x*(1-x) for x in p2)
            yield 'mixture_count',[2,n,obs,.7,*p,.3,*p2],[mu,within,var-within,var,f[obs],1,.7*f0[obs]/f[obs],.3*f1[obs]/f[obs],*f],'independent systemic-state mixture enumeration'
    # Derivatives: enumerate paths, or recursive exercise decisions without recombining.
    for n in range(1,7):
        for rate in (.98,1.02):
            s,k,u,d=100,95,1.15,.87;q=(rate-d)/(u-d)
            for cp in (-1,1):
                h=90;rebate=2;ko=reb=plain=hit=0
                for bits in it.product((0,1),repeat=n):
                    prob=product_probability(bits,[q]*n);ss=[s]
                    for b in bits:ss.append(ss[-1]*(u if b else d))
                    pay=max(cp*(ss[-1]-k),0)
                    plain+=prob*pay/rate**n
                    touches=[t for t,z in enumerate(ss) if z<=h]
                    if touches:hit+=prob;reb+=prob*rebate/rate**touches[0]
                    else:ko+=prob*pay/rate**n
                yield 'barrier_rebate',[s,k,u,d,rate,n,cp,h,rebate],[ko+reb,ko,reb,plain-ko,plain,hit,0],'first passage path enumeration'
                for gap in (1,2):
                    def option(t,j,r,c):
                        if r==0:return 0.
                        pay=max(cp*(s*u**j*d**(t-j)-k),0)
                        if t==n:return pay if c==0 else 0.
                        def cont(rr,cc):return (q*option(t+1,j+1,rr,cc)+(1-q)*option(t+1,j,rr,cc))/rate
                        wait=cont(r,max(c-1,0))
                        return max(wait,pay+cont(r-1,gap-1)) if c==0 else wait
                    # Full recursive tree for small n; compare values rather than tied exercise flags.
                    value=option(0,0,2,0);single=option(0,0,1,0)
                    yield 'swing_rights',[s,k,u,d,rate,n,cp,2,gap],{0:value,1:single,3:value-single},'non-recombining multiple exercise recursion'
            ev=em=pr=plain=0
            for bits in it.product((0,1),repeat=n):
                prob=product_probability(bits,[q]*n);ss=[s]
                for b in bits:ss.append(ss[-1]*(u if b else d))
                m=max(ss);ev+=prob*max(m-k,0)/rate**n;em+=prob*m;pr+=prob*(m>k);plain+=prob*max(ss[-1]-k,0)/rate**n
            yield 'lookback_max',[s,k,u,d,rate,n,1],{0:ev,1:em,2:pr,3:1,5:plain},'full maximum-path enumeration'
    for n in range(1,8):
        for _ in range(4):
            p=[[.6,.4],[.25,.75]];m=[rng.uniform(.7,1.1),rng.uniform(.7,1.1)];h=[2,-1]
            price=[0.,0.];prob=[0.,0.]
            for js,pr in paths(p,0,n):
                df=math.prod(m[i] for i in (0,)+js[:-1]);price[js[-1]]+=pr*df;prob[js[-1]]+=pr
            yield 'state_price_chain',[2,n,1,0,*m,*h,*flatten(p)],[sum(a*b for a,b in zip(price,h)),sum(price),sum(a*b for a,b in zip(prob,h)),1,*price,*prob],'discount each complete regime path'
    # Integer decisions and path risk.
    for q in range(2,9):
        for risk in (0.,.13,.4):
            n=3;C=[q,q//2+1,q];c=[.1,.3,0];eta=[.6,1.,.8];vol=[.2,.1,.3];best=math.inf
            for a in it.product(*(range(z+1) for z in C)):
                if sum(a)!=q:continue
                rem=q;obj=0
                for t,z in enumerate(a):rem-=z;obj+=c[t]*z+eta[t]*z*z+risk*vol[t]*rem*rem
                best=min(best,obj)
            yield 'inventory_schedule',[q,risk]+flatten(zip(c,eta,vol,C)),{0:best,3:0,4:0},'enumerate all feasible integer schedules'
    for _ in range(30):
        cash=50;a,b=rng.randint(0,4),rng.randint(0,4);p,q=7,11;fee=rng.randint(0,3);w1,w2=.45,.35;gamma=.3
        value=cash+a*p+b*q;best=math.inf
        for i,j in it.product(range(9),repeat=2):
            f=fee*(abs(i-a)+abs(j-b));wealth=value-f;left=wealth-i*p-j*q
            if left<0 or wealth<=0:continue
            e=(i*p/wealth-w1)**2+(j*q/wealth-w2)**2+(left/wealth-1+w1+w2)**2
            best=min(best,e+gamma*f/value)
        yield 'lot_rebalance',[cash,a,b,p,q,fee,w1,w2,gamma,8,8],{6:best,7:0},'integer cash feasibility exhaustive search'
    for n in range(1,6):
        for fee in (0,.02,.08):
            p=[[.75,.25],[.2,.8]];w=[0,.5,1];r=[-.1,.2]
            def val(t,i,a):
                if t==n:return 0.
                return max(math.log(1-fee*abs(w[b]-w[a]))+sum(p[i][j]*(math.log(1+w[b]*r[j])+val(t+1,j,b)) for j in range(2)) for b in range(3))
            v=val(0,0,1)
            yield 'regime_policy',[n,2,3,0,1,fee,*w,*r,*flatten(p)],{0:v,2:v,4:1,5:0},'enumeration of short-horizon decision trees'
    for n in range(2,9):
        for h in (1,2,3):
            p=[[.7,.3],[.4,.6]];moves=[-1,2];step=.03
            hit=wealth=elog=dd=0
            for js,pr in paths(p,0,n):
                a=[0]+list(it.accumulate(moves[j] for j in js))
                drops=[max(a[:i+1])-x for i,x in enumerate(a)]
                hit+=pr*(max(drops)>=h);wealth+=pr*math.exp(step*a[-1]);elog+=pr*step*a[-1];dd+=pr*(1-math.exp(-step*drops[-1]))
            yield 'drawdown_chain',[n,2,0,h,step,*moves,*flatten(p)],[hit,wealth,elog,dd,1],'enumerate every regime return path'
    # Conditional exact tests and sequential stopping probabilities.
    for _ in range(30):
        layers=[];choices=[];obs=0
        for n,k,m in ((5,2,2),(6,3,3)):
            vals=[sum(i<k for i in js) for js in it.combinations(range(n),m)]
            o=rng.choice(vals);obs+=o;layers.extend([n,k,m,o]);choices.append(vals)
        values=[sum(z) for z in it.product(*choices)]
        f=[values.count(i)/len(values) for i in range(sum(min(layers[j+1],layers[j+2]) for j in range(0,len(layers),4))+1)]
        yield 'stratified_exact',layers,[obs,*tail_summary(values,obs),*f],'enumerate stratified treatment assignments'
    for n in range(4,10):
        for _ in range(4):
            m=n//2;y=[rng.randrange(4) for _ in range(n)];rank=midranks(y)
            values=[sum(rank[i] for i in js) for js in it.combinations(range(n),m)]
            obs=sum(rank[:m])
            yield 'rank_sum_exact',[m,*y],[obs,obs-m*(m+1)/2,*tail_summary(values,obs)],'enumerate label permutations with tied ranks'
            z=[rng.choice([-3,-2,-1,1,2,3]) for _ in range(n)];rank=midranks([abs(x) for x in z]);obs=sum(v for x,v in zip(z,rank) if x>0)
            values=[sum(v for b,v in zip(bits,rank) if b) for bits in it.product((0,1),repeat=n)]
            yield 'signed_rank_exact',[0,*z],[n,obs,*tail_summary(values,obs)],'enumerate independent conditional sign assignments'
    for n in range(2,9):
        for p in (.2,.5,.8):
            lo=-1.2;hi=1.4;p0=.3;p1=.7
            a=b=c=et=0
            for bits in it.product((0,1),repeat=n):
                pr=product_probability(bits,[p]*n);lr=0;end=0
                for t,x in enumerate(bits,1):
                    lr+=math.log(p1/p0) if x else math.log((1-p1)/(1-p0))
                    if lr<=lo:a+=pr;end=t;break
                    if lr>=hi:b+=pr;end=t;break
                if not end:c+=pr;end=n
                et+=pr*end
            yield 'sequential_test',[n,p0,p1,lo,hi,p],[a,b,c,et,1],'enumerate all stopped Bernoulli paths'
    # Queue references: event histories instead of probability state compression.
    for n in range(1,6):
        for q in (0,1,2):
            e,c,a,w=.35,.15,.1,.1;p=[e,c,a,w,1-e-c-a-w];cap=3
            fill=withdraw=live=tf=tw=time=0
            for events in it.product(range(5),repeat=n):
                prob=math.prod(p[j] for j in events);pos=q;end=n
                for t,z in enumerate(events,1):
                    if z==0 and pos==0:fill+=prob;tf+=t*prob;end=t;break
                    if z==3:withdraw+=prob;tw+=t*prob;end=t;break
                    if z in (0,1):pos=max(0,pos-1)
                    if z==2:pos=min(cap,pos+1)
                else:live+=prob
                time+=prob*end
            yield 'tagged_queue',[n,cap,q,e,c,a,w],[fill,withdraw,live,tf,tw,time,1],'enumerate tagged queue event histories'
            e=[.3,.5];a=[.1,.2];sw=[.2,.1];c=[.15,.1]
            p=[[*v,1-sum(v)] for v in zip(e,a,sw,c)]
            def visit(t,i,j,pr):
                if t==n:return (0,0,pr,0)
                vals=[0.,0.,0.,pr]
                for z,pp in enumerate(p[i]):
                    zprob=pr*pp
                    if z==0 and j==0:vals[0]+=zprob
                    elif z==1:vals[1]+=zprob
                    else:
                        v=visit(t+1,1-i if z==2 else i,max(0,j-1) if z in (0,3) else j,zprob)
                        vals=[a+b for a,b in zip(vals,v)]
                return tuple(vals)
            f,a0,l,t=visit(0,0,q,1)
            yield 'queue_regime',[n,q,0,*e,*a,*sw,*c,2],[f,a0,l,2*f,t,1],'regime-conditioned full queue tree'
            e,c,a=.4,.15,.1;wait,fc,mc,shock=.04,-.1,1.,.4
            def val(t,j):
                if t==n:return mc
                return min(mc,wait+e*(fc if j==0 else val(t+1,j-1))+c*val(t+1,max(0,j-1))+a*(mc+shock)+(1-e-c-a)*val(t+1,j))
            v=val(0,q)
            yield 'queue_execution_choice',[n,q,e,c,a,wait,fc,mc,shock],{0:v,6:v,7:0},'non-recombining quote decision recursion'
            own=3;p=[.4,.4,.2];f=[0.]*(own+1)
            for b in it.product(range(3),repeat=n):f[min(own,max(0,sum(b)-q))]+=math.prod(p[j] for j in b)
            yield 'queue_batch_fill',[n,q,own,2,*p],[sum(i*v for i,v in enumerate(f)),f[0],f[-1],1,*f],'enumerate batch order size paths'
    # Discrete optimization references.
    for _ in range(40):
        supply=[2,3];demand=[1,2,2];cost=[[rng.randint(-4,8) for _ in demand] for _ in supply]
        best=math.inf
        for r in it.product(*(range(v+1) for v in demand)):
            if sum(r)!=supply[0]:continue
            b=[d-v for d,v in zip(demand,r)]
            best=min(best,sum(v*c for v,c in zip(r,cost[0]))+sum(v*c for v,c in zip(b,cost[1])))
        yield 'transport_flow',[2,3,*supply,*demand,*flatten(cost)],{0:best,1:5,2:0},'enumerate all feasible two-source transportation flows'
    for n in range(3,7):
        for _ in range(5):
            a=[rng.randrange(4) for _ in range(n)];b=[rng.randrange(4) for _ in range(n)];pen=.13;band=2
            def v(i,j):
                here=(a[i]-b[j])**2
                if i==n-1 and j==n-1:return here
                options=[v(i+di,j+dj)+pen*(di!=dj) for di,dj in ((1,1),(1,0),(0,1)) if i+di<n and j+dj<n and abs(i+di-j-dj)<=band]
                return here+min(options,default=math.inf)
            yield 'band_dtw',[n,n,band,pen,*a,*b],{0:v(0,0),4:0},'enumerate every monotone band-limited alignment'
            cost=[[rng.randint(-4,9) for _ in range(n)] for _ in range(n)]
            best=min(sum(cost[i][p[i]] for i in range(n)) for p in it.permutations(range(n)))
            yield 'assignment',[n,*flatten(cost)],{0:best,1:best},'enumerate every bijection'
    for _ in range(35):
        a=[rng.randrange(-4,5) for _ in range(5)];b=[rng.randrange(-4,5) for _ in range(5)]
        x=sorted(a);y=sorted(b);w1=st.mean(abs(i-j) for i,j in zip(x,y));w2=math.sqrt(st.mean((i-j)**2 for i,j in zip(x,y)))
        yield 'wasserstein',[5,5,*flatten((i,.2) for i in a),*flatten((i,.2) for i in b)],[w1,w2,w1,1,0],'equal-mass sorted quantile pairing'
    # Control: enumerate policies and verify forward cash flows and robust probability budgets.
    for n in range(1,5):
        for _ in range(6):
            d=.8
            r=[[rng.uniform(-1,2) for _ in range(2)] for _ in range(2)]
            p=[[[.8,.2],[.2,.8]],[[.6,.4],[.1,.9]]]
            x=[n,2,2,0,d]+flatten([r[i][a],*p[i][a]] for i,a in it.product(range(2),repeat=2))
            # Exhaustive time-dependent Markov policies (four per date).
            best=-math.inf
            for seq in it.product(list(it.product(range(2),repeat=2)),repeat=n):
                value=0
                for js in it.product(range(2),repeat=n):
                    prob=1;gain=0;state=0
                    for t,j in enumerate(js):
                        a=seq[t][state];gain+=d**t*r[state][a];prob*=p[state][a][j];state=j
                    value+=prob*gain
                best=max(best,value)
            yield 'mdp_optimal',x,{0:best,1:best,2:0,3:1},'enumerate all finite feedback policies and paths'
            best=-math.inf
            for a in it.product(range(2),repeat=2):
                M=[[(i==j)-d*p[i][a[i]][j] for j in range(2)] for i in range(2)]
                v=solve(M,[r[i][a[i]] for i in range(2)])
                best=max(best,v[0])
            yield 'policy_iteration',x,{0:best,1:0,2:0},'enumerate all stationary policies and solve linear systems'
            g=[rng.uniform(0,2),rng.uniform(1,3)];p0=[[.6,.4],[.3,.7]];c=.1
            def v(t,i):
                return g[i] if t==n else max(g[i],-c+d*sum(p0[i][j]*v(t+1,j) for j in range(2)))
            val=v(0,0)
            yield 'stopping_policy',[n,2,0,d,c,*g,*flatten(p0)],{0:val,4:val,5:0,6:1},'unmerged optimal stopping decision tree'
    for a,b in it.product(range(5),repeat=2):
        if a+b>10:continue
        p=[a/10,b/10,1-(a+b)/10];v=[0,1,3]
        for e in (.1,.3,.5):
            possible=[]
            for i in range(11):
                for j in range(11-i):
                    q=[i/10,j/10,1-(i+j)/10]
                    if sum(abs(x-y) for x,y in zip(p,q))/2<=e+1e-12:possible.append(sum(x*y for x,y in zip(q,v)))
            nominal=sum(x*y for x,y in zip(p,v));worst=min(possible)
            yield 'worst_tv',[e,*flatten(zip(p,v))],{0:nominal,1:worst,3:nominal-worst},'enumerate simplex mass grid for rational TV budget'

if __name__=='__main__':
    import json
    import sys
    for method,args,expected,design in experiments():
        if len(sys.argv)==1 or method==sys.argv[1]:
            print(json.dumps(dict(method=method,input=args,expected=expected,design=design),ensure_ascii=False))
