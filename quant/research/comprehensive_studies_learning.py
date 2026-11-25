"""Likelihood, prediction and sparsity checks against independent formulas."""
import math
from comprehensive_study_common import near,check_vector,solve,mv,dot,transpose,mm,ridge,sigmoid

def studies(model,report):
    results=[]
    for lam in (.1,1,4):
        x=[[1,1,-2],[1,1,-1],[1,2,0],[1,2,.5],[1,3,1],[1,3,2],[0,2,3]]
        out=model("softmax-newton-fit",x,[3,lam,1e-9,100])
        b=out[0][-2:];g=[lam*v for v in b]
        for a,r in zip(x,out):
            z=[b[0]*a[2],b[1]*a[2],0];m=max(z);q=[math.exp(v-m) for v in z];q=[v/sum(q) for v in q]
            check_vector(r[4:7],q)
            if a[0]:
                for j in range(2):g[j]+=a[2]*(q[j]-(a[1]==j+1))
            results.append([lam,r[0],*q,*b])
        assert max(map(abs,g))<1e-7
        changed=[*x[:-1],[0,3,100]]
        again=model("softmax-newton-fit",changed,[3,lam,1e-9,100])
        check_vector(again[0][-2:],b)
    report("softmax_score_and_holdout",["Penalty","Row","P1","P2","P3","Beta1","Beta2"],results)

    results=[]
    for lam in (.5,1,3):
        x=[[1,1,2,2],[1,1,0,0],[1,2,3,3],[1,2,1,1],[0,3,5,100]]
        out=model("ranknet-linear-fit",x,[lam,1e-9,100]);b=out[0][-1]
        # Both training groups contain exactly one ordered pair, each feature difference is two.
        g=lam*b-2/(1+math.exp(2*b))
        near(g,0,atol=1e-7)
        for a,r in zip(x,out):near(r[4],a[3]*b);results.append([lam,r[0],r[4],b,g])
    report("ranknet_pair_score",["Penalty","Row","Score","Coefficient","IndependentGradient"],results)

    results=[]
    for shape in (.5,1,3):
        x=[[1,1,1],[1,2,1],[1,4,1],[0,100,1]]
        lam=1;out=model("gamma-log-glm",x,[shape,lam,1e-9,100]);b=out[0][-1]
        g=shape*(3-7*math.exp(-b))+lam*b
        near(g,0,atol=1e-7)
        for r in out:near(r[3],math.exp(b));near(r[4],math.exp(2*b)/shape);results.append([shape,*r])
    report("gamma_glm_score",["Shape","Row","Train","Actual","Mean","Variance","UnitDeviance","PenalizedNLL","ScoreResidual","Coefficient"],results)

    results=[]
    for lam in (.5,1,2):
        x=[[1,1,-2],[1,2,0],[1,3,2],[1,2,1],[0,1,5]]
        out=model("ordinal-logit-fixed-cuts",x,[3,lam,1e-9,100,-1,1]);b=out[0][-1]
        def objective(v):
            s=.5*lam*v*v
            for train,y,z in x:
                if train:
                    a=sigmoid(-1-v*z);c=sigmoid(1-v*z);q=[a,c-a,1-c]
                    s-=math.log(q[y-1])
            return s
        near((objective(b+1e-5)-objective(b-1e-5))/2e-5,0,atol=1e-6)
        for a,r in zip(x,out):
            u=sigmoid(-1-b*a[2]);v=sigmoid(1-b*a[2]);check_vector(r[4:7],[u,v-u,1-v]);results.append([lam,r[0],*r[4:7],b])
    report("ordinal_probability_derivative",["Penalty","Row","P1","P2","P3","Coefficient"],results)

    results=[]
    X=[[1,t] for t in (-2,-1,0,1,3)];y=[0,1,1,3,6]
    for lam in (.1,1,10):
        out=model("ridge-loocv-gcv-path",[[z,*r] for z,r in zip(y,X)],[lam])[0]
        H=mm(transpose(X),X)
        for j in range(2):H[j][j]+=lam
        b=solve(H,mv(transpose(X),y));err=[]
        for i in range(len(y)):
            xx=X[:i]+X[i+1:];yy=y[:i]+y[i+1:];h=mm(transpose(xx),xx)
            for j in range(2):h[j][j]+=lam
            bb=solve(h,mv(transpose(xx),yy));err.append((y[i]-dot(X[i],bb))**2)
        near(out[1],sum(err)/len(y));check_vector(out[-2:],b)
        results.append([lam,out[1],sum(err)/len(y),out[2],out[4],*b])
    report("ridge_explicit_leave_one_out",["Penalty","LOOCV","ExplicitLOOCV","GCV","EffectiveDF","Beta0","Beta1"],results)

    results=[]
    for n1 in (1,2,4,7):
        x=[[1,0]]*n1+[[0,1]]*(8-n1)
        out=model("label-shift-prior-em",x,[1e-10,100,.3,.7])
        for r in out:
            near(r[5],n1/8);near(r[6],1-n1/8);near(r[3]+r[4],1)
        results.append([n1,out[0][5],out[0][6],out[0][7],out[0][8]])
    report("label_shift_exact_separation",["PositiveCount","Prior1","Prior2","LogLikelihood","FixedPointResidual"],results)

    results=[]
    for lam in (.5,1,2):
        x=[[y,1] for y in (-3,-2,-1,1,2,3)]
        out=model("mixture-regression-em",x,[1,lam,1e-8,10000,.5,-2,2]);b1,b2=out[0][-2:];prior=out[0][4]
        g=[-lam*b1,-lam*b2];mass=0
        for a,r in zip(x,out):
            z=a[0];q1=prior*math.exp(-.5*(z-b1)**2);q2=(1-prior)*math.exp(-.5*(z-b2)**2);q=q1/(q1+q2)
            near(r[1],q);g[0]+=q*(z-b1);g[1]+=(1-q)*(z-b2);mass+=q
            results.append([lam,z,r[1],prior,b1,b2])
        near(mass/len(x),prior,atol=1e-6);assert max(map(abs,g))<1e-6
    report("mixture_responsibility_score",["Penalty","Y","Responsibility","Prior","Beta1","Beta2"],results)

    results=[]
    for lam in (.1,1,3,6):
        x=[[3,4,1,0],[0,0,0,1]]
        out=model("multitask-group-lasso",x,[2,lam,1e-9,100])
        b=[max(0,1-lam/5)*v for v in (3,4)]
        check_vector(out[0][-2:],b);check_vector(out[1][-2:],[0,0]);near(out[0][1],math.hypot(*b))
        results.extend([[lam,*r] for r in out])
    report("group_lasso_orthogonal_proximal",["Penalty","Feature","CoefficientNorm","Objective","KKTResidual","Beta1","Beta2"],results)
