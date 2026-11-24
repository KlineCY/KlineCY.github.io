"""Independent numerical references for the public research examples.

Only the Python standard library is used. The C++ executable is treated
as an external program, with named CSV outputs checked against the model.
"""
import csv
import io
import json
import math
import statistics as st
import subprocess
import sys
import tempfile
from collections import Counter, defaultdict
from pathlib import Path

EXAMPLES = Path(__file__).resolve().parents[1] / "research" / "examples"
CATALOG = json.loads((EXAMPLES / "catalog.json").read_text(encoding="utf-8"))
BIN = str(Path(sys.argv[1]).resolve())
M = st.mean
S = sum


def load(path):
    with path.open(newline="", encoding="utf-8") as f:
        r = csv.reader(f)
        return next(r), [list(map(float, row)) for row in r]


def run(command, path, params, valid=True):
    r = subprocess.run([BIN, command, str(path), *map(str, params)],
                       capture_output=True, text=True, timeout=30)
    if not valid:
        assert r.returncode == 1 and "Quant research input error:" in r.stderr, (command, r.stderr)
        return
    assert r.returncode == 0, (command, r.stderr)
    reader = csv.reader(io.StringIO(r.stdout))
    names = next(reader)
    rows = [list(map(float, row)) for row in reader]
    assert names and len(names) == len(set(names)) and rows, command
    assert all(len(r) == len(names) and all(map(math.isfinite, r)) for r in rows), command
    return names, rows


def near(a, b, label):
    assert math.isclose(a, b, rel_tol=2e-7, abs_tol=2e-9), (label, a, b)


def compare(actual, expected, label):
    assert len(actual) == len(expected), (label, len(actual), len(expected))
    for i, (a, b) in enumerate(zip(actual, expected)):
        assert len(a) == len(b), (label, i)
        for j, (u, v) in enumerate(zip(a, b)):
            near(u, v, (label, i, j))


def dot(a, b):
    return math.fsum(x*y for x, y in zip(a, b))


def solve(a, b):
    """Gaussian elimination with partial pivoting, not the C++ Cholesky solver."""
    a = [list(row)+[v] for row, v in zip(a, b)]
    n = len(a)
    for j in range(n):
        k = max(range(j, n), key=lambda i: abs(a[i][j]))
        a[j], a[k] = a[k], a[j]
        pivot = a[j][j]
        assert pivot != 0
        a[j] = [v/pivot for v in a[j]]
        for i in range(n):
            if i != j:
                c = a[i][j]
                a[i] = [u-c*v for u, v in zip(a[i], a[j])]
    return [row[-1] for row in a]


def covariance(a, shrink, ridge):
    cols = list(zip(*a))
    s = [[st.covariance(x, y) for y in cols] for x in cols]
    mu = M([s[i][i] for i in range(len(cols))])
    return [[(1-shrink)*v+(shrink*mu+ridge if i == j else 0)
             for j, v in enumerate(row)] for i, row in enumerate(s)]


def ridge(a, lam):
    features = list(zip(*(row[:-1] for row in a)))
    center = [M(c) for c in features]
    scales = [st.pstdev(c) for c in features]
    scales = [v if v >= 1e-12 else 1.0 for v in scales]
    X = [[(v-m)/s for v, m, s in zip(row, center, scales)] for row in a]
    y = [row[-1] for row in a]
    ym = M(y)
    cols = list(zip(*X))
    n = len(a)
    A = [[dot(u, v)/n+(lam if i == j else 0)
          for j, v in enumerate(cols)] for i, u in enumerate(cols)]
    b = [dot(c, [v-ym for v in y])/n for c in cols]
    w = solve(A, b)
    beta = [u/s for u, s in zip(w, scales)]
    intercept = ym-dot(beta, center)
    return lambda row: intercept+dot(beta, row[:-1]), ym, beta


def tail(loss, alpha):
    q = sorted(loss)
    v = q[math.ceil(alpha*len(q))-1]
    return v, v+math.fsum(max(z-v, 0) for z in q)/((1-alpha)*len(q))


