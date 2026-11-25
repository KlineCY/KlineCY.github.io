"""Economic decision, equilibrium and data provenance cross-model studies."""
import math
from comprehensive_study_common import near,check_vector,dot,solve,transpose

def input_output_ces(model,report):
    output=[]
    A=[[.2,.1],[.15,.25]];f=[2,1]
    for shock in (0,.05,.2):
        v=[.5+shock,.4]
        rows=model("leontief-dual-price-system",[[*A[i],f[i],v[i]] for i in range(2)],[])
        q=[r[1] for r in rows];p=[r[2] for r in rows]
        near(dot(p,f),dot(v,q))
        for sigma in (1.5,2,4):
            ces=model("ces-armington-trade-counterfactual",[[i+1,p[i],.5,1,1+shock*(i==0)] for i in range(2)],[sigma,100])
            p0=sum(.5*z**(1-sigma) for z in p)**(1/(1-sigma))
            p1=sum(.5*(z*(1+shock*(i==0)))**(1-sigma) for i,z in enumerate(p))**(1/(1-sigma))
            near(ces[0][3],p0);near(ces[0][4],p1);near(sum(r[6] for r in ces),100)
            output.append([shock,sigma,*p,*q,p0,p1,p0/p1,ces[0][2]])
    report("production_network_ces_price_counterfactual",["InputCostShock","Elasticity","Price1","Price2","Output1","Output2","BaselinePriceIndex","CounterfactualPriceIndex","RealExpenditureRatio","Origin1Share"],output)

def matching_income_saving(model,report):
    output=[]
    # Map continuous-time rates to a two-state discrete transition using exact CTMC formulas.
    for c in (2,3,5):
        matching=model("matching-free-entry-equilibrium",[[2,1,.1,c]],[1,.5,.1])[0]
        f=matching[1];s=.1;dt=.2
        z=-math.expm1(-(f+s)*dt)/(f+s)
        P=[[1-f*z,f*z],[s*z,1-s*z]]
        pi=[s/(s+f),f/(s+f)]
        check_vector([sum(pi[i]*P[i][j] for i in range(2)) for j in range(2)],pi)
        for gamma in (1,2):
            rows=model("household-saving-bellman",[[.5,*P[0]],[1,*P[1]]],[3,.95,gamma,0,0,.25,.5,1])
            final=[r for r in rows if r[0]==3 and r[2]==.5]
            assert len(final)==2
            for r in final:near(r[4]+r[3],.5+[.5,1][int(r[1])-1])
            output.append([c,gamma,f,matching[3],P[0][1],P[1][0],final[0][3],final[1][3],final[0][-1],final[1][-1]])
    report("labor_matching_to_household_saving",["VacancyCost","RiskAversion","FindingRate","Unemployment","EmploymentTransition","SeparationTransition","UnemployedSaving","EmployedSaving","UnemployedValue","EmployedValue"],output)

def cournot_network_cost(model,report):
    output=[]
    A=[[.15,.1],[.1,.2]];f=[1,1]
    for shock in (0,.2,.5):
        v=[.4+shock,.5]
        rows=model("leontief-dual-price-system",[[*A[i],f[i],v[i]] for i in range(2)],[])
        costs=[r[2] for r in rows]
        for capacity in (.5,2,10):
            market=model("cournot-capacity-equilibrium",[[costs[0],capacity],[costs[1],capacity]],[6,1,1e-10])
            q=[r[1] for r in market];P=6-sum(q)
            for i,r in enumerate(market):
                near(r[2],P);near(r[3],(P-costs[i])*q[i]);near(r[5],0)
            output.append([shock,capacity,*costs,*q,P,market[0][3]+market[1][3]])
    report("network_unit_cost_to_cournot_capacity",["CostShock","Capacity","Cost1","Cost2","Quantity1","Quantity2","Price","IndustryProfit"],output)

def search_replacement_comparison(model,report):
    output=[]
    for beta in (.3,.6,.9):
        search=model("mccall-reservation-wage",[[1,.4],[2,.3],[4,.3]],[beta,.8,1e-10,100000])
        R=search[0][4];near(R,.8+beta*sum(r[1]*max(r[3],R) for r in search))
        accept=sum(r[1]*r[2] for r in search)
        for cost in (.5,1,2):
            rows=model("dynamic-logit-replacement",[[1,1,1,4,3]],[beta,cost,cost,2,1e-10,10000])
            r=rows[0];near(r[2],1/(1+math.exp(cost-1)))
            near(r[1],math.log(math.exp(-1)+math.exp(-cost))/(1-beta),atol=1e-8)
            output.append([beta,cost,search[0][5],accept,r[2],r[1],r[6]])
    report("search_and_replacement_dynamic_value",["Discount","ReplacementCost","ReservationWage","AcceptanceMass","ReplacementProbability","ReplacementValue","BellmanResidual"],output)

def growth_resource_risk(model,report):
    output=[]
    for alpha in (.3,.5,.7):
        beta=.95;delta=.1;k=(alpha/(1/beta-1+delta))**(1/(1-alpha));c=k**alpha-delta*k
        rows=model("ramsey-steady-state-transition",[[k,c]],[alpha,delta,beta,2,5])
        for r in rows:near(r[5]+r[3],r[4]+(1-delta)*r[2])
        # A finite household problem with a separate income-risk assumption.
        household=model("household-saving-bellman",[[.9*c,.9,.1],[1.1*c,.2,.8]],[3,beta,2,.02,0,c/4,c/2,c])
        final=[r for r in household if r[0]==3 and r[2]==c/2]
        for i,r in enumerate(final):near(r[4]+r[3],1.02*c/2+[.9*c,1.1*c][i])
        output.append([alpha,k,c,rows[-1][2],final[0][3],final[1][3],final[0][-1],final[1][-1]])
    report("growth_steady_state_to_income_risk_experiment",["CapitalShare","SteadyCapital","SteadyConsumption","FinalCapital","LowIncomeSaving","HighIncomeSaving","LowIncomeValue","HighIncomeValue"],output)

def label_vintage_uniqueness(model,report):
    output=[]
    prices=[[0,1,t,100*(1.01)**t] for t in range(1,10)]
    for delay in (0,.5,2):
        queries=[[1,1,t,7] for t in range(1,8)]
        labels=model("label-window-availability",prices+queries,[2,delay])
        eligible=[r for r in labels if r[5]==1]
        assert len(eligible)==math.floor(7-2-delay)
        # Return increments are at (start,end], i.e. integer start+1 through end.
        ranges=[[r[1]+1,r[2]] for r in eligible]
        u=model("overlapping-label-uniqueness",ranges,[])
        near(sum(r[-1] for r in u),len(u))
        for r,z in zip(eligible,u):
            near(r[3],1.01**2-1)
            count=lambda t:sum(a<=t<=b for a,b in ranges)
            a,b=map(int,[z[1],z[2]])
            near(z[4],sum(1/count(t) for t in range(a,b+1))/(b-a+1))
            output.append([delay,r[1],r[2],r[4],r[3],z[4],z[5]])
    report("available_labels_to_uniqueness_weights",["Delay","Start","End","Available","Label","Uniqueness","Weight"],output)

def studies(model,report):
    for fn in (input_output_ces,matching_income_saving,cournot_network_cost,search_replacement_comparison,growth_resource_risk,label_vintage_uniqueness):fn(model,report)
