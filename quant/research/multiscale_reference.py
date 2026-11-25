"""Independent numerical studies for multiscale research; synthetic inputs only."""
import itertools as it
import math
import random
import statistics as st
from collections import defaultdict

def flat(rows):return [z for row in rows for z in row]
def pv(x):return st.pvariance(x)
def cov(x,y):return sum((a-st.mean(x))*(b-st.mean(y)) for a,b in zip(x,y))/len(x)
def cdf(x):return .5*math.erfc(-x/math.sqrt(2))
def pdf(x):return math.exp(-x*x/2)/math.sqrt(2*math.pi)
def choose(x,k):return list(it.combinations(x,k))

def bs(s,k,r,v,t):
    a=(math.log(s/k)+(r+v*v/2)*t)/(v*math.sqrt(t))
    return s*cdf(a)-k*math.exp(-r*t)*cdf(a-v*math.sqrt(t))

def coupled(s,k,r,v,T,z):
    n=len(z);w=[0]+list(it.accumulate(z))
    p=[s*math.exp((r-v*v/2)*T*j/n+v*math.sqrt(T/n)*w[j]) for j in range(1,n+1)]
    a,b=st.mean(p),st.mean(p[1::2]);df=math.exp(-r*T)
    f,g=df*max(a-k,0),df*max(b-k,0)
    return [a,b,f,g,f-g,p[-1],p[-1],0]

def hy_ref(a,b,shift=0):
    da=[(t,s,math.log(q/p)) for (t,p),(s,q) in zip(a,a[1:])]
    db=[(t+shift,s+shift,math.log(q/p)) for (t,p),(s,q) in zip(b,b[1:])]
    pairs=[(u*v,min(s,r)-max(t,l)) for t,s,u in da for l,r,v in db if min(s,r)>max(t,l)]
    return [sum(z for z,_ in pairs),sum(v*v for _,_,v in da),sum(v*v for _,_,v in db),len(pairs),sum(w for _,w in pairs)]

def refresh_ref(a,b):
    t=max(a[0][0],b[0][0]);nodes=[t]
    while True:
        u=[s for s,p in a if s>t];v=[s for s,p in b if s>t]
        if not u or not v:break
        t=max(min(u),min(v));nodes.append(t)
    if len(nodes)<2:return None
    y=[math.log(max((s,p) for s,p in a if s<=t)[1]) for t in nodes]
    z=[math.log(max((s,p) for s,p in b if s<=t)[1]) for t in nodes]
    dy=[b-a for a,b in zip(y,y[1:])];dz=[b-a for a,b in zip(z,z[1:])]
    return [sum(a*b for a,b in zip(dy,dz)),sum(a*a for a in dy),sum(b*b for b in dz),len(nodes),nodes[-1]-nodes[0],*nodes]

def rebalance_ref(cash,fee,rows):
    # Enumerate the intervals on which every absolute value has a fixed sign.
    h=[q*p for q,p,w,pn in rows];w=[r[2] for r in rows];v=cash+sum(h)
    nodes=sorted({0.,v,*[a/b for a,b in zip(h,w) if b>0 and 0<a/b<v]})
    for l,u in zip(nodes,nodes[1:]):
        sign=[1 if b*(l+u)/2>=a else -1 for a,b in zip(h,w)]
        root=(v+fee*sum(s*a for s,a in zip(sign,h)))/(1+fee*sum(s*b for s,b in zip(sign,w)))
        if l-1e-10<=root<=u+1e-10:break
    else:raise ValueError('no feasible wealth root')
    tr=sum(abs(b*root-a) for a,b in zip(h,w));cost=fee*tr
    q=[root*b/r[1] for b,r in zip(w,rows)];c=root*(1-sum(w));end=c+sum(a*r[3] for a,r in zip(q,rows))
    return [v,root,cost,tr,c,end,v-root-cost,*q]