def portfolio(fn, x, p, rows):
    look, cost = int(p[0]), p[1]
    d = len(x[0])
    old = [0]*d
    nav = peak = 1.0
    expected = []
    for t in range(look, len(x)):
        h = x[t-look:t]
        if fn == "cta":
            z = []
            for col in zip(*h):
                trend = math.prod(1+r for r in col)-1
                z.append((1 if trend > 0 else -1)/max(p[2], st.stdev(col))
                         if abs(trend)>1e-12 else 0)
            den = S(map(abs, z))
            w = [max(-p[4], min(p[4], p[3]*q/den)) if den else 0 for q in z]
        elif fn == "cross_section":
            k, gross = int(p[2]), p[3]
            scores = [math.prod(1+v for v in c) for c in zip(*h)]
            ix = sorted(range(d), key=lambda j:scores[j])
            w = [0.0]*d
            if scores[ix[k-1]] != scores[ix[k]] and scores[ix[d-k-1]] != scores[ix[d-k]]:
                for j in ix[:k]: w[j] = -gross/(2*k)
                for j in ix[-k:]: w[j] = gross/(2*k)
        else:
            cov = covariance(h,p[2],p[3])
            if fn == "minimum_variance":
                z = solve(cov,[1.0]*d)
                w = [v/S(z) for v in z]
            else:
                w = rows[t-look][9:]
                assert all(v>0 for v in w)
                near(S(w),1,fn)
                var = dot(w,[dot(c,w) for c in cov])
                for j in range(d): near(w[j]*dot(cov[j],w)/var,1/d,fn)
        fee = 0.0
        # Fixed-point fee solution provides an independent numerical method.
        for _ in range(100):
            fee = cost*S(abs((1-fee)*a-b) for a,b in zip(w,old))
        turn = S(abs((1-fee)*a-b) for a,b in zip(w,old))
        gross = dot(w,x[t]);g=1+gross;net=(1-fee)*g-1
        nav *= 1+net;peak=max(peak,nav)
        expected.append([t,gross,net,turn,fee,nav,1-nav/peak,S(map(abs,w)),S(w),*w])
        old = [a*(1+r)/g for a,r in zip(w,x[t])]
    compare(rows,expected,fn)


