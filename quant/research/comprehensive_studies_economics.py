"""Structural equations, dynamic programming and equilibrium counterfactuals."""
import math
from comprehensive_study_common import near,check_vector,solve,transpose,dot

def studies(model,report):
    out=[]
    for beta in (.5,.9,1):
        data=[[1,1,0],[2,0,1]];grid=[0,1,2];rows=model("household-saving-bellman",data,[3,beta,1,0,*grid]);v=[[0.]*3 for _ in data]
        for h in range(1,4):
            nxt=[[0.]*3 for _ in data]
            for i in range(2):
                for j,a in enumerate(grid):
                    opts=[(math.log(a+data[i][0]-z)+beta*v[i][k],z) for k,z in enumerate(grid) if a+data[i][0]>z]
                    ref=max(t[0] for t in opts);r=rows[(h-1)*6+i*3+j]
                    near(r[-1],ref);near(r[4],a+data[i][0]-r[3]);nxt[i][j]=ref;out.append([beta,*r])
            v=nxt
    report("household_finite_bellman",["Beta","Remaining","IncomeState","Asset","NextAsset","Consumption","Value"],out)
    out=[]
    for beta in (.2,.5,.8):
        rows=model("mccall-reservation-wage",[[1,.5],[3,.5]],[beta,1,1e-11,10000])
        R=rows[0][4];near(R,1+beta*.5*(max(1/(1-beta),R)+max(3/(1-beta),R)))
        for r in rows:near(r[5],(1-beta)*R);out.append([beta,*r])
    report("search_reservation_fixed_point",["Beta","Wage","Probability","Accept","AcceptValue","RejectValue","ReservationWage","Residual"],out)
    out=[]
    for c in (0,.5,2):
        rows=model("dynamic-logit-replacement",[[1,1,1,4,3]],[.8,c,c,2,1e-11,10000]);r=rows[0]
        z=math.log(math.exp(-1)+math.exp(-c));P=math.exp(-c-z)
        check_vector(r[1:6],[z/.2,P,1-P,c,4*math.log(1-P)+3*math.log(P)],atol=1e-8);out.append(r)
    report("dynamic_logit_single_state_exact",["State","Value","ReplaceProbability","KeepProbability","ReplacementCost","LogLikelihood","Residual"],out)
    out=[]
    for cost in (.5,1,2):
        r=model("matching-free-entry-equilibrium",[[2,1,.1,cost]],[1,.5,.1])[0]
        J=5;th=(J/cost)**2;f=math.sqrt(th);q=1/f;u=.1/(.1+f)
        check_vector(r,[th,f,q,u,J,0]);out.append([cost,*r])
    report("matching_free_entry_flow_balance",["Cost","Tightness","Finding","Filling","Unemployment","JobValue","Residual"],out)
    out=[]
    for sigma in (1.5,2,4):
        for ice in (1,2,3):
            rows=model("ces-armington-trade-counterfactual",[[1,1,.5,1,1],[2,1,.5,1,ice]],[sigma,100])
            z=ice**(1-sigma);P=(.5+.5*z)**(1/(1-sigma))
            near(rows[0][2],1/(1+z));near(rows[1][2],z/(1+z));near(rows[0][4],P);near(sum(r[6] for r in rows),100)
            out.extend([[sigma,ice,*r] for r in rows])
    report("ces_price_share_reference",["Elasticity","Iceberg","Origin","BaselineShare","CounterfactualShare","P0","P1","WelfareRatio","Spending"],out)
    out=[]
    for scale in (.2,.5,1):
        A=[[.2*scale,.3*scale],[.1*scale,.1*scale]];f=[1,2];v=[.5,.7]
        H=[[float(i==j)-A[i][j] for j in range(2)] for i in range(2)]
        q=solve(H,f);p=solve(transpose(H),v);rows=model("leontief-dual-price-system",[[*A[i],f[i],v[i]] for i in range(2)],[])
        near(dot(p,f),dot(v,q))
        for i,r in enumerate(rows):check_vector(r[1:3],[q[i],p[i]],atol=1e-8);out.append([scale,*r])
    report("leontief_dense_dual_reference",["Scale","Sector","Output","Price","Intermediate","FinalDemand","ValueAdded","QuantityResidual","PriceResidual"],out)
    out=[]
    for cap in (1,3,20):
        rows=model("cournot-capacity-equilibrium",[[0,cap],[0,cap]],[12,1,1e-10]);q=min(4,cap)
        for r in rows:near(r[1],q);near(r[2],12-2*q);near(r[-1],0);out.append([cap,*r])
    report("cournot_symmetric_capacity",["Capacity","Firm","Quantity","Price","Profit","MarginalProfit","KKTResidual"],out)
    out=[]
    for alpha in (.3,.5,.7):
        delta=.1;beta=.95;k=(alpha/(1/beta-1+delta))**(1/(1-alpha));c=k**alpha-delta*k
        rows=model("ramsey-steady-state-transition",[[k,c]],[alpha,delta,beta,2,10])
        for r in rows:check_vector(r[2:4],[k,c]);check_vector(r[7:],[k,c]);out.append([alpha,*r])
    report("ramsey_steady_state_invariance",["Alpha","Scenario","Period","Capital","Consumption","Output","NextCapital","NextConsumption","SteadyCapital","SteadyConsumption"],out)
