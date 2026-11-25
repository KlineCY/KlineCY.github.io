"""Connected prediction, calibration, preprocessing and uncertainty research."""
import math
from comprehensive_study_common import near,check_vector,solve,transpose,mm,mv,dot

def transformed_softmax(model,report):
    output=[]
    for extreme in (5,50,500):
        raw=[[-3,1],[-2,1],[0,2],[.5,2],[2,3],[3,3],[extreme,3]]
        data=[[int(i<6),a] for i,(a,_) in enumerate(raw)]
        trans=model("train-only-winsor-standardize",data,[0,1,1e-12])
        x=[[int(i<6),b,r[-1]] for i,((a,b),r) in enumerate(zip(raw,trans))]
        rows=model("softmax-newton-fit",x,[3,1,1e-9,100])
        coef=rows[0][-2:]
        train=[r[0] for r in raw[:6]];m=sum(train)/6;s=math.sqrt(sum((v-m)**2 for v in train)/6)
        for i,r in enumerate(rows):
            z=(min(3,max(-3,raw[i][0]))-m)/s
            a=[coef[0]*z,coef[1]*z,0];z0=max(a);p=[math.exp(v-z0) for v in a];p=[v/sum(p) for v in p]
            check_vector(r[4:7],p)
            near(trans[i][7],m);near(trans[i][8],s)
            output.append([extreme,i+1,x[i][0],x[i][2],*p,*coef])
        if extreme==5:base=coef
        else:check_vector(base,coef)
    report("train_transform_softmax_frozen_holdout",["Extreme","Row","Train","Standardized","P1","P2","P3","Beta1","Beta2"],output)

def rank_signal_portfolio(model,report):
    output=[]
    # Train pairwise score and use frozen direction in a subsequent date panel.
    for lam in (.2,1,5):
        data=[]
        for group in (1,2,3):
            for v in (-1,0,1):data.append([1,group,v,v])
        for v in (-2,0,2):data.append([0,4,v,v])
        rank=model("ranknet-linear-fit",data,[lam,1e-9,100])
        b=rank[0][-1];assert b>0
        # Feature difference has six pairs of size 1 and three pairs of size 2.
        g=lam*b-(6/(1+math.exp(b))+6/(1+math.exp(2*b)))/9
        near(g,0,atol=1e-7)
        panel=[]
        for t in range(1,7):
            for j,v in enumerate((-1,0,1),1):
                panel.append([t,j,.01*v,b*v])
        rows=model("rolling-panel-ridge-longshort",panel,[2,.01,.001,1])
        for t in range(3,7):
            group=rows[(t-3)*3:(t-2)*3]
            check_vector([r[3] for r in group],[-.5,0,.5])
            ref=.04*b/(.01+4*b*b)
            near(group[0][-1],ref)
            near(group[0][6],.01)
            output.append([lam,t,b,ref,group[0][6],group[0][7],group[0][10]])
    report("ranknet_frozen_score_to_panel_positions",["RankPenalty","Date","RankCoefficient","RollingCoefficient","Gross","Net","NAV"],output)

def class_prior_calibration(model,report):
    output=[]
    # Frozen source posterior is estimated from a balanced three-class training set.
    for lam in (.1,1,5):
        train=[[1,1,-3],[1,1,-2],[1,2,0],[1,2,0],[1,3,2],[1,3,3]]
        target=[-3,-2,-1,0,1,2,3,4]
        rows=model("softmax-newton-fit",train+[[0,1,v] for v in target],[3,lam,1e-10,100])
        # Use a strictly positive, regularized EM case by duplicating each class-specific
        # source posterior plus its cyclic permutations. The likelihood is symmetric.
        prob=rows[-8][4:7]
        data=[prob,prob[1:]+prob[:1],prob[2:]+prob[:2]]*3
        adjusted=model("label-shift-prior-em",data,[1e-10,10000,1/3,1/3,1/3])
        for i,r in enumerate(adjusted):
            check_vector(r[4:7],data[i]);check_vector(r[7:10],[1/3]*3)
            near(sum(r[4:7]),1)
            output.append([lam,i+1,*r[4:7],*r[7:10],r[-1]])
    report("softmax_label_shift_symmetric_calibration",["Penalty","Row","AdjustedP1","AdjustedP2","AdjustedP3","TargetPrior1","TargetPrior2","TargetPrior3","FixedPointResidual"],output)