def rank_weights(k,g,rows):
    elig=[i for i,(a,s) in enumerate(rows) if a]
    lo=sorted(rows[i][1] for i in elig)[k-1];hi=sorted(rows[i][1] for i in elig)[-k]
    if not lo<hi:return None
    def all_tied(side):
        bound=hi if side==1 else lo
        strict=[i for i in elig if side*(rows[i][1]-bound)>0]
        ties=[i for i in elig if rows[i][1]==bound]
        return [set(strict)|set(c) for c in it.combinations(ties,k-len(strict))]
    top,bottom=all_tied(1),all_tied(-1)
    w=[g/2/k*(sum(i in s for s in top)/len(top)-sum(i in s for s in bottom)/len(bottom)) for i in range(len(rows))]
    return [sum(w),sum(map(abs,w)),lo,hi,sum(a*b[1] for a,b in zip(w,rows)),*w]

def sign_ref(rows):
    cols=list(zip(*rows));k=len(cols);n=len(rows)
    den=[math.sqrt(sum(v*v for v in col)) for col in cols]
    obs=[abs(sum(col))/d for col,d in zip(cols,den)]
    perm=[[abs(sum(s*v for s,v in zip(sign,col)))/d for col,d in zip(cols,den)] for sign in it.product((-1,1),repeat=n)]
    raw=[sum(row[j]>=obs[j]-1e-12 for row in perm)/len(perm) for j in range(k)]
    single=[sum(max(row)>=v-1e-12 for row in perm)/len(perm) for v in obs]
    order=sorted(range(k),key=lambda j:-obs[j]);step=[0.]*k;prior=0
    for i,j in enumerate(order):
        p=sum(max(row[t] for t in order[i:])>=obs[j]-1e-12 for row in perm)/len(perm)
        prior=max(prior,p);step[j]=prior
    return [len(perm),*obs,*raw,*single,*step]

def hac_ref(x,lag):
    # Direct quadratic form of the Bartlett Toeplitz kernel.
    m=st.mean(x);z=[a-m for a in x]
    return sum(a*b*max(0,1-abs(i-j)/(lag+1)) for i,a in enumerate(z) for j,b in enumerate(z))/len(z)

def sharpe_ref(r,lag):
    m=st.mean(r);q=st.mean(z*z for z in r);v=q-m*m;s=math.sqrt(v)
    # Numerical derivatives of the moment functional independently of the symbolic gradient.
    eps=1e-5*min(v,1)
    f=lambda a,b:a/math.sqrt(b-a*a)
    a=(f(m+eps,q)-f(m-eps,q))/(2*eps);b=(f(m,q+eps)-f(m,q-eps))/(2*eps)
    z=[a*(x-m)+b*(x*x-q) for x in r];lr=hac_ref(z,lag);se=math.sqrt(max(0,lr)/len(r))
    return [m/s,m,s,se,math.sqrt(hac_ref(z,0)/len(r)),lr,m/s-1.959963984540054*se,m/s+1.959963984540054*se]

def rank_ic(a,b):
    r=[(1+sum(z<x for z in a)+sum(z<=x for z in a))/2 for x in a]
    s=[(1+sum(z<x for z in b)+sum(z<=x for z in b))/2 for x in b]
    return cov(r,s)/math.sqrt(pv(r)*pv(s))

