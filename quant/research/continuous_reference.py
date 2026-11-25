"""Independent small-problem references for continuous-state numerical research.

Dense joint conditioning, analytic moments and direct integration are deliberately
separate from the native recursive and tridiagonal implementations.
"""
import math
import random
import statistics as st
import itertools as it

def dot(a,b):
    return math.fsum(x*y for x,y in zip(a,b))

def mv(a,b):
    return [dot(row,b) for row in a]

def transpose(a):
    return list(map(list,zip(*a)))

def mm(a,b):
    return [mv(transpose(b),row) for row in a]

def solve(a,b):
    z=[list(row)+[rhs] for row,rhs in zip(a,b)]
    n=len(z)
    for i in range(n):
        k=max(range(i,n),key=lambda j:abs(z[j][i]))
        z[i],z[k]=z[k],z[i]
        if abs(z[i][i])<1e-14: raise ValueError("singular matrix")
        for j in range(i+1,n):
            f=z[j][i]/z[i][i]
            z[j]=[x-f*y for x,y in zip(z[j],z[i])]
    x=[0.]*n
    for i in reversed(range(n)):
        x[i]=(z[i][-1]-dot(z[i][i+1:n],x[i+1:]))/z[i][i]
    return x

def logdet_spd(a):
    n=len(a);l=[[0.]*n for _ in a]
    for i in range(n):
        for j in range(i+1):
            v=a[i][j]-dot(l[i][:j],l[j][:j])
            l[i][j]=math.sqrt(v) if i==j else v/l[j][j]
    return 2*math.fsum(math.log(l[i][i]) for i in range(n))

