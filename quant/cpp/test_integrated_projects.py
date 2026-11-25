"""Independent references for the native integrated research applications."""
import csv,math,io,json,sys,subprocess,tempfile
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"research"))
from model_study_common import csvfile,near,dot,ridge
ROOT=Path(__file__).resolve().parents[1]/"research"/"integrated-examples"
CAT={c["id"]:c for c in json.loads((ROOT/"catalog.json").read_text(encoding="utf-8"))}
binary=Path(sys.argv[1]).resolve()
def load(name):
    with (ROOT/CAT[name]["file"]).open(newline="") as f: rows=list(csv.reader(f))
    return [[float(v) for v in r] for r in rows[1:]]
def run(name,data=None,p=None,good=True):
    c=CAT[name]
    with tempfile.TemporaryDirectory() as tmp:
        file=ROOT/c["file"] if data is None else csvfile(Path(tmp)/"input.csv",c["columns"],data)
        proc=subprocess.run([str(binary),name,str(file),*map(str,c["params"] if p is None else p)],capture_output=True,text=True,timeout=90)
    if not good:
        assert proc.returncode==1,(name,proc.returncode,proc.stderr)
        return None
    assert proc.returncode==0,(name,proc.stderr)
    raw=list(csv.reader(io.StringIO(proc.stdout)))
    assert len(raw)>1 and all(len(r)==len(raw[0]) for r in raw[1:]),name
    rows=[[float(v) for v in r] for r in raw[1:]]
    assert all(math.isfinite(v) for r in rows for v in r),name
    return rows
def nested():
    name="nested-panel-risk-strategy";data=load(name);p=CAT[name]["params"]
    L,V=map(int,p[:2]);gamma,shrink,floor,cost,turn=p[2:7];penalties=p[7:]
    n=6;d=2;days=[data[i:i+n] for i in range(0,len(data),n)]
    R=[[r[2] for r in day] for day in days];Y=[[v-sum(y)/n for v in y] for y in R]
    X=[]
    for day in days:
        avg=[sum(r[j+3] for r in day)/n for j in range(d)]
        X.append([[r[j+3]-avg[j] for j in range(d)] for r in day])
    def fit(end,lam):
        return ridge([x for day in X[end-L:end] for x in day],[y for day in Y[end-L:end] for y in day],lam)
    rows=run(name);assert len(rows)==(len(days)-L-V)*n
    old=[0.]*n;nav=1;peak=1
    for t in range(L+V,len(days)):
        group=rows[(t-L-V)*n:(t-L-V+1)*n];losses=[]
        for lam in penalties:
            err=0
            for v in range(t-V,t):
                b=fit(v,lam);err+=sum((y-dot(x,b))**2 for x,y in zip(X[v],Y[v]))/(V*n)
            losses.append(err)
        j=min(range(len(penalties)),key=lambda j:losses[j]);lam=penalties[j];b=fit(t,lam)
        score=[dot(x,b) for x in X[t]];w=[r[5] for r in group]
        near(sum(w),0);assert max(map(abs,w))<=1/n+1e-9
        mean=[sum(R[s][i] for s in range(t-L,t))/L for i in range(n)]
        S=[[sum((R[s][i]-mean[i])*(R[s][j]-mean[j]) for s in range(t-L,t))/(L-1) for j in range(n)] for i in range(n)]
        tr=sum(S[i][i] for i in range(n))/n
        S=[[(1-shrink)*S[i][j]+(shrink*tr+floor)*(i==j) for j in range(n)] for i in range(n)]
        risk=sum(w[i]*S[i][j]*w[j] for i in range(n) for j in range(n))
        g=[gamma*sum(S[i][j]*w[j] for j in range(n))-score[i]+turn*(w[i]-old[i]) for i in range(n)]
        lower=-math.inf;upper=math.inf
        for i in range(n):
            if w[i]>=1/n-1e-7:upper=min(upper,-g[i])
            elif w[i]<=-1/n+1e-7:lower=max(lower,-g[i])
            else:lower=max(lower,-g[i]);upper=min(upper,-g[i])
        assert lower<=upper+2e-7,(t,g,w)
        fee=group[0][15];gross=dot(w,R[t]);net=(1-fee)*(1+gross)-1
        near(fee,cost*sum(abs((1-fee)*a-z) for a,z in zip(w,old)))
        nav*=1+net;peak=max(peak,nav)
        for i,r in enumerate(group):
            near(r[2],lam);near(r[3],losses[j]);near(r[4],score[i]);near(r[6],old[i])
            near(r[9],risk);near(r[10],gamma*risk/2-dot(score,w)+turn/2*sum((a-z)**2 for a,z in zip(w,old)))
            near(r[12],gross);near(r[13],net);near(r[16],nav);near(r[17],1-nav/peak)
            for a,z in zip(r[-d:],b):near(a,z)
        old=[w[i]*(1+R[t][i])/(1+gross) for i in range(n)]
    changed=[r[:] for r in data]
    for r in changed[-n:]:r[2]+=.1
    again=run(name,changed)
    for a,b in zip(rows,again):
        for j in (2,3,4,5,6,9,10):near(a[j],b[j])
    invalid=[r[:] for r in data];invalid[7][1]=invalid[6][1];run(name,invalid,good=False)
    return len(rows)
