"""Independent numerical certificates and parameter scans."""
import math
from comprehensive_study_common import near,check_vector,solve,mv,dot,transpose,mm,inv,grid

def studies(model,report):
    results=[]
    for s in (.01,.5,1,10):
        X=[[1,s*t] for t in (-3,-1,0,2,4)]
        y=[2+3*r[1]+.02*v for r,v in zip(X,[1,-2,0,2,-1])]
        ref=solve(mm(transpose(X),X),mv(transpose(X),y))
        out=model("rrqr-regression",[[z,*r] for z,r in zip(y,X)],[1e-11])
        check_vector([r[1] for r in out],ref,atol=1e-6)
        residual=[a-dot(b,ref) for a,b in zip(y,X)]
        near(out[0][3],math.sqrt(dot(residual,residual)))
        results.append([s,*ref,out[0][2],out[0][3],out[0][4]])
    report("qr_dense_reference",["Scale","Intercept","Slope","Rank","Residual","NormalResidual"],results)

    results=[]
    for s in (.1,.5,1,3):
        D=[1,2,3];B=[[s,.2],[.5,s],[.1,.4]];b=[1,-2,3]
        A=[[D[i]*(i==j)+dot(B[i],B[j]) for j in range(3)] for i in range(3)]
        ref=solve(A,b)
        det=(A[0][0]*(A[1][1]*A[2][2]-A[1][2]*A[2][1])
             -A[0][1]*(A[1][0]*A[2][2]-A[1][2]*A[2][0])
             +A[0][2]*(A[1][0]*A[2][1]-A[1][1]*A[2][0]))
        out=model("woodbury-risk-solve",[[D[i],*B[i],b[i]] for i in range(3)],[])
        for i,r in enumerate(out):
            near(r[1],ref[i]);near(r[2],math.log(det));near(r[3],dot(b,ref));near(r[4],0)
            results.append([s,*r])
    report("woodbury_dense_covariance",["Scale","Asset","Solution","LogDet","QuadraticForm","Residual"],results)

    results=[]
    for scale in (1,10,100,1000):
        A=[[2,.1,0],[.1,scale+1,.05],[0,.05,3]];b=[1,2,-1];ref=solve(A,b)
        for pre in (0,1):
            out=model("pcg-jacobi",[[*r,y] for r,y in zip(A,b)],[1e-10,100,pre])
            near(out[-1][3],-.5*dot(b,ref),atol=1e-7)
            assert out[-1][4]==1 and out[-1][2]<1e-9
            assert all(y[3]<=x[3]+1e-10 for x,y in zip(out,out[1:]))
            results.extend([[scale,pre,*r] for r in out])
    report("pcg_energy_certificate",["ConditionScale","Precondition","Iteration","Residual","RelativeResidual","Energy","Converged"],results)

    results=[]
    for a in (.2,1,3,7):
        A=[[2,a,0],[a,4,a],[0,a,6]]
        out=model("lanczos-krylov",[[*r,float(i==0)] for i,r in enumerate(A)],[3,1e-12])
        for i,r in enumerate(out):
            near(r[1],A[i][i]);near(r[2],a if i<2 else 0);near(r[3],0);near(r[4],0)
            results.append([a,*r])
    report("lanczos_tridiagonal_identity",["OffDiagonal","Step","Alpha","Beta","OrthogonalityError","ProjectionResidual"],results)

    results=[]
    for lo,hi in ((-3,1),(-1,4),(0,3),(2,9)):
        mid=(lo+hi)/2
        out=model("polytope-analytic-center",[[1,hi],[-1,-lo]],[1e-11,200,mid+.1])[0]
        near(out[1],mid);near(out[2],-2*math.log((hi-lo)/2));near(out[4],(hi-lo)/2)
        results.append([lo,hi,*out])
    report("barrier_interval_center",["Lower","Upper","Variable","Center","Barrier","GradientNorm","MinSlack","Iterations"],results)

    results=[]
    for c in (-5,-1,0,1,5):
        A=[[2,.3],[.3,3]];b=[c,-.2]
        w=max(0,min(1,(3-.3-b[0]+b[1])/(2+3-.6)))
        out=model("simplex-active-set",[[*A[i],b[i]] for i in range(2)],[])
        near(out[0][1],w);near(out[1][1],1-w)
        obj=lambda q:.5*dot([q,1-q],mv(A,[q,1-q]))+dot(b,[q,1-q])
        near(out[0][3],obj(w));assert out[0][3]<=min(obj(j/1000) for j in range(1001))+1e-9
        results.extend([[c,*r] for r in out])
    report("simplex_two_asset_reference",["Linear","Asset","Weight","Multiplier","Objective","StationarityResidual","BudgetResidual","Active"],results)

    results=[]
    for s in (.1,1,3):
        data=[[0,0],[1,s],[3,s],[4,2*s]]
        q=[i/50 for i in range(201)]
        out=model("pchip-monotone",data,q)
        assert all(y[1]>=x[1]-1e-10 for x,y in zip(out,out[1:]))
        assert all(r[2]>=-1e-10 and -1e-10<=r[1]<=2*s+1e-10 for r in out)
        for x,y in data:near(out[round(x*50)][1],y)
        for i,r in enumerate(out[1:-1],1):
            if abs(r[0]-round(r[0]))>.05:near(r[2],(out[i+1][1]-out[i-1][1])/.04,atol=.01*s)
        results.extend([[s,*r] for r in out])
    report("pchip_shape_and_derivative",["Scale","Query","Value","Derivative","LowerKnot","UpperKnot"],results)

    results=[]
    for n in (4,8,16,32):
        nodes=[math.cos(math.pi*(j+.5)/n) for j in range(n)]
        q=[-1+j/50 for j in range(101)]
        data=[[math.exp(z)] for z in nodes]
        out=model("chebyshev-interpolant",data,q)
        errors=[abs(r[1]-math.exp(r[0])) for r in out]
        assert max(errors)<(0.01 if n==4 else 1e-6 if n==8 else 1e-10)
        results.extend([[n,r[0],r[1],math.exp(r[0]),e] for r,e in zip(out,errors)])
    report("chebyshev_analytic_function",["Nodes","Query","Estimate","Exact","Error"],results)
