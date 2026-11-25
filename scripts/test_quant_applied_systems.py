import itertools
import json
import math
import pathlib
import random
import statistics
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[1]
exe = sys.argv[1]
groups = ['forecast','inference','liquidity','rates','options','credit','accounts','allocation','synthesis']
catalog = [c for g in groups for c in json.loads((root/'quant/research'/f'applied-{g}-cases.json').read_text(encoding='utf8'))]
ids = {c['id'] for c in catalog}
assert len(ids) == len(catalog) == 72
assert set(subprocess.check_output([exe,'--list'],text=True).splitlines()) == ids

def batch(rows):
    lines = [' '.join([method]+[str(v) for v in args]) for method,args in rows]
    raw = subprocess.run([exe,'--batch'],input='\n'.join(lines)+'\n',text=True,check=True,capture_output=True).stdout.splitlines()
    assert len(raw)==len(rows), (len(raw),len(rows))
    return [None if line=='ERROR' else [float(v) for v in line.split(',')] for line in raw]

def close(a,b,label):
    assert math.isfinite(a) and math.isclose(a,b,rel_tol=2e-9,abs_tol=2e-10),(label,a,b)

def compare(a,b,label):
    assert a is not None and len(a)==len(b),(label,a,b)
    for i,(x,y) in enumerate(zip(a,b)): close(x,y,(label,i))

rows = [(c['id'],case['input']) for c in catalog for case in c['cases']]
expected = [case['expected'] for c in catalog for case in c['cases']]
for (method,args),a,b in zip(rows,batch(rows),expected):
    compare(a,b,(method,args))
invalid = [(c['id'],c['invalid']) for c in catalog]+[(c['id'],[]) for c in catalog]
invalid += [('unknown_method',[1]),('forecast_brier_groups',['NaN',0]),('forecast_brier_groups',['0.2bad',0])]
for row,result in zip(invalid,batch(invalid)):
    assert result is None,(row,result)
print(f'PASS: {len(rows)} fixed independent cases, {sum(map(len,expected))} numerical coordinates, {len(invalid)} malformed or out-of-domain inputs')

rng = random.Random(20261125)
checks = []
def add(method,args,expected):
    checks.append((method,args,expected))

# Weighted isotonic fit: exhaustive contiguous block partitions independently minimize SSE.
for _ in range(40):
    n=5
    y=[rng.randrange(-4,8)/4 for _ in range(n)]
    w=[rng.randrange(1,5) for _ in range(n)]
    best=math.inf
    ref=None
    for mask in range(1<<(n-1)):
        starts=[0]+[i for i in range(1,n) if mask&(1<<(i-1))]+[n]
        means=[sum(w[j]*y[j] for j in range(a,b))/sum(w[a:b]) for a,b in zip(starts,starts[1:])]
        if any(a>b+1e-13 for a,b in zip(means,means[1:])): continue
        fit=[m for m,a,b in zip(means,starts,starts[1:]) for _ in range(a,b)]
        err=sum(v*(a-b)**2 for a,b,v in zip(fit,y,w))
        if err<best: best,ref=err,fit
    add('forecast_isotonic_fit',[z for pair in zip(y,w) for z in pair],ref)

