"""Independent reference calculations: constrained allocation, interval combinations and exhaustive blocks."""
import itertools as it
import math
import random
import statistics as st

def studies(seed=20261126):
    rng=random.Random(seed)
    for _ in range(50):
        e=rng.uniform(.12,.7); t=rng.uniform(.25,.75); a=rng.uniform(.7,1.5); b=rng.uniform(.6,1.5); g=rng.uniform(.6,1.5)
        L=next(l for l in range(21) if 2**(-a*l)<=e*math.sqrt(1-t))
        v=[.4*2**(-b*l) for l in range(L+1)]; c=[2*2**(g*l) for l in range(L+1)]
        h=sum(math.sqrt(x*y) for x,y in zip(v,c))
        N=[max(2,math.ceil(h*math.sqrt(x/y)/(e*e*t))) for x,y in zip(v,c)]
        s=sum(x/n for x,n in zip(v,N)); bias=2**(-a*L)
        yield 'mlmc_bias_budget',[e,t,1,a,.4,b,2,g,20],[L,bias,s,bias*bias+s,sum(x*n for x,n in zip(c,N)),h*h/(e*e*t),*N],'closed-form constraint'
    for _ in range(80):
        k=rng.randint(2,4); lo=[rng.randint(1,2) for j in range(k)]; hi=[n+rng.randint(1,5) for n in lo]
        N=rng.randint(sum(lo),sum(hi)); hi=[min(n,N) for n in hi]
        p=[rng.randint(1,6) for j in range(k)]; p=[x/sum(p) for x in p]; v=[rng.uniform(.2,4) for j in range(k)]
        a=[x*x*y for x,y in zip(p,v)]
        obj=min(sum(x/y for x,y in zip(a,n)) for n in it.product(*(range(l,h+1) for l,h in zip(lo,hi))) if sum(n)==N)
        args=[N,*it.chain.from_iterable(zip(p,v,lo,hi))]
        yield 'neyman_integer',args,{0:obj,1:sum(math.sqrt(x) for x in a)**2/N,2:N},'all feasible integer allocations'
    for k in range(2,65):
        w=[math.sqrt(2/(k+1))*math.sin(math.pi*i/(k+1)) for i in range(1,k+1)]
        A=sum(x*x for x in w); B=sum((y-x)**2 for x,y in zip([0,*w],[*w,0]))
        yield 'preavg_optimal',[k],[A,B/A,2-2*math.cos(math.pi/(k+1)),0,*w],'Dirichlet boundary spectrum'
    for n in (4,5,8,12):
        for k in range(2,n):
            x=[rng.uniform(-.1,.1) for i in range(n)]; y=[rng.uniform(-.1,.1) for i in range(n)]
            def r(h):
                dx=[x[j+h]-x[j] for j in range(n-h)]; dy=[y[j+h]-y[j] for j in range(n-h)]
                f=(n-1)/(h*(n-h))
                return [f*sum(z*z for z in dx),f*sum(z*z for z in dy),f*sum(a*b for a,b in zip(dx,dy))]
            a,b=r(1),r(k); v=[(k*z-w)/(k-1) for w,z in zip(a,b)]; e=[(w-z)/(2*(n-1)*(1-1/k)) for w,z in zip(a,b)]
            yield 'two_scale_unknown',[k,*it.chain.from_iterable(zip(x,y))],[*v,*e,v[0]*v[1]-v[2]**2,e[0]*e[1]-e[2]**2,*a,*b],'independent moment inversion'
    for N in range(3,8):
        for K in range(1,N):
            for gap in (0,1,2):
                records=[(g,2*g+j,2*g+j+rng.randint(1,4)) for g in range(N) for j in range(2)]
                folds=list(it.combinations(range(N),K)); phi=math.comb(N-1,K-1); out=[len(folds),phi,N,K]
                for fold in folds:
                    test=[(a,b) for g,a,b in records if g in fold]; counts=[len(test),0,0,0]
                    for g,a,b in records:
                        if g in fold:continue
                        if any(max(a,c)<min(b,d) for c,d in test):counts[2]+=1
                        elif any(d<=a<d+gap for c,d in test):counts[3]+=1
                        else:counts[1]+=1
                    out.extend(counts)
                slots=[[f for f,fold in enumerate(folds) if g in fold] for g in range(N)]
                out.extend(it.chain.from_iterable(zip(*slots)))
                yield 'cpcv_intervals',[N,K,gap,*it.chain.from_iterable(records)],out,'combination and pairwise interval enumeration'
    for _ in range(60):
        rows=[(i,j,rng.uniform(-2,2),rng.uniform(-1,1)) for i in range(4) for j in range(3) if (i,j)!=(1,2)]
        x=[r[2] for r in rows]; y=[r[3] for r in rows]; xm=st.mean(x); ym=st.mean(y)
        sxx=sum((z-xm)**2 for z in x); b=sum((a-xm)*(c-ym) for a,c in zip(x,y))/sxx; a=ym-b*xm
        e=[z-a-b*w for w,z in zip(x,y)]; u=[(z-xm)*r for z,r in zip(x,e)]
        def meat(f):return sum(u[i]*u[j] for i,r in enumerate(rows) for j,s in enumerate(rows) if f(r,s))/sxx**2
        v1=meat(lambda r,s:r[0]==s[0]); v2=meat(lambda r,s:r[1]==s[1]); v12=meat(lambda r,s:r[:2]==s[:2]); v=v1+v2-v12
        yield 'two_way_cluster',list(it.chain.from_iterable(rows)),[a,b,sum(z*z for z in e),v1,v2,v12,v,math.sqrt(max(0,v)),4,3,11],'pairwise covariance membership'
    for n,k in ((3,1),(4,1),(4,2),(6,2),(6,3),(8,2),(8,4),(9,3)):
        for _ in range(10):
            x=[rng.randint(-3,3) for i in range(n)]
            if len(set(x))==1:x[-1]+=1
            rows=[[rng.randint(-5,5)/4 for j in range(3)] for i in range(n)]
            mu=[st.mean(c) for c in zip(*rows)]; vals=[]; null=[]
            for starts in it.product(range(n),repeat=n//k):
                indices=[(s+j)%n for s in starts for j in range(k)]
                a=[x[i] for i in indices]
                if len(set(a))>1:vals.append(st.mean(a)/st.pstdev(a))
                null.append(max(st.mean(rows[i][j]-mu[j] for i in indices) for j in range(3)))
            vals.sort(); P=len(null)
            q=lambda p:vals[max(1,math.ceil(p*len(vals)))-1]
            yield 'circular_sharpe_distribution',[k,*x],[P,len(vals),1-len(vals)/P,st.mean(vals),st.pstdev(vals),q(.025),q(.5),q(.975)],'full reconstructed bootstrap paths'
            obs=max(mu)
            yield 'reality_check_circular',[n,3,k,*it.chain.from_iterable(rows)],[obs,sum(z+1e-12>=obs for z in null)/P,st.mean(null),st.pstdev(null),P,*mu],'joint centered block resampling'

if __name__=='__main__':
    import json
    for m,a,e,d in studies():
        print(json.dumps(dict(method=m,input=a,expected=e,design=d)))
