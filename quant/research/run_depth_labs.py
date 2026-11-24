"""Reproduce all deep research cases and sixteen connected parameter studies."""
import csv
import io
import json
import math
import subprocess
import sys
import tempfile
from pathlib import Path

root=Path(__file__).resolve().parent/"depth-examples"
binary=Path(sys.argv[1]).resolve()
out=Path(sys.argv[2])
cases=out/"cases"
studies=out/"studies"
cases.mkdir(parents=True,exist_ok=True)
studies.mkdir(parents=True,exist_ok=True)
catalog=[c for name in ["foundations","learning","finance","research"] for c in json.loads((root/f"catalog-{name}.json").read_text(encoding="utf-8"))]
index={c["command"]:c for c in catalog}

def close(a,b,rel=1e-8,abs_tol=1e-9):
    assert math.isclose(a,b,rel_tol=rel,abs_tol=abs_tol),(a,b)

def run(command,params=None,file=None):
    c=index["depth-"+command]
    result=subprocess.run([str(binary),c["command"],str(file or root/c["file"]),*map(str,c["params"] if params is None else params)],capture_output=True,text=True,check=True,timeout=30)
    r=list(csv.reader(io.StringIO(result.stdout)))
    return r[0],[[float(v) for v in row] for row in r[1:]],result.stdout

def write_input(path,header,rows):
    with path.open("w",encoding="utf-8",newline="") as f:
        w=csv.writer(f)
        w.writerow(header)
        w.writerows(rows)
    return path

def report(name,header,rows):
    assert rows and all(len(r)==len(header) for r in rows),name
    write_input(studies/(name+".csv"),header,rows)

def call_bs(s,k,r,q,v,t):
    d1=(math.log(s/k)+(r-q+.5*v*v)*t)/(v*math.sqrt(t))
    d2=d1-v*math.sqrt(t)
    N=lambda z:.5*math.erfc(-z/math.sqrt(2))
    return s*math.exp(-q*t)*N(d1)-k*math.exp(-r*t)*N(d2)

for c in catalog:
    h,r,text=run(c["command"][6:])
    assert h==c["header"],c["command"]
    assert len(r)==len(c["expected"]),c["command"]
    for a,b in zip(r,c["expected"]):
        assert len(a)==len(b),c["command"]
        for u,v in zip(a,b):
            try: close(u,v)
            except AssertionError as e: raise AssertionError((c["command"],a,b)) from e
    (cases/c["file"]).write_text(text,encoding="utf-8")