def reference(fn, x, p, rows):
    if fn in {"cta","cross_section","minimum_variance","equal_risk"}:
        portfolio(fn,x,p,rows);return
    expected=[]
    n=len(x)
    if fn=="performance":
        annual,rf,alpha=p
        r,b=zip(*x);nav=math.prod(1+v for v in r);bnav=math.prod(1+v for v in b)
        z=peak=1;mdd=0
        for v in r:
            z*=1+v;peak=max(peak,z);mdd=max(mdd,1-z/peak)
        active=[u-v for u,v in zip(r,b)]
        var,es=tail([-v for v in r],alpha)
        expected=[[n,nav-1,bnav-1,nav**(annual/n)-1,st.stdev(r)*math.sqrt(annual),
                   (M(r)-math.expm1(math.log1p(rf)/annual))/st.stdev(r)*math.sqrt(annual),mdd,
                   st.stdev(active)*math.sqrt(annual),M(active)/st.stdev(active)*math.sqrt(annual),
                   st.covariance(r,b)/st.variance(b),var,es]]
    elif fn in {"rolling_ridge","nested_ridge"}:
        train,gap=map(int,p[:2])
        for t in range(train+gap,n):
            end=t-gap;start=end-train;a=x[t][-1]
            if fn=="rolling_ridge":
                pred,base,_=ridge(x[start:end],p[2]);v=pred(x[t])
                expected.append([t,start,end,v,a,base,(v-a)**2,(base-a)**2])
            else:
                val=int(p[2]);split=end-val;losses=[]
                for lam in p[3:]:
                    f,_,_=ridge(x[start:split-gap],lam)
                    losses.append(M([(f(r)-r[-1])**2 for r in x[split:end]]))
                j=min(range(len(losses)),key=lambda k:losses[k])
                chosen=p[3+j];f,_,_=ridge(x[start:end],chosen);v=f(x[t])
                expected.append([t,start,end,chosen,losses[j],v,a,(v-a)**2])
    elif fn=="conformal_rolling":
        look,gap=map(int,p[:2]);rank=math.ceil((look+1)*(1-p[2]))
        for t in range(look+gap,n):
            end=t-gap;start=end-look
            q=sorted(abs(a-b) for a,b in x[start:end])[rank-1]
            v,a=x[t];expected.append([t,start,end,v,v-q,v+q,a,int(v-q<=a<=v+q),2*q])
    elif fn=="rolling_calibration":
        look,gap,bins=map(int,p[:3]);a,b=p[3:]
        ix=lambda v:min(bins-1,int(v*bins))
        for t in range(look+gap,n):
            raw,y=x[t];h=[r[1] for r in x[t-gap-look:t-gap] if ix(r[0])==ix(raw)]
            q=(S(h)+a)/(len(h)+a+b)
            expected.append([t,raw,q,y,len(h),(raw-y)**2,(q-y)**2,-math.log(q if y else 1-q)])
    elif fn=="forecast_comparison":
        e1=[r[0]-r[2] for r in x];e2=[r[1]-r[2] for r in x];d=[a*a-b*b for a,b in zip(e1,e2)]
        mu=M(d);z=[v-mu for v in d];lag=int(p[0])
        v=dot(z,z)/n
        for k in range(1,lag+1):v+=2*(1-k/(lag+1))*dot(z[k:],z[:-k])/n
        se=math.sqrt(v/n)
        expected=[[n,M([a*a for a in e1]),M([a*a for a in e2]),M(list(map(abs,e1))),M(list(map(abs,e2))),
                   M([int((r[0]>0)==(r[2]>0)) for r in x]),M([int((r[1]>0)==(r[2]>0)) for r in x]),mu,se,mu/se]]
    elif fn=="rolling_ar":
        look,order=map(int,p[:2]);y=[r[0] for r in x]
        for t in range(look,n):
            train=[[y[k-j] for j in range(1,order+1)]+[y[k]] for k in range(t-look+order,t)]
            f,_,_=ridge(train,p[2]);v=f([y[t-j] for j in range(1,order+1)]+[y[t]])
            expected.append([t,v,y[t],y[t-1],(v-y[t])**2,(y[t-1]-y[t])**2])
    elif fn=="rolling_var":
        look=int(p[0]);d=len(x[0])
        for t in range(look,n):
            for j in range(d):
                train=[x[k-1]+[x[k][j]] for k in range(t-look+1,t)]
                f,_,_=ridge(train,p[1]);v=f(x[t-1]+[x[t][j]]);a=x[t][j];b=x[t-1][j]
                expected.append([t,j+1,v,a,b,(v-a)**2,(b-a)**2])
    elif fn=="volatility_forecast":
        e=g=p[0];decay,omega,alpha,beta=p[1:]
        for t,(r,) in enumerate(x):
            expected.append([t,r,e,g,r/math.sqrt(e),r/math.sqrt(g),math.log(e)+r*r/e,math.log(g)+r*r/g])
            e=decay*e+(1-decay)*r*r;g=omega+alpha*r*r+beta*g
    elif fn=="regime_forecast":
        look=int(p[0]);prior=p[1]
        for t in range(look,n):
            h=x[t-look:t];prev=h[-1][0];trans=[b[0] for a,b in zip(h,h[1:]) if a[0]==prev]
            q=(S(trans)+prior)/(len(trans)+2*prior)
            pooled=M([r[1] for r in h]);g=[[r[1] for r in h if r[0]==i] for i in range(2)]
            means=[M(a) if a else pooled for a in g];v=(1-q)*means[0]+q*means[1];state,y=x[t]
            expected.append([t,prev,state,q,(q-state)**2,v,y,(v-y)**2])
    elif fn=="filter_forecast":
        m,v,q=p
        for t,(y,r) in enumerate(x):
            a=v+q;z=a+r;e=y-m;post=(r*m+a*y)/z;b=a*r/z
            expected.append([t,m,a,y,e,z,post,b,-.5*(math.log(2*math.pi*z)+e*e/z)])
            m,v=post,b
    elif fn=="rolling_tail":
        look=int(p[0]);alpha=p[1]
        for t in range(look,n):
            var,es=tail([-r[0] for r in x[t-look:t]],alpha);l=-x[t][0];e=l-var
            expected.append([t,l,var,es,int(l>var),max(alpha*e,(alpha-1)*e)])
    elif fn=="coverage":
        a=[int(r[0]) for r in x];hits=S(a);q=p[0];c=Counter(zip(a,a[1:]))
        def ll(v,z):
            return S(math.log(z if i else 1-z) for i in v)
        def mle(v):
            if not v or S(v) in {0,len(v)}:return 0.0
            return ll(v,M(v))
        uc=2*(mle(a)-ll(a,q));ind=2*(S(mle([v for u,v in zip(a,a[1:]) if u==i]) for i in [0,1])-mle(a[1:]))
        uc=max(0,uc);ind=max(0,ind)
        expected=[[n,hits,n*q,hits/n,c[0,0],c[0,1],c[1,0],c[1,1],uc,ind,uc+ind,
                   math.erfc(math.sqrt(uc/2)),math.erfc(math.sqrt(ind/2)),math.exp(-(uc+ind)/2)]]
    elif fn=="moving_block_paths":
        block,horizon,scenarios,state=map(int,p);avail=n-block+1;limit=2**32-2**32%avail
        for j in range(scenarios):
            path=[]
            while len(path)<horizon:
                state=(1664525*state+1013904223)%2**32
                while state>=limit:state=(1664525*state+1013904223)%2**32
                start=state%avail;path += [r[0] for r in x[start:start+block]]
            nav=peak=1;mdd=0
            for r in path[:horizon]:
                nav*=1+r;peak=max(peak,nav);mdd=max(mdd,1-nav/peak)
            expected.append([j,nav,nav-1,mdd])
    elif fn=="factor_attribution":
        look=int(p[0]);lam=p[1];shock=p[2:]
        for t in range(look,n):
            h=x[t-look:t];f,_,beta=ridge(h,lam);v=f(x[t]);a=x[t][-1]
            expected.append([t,a,v,a-v,dot(beta,shock),math.sqrt(M([(r[-1]-f(r))**2 for r in h])),*beta])
    elif fn=="risk_decomposition":
        w=p[2:];s=covariance(x,*p[:2]);m=[dot(r,w) for r in s];v=dot(w,m);vol=math.sqrt(v)
        expected=[[j+1,a,b/vol,a*b/vol,a*b/v,vol] for j,(a,b) in enumerate(zip(w,m))]
    elif fn=="order_tca":
        groups=defaultdict(list)
        for r in x:groups[r[0]].append(r)
        for id,group in sorted(groups.items()):
            _,side,arrival,target,_,_,_,mark=group[0];fill=S(r[5] for r in group)
            execution=S(side*(r[4]-arrival)*r[5] for r in group);opp=side*(mark-arrival)*(target-fill)
            fees=S(r[6] for r in group);total=execution+opp+fees
            expected.append([id,side,target,fill,target-fill,fill/target,execution,opp,fees,total,total/(target*arrival)*1e4])
    elif fn=="participation_audit":
        side,target,arrival,cap=p
        for t,r in enumerate(x):
            h=x[:t+1];fill=S(z[2] for z in h);fees=S(z[4] for z in h);share=r[2]/r[0]
            market=S(z[0]*z[1] for z in h)/S(z[0] for z in h)
            exe=S(z[2]*z[3] for z in h)/fill if fill else 0
            expected.append([t,r[2],fill,target-fill,share,int(share>cap+1e-12),market,exe,side*fill*(exe-arrival)+fees,fees])
    elif fn=="spread_decomposition":
        values=[]
        for t,(s,mid,price,future,qty) in enumerate(x):
            e=2*s*(price-mid)/mid*1e4;r=2*s*(price-future)/mid*1e4;z=e-r
            values.append((e,r,z,qty));total=S(v[3] for v in values)
            expected.append([t,e,r,z,*[S(v[j]*v[3] for v in values)/total for j in range(3)]])
    elif fn=="optimal_participation":
        target,cap=p;quantities=[r[2] for r in rows];near(S(quantities),target,fn)
        dual=rows[0][6];cumulative=0
        for t,(r,q) in enumerate(zip(x,quantities)):
            volume,linear,quadratic=r;cumulative+=q
            assert -1e-8<=q<=cap*volume+1e-8
            marginal=linear+2*quadratic*q/volume
            if q<1e-8:assert marginal>=dual-1e-8
            elif q>=cap*volume-1e-8:assert marginal<=dual+1e-8
            else:near(marginal,dual,fn)
            expected.append([t,volume,q,q/volume,linear*q+quadratic*q*q/volume,cumulative,dual])
    elif fn=="competing_fill":
        surv=1.0;fill=cancel=0.0
        for time in sorted(set(r[0] for r in x)):
            risk=S(r[0]>=time for r in x);c=Counter(r[1] for r in x if r[0]==time)
            fill+=surv*c[1]/risk;cancel+=surv*c[2]/risk
            surv*=1-(c[1]+c[2])/risk
            expected.append([time,risk,c[1],c[2],c[0],surv,fill,cancel])
            near(surv+fill+cancel,1,fn)
    else:raise AssertionError(fn)
    compare(rows,expected,fn)