def probability_decision(model,report):
    output=[]
    for k in (.5,1,3):
        train=[[1,1,1],[1,2,1],[1,4,1],[1,3,1],[0,2,1]]
        gamma=model("gamma-log-glm",train,[k,1,1e-9,100])
        mean=gamma[-1][3];var=gamma[-1][4]
        # Use Gamma conditional mean and variance as a positive, fixed risk input.
        for target in (.1,.2,.4):
            cov=[[var,.1*var],[.1*var,2*var]]
            mu=[.2/mean,.1/mean]
            front=model("active-tracking-frontier",[[*cov[i],mu[i],.5] for i in range(2)],[target])
            w=target/math.sqrt(2.8*var)
            check_vector([r[2] for r in front],[w,-w])
            near(front[0][4],w*(mu[0]-mu[1]))
            output.append([k,target,mean,var,w,front[0][4],front[0][5]])
    report("gamma_positive_risk_to_active_allocation",["Shape","TargetTE","Mean","Variance","ActiveWeight","ExpectedActiveReturn","TrackingError"],output)

def multitask_factor_risk(model,report):
    output=[]
    for lam in (.05,.2,1):
        # Same design rows for multiple assets, jointly estimated coefficient support.
        X=[[1,0],[0,1],[-1,0],[0,-1],[1,1],[-1,-1]]
        Y=[[2*a+.1*b,a-.1*b] for a,b in X]
        rows=model("multitask-group-lasso",[[*y,*x] for y,x in zip(Y,X)],[2,lam,1e-9,20000])
        B=[r[-2:] for r in rows]
        pred=[[sum(x[j]*B[j][k] for j in range(2)) for k in range(2)] for x in X]
        residual=[[y[k]-p[k] for k in range(2)] for y,p in zip(Y,pred)]
        n=len(X);F=mm(transpose(X),X);F=[[v/(n-1) for v in row] for row in F]
        C=mm(mm(transpose(B),F),B)
        for k in range(2):C[k][k]+=.01+sum(r[k]**2 for r in residual)/(n-1)
        alloc=model("maximum-diversification-kkt",C,[1e-9,20000])
        w=[r[1] for r in alloc];near(sum(w),1)
        var=dot(w,mv(C,w));near(alloc[0][4],var)
        output.append([lam,*B[0],*B[1],*w,var,alloc[0][3]])
    report("multitask_sparse_loadings_to_risk",["Penalty","Feature1Asset1","Feature1Asset2","Feature2Asset1","Feature2Asset2","Weight1","Weight2","PortfolioVariance","DiversificationRatio"],output)

def regularization_validation(model,report):
    output=[]
    for noise in (.01,.1,.4):
        X=[[1,t] for t in (-2,-1,0,1,2,3)]
        y=[.2+.5*r[1]+noise*z for r,z in zip(X,[1,-1,2,-2,1,-1])]
        path=model("ridge-loocv-gcv-path",[[z,*x] for x,z in zip(X,y)],[0,.01,.1,1,10])
        selected=min(path,key=lambda r:r[1])
        # RRQR is an independent unregularized full-design baseline.
        qr=model("rrqr-regression",[[z,*x] for x,z in zip(X,y)],[1e-11])
        check_vector(path[0][-2:],[r[1] for r in qr])
        for r in path:
            lam=r[0];beta=r[-2:]
            A=mm(transpose(X),X)
            for j in range(2):A[j][j]+=lam
            check_vector(beta,solve(A,mv(transpose(X),y)))
            output.append([noise,lam,r[1],r[2],r[4],*beta,int(lam==selected[0])])
    report("rrqr_ridge_validation_path",["Noise","Penalty","LOOCV","GCV","EffectiveDF","Intercept","Slope","Selected"],output)

def studies(model,report):
    for fn in (transformed_softmax,rank_signal_portfolio,class_prior_calibration,probability_decision,multitask_factor_risk,regularization_validation):fn(model,report)