def option():
    name="repaired-option-strip";data=load(name);p=CAT[name]["params"];F,T=p[:2]
    rows=run(name);C=[r[3] for r in rows];K=[r[0] for r in rows]
    assert all(max(F-k,0)-1e-7<=c<=F+1e-7 for k,c in zip(K,C))
    slope=[(b-a)/(k1-k0) for a,b,k0,k1 in zip(C,C[1:],K,K[1:])]
    assert all(-1-1e-7<=s<=1e-7 for s in slope)
    assert all(y>=x-1e-7 for x,y in zip(slope,slope[1:]))
    f=[];cum=0
    for i,r in enumerate(rows):
        near(r[3]-r[4],F-r[0]);v=(r[4] if r[0]<F else r[3] if r[0]>F else (r[3]+r[4])/2)/r[0]**2
        f.append(v);near(r[5],v)
        if i:cum+=(K[i]-K[i-1])*(f[i]+f[i-1])/T
        near(r[7],cum)
    near(rows[0][8],.5*sum(r[2]*(r[3]-r[1])**2 for r in rows))
    assert abs(rows[0][9])<1e-6
    run(name,[r for r in data if r[0]!=F],good=False)
    return len(rows)
def factor():
    name="factor-filter-market-neutral";data=load(name);p=CAT[name]["params"]
    phi,Q,m,V=p[:4];loadings=p[4:6];noise=p[6:8];threshold,gross,cost=p[8:];state=[]
    for r in data:
        for _ in range(int(r[0])):m*=phi;V=phi**2*V+Q
        prec=1/V;rhs=m/V
        for j in range(2):
            if r[2+2*j]:prec+=loadings[j]**2/noise[j];rhs+=loadings[j]*r[1+2*j]/noise[j]
        V=1/prec;m=V*rhs;state.append((m,V))
    rows=run(name);nav=1;old=[0.,0.]
    for t,r in enumerate(rows,1):
        m,V=state[t-1];near(r[1],m);near(r[2],V)
        z=m/math.sqrt(V);a=gross/2 if z>threshold else -gross/2 if z<-threshold else 0
        w=[a,-a];ret=data[t][5:];near(r[3],a);near(r[4],-a)
        g=dot(w,ret);fee=r[10];near(fee,cost*sum(abs((1-fee)*a-b) for a,b in zip(w,old)))
        nav*=(1-fee)*(1+g);near(r[11],nav);old=[a*(1+b)/(1+g) for a,b in zip(w,ret)]
    changed=[r[:] for r in data];changed[-1][1]+=100
    new=run(name,changed)
    for a,b in zip(rows,new):
        for u,v in zip(a,b):near(u,v)
    bad=[r[:] for r in data];bad[2][2]=2;run(name,bad,good=False)
    return len(rows)
def execution():
    name="order-replay-cost-report";rows=run(name);assert len(rows)==3
    expected=[(3,1,2,101),(5,-1,4,99),(6,1,1,101)]
    for r,(event,side,size,price) in zip(rows,expected):
        near(r[0],event);near(r[1],side);near(r[2],side);near(r[3],size);near(r[4],price);near(r[9],size*price);near(r[10],0)
    data=load(name);data.append(data[0]);run(name,data,good=False)
    return len(rows)
for c in CAT.values():run(c["id"],p=c["params"]+["bad"],good=False)
n=sum(fn() for fn in (nested,option,factor,execution))
print(f"PASS: four native integrated research projects, {n} output rows, independent references and eight input guards")