# Exact conditional distribution of runs via enumeration, without using combinatorial formulas.
for n in range(4,9):
    for a in range(1,n):
        arrangements=[]
        for subset in itertools.combinations(range(n),a):
            s=[1 if i in subset else -1 for i in range(n)]
            r=1+sum(x!=y for x,y in zip(s,s[1:]))
            arrangements.append((s,r))
        for s,r in (arrangements[0],arrangements[len(arrangements)//2],arrangements[-1]):
            lo=sum(z<=r for _,z in arrangements)/len(arrangements)
            hi=sum(z>=r for _,z in arrangements)/len(arrangements)
            add('inference_runs_exact',s,[r,statistics.mean(z for _,z in arrangements),lo,hi,min(1,2*min(lo,hi))])
for _ in range(24):
    data=[rng.randrange(-5,6) for _ in range(4)]
    blocks=[(data[i]+data[(i+1)%4])/2 for i in range(4)]
    dist=[(a+b)/2 for a,b in itertools.product(blocks,repeat=2)]
    add('inference_circular_boot_var',[2]+data,[statistics.mean(data),statistics.pvariance(blocks),statistics.pvariance(dist)])

# Integer queue allocation checked by exact rational arithmetic and conservation.
for _ in range(50):
    qty=[rng.randint(1,30) for _ in range(5)]
    q=rng.randint(1,sum(qty))
    initial=[q*s//sum(qty) for s in qty]
    order=sorted(range(len(qty)),key=lambda i:(-(q*qty[i]%sum(qty)),i))
    for i in order[:q-sum(initial)]: initial[i]+=1
    assert sum(initial)==q and all(a<=b for a,b in zip(initial,qty))
    add('liquidity_pro_rata_lots',[q]+qty,initial)

# Cash flow bond yields generated from independent discounted prices.
for _ in range(40):
    times=[.5,1.5,3]
    flows=[rng.randint(5,20),rng.randint(5,20),rng.randint(90,130)]
    y=rng.uniform(-.1,.2)
    pv=sum(c*math.exp(-y*t) for t,c in zip(times,flows))
    add('rates_yield_bisect',[pv]+[z for p in zip(times,flows) for z in p],[y,pv,0])
    d=sum(t*c*math.exp(-y*t) for t,c in zip(times,flows))
    v=sum(t*t*c*math.exp(-y*t) for t,c in zip(times,flows))
    add('rates_continuous_duration',[y]+[z for p in zip(times,flows) for z in p],[pv,d/pv,v/pv,d])

# Reprice each of four paths with a separately enumerated risk-neutral tree.
for cp in [-1,1]:
    for r in [.98,1,1.05]:
        s,k,u,d=100,95,1.2,.8
        q=(r-d)/(u-d)
        h=lambda x:max(0,cp*(x-k))
        for p1,p2 in itertools.product([-1,1],repeat=2):
            s1=s*(u if p1==1 else d)
            s2=s1*(u if p2==1 else d)
            root=sum((q if a==1 else 1-q)*(q if b==1 else 1-q)*h(s*(u if a==1 else d)*(u if b==1 else d))/r**2 for a,b in itertools.product([-1,1],repeat=2))
            vu=(q*h(s*u*u)+(1-q)*h(s*u*d))/r
            vd=(q*h(s*u*d)+(1-q)*h(s*d*d))/r
            v1=(q*h(s1*u)+(1-q)*h(s1*d))/r
            add('synthesis_replication_path',[s,k,u,d,r,cp,p1,p2],[root,v1,h(s2),(vu-vd)/(s*(u-d)),(h(s1*u)-h(s1*d))/(s1*(u-d)),0])

# Netting and collateral: independently aggregate scenario matrices.
for _ in range(50):
    rows2=[[rng.randint(-60,80) for _ in range(3)] for _ in range(5)]
    h=rng.randint(0,40); p=.07; df=.95; lgd=.6
    gross=statistics.mean(sum(max(0,v) for v in z) for z in rows2)
    net=statistics.mean(max(0,sum(z)) for z in rows2)
    coll=statistics.mean(min(h,max(0,sum(z))) for z in rows2)
    a,b,c=[v*p*df*lgd for v in (gross,net,coll)]
    add('synthesis_netting_cva',[1-lgd,df,p,5,3,h]+[v for z in rows2 for v in z],[a,b,c,a-b,b-c])

# Exact TWR, Dietz and gain from a generated cashflow path.
for _ in range(50):
    start=100
    rs=[rng.uniform(-.12,.14) for _ in range(5)]
    fs=[rng.randint(-8,12) for _ in rs]
    value=start
    for r,f in zip(rs,fs): value=value*(1+r)+f
    gain=value-start-sum(fs)
    dietz=gain/(start+sum((1-(i+1)/len(rs))*f for i,f in enumerate(fs)))
    twr=math.prod(1+r for r in rs)-1
    add('accounts_flow_performance_pipeline',[start]+[z for p in zip(rs,fs) for z in p],[twr,dietz,gain,value,twr-dietz])

# Tail ES via independently minimized Rockafellar-Uryasev piecewise-linear objective.
for _ in range(60):
    w=[rng.uniform(-.5,1.2),rng.uniform(-.5,1.2)]
    a=.7
    loss=[[rng.uniform(-.1,.3),rng.uniform(-.1,.3)] for _ in range(5)]
    tot=[sum(v*z for v,z in zip(w,l)) for l in loss]
    es=min(t+sum(max(0,v-t) for v in tot)/len(tot)/(1-a) for t in tot)
    # A direct sorted weighted integral allocates 1.5 of five equal-mass scenarios.
    ix=sorted(range(5),key=lambda i:tot[i],reverse=True)
    c=[w[j]*(loss[ix[0]][j]+.5*loss[ix[1]][j])/1.5 for j in range(2)]
    add('allocation_tail_euler',w+[a]+[z for l in loss for z in [.2,*l]],[es,*c,0])

for (method,args,ref),a in zip(checks,batch([(m,x) for m,x,_ in checks])):
    compare(a,ref,(method,args))
print(f'PASS: {len(checks)} deterministic exhaustive or independently generated numerical studies; {sum(len(r) for _,_,r in checks)} additional numerical checks')
