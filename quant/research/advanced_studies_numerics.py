"""Independent matrix, stochastic-process, and market microstructure experiments."""
import math
from itertools import product
from advanced_study_common import near,dot,solve,ridge,inv,mv,mm,transpose,grid

def studies(model,report):
    # 25. GMRES solution and true residual on nonsymmetric systems.
    rows=[]
    for d in [3,5,8]:
        A=[[3.+.1*i if i==j else .12*math.sin(1.3*(i+1)*(j+2)) for j in range(d)] for i in range(d)]
        truth=[math.cos(.7*i) for i in range(d)];b=mv(A,truth);ref=solve(A,b)
        for restart in [1,min(3,d),d]:
            fit=model("gmres-restarted",[r+[v] for r,v in zip(A,b)],[restart,1e-10,3000])
            for j,r in enumerate(fit):near(r[1],ref[j]);near(r[2],0);rows.append([d,restart,*r,ref[j]])
    report("gmres_true_residual",["Dimension","Restart","Index","Solution","TrueResidual","ResidualNorm","Iterations","ReferenceSolution"],rows)

    # 26. Prespecified orthogonal eigenbasis and full spectral reconstruction.
    rows=[]
    for angle in [.1,.4,.8,1.2]:
        c=math.cos(angle);s=math.sin(angle);Q=[[c,-s,0],[s,c,0],[0,0,1]]
        for lam in [[5,2,.3],[3,1,-.4],[2,2,.5]]:
            A=mm(mm(Q,[[lam[i] if i==j else 0 for j in range(3)] for i in range(3)]),transpose(Q))
            fit=model("jacobi-eigensystem",A,[1e-12,5000]);V=[[fit[3*i+j][3] for j in range(3)] for i in range(3)]
            vals=[fit[3*i][2] for i in range(3)]
            for u,v in zip(vals,sorted(lam,reverse=True)):near(u,v)
            rec=mm(mm(transpose(V),[[vals[i] if i==j else 0 for j in range(3)] for i in range(3)]),V)
            for i in range(3):
                for j in range(3):near(rec[i][j],A[i][j]);near(dot(V[i],V[j]),float(i==j))
            for r in fit:rows.append([angle,lam[0],lam[1],lam[2],*r])
    report("jacobi_spectral_reconstruction",["Angle","Eigenvalue1","Eigenvalue2","Eigenvalue3","EigenIndex","Coordinate","Eigenvalue","Eigenvector","ReconstructionResidual","OrthogonalityResidual","Iterations"],rows)

    # 27. Equality constrained ridge against an independent indefinite KKT solve.
    rows=[];X=[[1,math.sin(.3*i),math.cos(.7*i)] for i in range(20)];y=[.3+.8*r[1]-.5*r[2]+.2*math.sin(1.7*i) for i,r in enumerate(X)]
    A=[[1,1,1],[0,1,-1]]
    for lam in [.01,.1,1,5]:
        for target in [[1,0],[0,.2],[.5,1]]:
            H=[[sum(r[i]*r[j] for r in X)+(lam if i==j else 0) for j in range(3)] for i in range(3)]
            rhs=[sum(r[j]*v for r,v in zip(X,y)) for j in range(3)]
            K=[H[i]+[A[j][i] for j in range(2)] for i in range(3)]+[r+[0,0] for r in A]
            ref=solve(K,rhs+target)[:3]
            data=[[0,*r,v] for r,v in zip(X,y)]+[[1,*r,v] for r,v in zip(A,target)]
            fit=model("equality-ridge",data,[lam])
            for r,v in zip(fit,ref):near(r[1],v);near(r[3],0);near(r[4],0);rows.append([lam,*target,*r,v])
    report("equality_ridge_kkt_reference",["Penalty","BudgetTarget","ContrastTarget","Feature","Coefficient","Objective","EqualityResidual","StationarityResidual","ReferenceCoefficient"],rows)

    # 28. Two-state continuous-time chain closed form and truncation bounds.
    rows=[]
    for a,b in [(.2,.1),(1,.4),(3,2)]:
        for t in [.1,1,3]:
            for K in [4,12,60]:
                fit=model("ctmc-uniformization",[[-a,a],[b,-b]],[t,K]);e=math.exp(-(a+b)*t)
                exact=[[b/(a+b)+a/(a+b)*e,a/(a+b)*(1-e)],[b/(a+b)*(1-e),a/(a+b)+b/(a+b)*e]]
                for r in fit:
                    v=exact[int(r[0])-1][int(r[1])-1]
                    assert -1e-10<=v-r[2]<=r[3]+1e-10
                    near(r[4],0);rows.append([a,b,t,K,*r,v])
    report("ctmc_closed_form_bound",["Rate01","Rate10","Time","Cutoff","From","To","Probability","PoissonTail","RowMassResidual","ExactProbability"],rows)

    # 29. Two-by-two entropic transport: independent scalar convex minimizer.
    rows=[]
    for a,b in [(.3,.6),(.45,.2),(.7,.7)]:
        for eps in [.1,.4,1]:
            C=[[0,1.2],[.8,.1]]
            lo=max(0,a+b-1)+1e-14;hi=min(a,b)-1e-14
            def deriv(z):return C[0][0]-C[0][1]-C[1][0]+C[1][1]+eps*math.log(z*(1-a-b+z)/((a-z)*(b-z)))
            for _ in range(100):
                m=(lo+hi)/2
                if deriv(m)<0:lo=m
                else:hi=m
            z=(lo+hi)/2;ref=[[z,a-z],[b-z,1-a-b+z]]
            fit=model("sinkhorn-log",[C[0]+[a],C[1]+[1-a]],[eps,1e-10,10000,b,1-b])
            for r in fit:
                v=ref[int(r[0])-1][int(r[1])-1];near(r[2],v,atol=2e-8);near(r[5],0)
                rows.append([a,b,eps,*r,v])
    report("sinkhorn_scalar_transport",["SourceMass1","TargetMass1","EntropyWeight","Source","Target","Coupling","Cost","EntropyRegularizedObjective","MarginalResidual","Iterations","ReferenceCoupling"],rows)

    # 30. Trust-region KKT from independent spectral multiplier evaluation.
    rows=[];H=[[2,.4],[.4,1]];g=[-3,2]
    for radius in [.1,.5,1,2,5]:
        fit=model("trust-region-quadratic",[r+[v] for r,v in zip(H,g)],[radius,1e-10])
        nu=fit[0][2];s=[r[1] for r in fit];M=[[H[i][j]+(nu if i==j else 0) for j in range(2)] for i in range(2)]
        ref=solve(M,[-v for v in g]);n=math.sqrt(dot(s,s))
        for u,v in zip(s,ref):near(u,v)
        assert n<=radius+1e-8 and nu>=0;near(nu*(n-radius),0)
        obj=dot(g,s)+.5*dot(s,mv(H,s));near(fit[0][4],obj)
        for r in fit:rows.append([radius,*r])
    report("trust_region_certificate",["Radius","Index","Step","Multiplier","StepNorm","Objective","StationarityResidual","ComplementarityResidual"],rows)

    # 31. ACD grid fit and held-out frozen forecasts.
    rows=[];d=[.7+.5*math.sin(.31*t)**2+.3*(1+math.cos(1.7*t)) for t in range(70)];nt=50
    for am,bm,g in [(.5,.4,6),(.7,.2,8),(.2,.7,8)]:
        mean=sum(d[:nt])/nt
        def sequence(a,b):
            w=(1-a-b)*mean;psi=mean;out=[]
            for v in d:
                out.append((psi,math.log(psi)+v/psi));psi=w+a*v+b*psi
            return out
        f,a,b=min((sum(r[1] for r in sequence(a,b)[:nt]),a,b) for a in grid(0,am,g) for b in grid(0,bm,g) if a+b<1)
        fit=model("acd-duration-grid",[[v] for v in d],[nt,am,bm,g])
        aa,bb=fit[0][6:8];ref=sequence(aa,bb)
        near(fit[0][9],f)
        for r,v in zip(fit,ref):near(r[3],v[0]);near(r[5],v[1]);rows.append([am,bm,g,*r])
    report("acd_frozen_duration_grid",["AlphaMax","BetaMax","GridCount","Time","Train","Duration","ExpectedDuration","StandardizedDuration","ConditionalNLL","Alpha","Beta","Omega","TrainingNLL"],rows)

    # 32. Known distributed lag and frozen held-out signal.
    rows=[];flow=[math.sin(.7*t)+.35*math.cos(1.23*t) for t in range(90)]
    y=[sum(w*flow[t-j] for j,w in enumerate([.8,-.3,.15],1) if t>=j)+.03*math.cos(1.8*t) for t in range(90)]
    data=list(map(list,zip(flow,y)));nt=60
    for L in [2,3,5]:
        for lam in [.01,.1,1]:
            design=lambda t:[flow[t-j] for j in range(1,L+1)]
            ref=ridge([design(t) for t in range(L,nt)],y[L:nt],lam)
            fit=model("flow-propagator-ridge",data,[nt,L,lam])
            for r in fit:
                t=int(r[0])-1;near(r[2],dot(ref,design(t)))
                for j in range(L):near(r[4+L+j],ref[j]);near(r[4+2*L+j],sum(ref[:j+1]))
                rows.append([L,lam,*r[:4],sum(ref),dot(ref,design(t))])
    report("flow_propagator_frozen",["LagCount","Ridge","Time","Actual","Forecast","Error","CumulativeKernel","ReferenceForecast"],rows)

    # 33. Pure-death Erlang absorption law; births use a dense Markov reference.
    rows=[]
    for q in [1,3,5]:
        for birth in [0,.4,1.2]:
            death=1.3;M=7;times=[.1,.5,1,2,4];fit=model("queue-fill-uniformization",[[t] for t in times],[q,birth,death,M,100])
            for r in fit:
                t=r[0]
                if birth==0:ref=1-math.exp(-death*t)*sum((death*t)**j/math.factorial(j) for j in range(q))
                else:
                    nu=birth+death;P=[[0.]*(M+1) for _ in range(M+1)];P[0][0]=1
                    for i in range(1,M+1):
                        P[i][i-1]=death/nu
                        if i<M:P[i][i+1]=birth/nu
                        P[i][i]=1-sum(P[i])
                    v=[float(i==q) for i in range(M+1)];w=math.exp(-nu*t);ref=0
                    for k in range(1,101):
                        v=mv(transpose(P),v);w*=nu*t/k;ref+=w*v[0]
                near(r[2],ref);near(r[2]+r[3]+r[4],1);rows.append([q,birth,death,*r,ref])
    report("queue_absorption_reference",["Queue","BirthRate","DeathRate","Time","InitialQueue","FillProbability","OutstandingProbability","TruncationTail","MassResidual","ReferenceFill"],rows)

    # 34. Bivariate Hawkes intensities recomputed by direct history sums.
    rows=[];events=[[.2,1],[.4,2],[.55,1],[1.1,1],[1.8,2],[2.2,1],[2.7,2]]
    for beta in [.5,1,3]:
        for A in [[[.1,.2],[.05,.15]],[[.3,.1],[.2,.2]]]:
            mu=[.7,1.1];T=3.2;fit=model("bivariate-hawkes-likelihood",events,[T,beta,*mu,*A[0],*A[1]])
            comp=[mu[j]*T+sum(A[j][int(v)-1]*(-math.expm1(-beta*(T-t))) for t,v in events) for j in range(2)]
            ll=-sum(comp)
            for k,r in enumerate(fit):
                t,v=events[k];z=[mu[j]+sum(A[j][int(m)-1]*beta*math.exp(-beta*(t-u)) for u,m in events[:k]) for j in range(2)]
                near(r[3],z[0]);near(r[4],z[1]);near(r[5],math.log(z[int(v)-1]));ll+=math.log(z[int(v)-1])
                near(r[6],comp[0]);near(r[7],comp[1]);rows.append([beta,*A[0],*A[1],*r])
            near(fit[0][8],ll)
    report("bivariate_hawkes_history_sum",["Decay","A11","A12","A21","A22","Event","Time","Type","Intensity1","Intensity2","EventLogIntensity","Compensator1","Compensator2","LogLikelihood","SpectralRadius"],rows)

    # 35. Parzen kernel as a direct quadratic form over all return pairs.
    rows=[]
    for noise in [0,.002,.01]:
        r=[.003*math.sin(.31*i)+noise*(-1)**i for i in range(80)]
        for H in [0,1,3,8,15]:
            def k(h):
                z=abs(h)/(H+1)
                return 1-6*z*z+6*z*z*z if z<=.5 else 2*(1-z)**3 if z<=1 else 0
            ref=sum(r[i]*r[j]*k(i-j) for i in range(len(r)) for j in range(len(r)))
            fit=model("parzen-realized-kernel",[[v] for v in r],[H])[0]
            near(fit[2],ref);near(fit[3],ref-sum(v*v for v in r));rows.append([noise,*fit,ref])
    report("parzen_kernel_quadratic_form",["NoiseScale","Bandwidth","RealizedVariance","RealizedKernel","AutocovarianceAdjustment","NegativeFlag","ReferenceKernel"],rows)

    # 36. Structural VAR responses by repeated matrix powers and variance decomposition.
    rows=[]
    A=[[.6,.15],[-.1,.4]]
    for rho in [-.5,0,.5]:
        S=[[1,rho*2],[rho*2,4]];L=[[1,0],[2*rho,2*math.sqrt(1-rho*rho)]]
        data=[A[i]+S[i] for i in range(2)];fit=model("var-structural-irf",data,[12]);P=[[1.,0.],[0.,1.]];cum=[[0.,0.],[0.,0.]];var=[[0.,0.],[0.,0.]]
        for h in range(13):
            ir=mm(P,L)
            for i in range(2):
                for j in range(2):cum[i][j]+=ir[i][j];var[i][j]+=ir[i][j]**2
                for j in range(2):
                    r=fit[4*h+2*i+j];near(r[3],ir[i][j]);near(r[4],cum[i][j]);near(r[5],var[i][j]/sum(var[i]));rows.append([rho,*r])
            P=mm(P,A)
    report("var_structural_irf_reference",["InnovationCorrelation","Horizon","Response","Shock","ImpulseResponse","CumulativeResponse","FEVD"],rows)