def kalman_joint(x):
    n=int(x[0]); F=[x[1:3],x[3:5]]
    Q=[[x[5],x[6]],[x[6],x[7]]]
    P=[[x[10],x[11]],[x[11],x[12]]]
    mu=x[8:10]; C=[row[:] for row in P]
    # Build the unconditional covariance of the entire latent trajectory.
    for t in range(1,n):
        cross=mm(F,C[-2:])
        B=mm(mm(F,[row[-2:] for row in C[-2:]]),transpose(F))
        C=[row+[cross[j][i] for j in range(2)] for i,row in enumerate(C)]
        C += [cross[j]+[B[j][k]+Q[j][k] for k in range(2)] for j in range(2)]
        mu+=mv(F,mu[-2:])
    obs=[]
    for t in range(n):
        for j in range(2):
            if int(x[15+3*t])&(1<<j):
                obs.append((2*t+j,x[16+3*t+j],x[13+j]))
    def condition(t,selected):
        k=2*t
        if not selected:
            return [mu[k],mu[k+1],C[k][k],C[k][k+1],C[k+1][k+1]],0.
        S=[[C[i][j]+(r if a==b else 0.) for b,(j,_,_) in enumerate(selected)]
           for a,(i,_,r) in enumerate(selected)]
        e=[y-mu[i] for i,y,_ in selected]; z=solve(S,e)
        U=[[C[k+h][i] for i,_,_ in selected] for h in range(2)]
        m=[mu[k+h]+dot(U[h],z) for h in range(2)]
        V=[solve(S,row) for row in U]
        p=[C[k][k]-dot(U[0],V[0]),C[k][k+1]-dot(U[0],V[1]),C[k+1][k+1]-dot(U[1],V[1])]
        ll=-.5*(len(e)*math.log(2*math.pi)+logdet_spd(S)+dot(e,z))
        return m+p,ll
    out=[condition(n-1,obs)[1],len(obs)]
    for t in range(n):
        out+=condition(t,[v for v in obs if v[0]//2<=t])[0]+condition(t,obs)[0]
    return out

def gh():
    a=math.sqrt(5+math.sqrt(10));b=math.sqrt(5-math.sqrt(10))
    return [-a,-b,0,b,a],[(7-2*math.sqrt(10))/60,(7+2*math.sqrt(10))/60,8/15,(7+2*math.sqrt(10))/60,(7-2*math.sqrt(10))/60]

def nonlinear_moments(x):
    a,b,s,t,rho,c,d=x
    vv=c*c*s*s+2*c*d*rho*s*t+d*d*t*t
    e=math.exp(c*a+d*b+vv/2);u=a*a+s*s;z=c*s*s+d*rho*s*t
    out=[u,e,2*s**4+4*a*a*s*s,e*(2*a*z+z*z),e*e*math.expm1(vv)]
    def moments(nodes):
        v=[(w,(a+s*p)**2,math.exp(c*(a+s*p)+d*(b+t*(rho*p+math.sqrt(1-rho*rho)*q)))) for p,q,w in nodes]
        m1=sum(w*y for w,y,z in v);m2=sum(w*z for w,y,z in v)
        return [m1,m2,sum(w*y*y for w,y,z in v)-m1*m1,sum(w*y*z for w,y,z in v)-m1*m2,sum(w*z*z for w,y,z in v)-m2*m2]
    h=math.sqrt(2);out+=moments([(h,0,.25),(-h,0,.25),(0,h,.25),(0,-h,.25)])
    nodes,weights=gh()
    return out+moments([(p,q,w*v) for p,w in zip(nodes,weights) for q,v in zip(nodes,weights)])

def density_filter(x):
    m,v,phi,q,r,power=x[:6];power=int(power)
    em,ev=m,v;ll=el=0.;out=[];nodes,weights=gh()
    for y in x[6:]:
        pm=phi*m;pv=phi*phi*v+q
        pts=[pm+math.sqrt(pv)*z for z in nodes]
        w=[a*math.exp(-(y-z**power)**2/(2*r))/math.sqrt(2*math.pi*r) for a,z in zip(weights,pts)]
        total=sum(w);ll+=math.log(total)
        m=dot(w,pts)/total;v=dot(w,[z*z for z in pts])/total-m*m
        em*=phi;ev=phi*phi*ev+q
        h=power*em**(power-1);S=h*h*ev+r;e=y-em**power
        el-=.5*(math.log(2*math.pi*S)+e*e/S)
        em+=ev*h*e/S;ev=ev*r/S
        out.extend([m,v,em,ev])
    return [ll,el]+out

def ensemble(x):
    n=int(x[0]);y,r=x[1:3];h=x[3:5];z=list(zip(x[5::2],x[6::2]))
    a,b=map(st.mean,zip(*z))
    p=st.variance([v[0] for v in z]);t=st.variance([v[1] for v in z])
    c=sum((u-a)*(v-b) for u,v in z)/(n-1)
    ph=[p*h[0]+c*h[1],c*h[0]+t*h[1]];S=dot(h,ph)+r
    m=[a+ph[0]*(y-dot(h,[a,b]))/S,b+ph[1]*(y-dot(h,[a,b]))/S]
    target=m+[p-ph[0]**2/S,c-ph[0]*ph[1]/S,t-ph[1]**2/S]
    # Independent certificate uses only target posterior moments.
    return [a,b,p,c,t]+target+target+[0.]

def cdf(x):return .5*math.erfc(-x/math.sqrt(2))
def pdf(x):return math.exp(-x*x/2)/math.sqrt(2*math.pi)

def bs(s,k,r,q,v,T):
    if T==0:return max(s-k,0.)
    d=(math.log(s/k)+(r-q+v*v/2)*T)/(v*math.sqrt(T))
    return s*math.exp(-q*T)*cdf(d)-k*math.exp(-r*T)*cdf(d-v*math.sqrt(T))

def pde_dense(x):
    s,k,r,q,v,T,width,M,N=x;M=int(M);N=int(N)
    h=2*width/M;dt=T/N;drift=r-q-v*v/2
    a=v*v/(2*h*h)-drift/(2*h);b=-v*v/(h*h)-r;c=v*v/(2*h*h)+drift/(2*h)
    spots=[s*math.exp((j-M/2)*h) for j in range(M+1)]
    L=[[a if j==i-1 else b if j==i else c if j==i+1 else 0. for j in range(M+1)] for i in range(M+1)]
    def evolve(steps):
        w=[max(p-k,0.) for p in spots];t=0.
        for d,theta in steps:
            t+=d
            A=[[float(i==j)-theta*d*L[i][j] for j in range(M+1)] for i in range(M+1)]
            rhs=[w[i]+(1-theta)*d*dot(L[i],w) for i in range(M+1)]
            for j in (0,M):
                A[j]=[float(i==j) for i in range(M+1)]
                rhs[j]=bs(spots[j],k,r,q,v,t)
            w=solve(A,rhs)
        j=M//2;dx=(w[j+1]-w[j-1])/(2*h);dxx=(w[j+1]-2*w[j]+w[j-1])/(h*h)
        return w[j],dx/s,(dxx-dx)/s**2
    p=evolve([(dt,.5)]*N);z=evolve([(dt/2,1)]*2+[(dt,.5)]*(N-1))
    exact=bs(s,k,r,q,v,T);d1=(math.log(s/k)+(r-q+v*v/2)*T)/(v*math.sqrt(T))
    return [p[0],z[0],exact,p[0]-exact,z[0]-exact,p[1],z[1],math.exp(-q*T)*cdf(d1),p[2],z[2],math.exp(-q*T)*pdf(d1)/(s*v*math.sqrt(T)),0.]

def adjoint_dense(x):
    n=int(x[0]);d=x[1:n+1];l=x[n+1:2*n];u=x[2*n:3*n-1];b=x[3*n-1:4*n-1];c=x[4*n-1:]
    A=[[d[i] if i==j else l[j] if i==j+1 else u[i] if j==i+1 else 0. for j in range(n)] for i in range(n)]
    z=solve(A,b);a=solve(transpose(A),c)
    return [dot(c,z)]+z+a+[-a[i]*z[i] for i in range(n)]+[-a[i+1]*z[i] for i in range(n-1)]+[-a[i]*z[i+1] for i in range(n-1)]+a+[0.,0.]

def adaptive(f,a,b,tol,depth=24):
    m=(a+b)/2;fa,fb,fm=f(a),f(b),f(m)
    whole=(b-a)*(fa+4*fm+fb)/6
    def rec(a,b,fa,fm,fb,old,tol,depth):
        m=(a+b)/2;l=(a+m)/2;r=(m+b)/2;fl,fr=f(l),f(r)
        left=(m-a)*(fa+4*fl+fm)/6;right=(b-m)*(fm+4*fr+fb)/6
        err=left+right-old
        if depth==0 or abs(err)<=15*tol:return left+right+err/15
        return rec(a,m,fa,fl,fm,left,tol/2,depth-1)+rec(m,b,fm,fr,fb,right,tol/2,depth-1)
    return rec(a,b,fa,fm,fb,whole,tol,depth)

def spread_integral(x):
    s1,s2,k,v1,v2,rho,r,q1,q2,T,n=x
    sig=v1*math.sqrt(T*(1-rho*rho))
    def f(z):
        B=s2*math.exp((r-q2-v2*v2/2)*T+v2*math.sqrt(T)*z)+k
        F=s1*math.exp((r-q1-v1*v1/2)*T+v1*math.sqrt(T)*rho*z+sig*sig/2)
        d=math.log(F/B)/sig+sig/2
        return math.exp(-r*T)*pdf(z)*(F*cdf(d)-B*cdf(d-sig))
    value=sum(adaptive(f,j,j+1,1e-11) for j in range(-10,10))
    out={0:value,1:value,4:float(k==0)}
    if k==0:
        vol=math.sqrt(v1*v1+v2*v2-2*rho*v1*v2)
        ex=bs(s1,s2,q2,q1,vol,T)
        out.update({3:ex,5:0.})
    else:out.update({3:0.,5:0.})
    return out

def ou_closed(x):
    x0,mu,k,v,T,n=x;n=int(n);h=T/n
    a=math.exp(-k*h);b=1-k*h
    m1=mu+a**n*(x0-mu);m2=mu+b**n*(x0-mu)
    v1=v*v*(-math.expm1(-2*k*T))/(2*k)
    v2=v*v*h*math.fsum(b**(2*j) for j in range(n))
    c=v*v*(-math.expm1(-k*h))/k*math.fsum((a*b)**j for j in range(n))
    return [m1,m2,v1,v2,c,math.sqrt(max(0.,(m1-m2)**2+v1+v2-2*c)),m2-m1,v2-v1,m1,v1]

def experiments(seed=20261125):
    rng=random.Random(seed)
    for n in range(1,8):
        for _ in range(12):
            x=[n,.8,.25,-.1,.7,.2,.04,.3,.2,-.4,.7,.15,.8,.3,.6]
            for t in range(n):x += [rng.randrange(4),rng.uniform(-2,2),rng.uniform(-2,2)]
            yield 'kalman_missing',x,kalman_joint(x),'joint Gaussian conditioning on observed coordinates'
    for _ in range(90):
        x=[rng.uniform(-1,1),rng.uniform(-1,1),rng.uniform(.2,1.2),rng.uniform(.2,1.2),rng.uniform(-.8,.8),rng.uniform(-.6,.6),rng.uniform(-.6,.6)]
        yield 'nonlinear_gaussian_moments',x,nonlinear_moments(x),'analytic moments and raw-moment quadrature'
    for p,n in it.product((1,2),range(1,7)):
        for _ in range(8):
            x=[rng.uniform(-.5,.5),.7,.85,.2,.8,p]+[rng.uniform(-.5,1.5) for _ in range(n)]
            yield 'gaussian_density_filter',x,density_filter(x),'Hermite roots and raw posterior moments'
    for n in range(2,15):
        for _ in range(6):
            x=[n,rng.uniform(-1,1),rng.uniform(.2,2),.8,-.6]+[rng.uniform(-2,2) for _ in range(2*n)]
            yield 'ensemble_analysis',x,ensemble(x),'sample covariance posterior identity'
    for M,N,r in it.product((8,12,16),(2,5,9),(0,.02,.04)):
        x=[100,100,r,0,.25,.75,1.,M,N]
        yield 'bs_rannacher',x,pde_dense(x),'full dense system per time layer'
    for n in range(2,14):
        for _ in range(6):
            x=[n]+[rng.uniform(3,5) for _ in range(n)]+[rng.uniform(-1,1) for _ in range(2*n-2)]+[rng.uniform(-2,2) for _ in range(2*n)]
            yield 'tridiagonal_adjoint',x,adjoint_dense(x),'dense primal and transpose systems'
    for k,rho,v in it.product((0,5,20,40),(-.7,-.2,.3,.75),(.15,.35)):
        x=[105,95,k,v,.25,rho,.03,.01,.02,1.2,400]
        yield 'spread_conditional',x,spread_integral(x),'adaptive quadrature and exchange formula'
    for k,v,n in it.product((.1,.5,1.5,3.),(0,.2,1.),(1,2,4,8,16,64,256)):
        x=[1.2,-.1,k,v,1.2,n]
        yield 'ou_coupling',x,ou_closed(x),'closed Gaussian moments and finite geometric sums'