def studies(seed=20261125):
    rng=random.Random(seed)
    # Independent allocation and sample moments.
    for _ in range(40):
        l=3;v=[rng.uniform(.1,2) for _ in range(l)];c=[1,3,8];e=rng.uniform(.12,.45)
        h=sum(math.sqrt(a*b) for a,b in zip(v,c));n=[max(2,math.ceil(h*math.sqrt(a/b)/e**2)) for a,b in zip(v,c)]
        yield 'mlmc_plan',[e,*flat(zip(v,c))],[sum(a/b for a,b in zip(v,n)),sum(a*b for a,b in zip(c,n)),h*h/e**2,h,*n],'continuous allocation plus integer feasibility'
        rows=[];means=[];variances=[];counts=[];cost=0
        for j in range(l):
            n=rng.randrange(3,10);a=[rng.uniform(-1,4) for _ in range(n)];b=[0.]*n if j==0 else [z+rng.uniform(-.5,.5) for z in a];d=[u-v for u,v in zip(a,b)]
            rows.extend([n,c[j],*flat(zip(a,b))]);means.append(st.mean(d));variances.append(st.variance(d));counts.append(n);cost+=n*c[j]
        se=sum(a/b for a,b in zip(variances,counts))
        yield 'mlmc_estimate',[l,*rows],[sum(means),se,math.sqrt(se),cost,*means,*variances,*counts],'direct layer sample statistics'
    for n in (2,4,8,16,32):
        for _ in range(12):
            s,k,r,v,T=100,rng.uniform(85,115),.03,rng.uniform(.1,.5),1.2;z=[rng.gauss(0,1) for _ in range(n)]
            yield 'asian_coupling',[s,k,r,v,T,*z],coupled(s,k,r,v,T,z),'independent cumulative Brownian trajectory'
    for n in (1,2,3,5,8):
        for _ in range(8):
            t=[(i+1)/n for i in range(n)];w=[rng.uniform(.2,1) for i in t];w=[x/sum(w) for x in w];s=100;r=.02;sig=rng.uniform(.15,.5)
            mu=[math.log(s)+(r-sig*sig/2)*z for z in t]
            def moment(a):
                return math.exp(sum(x*y for x,y in zip(a,mu))+.5*sig*sig*sum(a[i]*a[j]*min(t[i],t[j]) for i in range(n) for j in range(n)))
            basis=[[float(i==j) for j in range(n)] for i in range(n)]
            ea=sum(a*moment(b) for a,b in zip(w,basis));eg=moment(w)
            va=sum(w[i]*w[j]*moment([a+b for a,b in zip(basis[i],basis[j])]) for i in range(n) for j in range(n))-ea*ea
            vg=moment([2*a for a in w])-eg*eg;co=sum(w[i]*moment([a+b for a,b in zip(basis[i],w)]) for i in range(n))-ea*eg
            yield 'asian_moments',[s,r,sig,*flat(zip(t,w))],[ea,va,eg,vg,co,co/vg,max(0,va-co*co/vg)],'joint normal moment generating function'
    for sigma in (.15,.25,.4):
        for K in (85,100,115):
            for cuts in ([-1,0,1],[-.5,.5]):
                S,r,T=100,.03,1;bounds=[-12,*cuts,12];k=(math.log(K/S)-(r-sigma*sigma/2)*T)/(sigma*math.sqrt(T))
                def integral(a,b,j):
                    # Independent quadrature of the actual payoff density.
                    l=max(a,k);u=b
                    if l>=u:return 0.
                    n=4096;h=(u-l)/n
                    def f(z):return (math.exp(-r*T)*max(S*math.exp((r-sigma*sigma/2)*T+sigma*math.sqrt(T)*z)-K,0))**j*pdf(z)
                    return h/3*(f(l)+f(u)+4*sum(f(l+h*i) for i in range(1,n,2))+2*sum(f(l+h*i) for i in range(2,n,2)))
                rows=[];mu=second=within=0
                for a,b in zip(bounds,bounds[1:]):
                    p=cdf(b)-cdf(a);u,v=integral(a,b,1),integral(a,b,2);vv=max(0,v/p-(u/p)**2);rows.extend([p,u/p,vv]);mu+=u;second+=v;within+=p*vv
                yield 'stratified_call',[S,K,r,sigma,T,*cuts],[mu,second-mu*mu,within,1,*rows],'numerical integration over disjoint payoff regions'
    for _ in range(35):
        S,K,r,sigma,T=100,95,.02,.25,1;z=[rng.uniform(-2,2) for _ in range(12)];df=math.exp(-r*T);eps=1e-4
        def price(s,v,x):return s*math.exp((r-v*v/2)*T+v*math.sqrt(T)*x)
        def pay(s,v,x):return df*max(price(s,v,x)-K,0)
        values=[[] for _ in range(6)]
        for x in z:
            p=price(S,sigma,x);y=pay(S,sigma,x)
            def density_log(s,v):
                z0=(math.log(p/s)-(r-v*v/2)*T)/(v*math.sqrt(T))
                return -math.log(v)-z0*z0/2
            ds=(density_log(S+eps,sigma)-density_log(S-eps,sigma))/(2*eps)
            dv=(density_log(S,sigma+eps*1e-2)-density_log(S,sigma-eps*1e-2))/(2*eps*1e-2)
            sample=[y,(pay(S+eps,sigma,x)-pay(S-eps,sigma,x))/(2*eps),y*ds,(pay(S,sigma+eps*1e-2,x)-pay(S,sigma-eps*1e-2,x))/(2*eps*1e-2),y*dv,df*(p>K)*ds]
            for a,b in zip(values,sample):a.append(b)
        out=flat((st.mean(a),st.variance(a)) for a in values)
        # Only numerical finite differences, not repeated analytic C++ score formulas.
        yield 'greek_estimators',[S,K,r,sigma,T,*z],out,'common-random-number finite differences of payoff and density'
    for n in (2,3,5,8):
        for _ in range(10):
            h=80;sigma=.25;r=.02;K=95;S=100;times=[(i+1)/n for i in range(n)];p=[rng.uniform(75,120) for _ in times]
            q=[max(0,1-math.exp(-2*math.log(a/h)*math.log(b/h)/(sigma*sigma*(v-u)))) if min(a,b)>h else 0 for a,b,u,v in zip([S,*p[:-1]],p,[0,*times[:-1]],times)]
            pay=math.exp(-r)*max(p[-1]-K,0);alive=all(a>h for a in [S,*p]);surv=math.prod(q)
            yield 'bridge_path',[h,sigma,r,K,S,1,*flat(zip(times,p))],[pay if alive else 0,pay*surv,surv,pay,*q],'conditional interval product'
    for _ in range(35):
        n=8;rows=[(rng.uniform(-1,1),rng.uniform(-1,1)) for _ in range(2*n)];p,q=zip(*rows[:n]);y,z=zip(*rows[n:]);beta=cov(p,q)/pv(q);a=[v-beta*c for v,c in zip(y,z)]
        yield 'frozen_control',[n,0,*flat(rows)],[beta,st.mean(y),st.mean(a),st.variance(y),st.variance(a),st.variance(y)-st.variance(a),st.mean(z),n],'independent pilot and external control regression'
    # Exhaustive overlap, direct time searches and unit-volume splitting.
    for _ in range(60):
        a=[(i,math.exp(rng.uniform(-.3,.3))) for i in (0,2,4,7,10)]
        b=[(i,math.exp(rng.uniform(-.3,.3))) for i in (0,1,3,6,9,10)]
        raw=[len(a),len(b),*flat(a),*flat(b)]
        yield 'hy_covariance',raw,hy_ref(a,b),'all interval pairs'
        yield 'refresh_covariance',raw,refresh_ref(a,b),'direct next-time and previous-tick searches'
        grid=[-2,-1,0,1,2];z=[hy_ref(a,b,g) for g in grid];best=max(range(len(z)),key=lambda i:abs(z[i][0]))
        yield 'hy_shift_grid',[len(grid),*grid,*raw],[grid[best],z[best][0],abs(z[best][0]),*flat([g,v[0],v[3],v[4]] for g,v in zip(grid,z))],'all shifted interval pairs'
    for n in (4,6,10,20,40):
        for _ in range(12):
            a=[rng.uniform(-.04,.04) for _ in range(n)];rv=sum(v*v for v in a);bv=math.pi/2*n/(n-1)*sum(abs(x*y) for x,y in zip(a,a[1:]))
            yield 'bipower_variation',a,[rv,bv,rv-bv,max(0,rv-bv),n/3*sum(v**4 for v in a)],'direct sum definition'
            m=st.mean(a);v=pv(a);c=sum((x-m)*(y-m) for x,y in zip(a,a[1:]))/(n-1)
            yield 'noise_moments',a,[m,v,c,-c,v+2*c,max(0,-c),max(0,v+2*c)],'two population moment equations'
            p=list(it.accumulate([0,*a]));q=list(it.accumulate([0,*[rng.uniform(-.05,.05) for _ in a]]));k=rng.randint(1,n);u,v,c=.0001,.0002,.00005
            r=[p[j+k]-p[j] for j in range(n+1-k)];s=[q[j+k]-q[j] for j in range(n+1-k)];scale=n/((n+1-k)*k)
            a0,b0,c0=scale*sum(z*z for z in r),scale*sum(z*z for z in s),scale*sum(z*w for z,w in zip(r,s))
            yield 'lag_cov_noise',[k,u,v,c,*flat(zip(p,q))],[a0,b0,c0,a0-2*n*u/k,b0-2*n*v/k,c0-2*n*c/k,n+1-k,scale],'explicit lagged price increments and bias'
            k=rng.randint(2,n-1);g=[rng.uniform(-1,1) for _ in range(k)]
            # Compute noise loadings via the return-to-price incidence matrix.
            B=sum(sum(g[j]*((i==j+1)-(i==j)) for j in range(k))**2 for i in range(k+1));A=sum(z*z for z in g);raw=sum(sum(g[j]*a[i+j] for j in range(k))**2 for i in range(n-k+1));m=n-k+1;fac=n/(m*A)
            yield 'weighted_preaverage',[k,u,*g,*a],[raw,A,B,m*u*B,raw*fac,(raw-m*u*B)*fac,m,fac],'incidence-matrix noise propagation'
    for _ in range(50):
        target=rng.randrange(3,11);events=[(i,rng.randrange(8,16),rng.randrange(1,15),rng.choice((-1,1))) for i in range(8)]
        units=[(t,p,s) for t,p,q,s in events for _ in range(q)];bars=[units[i:i+target] for i in range(0,len(units),target)]
        z=[len(bars),len(units),sum(p for _,p,_ in units),sum(s for _,_,s in units),0]
        for b in bars:
            p=[r[1] for r in b];z.extend([b[0][0],b[-1][0],p[0],max(p),min(p),p[-1],len(b),sum(r[2] for r in b),st.mean(p),int(len(b)==target)])
        yield 'volume_bars',[target,*flat(events)],z,'explicit unit-size transaction expansion'
    # Causal data vintages, execution and cash ledgers.
    for _ in range(40):
        n=5;dec=list(range(1,8));r=[]
        for i in range(n):
            r.extend([(i,0,0,rng.randrange(2)),(i,2,3,rng.randrange(2)),(i,2,5,rng.randrange(2)),(i,6,4,rng.randrange(2))])
        rng.shuffle(r);out=[]
        for t in dec:
            v=[]
            for i in range(n):
                z=sorted((e,a,f) for j,e,a,f in r if j==i and e<=t and a<=t);v.append(z[-1][2] if z else 0)
            out.extend([sum(v),*v])
        yield 'membership_vintage',[n,len(dec),len(r),*dec,*flat(r)],out,'filter and sort complete vintage history'
        rows=[(rng.randint(0,4),rng.randint(5,15),rng.uniform(0,.2),rng.randint(6,16)) for _ in range(3)];cash=50;fee=rng.uniform(0,.03)
        yield 'fee_rebalance',[cash,fee,*flat(rows)],rebalance_ref(cash,fee,rows),'piecewise-affine fee equation, not bisection'
        rows=[(1,rng.randrange(5)) for _ in range(8)]+[(0,100)]
        result=rank_weights(2,2,rows)
        if result:yield 'fractional_rank',[2,2,*flat(rows)],result,'enumeration of boundary tie subsets'
    for _ in range(40):
        n=3;t=5;qty=[rng.randint(1,4) for _ in range(n)];prices=[10,20,30];cash=20;initial=cash+sum(a*b for a,b in zip(qty,prices));active=[True]*n;data=[];div=delist=0;out=[]
        for j in range(t):
            stale=0
            for i in range(n):
                if not active[i]:data.extend([0,0,0]);continue
                state=2 if j==2 and i==1 else (0 if j==3 and i==0 else 1)
                if state==0:data.extend([0,0,0]);stale+=1;continue
                p=rng.randint(5,35);d=rng.randint(0,2);cash+=qty[i]*d;div+=qty[i]*d;data.extend([state,p,d])
                if state==2:cash+=p*qty[i];delist+=p*qty[i];active[i]=False
                else:prices[i]=p
            wealth=cash+sum(q*p for q,p,a in zip(qty,prices,active) if a);out.extend([wealth,cash,sum(active),stale])
        yield 'delist_wealth',[n,t,20,*flat(zip(qty,[10,20,30])),*data],[wealth,cash,wealth-initial,div,delist,wealth/initial-1,*out],'explicit dividend and terminal cash accounting'
        orders=[(0,1,4,3,12),(1,1,4,2,11),(1,2,5,-4,9)]
        quotes=[(t,9+rng.randint(0,1),11+rng.randint(0,1),rng.randint(0,4),rng.randint(0,4)) for t in range(1,6)]
        fills=[[] for _ in orders]
        for t,b,a,bs,ass in quotes:
            for side,price,capacity in ((1,a,ass),(-1,b,bs)):
                for _ in range(capacity):
                    eligible=[i for i,(s,d,e,q,l) in enumerate(orders) if s+d<=t<=e and side*q>0 and len(fills[i])<abs(q) and side*(price-l)<=0]
                    if not eligible:break
                    fills[eligible[0]].append((t,price))
        out=[sum(len(fills[i]) for i,o in enumerate(orders) if o[3]>0),sum(len(fills[i]) for i,o in enumerate(orders) if o[3]<0),sum(p for f in fills for _,p in f)]
        for o,f in zip(orders,fills):out.extend([len(f),abs(o[3])-len(f),sum(p for _,p in f),st.mean(p for _,p in f) if f else 0,f[0][0] if f else -1,f[-1][0] if f else -1])
        yield 'delayed_orders',[len(orders),len(quotes),*flat(orders),*flat(quotes)],out,'unit-lot allocation under shared event capacities'
        records=[(i,i,i+1,i+2,i+1,2*(i+1)+rng.uniform(-.2,.2)) for i in range(1,8)];now=8;query=9;lam=.7;embargo=rng.choice((0,1,2))
        train=[(x,y) for t,f,e,a,x,y in records if a<=now-embargo];xx,yy=zip(*train);mu,ym=st.mean(xx),st.mean(yy);scale=math.sqrt(pv(xx));beta=sum((x-mu)*(y-ym) for x,y in train)/(sum((x-mu)**2 for x,y in train)+lam*scale**2)
        expected=[ym+beta*(query-mu),len(xx),mu,scale,ym,beta*scale,beta,sum((y-ym-beta*(x-mu))**2 for x,y in train)]
        yield 'matured_ridge',[now,query,lam,embargo,len(records),*flat(records)],expected,'raw-scale penalized regression'
        modified=[(t,f,e,a,x,y if a<=now-embargo else y+10000) for t,f,e,a,x,y in records]
        yield 'matured_ridge',[now,query,lam,embargo,len(modified),*flat(modified)],expected,'unavailable-label perturbation invariance'
        lag=rng.choice((0,1,2));cash0=80;shares=2;fills=[];accepted=[];out=[];fee0=0
        tr=[(i,rng.choice((-2,-1,1,2,3)),rng.randrange(8,16),1) for i in range(8)]
        for t,q,p,f in tr:
            settled=cash0+sum(cf for d,cf in fills if d<=t);reserve=sum(-cf for d,cf in fills if d>t and cf<0);cf=-q*p-f
            ok=shares+q>=0 and max(0,-cf)<=settled-reserve
            accepted.append(ok)
            if ok:shares+=q;fee0+=f;fills.append((t+lag,cf))
            settled=cash0+sum(cf for d,cf in fills if d<=t);reserve=sum(-cf for d,cf in fills if d>t and cf<0);pending=sum(cf for d,cf in fills if d>t)
            out.extend([int(ok),settled,reserve,settled-reserve,pending,shares])
        cash=cash0+sum(v for _,v in fills)
        yield 'settlement_cash',[cash0,2,lag,*flat(tr)],[cash,shares,fee0,sum(accepted),len(tr)-sum(accepted),cash-cash0,*out],'explicit accepted-trade cashflow list'
        revisions=[(i,i+1,i,10+i,2,1) for i in range(6)]+[(1,7,1,13,3,1),(2,8,2,0,0,0)]
        rng.shuffle(revisions);queries=[(t,0,min(t,6)) for t in (3,5,6,7,8)];out=[]
        for t,l,u in queries:
            d={}
            for a in sorted(revisions,key=lambda z:z[1]):
                if a[1]<=t:d[a[0]]=a
            z=[a for a in d.values() if a[-1] and l<=a[2]<u];vol=sum(a[4] for a in z);nt=sum(a[3]*a[4] for a in z)
            out.extend([len(z),vol,nt,nt/vol if vol else 0])
        yield 'trade_revisions',[len(revisions),len(queries),*flat(revisions),*flat(queries)],out,'versioned event dictionary replay'
    # Orthogonal design gives analytic pricing and spanning reference.
    for _ in range(50):
        a,b=rng.uniform(-.5,.5),rng.uniform(-.5,.5);s,t=rng.uniform(.3,1.5),rng.uniform(.3,1.5);m=rng.uniform(.8,1.2)
        rows=[(a+s*i,b+t*j) for i,j in it.product((-1,1),repeat=2)];z=[a/s**2,b/t**2];sr=math.sqrt((a/s)**2+(b/t)**2)
        sdf=[m*(1-z[0]*(u-a)-z[1]*(v-b)) for u,v in rows]
        yield 'hj_bound',[2,4,m,*flat(rows)],[sr,m*sr,m,m*sr,sum(q<0 for q in sdf)/4,0,*z,0,0],'orthogonal two-factor analytic minimum variance'
        alpha=rng.uniform(-.3,.3);beta=rng.uniform(-1,1);res=rng.uniform(.3,1)
        y=[(f,alpha+beta*f+res*j) for f,j in ((a-s,-1),(a-s,1),(a+s,-1),(a+s,1))];base=(a/s)**2;add=(alpha/res)**2
        yield 'factor_spanning',[1,4,*flat(y)],[base,base+add,add,alpha,res**2,0,beta],'orthogonal residual Sharpe decomposition'
        alpha,beta,gamma=[rng.uniform(-2,2) for _ in range(3)];xx=[-2,-1,0,1,2]
        yield 'timing_regression',flat((z,alpha+beta*z+gamma*z*z) for z in xx),[alpha,beta,gamma,0,2,0,beta-4*gamma,beta+4*gamma],'known polynomial regression'
        # Disjoint state payoffs make G diagonal: direct conditional mean correction.
        q=[rng.uniform(.1,.5),rng.uniform(.1,.5)];m=[rng.uniform(.5,1.5) for _ in range(4)];pay=[(1,0),(1,0),(0,1),(0,1)];err=[(m[0]+m[1])/4-q[0],(m[2]+m[3])/4-q[1]];new=[m[i]-2*err[i//2] for i in range(4)];d2=2*sum(a*a for a in err)
        yield 'sdf_distance',[2,4,*q,*flat([m0,*p] for m0,p in zip(m,pay))],[math.sqrt(d2),d2,0,sum(z<0 for z in new)/4,*err,0,0,*new],'disjoint-payoff projection in closed form'
    for n in (4,5,6,7,8,9):
        for _ in range(8):
            rows=[[rng.randrange(-4,5)+.2 for _ in range(4)] for t in range(n)]
            yield 'joint_sign_stepdown',[n,4,*flat(rows)],sign_ref(rows),'full joint-sign permutation matrix'
            p=[rng.uniform(0,1) for _ in range(8)];o=sorted(p);h=sum(1/i for i in range(1,9))
            bh=[min(1,min(8*v/(j+1) for j,v in enumerate(o) if v>=a)) for a in p];by=[min(1,h*v) for v in bh]
            yield 'fdr_stepup',[.15,*p],[sum(v<=.15 for v in bh),sum(v<=.15 for v in by),h,*bh,*by],'direct sorted suffix minima'
            r=[rng.uniform(-1,1) for _ in range(12)];lag=rng.randrange(4)
            yield 'sharpe_hac',[lag,*r],sharpe_ref(r,lag),'finite difference of moment functional and dense Bartlett kernel'
            panel=[]
            for t in range(5):
                k=4+t%3;a=list(range(k));b=[(i+t)%k for i in range(k)];rng.shuffle(b);panel.append(list(zip(a,b)))
            ic=[rank_ic(*zip(*p)) for p in panel];m=st.mean(ic);sd=st.stdev(ic);lag=2
            yield 'rank_ic_panel',[5,lag,*flat([len(p),*flat(p)] for p in panel)],[m,sd,m/sd if sd else 0,math.sqrt(max(0,hac_ref(ic,lag))/5),sum(z>0 for z in ic)/5,5,*ic],'pairwise comparison ranks and dense Bartlett kernel'

if __name__=='__main__':
    import json
    import sys
    for m,a,e,d in studies():
        if len(sys.argv)==1 or m==sys.argv[1]:
            print(json.dumps(dict(method=m,input=a,expected=e,design=d),ensure_ascii=False))
