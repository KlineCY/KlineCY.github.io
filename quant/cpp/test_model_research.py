"""Independent reference, invalid-domain and temporal invariance checks."""
import math
import sys
import tempfile
from pathlib import Path

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"research"))
from model_study_common import CATALOG,INDEX,ROOT,near,csvfile,run,verify,dot,ridge,N,pdf,bs

binary=Path(sys.argv[1]).resolve()
assert len(CATALOG)==len(INDEX)==36
diagnostics={"KKTResidual","PrimalResidual","DualResidual","ProjectedGradient","BudgetGap","ScoreResidual","MomentResidual","StationarityResidual","SpreadResidual","MaxComplementarityResidual","MaxLinearResidual","ModelKKT","MaxMomentResidual","PrimalDualGap","OuterKKT","BellmanResidual"}

with tempfile.TemporaryDirectory() as d:
    path=Path(d)/"input.csv"
    def model(name,rows,params,good=True):
        names=INDEX[name]["columns"]
        if len(names)!=len(rows[0]):names=[f"V{i+1}" for i in range(len(rows[0]))]
        v=run(binary,name,csvfile(path,names,rows),params,good)
        return v[1] if good else None

    # Analytical cases, with independent checks of every reported diagnostic.
    for c in CATALOG:
        name=c["command"][6:]
        a,_=verify(binary,c)
        for j,h in enumerate(c["header"]):
            if h in diagnostics:
                for r in a:assert abs(r[j])<=1e-6,(name,h,r[j])
        original=ROOT/c["file"]
        run(binary,name,original.with_suffix(".absent"),good=False)
        run(binary,name,params=c["params"]+["bad"],good=False)
        lines=original.read_text().splitlines()
        row=lines[1].split(",");row[0]="nan"
        path.write_text("\n".join([lines[0],",".join(row),*lines[2:]])+"\n")
        run(binary,name,path,good=False)

    bad=[
        ("fista-elastic",[[1,3]],[-1,.5,1e-8,100]),
        ("fused-admm",[[0],[1]],[1,0,1e-8,100]),
        ("box-covariance-qp",[[2,1,0,0,.4],[1,2,0,0,.4]],[1,1,1e-8,100]),
        ("entropy-balance-fit",[[0,0],[1,1]],[.5,1e-8,100]),
        ("nnls-coordinate",[[1,3]],[0,1e-8,100]),
        ("graph-smoother",[[0,1,0,1],[2,0,4,1]],[1]),
        ("nb2-exposure",[[1,0,2]],[.5,1,1e-8,100]),
        ("zip-em",[[0],[0],[0]],[1e-8,100]),
        ("cox-breslow-ridge",[[1,0,1],[2,0,2]],[1,1e-8,100]),
        ("tobit-known-scale",[[1,0,2,1],[1,0,1,0]],[1,1,1e-8,100]),
        ("shrinkage-lda",[[1,0,1],[1,0,2],[1,0,3],[0,1,4]],[.5,.1]),
        ("kernel-logistic-fit",[[1,0,1],[1,1,2],[0,0,3]],[0,1,.1,1e-8,100]),
        ("var-ridge-forecast",[[1],[2],[3],[4]],[4,1,1]),
        ("kalman-grid-fit",[[1],[2],[3],[4]],[3,.2,.1,.1,.2,2,0,1]),
        ("hawkes-fixed-decay-fit",[[1],[1]],[2,1,.01,5,.9,1e-8,100]),
        ("har-rv-forecast",[[1]]*23+[[-1]],[23,1]),
        ("ou-conditional-fit",[[0],[1],[2],[3],[4]],[4,1]),
        ("dcc-twoasset-grid",[[1,2],[2,4],[3,6],[4,8],[5,10]],[4,.2,.5,3]),
        ("american-put-pde",[[90]],[100,.03,0,.2,1,90,40,20,1e-8,1000]),
        ("localvol-call-pde",[[0,.2],[50,.2],[95,.2],[150,.2],[200,.2]],[100,.03,0,1,20]),
        ("asian-independent-control",[[100,100,.03,0,-.2,1]],[12,100,100,1]),
        ("cds-hazard-bootstrap",[[1,.02],[1,.03]],[.4,.02,.5,1e-8]),
        ("nelson-siegel-calibration",[[1,.02,1],[2,.03,-1],[3,.04,1]],[1,2,3,0]),
        ("risk-neutral-entropy-fit",[[80,1],[100,1],[120,1]],[1,100,130,5,1e-8,100]),
        ("rolling-elastic-portfolio",[[t,i,.01,1] for t in range(1,5) for i in range(1,5)][:-1],[2,.25,0,.1,0,1e-8,100]),
        ("rolling-covariance-qp",[[.01,.02]]*4,[2,.5,.001,1,.4,0,1e-8,100]),
        ("risk-budget-newton",[[1,0,.6],[0,4,.6]],[1e-8,100]),
        ("synthetic-control-fit",[[0,1,1],[1,2,2],[0,3,3]],[1,1e-8,100]),
        ("crossfit-dml-ridge",[[1,1,2,0],[1,-1,-2,0],[1,2,4,1],[1,-2,-4,-1]],[1]),
        ("rolling-conformal-ridge",[[t,2] for t in range(8)],[3,2,1,.01]),
        ("nested-elastic-selection",[[t,i,.01,1] for t in range(1,8) for i in range(1,5)][:-1],[3,2,.25,0,1e-9,1000,0,.1]),
        ("cvar-primal-dual",[[.01,.02],[-.01,.03]],[1,.1,1,1e-8,1000]),
        ("turnover-admm-qp",[[1,0,0,0,.4,.8],[0,1,0,0,.4,.2]],[1,1,.1,1,1e-8,1000]),
        ("rolling-factor-risk-qp",[[.01,.02,.03]]*6,[1,3,.1,.001,.4,0,1e-8,1000]),
        ("integer-execution-dp",[[1,1,0,1],[1,1,0,1]],[3]),
        ("inventory-quote-dp",[[1.1,.4,1,0]],[1,0,0,.5]),
    ]
    assert len(bad)==36 and {x[0] for x in bad}==set(INDEX)
    for name,rows,p in bad:model(name,rows,p,False)

    # 1. Zero L1 gives the independent ridge normal-equation solution.
    X=[[1,2],[2,1],[3,-1],[-2,1]];y=[2,1,4,-1]
    a=model("fista-elastic",[r+[v] for r,v in zip(X,y)],[0,.7,1e-9,5000])
    for r,v in zip(a,ridge(X,y,.7)):near(r[1],v)

    # 2. A sufficiently high TV penalty gives a constant equal to the sample mean.
    a=model("fused-admm",[[1],[3],[-2],[2]],[100,2,1e-8,20000])
    for r in a:near(r[2],1)

    # 3. Active capacity: exact bounded two-asset solution.
    a=model("box-covariance-qp",[[2,1,.1,0,.3],[1,2,.1,0,1]],[1,1,1e-9,2000])
    near(a[0][1],.3);near(a[1][1],.7)

    # 4. Reweighting is invariant to common scaling of prior masses.
    data=[[1,0],[2,1],[3,2]]
    a=model("entropy-balance-fit",data,[1.2,1e-10,200])
    b=model("entropy-balance-fit",[[10*p,x] for p,x in data],[1.2,1e-10,200])
    near(sum(r[1]*x[1] for r,x in zip(a,data)),1.2)
    for r,s in zip(a,b):near(r[1],s[1])

    # 5. Original NNLS gradient and complementarity.
    X=[[1,2],[2,1],[1,-1]];y=[1,-2,2]
    a=model("nnls-coordinate",[r+[v] for r,v in zip(X,y)],[.2,1e-9,10000])
    w=[r[1] for r in a]
    for j,r in enumerate(a):
        g=.2*w[j]+sum(z[j]*(dot(z,w)-v) for z,v in zip(X,y))
        near(r[2],g);assert g>=-1e-8 and w[j]>=0;near(g*w[j],0)

    # 6. Connected graph with unobserved node and one anchored constant.
    a=model("graph-smoother",[[0,1,0,4,1],[1,0,1,0,0],[0,1,0,0,0]],[3])
    for r in a:near(r[1],4)

    # 7. Count exposure: doubling all exposures shifts unpenalized intercept.
    a=model("nb2-exposure",[[1,1,2],[1,2,4],[1,3,6]],[.5,0,1e-9,200])
    b=model("nb2-exposure",[[1,2,2],[1,4,4],[1,6,6]],[.5,0,1e-9,200])
    near(a[0][1]-b[0][1],math.log(2))

    # 8. ZIP interior estimate matches observed zero and positive-count means.
    a=model("zip-em",[[0]]*8+[[1],[2],[3],[4]],[1e-8,20000])[0]
    near(a[0]/(-math.expm1(-a[0])),2.5);near(a[-1],8/12)

    # 9. Cox likelihood and estimate invariant to input row permutation.
    data=[[1,1,1],[2,0,-1],[3,1,.5],[3,1,2],[4,0,-2]]
    a=model("cox-breslow-ridge",data,[.5,1e-9,200])
    b=model("cox-breslow-ridge",data[::-1],[.5,1e-9,200])
    near(a[0][1],b[0][1]);near(a[0][3],b[0][3])

    # 10. Tobit fixed-scale score includes the censored CDF derivative.
    a=model("tobit-known-scale",[[1,0,0,1],[1,0,2,0],[1,0,3,0]],[1,0,1e-8,20000])[0]
    w=a[1];near(2*w-5+pdf(-w)/N(-w),0,atol=1e-7)

    # 11-12. Changing held-out labels does not alter fitted probabilities.
    for name,params in [("shrinkage-lda",[.4,.1]),("kernel-logistic-fit",[.7,.5,.1,1e-9,200])]:
        data=[[1,0,-2],[1,0,-1],[1,1,1],[1,1,2],[0,0,-.5],[0,1,.5]]
        a=model(name,data,params)
        b=model(name,[r if r[0] else [0,1-r[1],r[2]] for r in data],params)
        for r,s in zip(a,b):near(r[3],s[3])

    # 13-14. Appending future observations cannot alter already computed predictions.
    for name,data,p in [
        ("var-ridge-forecast",[[1,0],[0,1],[1,0],[0,1],[1,0],[0,1]],[4,1,1]),
        ("kalman-grid-fit",[[1],[1],[2],[2.5],[3]],[3,.05,.2,.1,.4,3,0,1]),
    ]:
        a=model(name,data,p);b=model(name,data+[[9]*len(data[0])],p)
        for r,s in zip(a,b):
            for u,v in zip(r,s):near(u,v)

    # 15. Hawkes rescaling of time changes baseline but not branching ratio.
    data=[[.2],[.3],[.4],[1.5],[3.1],[3.2],[4.4]]
    a=model("hawkes-fixed-decay-fit",data,[5,1,.01,10,.9,1e-8,10000])[0]
    b=model("hawkes-fixed-decay-fit",[[2*r[0]] for r in data],[10,.5,.005,5,.9,1e-8,10000])[0]
    near(a[0],2*b[0]);near(a[1],b[1]);near(b[2]-a[2],len(data)*math.log(2))

    # 16. HAR training and feature windows are time-safe.
    data=[[1+.1*math.sin(i)] for i in range(30)]
    a=model("har-rv-forecast",data,[26,1])
    b=model("har-rv-forecast",data+[[9]],[26,1])
    for r,s in zip(a,b):
        for u,v in zip(r,s):near(u,v)

    # 17. OU time-unit conversion leaves discrete predictions invariant.
    data=[[0],[1],[1.4],[1.6],[1.5],[1.8],[1.6],[1.7]]
    a=model("ou-conditional-fit",data,[6,1]);b=model("ou-conditional-fit",data,[6,.25])
    for r,s in zip(a,b):
        near(r[2],s[2]);near(r[4],s[4]);near(4*r[6],s[6]);near(2*r[7],s[7])

    # 18. DCC future extension cannot change the earlier state history.
    data=[[1,1],[1,-1],[-1,1],[-1,-1],[2,1],[-1,2]]
    a=model("dcc-twoasset-grid",data,[4,.2,.6,3])
    b=model("dcc-twoasset-grid",data+[[10,-10]],[4,.2,.6,3])
    for r,s in zip(a,b):
        for u,v in zip(r,s):near(u,v)

    # 19-20. Nondegenerate PDE price comparison and obstacle check.
    p=[100,.03,.01,.2,1,400,100,100,1e-7,3000]
    a=model("american-put-pde",[[80],[100],[120]],p)
    for r in a:
        assert r[1]>=max(100-r[0],0)-1e-6 and r[1]>=r[2]-1e-6
        assert abs(r[2]-bs(r[0],100,.03,.01,.2,1,False))<.6
    a=model("localvol-call-pde",[[2.5*i,.2] for i in range(161)],[100,.03,.01,1,160])
    assert abs(a[40][2]-bs(100,100,.03,.01,.2,1))<.4

    # 21. One monitoring date: control variate is exactly the vanilla payoff.
    a=model("asian-independent-control",[[100,100,.02,.01,.2,1]],[1,512,2048,17])[0]
    near(a[1],bs(100,100,.02,.01,.2,1));assert a[3]<1e-8

    # 22. Flat constant hazard can be recovered at every maturity.
    h=.07;dt=.25;spread=.6*math.expm1(h*dt)/dt
    a=model("cds-hazard-bootstrap",[[1,spread],[2,spread],[3,spread]],[.4,0,dt,1e-10])
    for r in a:near(r[1],h);near(r[2],math.exp(-h*r[0]))

    # 23. Yield curve exact factor recovery on the grid.
    data=[]
    for t in [.25,.5,1,2,5,10,20]:
        z=t/2;a=-math.expm1(-z)/z;data.append([t,.03-.02*a+.01*(a-math.exp(-z)),1])
    a=model("nelson-siegel-calibration",data,[.5,4,4,0])
    near(a[0][7],2);assert a[0][-1]<1e-20

    # 24. Risk-neutral prices are discounted moments.
    a=model("risk-neutral-entropy-fit",[[80,1],[100,1],[120,1]],[.95,100,100,4.75,1e-10,200])
    near(sum(r[2] for r in a),.95);near(sum(r[0]*r[1] for r in a),100);near(sum(r[2]*max(r[0]-100,0) for r in a),4.75)

    # 25. Today's realized label must not change today's scores or weights.
    data=[[t,j+1,.01*v,v] for t in range(1,7) for j,v in enumerate([-2,-1,1,2])]
    p=[3,.25,0,.1,.001,1e-9,1000]
    a=model("rolling-elastic-portfolio",data,p)
    b=model("rolling-elastic-portfolio",[[r[0],r[1],r[2]+.1 if r[0]==4 else r[2],r[3]] for r in data],p)
    for u,v in zip(a[0][10:],b[0][10:]):near(u,v)
    near(a[0][3],.001*a[0][4])

    # 26. Today's return must not change today's covariance allocation.
    data=[[.01,.02],[.02,.01],[.005,.03],[.04,-.02],[.01,.01]]
    p=[3,.5,.001,10,1,.001,1e-9,3000]
    a=model("rolling-covariance-qp",data,p)
    b=model("rolling-covariance-qp",data[:3]+[[.5,-.4]]+data[4:],p)
    for u,v in zip(a[0][9:],b[0][9:]):near(u,v)
    near(a[0][4],.001*a[0][5])

    # 27. Multiplying the covariance matrix by a common scalar leaves risk weights unchanged.
    a=model("risk-budget-newton",[[1,.3,.35],[.3,4,.65]],[1e-9,200])
    b=model("risk-budget-newton",[[10,3,.35],[3,40,.65]],[1e-9,200])
    for r,s in zip(a,b):near(r[1],s[1]);near(r[2],r[3]);near(s[2],s[3])

    # 28. Post-treatment outcomes never affect synthetic control donor weights.
    data=[[0,2,1,3],[0,2,3,1],[1,5,2,4],[1,7,4,6]]
    a=model("synthetic-control-fit",data,[.1,1e-9,2000])
    b=model("synthetic-control-fit",data[:2]+[[r[0],r[1]+999,*r[2:]] for r in data[2:]],[.1,1e-9,2000])
    for r,s in zip(a,b):
        for u,v in zip(r[7:],s[7:]):near(u,v)

    # 29. A held-out fold's outcomes do not enter its nuisance regression.
    data=[[i%3,(-1)**i+.1*i,2*(-1)**i+.2*i,float(i%4)] for i in range(12)]
    a=model("crossfit-dml-ridge",data,[1])
    b=model("crossfit-dml-ridge",[[r[0],r[1],r[2]+20 if r[0]==0 else r[2],r[3]] for r in data],[1])
    for r,s in zip(a,b):
        if r[1]==0:near(r[5],s[5])
    near(sum(r[8] for r in a),0)

    # 30. Current test label changes coverage but not the interval.
    data=[[i,2+.2*math.sin(i)] for i in range(10)]
    p=[4,3,1,.25]
    a=model("rolling-conformal-ridge",data,p)
    b=model("rolling-conformal-ridge",data[:7]+[[7,999]]+data[8:],p)
    for u,v in zip(a[0][2:6],b[0][2:6]):near(u,v)

    # 31. Nested tuning and prediction must be invariant to the current outcome.
    data=[[t,j+1,.01*v,v] for t in range(1,9) for j,v in enumerate([-2,-1,1,2])]
    p=[3,2,.25,.001,1e-9,1000,0,.1,.005,.1]
    a=model("nested-elastic-selection",data,p)
    b=model("nested-elastic-selection",[[r[0],r[1],r[2]+.15 if r[0]==6 else r[2],r[3]] for r in data],p)
    for u,v in zip(a[0][6:],b[0][6:]):near(u,v)
    assert a[0][12]<a[0][0]

    # 32. A nonsymmetric tail-risk objective has an independent feasible upper bound.
    data=[[-.08,.01],[.04,-.03],[-.025,-.02],[.03,.015]]
    a=model("cvar-primal-dual",data,[.5,.1,1,1e-6,20000])
    def obj(w):
        loss=sorted([-r[0]*w-r[1]*(1-w) for r in data],reverse=True)
        return .5*sum(loss[:2])+.05*(w*w+(1-w)**2)
    ref=min(obj(i/10000) for i in range(10001))
    near(sum(r[1] for r in a),1);near(a[0][3],obj(a[0][1]))
    assert a[0][4]<=ref+1e-8 and a[0][3]<=ref+1e-6

    # 33. Zero turnover cost reduces to the original covariance QP.
    a=model("turnover-admm-qp",[[1,.4,.1,0,1,.8],[.4,2,.3,0,1,.2]],[1,1,0,1,1e-8,5000])
    near(a[0][1],1.4/2.2,atol=1e-7);near(sum(r[1] for r in a),1)

    # 34. Current factor and asset observations cannot affect the current allocation.
    data=[[.01*math.sin(i),.02*math.cos(i),.003*math.sin(i)+.01] for i in range(10)]
    p=[1,5,.01,.001,1,.001,1e-9,2000]
    a=model("rolling-factor-risk-qp",data,p)
    b=model("rolling-factor-risk-qp",data[:5]+[[.5,.4,-.2]]+data[6:],p)
    for u,v in zip(a[0][6:],b[0][6:]):near(u,v)
    for r in a:near(r[6]+r[7],r[8])

    # 35. Exhaustive feasible schedules validate the integer optimizer.
    data=[[1,1,.2,3],[2,1,.2,4],[1,1,.2,4]]
    a=model("integer-execution-dp",data,[6])
    vals=[]
    for i in range(4):
        for j in range(5):
            k=6-i-j
            if 0<=k<=4:vals.append(i*i+.2*(6-i)**2+j*j/2+.2*k*k+k*k)
    near(a[-1][5],min(vals));near(sum(r[1] for r in a),6)

    # 36. A fee larger than every quote makes the zero-risk no-trade policy optimal.
    a=model("inventory-quote-dp",[[.8,.7,.3,0]]*4,[2,0,10,.1,.5,1])
    for r in a:near(r[2],0);near(r[5],0)

print("PASS: 36 complete model references, 108 malformed-input cases, 36 domain checks, 36 nontrivial mathematical/temporal invariants")