def write(path, header, rows):
    with path.open("w", newline="", encoding="utf-8") as f:
        w=csv.writer(f);w.writerow(header);w.writerows(rows)


def leakage(c, x, header, original, path):
    fn=c["fn"];p=c["params"];rows=original
    # Compare the first forecast or allocation after changing its target outcome
    # and all future rows. The forecast must only use the past information set.
    if fn in {"cta","cross_section","minimum_variance","equal_risk"}:
        t=int(p[0]);z=[r[:] for r in x]
        for j in range(t,len(z)):z[j]=[v+.001 for v in z[j]]
        write(path,header,z);_,changed=run(c["command"],path,p)
        compare([rows[0][9:]],[changed[0][9:]],fn+" timing")
    elif fn in {"rolling_ridge","nested_ridge","conformal_rolling","rolling_calibration"}:
        t=int(rows[0][0]);z=[r[:] for r in x]
        for j in range(t,len(z)):
            z[j][-1]=1-z[j][-1] if fn=="rolling_calibration" else z[j][-1]+.003
        write(path,header,z);_,changed=run(c["command"],path,p)
        indices={"rolling_ridge":[3],"nested_ridge":[3,4,5],"conformal_rolling":[3,4,5],"rolling_calibration":[2]}[fn]
        compare([[rows[0][i] for i in indices]],[[changed[0][i] for i in indices]],fn+" timing")
    elif fn in {"rolling_ar","rolling_var","rolling_tail","volatility_forecast","filter_forecast","regime_forecast","factor_attribution"}:
        t=int(rows[0][0]);z=[r[:] for r in x]
        if fn=="regime_forecast":
            for j in range(t,len(z)):z[j]=[1-z[j][0],z[j][1]+.003]
        elif fn=="filter_forecast":
            for j in range(t,len(z)):z[j][0]+=.003
        elif fn=="factor_attribution":
            for j in range(t,len(z)):z[j][-1]+=.003
        else:
            for j in range(t,len(z)):z[j]=[v+.003 for v in z[j]]
        write(path,header,z);_,changed=run(c["command"],path,p)
        indices={"rolling_ar":[1],"rolling_var":[2],"rolling_tail":[2,3],"volatility_forecast":[2,3],
                 "filter_forecast":[1,2],"regime_forecast":[3,5],"factor_attribution":[2,4,5]}[fn]
        compare([[rows[0][i] for i in indices]],[[changed[0][i] for i in indices]],fn+" timing")
    else:return 0
    return 1