with tempfile.TemporaryDirectory() as d:
    f=Path(d)/"input.csv"
    # 1. Ill-conditioned covariance: spectrum, precision and geometry.
    rows=[]
    for rho in [0,.2,.5,.8,.95,.99,.999]:
        write_input(f,["A1","A2"],[[1,rho],[rho,1]])
        _,s,_=run("two-by-two-spectrum",[],f)
        _,a,_=run("precision-conditional",[],f)
        write_input(f,["Cov1","Cov2","Delta"],[[1,rho,1],[rho,1,-1]])
        _,b,_=run("mahalanobis-residual",[],f)
        close(s[0][0],1-rho)
        close(s[0][1],1+rho)
        close(a[0][2],1-rho*rho)
        close(b[0][1],2/(1-rho))
        rows.append([rho,s[0][0],s[0][1],s[0][1]/s[0][0],a[0][1],a[0][2],b[0][1],max(r[-1] for r in a)])
    report("covariance_geometry",["Correlation","EigenLow","EigenHigh","ConditionNumber","PrecisionDiagonal","ConditionalVariance","ContrastDistanceSquared","SolveResidual"],rows)

    # 2. Feasible allocation and robust expected return under capacity limits.
    rows=[]
    for cap in [.35,.45,.55,.65,.75,.85,1]:
        write_input(f,["Utility","Curvature","Lower","Upper"],[[.07,.1,0,cap],[.03,.12,0,1]])
        _,a,_=run("bounded-budget",[1],f)
        w=[r[1] for r in a]
        close(sum(w),1)
        assert max(r[-1] for r in a)<1e-9
        assert w[0]<=cap+1e-12
        write_input(f,["Weight","Mean","UncertaintyScale"],[[w[0],.07,.02],[w[1],.03,.03]])
        for radius in [0,1,2,3]:
            _,b,_=run("ellipsoid-worst-mean",[radius],f)
            close(b[0][2],.07*w[0]+.03*w[1]-radius*math.hypot(.02*w[0],.03*w[1]))
            rows.append([cap,radius,*w,a[0][2],*b[0],max(r[-1] for r in a)])
    report("allocation_capacity_uncertainty",["AssetOneCap","Radius","Weight1","Weight2","BudgetMultiplier","NominalReturn","UncertaintyNorm","WorstCaseReturn","KKTResidual"],rows)

    # 3. Tilt common credit states, evaluate the entire default count and variance.
    rows=[]
    for theta in [-2,-1,0,1,2,3]:
        _,a,_=run("exponential-tilt",[theta])
        weights=[r[1] for r in a]
        write_input(f,["ConditionalPD","ScenarioProbability"],[[.1,weights[0]],[.5,weights[1]]])
        _,b,_=run("latent-binomial",[5],f)
        mean=sum(r[0]*r[1] for r in b)
        var=sum((r[0]-mean)**2*r[1] for r in b)
        tail=sum(r[1] for r in b if r[0]>=3)
        write_input(f,["Scenario","ScenarioProbability","LossGivenDefault","ConditionalPD"],[[j+1,w,1,q] for j,(w,q) in enumerate(zip(weights,[.1,.5])) for _ in range(5)])
        _,c,_=run("heterogeneous-credit-mixture",[],f)
        close(sum(r[1] for r in b),1)
        close(mean,c[0][0]);close(var,c[0][3])
        q=.1*weights[0]+.5*weights[1]
        iidtail=sum(math.comb(5,k)*q**k*(1-q)**(5-k) for k in range(3,6))
        rows.append([theta,weights[1],a[0][-1],mean,c[0][1],c[0][2],var,tail,iidtail,tail-iidtail])
    report("credit_tilt_tail",["Tilt","StressStateProbability","KL","ExpectedDefaults","WithinVariance","BetweenVariance","TotalVariance","MixtureTailGE3","IndependentTailGE3","TailDifference"],rows)

    # 4. Two distinct estimands under selective observation: full-population and always-selected.
    rows=[]
    for nsel in [3,4,5,6]:
        treatment=[[1,int(i<nsel),i+1 if i<nsel else 0] for i in range(6)]
        control=[[0,int(i<3),[2,4,6][i] if i<3 else 0] for i in range(6)]
        write_input(f,["Treatment","Selected","Outcome"],treatment+control)
        _,a,_=run("lee-trimming-bounds",[],f)
        write_input(f,["Observed","Outcome"],[r[1:] for r in treatment])
        _,b,_=run("manski-missing-bounds",[0,10],f)
        write_input(f,["Observed","Outcome"],[r[1:] for r in control])
        _,c,_=run("manski-missing-bounds",[0,10],f)
        lower=b[0][3]-c[0][4];upper=b[0][4]-c[0][3]
        assert lower<=upper and a[0][-2]<=a[0][-1]
        close(a[0][2],3/nsel)
        rows.append([nsel,*a[0],lower,upper])
    report("selection_identification",["SelectedTreatmentCount","TreatmentSelection","ControlSelection","RetainFraction","ControlMean","LeeLower","LeeUpper","FullPopulationLower","FullPopulationUpper"],rows)

    # 5. Dynamic impulse response and conditional forecast uncertainty.
    rows=[]
    write_input(f,["Horizon"],[[1],[2],[3],[5],[10],[20],[50]])
    for phi in [-.7,0,.5,.9,.99]:
        _,a,_=run("ar-impulse",[phi],f)
        _,b,_=run("ar-forecast-variance",[phi,2],f)
        for r,v in zip(a,b):
            h=int(r[0])
            close(r[1],phi**h)
            close(r[2],sum(phi**j for j in range(h+1)))
            close(v[1],2*sum(phi**(2*j) for j in range(h)))
            rows.append([phi,h,r[1],r[2],v[1],v[2],v[1]/v[2]])
    report("ar_persistence_uncertainty",["Phi","Horizon","Impulse","CumulativeImpulse","ForecastErrorVariance","LongRunVariance","VarianceFraction"],rows)

    # 6. Focal loss derivatives checked against independent finite differences.
    rows=[]
    def focal(z,y,g):
        t=(2*y-1)*z
        lp=-math.log1p(math.exp(-t)) if t>=0 else t-math.log1p(math.exp(t))
        return -((-math.expm1(lp))**g)*lp if g else -lp
    write_input(f,["Logit","Label"],[[z,y] for z in [-6,-2,0,2,6] for y in [0,1]])
    for gamma in [0,.5,1,2,4]:
        _,a,_=run("binary-focal",[gamma],f)
        for z,y,L,g in a:
            eps=1e-5
            ref=(focal(z+eps,y,gamma)-focal(z-eps,y,gamma))/(2*eps)
            close(L,focal(z,y,gamma))
            close(g,ref,rel=2e-5,abs_tol=1e-8)
            rows.append([gamma,z,y,L,g,ref,g-ref])
    report("focal_gradient",["Gamma","Logit","Label","Loss","AnalyticDerivative","FiniteDifference","Difference"],rows)

    # 7. Fixed disjoint training, calibration and test windows; conditional coverage.
    rows=[]
    write_input(f,["Start","End"],[[i,i+1] for i in range(30)])
    _,purge,_=run("purge-interval",[20,30,2],f)
    train=[r for r in purge if r[1]<10 and r[-1]==1]
    calibration=[r for r in purge if 10<=r[1]<20 and r[-1]==1]
    assert len(train)==10 and len(calibration)==10
    write_input(f,["TrainFlag","Value"],[[int(i<10),i] for i in range(30)])
    _,scales,_=run("train-only-standardization",[],f)
    close(scales[-1][3],4.5)
    cal=[[0,v] for v in [1,2,3,4,5]]+[[1,v] for v in [2,4,6,8,10]]
    tests={0:[1,4,5,6,7],1:[3,6,9,12,15]}
    for alpha in [.2,.4,.5]:
        write_input(f,["Group","Nonconformity"],cal)
        _,a,_=run("mondrian-calibration",[alpha],f)
        records=[]
        for group,n,k,q in a:
            values=tests[int(group)]
            hit=sum(v<=q for v in values)
            records.append((group,n,k,q,hit,len(values)))
        write_input(f,["Successes","Trials"],[[r[4],r[5]] for r in records])
        _,b,_=run("wilson-coverage",[1.96],f)
        for r,w in zip(records,b):
            close(w[2],r[4]/r[5])
            assert w[3]<=w[2]<=w[4]
            rows.append([alpha,*r,w[2],w[3],w[4],len(train),scales[0][3],scales[0][4]])
    report("conditional_coverage",["Alpha","Group","CalibrationCount","Rank","Threshold","TestHits","TestCount","ObservedCoverage","WilsonLower","WilsonUpper","TrainingCount","FrozenMean","FrozenScale"],rows)

    # 8. Independent full repricing, cross Greeks and joint spot-vol approximation.
    rows=[]
    for spot in [80,90,100,110,120]:
        for vol in [.1,.2,.4]:
            K=100;r=.02;q=.01;T=1
            write_input(f,["Spot","Strike","Rate","Dividend","Vol","Time"],[[spot,K,r,q,vol,T]])
            _,a,_=run("digital-decomposition",[],f)
            _,b,_=run("vanna-volga",[],f)
            c0=call_bs(spot,K,r,q,vol,T)
            close(a[0][2],c0)
            ev=1e-4;es=.01
            vega=(call_bs(spot,K,r,q,vol+ev,T)-call_bs(spot,K,r,q,vol-ev,T))/(2*ev)
            volga=(call_bs(spot,K,r,q,vol+ev,T)-2*c0+call_bs(spot,K,r,q,vol-ev,T))/ev**2
            vanna=(call_bs(spot+es,K,r,q,vol+ev,T)-call_bs(spot+es,K,r,q,vol-ev,T)-call_bs(spot-es,K,r,q,vol+ev,T)+call_bs(spot-es,K,r,q,vol-ev,T))/(4*es*ev)
            close(b[0][0],vega,rel=2e-5,abs_tol=1e-6)
            close(b[0][1],vanna,rel=2e-4,abs_tol=1e-5)
            close(b[0][2],volga,rel=2e-4,abs_tol=2e-4)
            d1=(math.log(spot/K)+(r-q+.5*vol**2)*T)/(vol*math.sqrt(T))
            delta=math.exp(-q*T)*.5*math.erfc(-d1/math.sqrt(2))
            gamma=math.exp(-q*T)*math.exp(-d1*d1/2)/(spot*vol*math.sqrt(2*math.pi*T))
            write_input(f,["Position","Delta","Gamma","Vega","Vanna","Volga"],[[1,delta,gamma,*b[0]]])
            for scale in [.25,.5,1]:
                ds=scale;dv=.005*scale
                _,c,_=run("spot-vol-taylor",[ds,dv],f)
                exact=call_bs(spot+ds,K,r,q,vol+dv,T)-c0
                rows.append([spot,vol,scale,c0,*b[0],exact,c[0][-1],exact-c[0][-1]])
    report("option_joint_repricing",["Spot","Vol","ShockScale","CallPrice","Vega","Vanna","Volga","ExactPnL","TaylorPnL","ApproximationError"],rows)

    # 9. Zero curve, forward coupons, swap annuity and cashflow consistency.
    rows=[]
    for rate in [-.01,0,.02,.05,.1]:
        times=[.5,1,1.5,2]
        D=[math.exp(-rate*t) for t in times]
        write_input(f,["Time","Discount"],[[0,1]]+list(zip(times,D)))
        _,a,_=run("log-discount-forward",[],f)
        for r in a: close(r[-1],rate)
        write_input(f,["Accrual","Discount"],[[.5,v] for v in D[1:]])
        _,b,_=run("forward-swap-annuity",[D[0],100,.03],f)
        L=[(D[j-1]/D[j]-1)/.5 for j in range(1,len(D))]
        floatpv=100*sum(.5*l*d for l,d in zip(L,D[1:]))
        close(b[0][3],floatpv)
        close(b[0][1],(D[0]-D[-1])/sum(.5*d for d in D[1:]))
        write_input(f,["FixingRate","Accrual"],[[l,.5] for l in L])
        _,c,_=run("fra-settlement",[100,.03],f)
        for row in c: close(row[3]*(1+row[0]*row[1]),row[2])
        rows.append([rate,*b[0],floatpv,b[0][3]-floatpv])
    report("curve_swap_fra",["ContinuousRate","Annuity","ParRate","FixedPV","FloatPV","ReceiveFixedPV","CouponPV01","CouponByCouponFloatPV","Reconciliation"],rows)

    # 10. Exact two-branch stopping problem and primal-dual martingale certificate.
    rows=[]
    for h0 in [0,1,2,3,4,5,6]:
        H=[[h0,3,2],[h0,0,4]]
        V=[max(h[1:]) for h in H]
        cont=sum(V)/2
        exact=max(h0,cont)
        policy=[[int(h0>=cont),int(h[1]>=h[2])] for h in H]
        write_input(f,["H0","H1","H2","Exercise0","Exercise1"],[h+p for h,p in zip(H,policy)])
        _,a,_=run("stopping-policy-value",[],f)
        lower=a[-1][-1]
        M=[[0,v-cont,v-cont] for v in V]
        close(sum(m[1] for m in M)/2,0)
        write_input(f,["H0","H1","H2","M0","M1","M2"],[h+m for h,m in zip(H,M)])
        _,b,_=run("dual-stopping-bound",[],f)
        upper=b[-1][-1]
        close(lower,exact);close(upper,exact)
        rows.append([h0,cont,exact,lower,upper,upper-lower])
    report("stopping_primal_dual",["InitialPayoff","ContinuationValue","SnellValue","PolicyValue","DualUpper","Gap"],rows)

    # 11. Noise contamination and expected gamma variance P&L.
    rows=[]
    write_input(f,["Start","End","VolStart","VolEnd"],[[1,2,.2,.2]])
    _,a,_=run("forward-variance",[],f)
    implied=a[0][-2]
    for noise in [0,1e-6,5e-6,1e-5]:
        write_input(f,["Intervals"],[[10],[50],[100],[500],[1000]])
        _,b,_=run("noise-signature",[implied,noise],f)
        for n,efficient,bias,rv in b:
            gamma_dollars=100
            efficient_pnl=gamma_dollars*(efficient-implied)
            apparent_pnl=gamma_dollars*(rv-implied)
            close(apparent_pnl-efficient_pnl,gamma_dollars*2*n*noise)
            rows.append([noise,n,implied,efficient,bias,rv,efficient_pnl,apparent_pnl])
    report("gamma_noise_bias",["NoiseVariance","Intervals","ImpliedForwardVariance","EfficientVariance","NoiseBias","ExpectedMeasuredRV","TrueExpectedGammaPnL","ApparentGammaPnL"],rows)

    # 12. Two-period convex execution: solve analytically and check the certificate.
    rows=[]
    Q=10;eta1=1;eta2=2
    for lam in [0,.1,.25,.5,1,2,5,10]:
        q1=Q*(eta2+lam)/(eta1+eta2+lam)
        q2=Q-q1
        write_input(f,["Trade","ImpactCurvature","RiskCoefficient"],[[q1,eta1,lam],[q2,eta2,0]])
        _,a,_=run("execution-kkt",[Q],f)
        cost=sum(r[3]+r[4] for r in a)
        assert max(r[-1] for r in a)<1e-9
        uniform=eta1*(Q/2)**2+eta2*(Q/2)**2+lam*(Q/2)**2
        assert cost<=uniform+1e-9
        rows.append([lam,q1,q2,cost,uniform,uniform-cost,max(r[-1] for r in a)])
    report("execution_risk_tradeoff",["RiskCoefficient","EarlyTrade","LateTrade","OptimalCost","UniformCost","CostImprovement","KKTResidual"],rows)

    # 13. Queue uncertainty and independent exponential event-clock conservation.
    rows=[]
    for ahead in [0,2,5,8,10]:
        _,a,_=run("queue-cancel-bounds",[ahead,3])
        for r in a:
            assert 0<=r[3]<=r[4]<=3 and r[1]<=r[2]
        for H in [.1,.5,1]:
            _,b,_=run("competing-execution-hazards",[H])
            for r in b: close(sum(r[:3]),1)
            rows.append([ahead,H,a[-1][3],a[-1][4],*b[0][:3]])
    report("queue_and_event_risk",["InitialAhead","Horizon","QueueMinFill","QueueMaxFill","ClockFillProbability","ClockCancelProbability","ClockNoEvent"],rows)

    # 14. Version-aware data join, deliberately comparing an invalid hindsight join.
    rows=[]
    records=[[0,1,1,3,10],[0,1,1,5,12],[0,1,4,6,20],[0,1,4,9,25],[0,2,2,7,40]]
    queries=[[1,asset,t,t,0] for asset in [1,2] for t in range(2,11)]
    write_input(f,["Type","Asset","EffectiveTime","AvailableTime","Value"],records+queries)
    _,a,_=run("bitemporal-asof",[100],f)
    for row in a:
        _,asset,t,found,e,av,value=row
        legal=[r for r in records if r[1]==asset and r[2]<=t and r[3]<=t]
        wrong=[r for r in records if r[1]==asset and r[2]<=t]
        l=max(legal,key=lambda r:(r[2],r[3])) if legal else None
        w=max(wrong,key=lambda r:(r[2],r[3])) if wrong else None
        close(found,int(l is not None))
        if l: close(value,l[4]);assert av<=t
        rows.append([asset,t,found,e,av,value,int(w is not None),w[4] if w else 0,int(w is not None and w[3]>t)])
    report("revision_lookahead",["Asset","DecisionTime","Found","EffectiveTime","AvailableTime","LegalValue","HindsightFound","HindsightValue","HindsightUsesFuture"],rows)

    # 15. Sector demand network and regional shift-share exposure.
    rows=[]
    A=[[.2,.1],[.1,.2]]
    base_d=[9,9]
    base_x=[90/7,90/7]
    for shock in [-.2,-.1,0,.1,.2,.4]:
        d0=base_d[0]*(1+shock)
        write_input(f,["Input1","Input2","FinalDemand"],[[*A[0],d0],[*A[1],base_d[1]]])
        _,a,_=run("leontief-demand",[1e-12,200],f)
        output=[r[2] for r in a]
        close(output[0],(.8*d0+.1*base_d[1])/.63)
        close(output[1],(.1*d0+.8*base_d[1])/.63)
        growth=[output[j]/base_x[j]-1 for j in range(2)]
        write_input(f,["Region","Industry","BaseShare","IndustryShock"],[[r+1,j+1,w,growth[j]] for r,weights in enumerate([[.75,.25],[.25,.75]]) for j,w in enumerate(weights)])
        _,b,_=run("shift-share-exposure",[],f)
        close(b[0][2],.75*growth[0]+.25*growth[1])
        close(b[1][2],.25*growth[0]+.75*growth[1])
        rows.append([shock,*output,*growth,b[0][2],b[1][2]])
    report("sector_network_exposure",["FinalDemandShock","Sector1Output","Sector2Output","Sector1Growth","Sector2Growth","Region1Exposure","Region2Exposure"],rows)

    # 16. Tie-aware portfolio formation: ranking symmetry and net exposure.
    rows=[]
    scores=[0,1,1,2,2,3]
    ret=[-.03,.01,.02,.04,-.01,.06]
    for frac in [.1,.2,.25,.3333333333333333,.4,.5]:
        write_input(f,["Score"],[[s] for s in scores])
        _,a,_=run("tie-aware-tail-weights",[frac],f)
        close(sum(r[2] for r in a),1);close(sum(r[3] for r in a),1);close(sum(r[4] for r in a),0)
        write_input(f,["Score"],[[s] for s in reversed(scores)])
        _,b,_=run("tie-aware-tail-weights",[frac],f)
        for r,v in zip(a,reversed(b)):
            close(r[4],v[4])
        gross=sum(abs(r[4]) for r in a)
        pnl=sum(r[4]*v for r,v in zip(a,ret))
        for r,v in zip(a,ret):
            rows.append([frac,r[0],r[1],r[2],r[3],r[4],v,r[4]*v,gross,pnl])
    report("factor_tie_portfolios",["TailFraction","Asset","Score","LongWeight","ShortWeight","NetWeight","RealizedReturn","Contribution","NetGrossExposure","PortfolioReturn"],rows)

print(f"Completed {len(catalog)} independent research workflows and 16 integrated studies")