def main():
    total=timing=0
    with tempfile.TemporaryDirectory() as tmp:
        path=Path(tmp)/"input.csv"
        for c in CATALOG:
            source=EXAMPLES/c["fixture"]
            h,x=load(source)
            _,rows=run(c["command"],source,c["params"])
            reference(c["fn"],x,c["params"],rows)
            total+=len(rows)
            timing+=leakage(c,x,h,rows,path)
            run(c["command"],source,c["params"]+["not-a-number"],False)
            run(c["command"],source.with_suffix(".missing"),c["params"],False)
            bad=[r[:] for r in x];bad[0][0]=float("nan");write(path,h,bad)
            run(c["command"],path,c["params"],False)
        # Exact closed-form special cases and domain-specific invalid inputs.
        write(path,["A","B"],[[.1,0],[.1,0],[.1,0]])
        _,rows=run("flow-minimum-variance",path,[2,.01,1,.01,2])
        near(rows[0][4],.01/1.01,"initial purchase fee")
        compare([rows[0][9:]],[ [.5,.5] ],"symmetric covariance")
        write(path,["Probability"],[[0],[0],[0]])
        _,rows=run("flow-coverage",path,[.2]);near(rows[0][8],-6*math.log(.8),"zero violations")
        write(path,["V","A","B"],[[100,0,1],[100,0,1]])
        _,rows=run("flow-optimal-participation",path,[20,.2])
        compare([[r[2] for r in rows]],[[10,10]],"symmetric execution")
        run("flow-optimal-participation",path,[50,.2],False)
        write(path,["Prediction","Actual"],[[1,0],[1,0],[1,0]])
        run("flow-conformal-rolling",path,[2,0,.01],False)
        write(path,["Order","Side","Arrival","Target","Price","Fill","Fee","Mark"],[[1,1,10,1,10,2,0,10]])
        run("flow-order-tca",path,[],False)
        write(path,["Return"],[[0],[.1],[.2]])
        run("flow-volatility-forecast",path,[.1,.9,.1,.4,.7],False)
        write(path,["Elapsed","Event"],[[1,3]])
        run("flow-competing-fill",path,[],False)
    print(f"PASS: {len(CATALOG)} independent research workflows, {total} output rows, {timing} temporal invariance tests, 75 malformed-input checks and 8 special/domain cases")


if __name__ == "__main__":
    main()
