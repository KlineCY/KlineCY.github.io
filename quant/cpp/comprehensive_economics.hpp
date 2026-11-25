#pragma once
#include "comprehensive_common.hpp"
namespace kline { namespace comprehensive {
inline Report household_saving_bellman(const Mat& x,const Vec& p){
    size_t K=x.size();validate(x,1,2,33);require(x[0].size()==K+1&&p.size()>=6,"invalid income grid");
    int H=count(p[0],1,1000);double beta=p[1],gamma=positive(p[2]),r=p[3];
    require(beta>0&&beta<=1&&r>-1,"invalid household controls");
    Vec a(p.begin()+4,p.end());require(a.front()==0&&std::is_sorted(a.begin(),a.end())&&std::adjacent_find(a.begin(),a.end())==a.end(),"asset grid must start at zero and increase");
    size_t M=a.size();require(double(H)*K*K*M*M<=5e7,"household DP too large");
    for(const auto&z:x){positive(z[0]);double s=0;for(size_t j=1;j<=K;++j){require(z[j]>=0,"negative transition");s+=z[j];}require(std::abs(s-1)<1e-10,"transition row sum");}
    Mat v(K,Vec(M));Report out;out.header={"Remaining","IncomeState","Asset","NextAsset","Consumption","Value"};
    for(int h=1;h<=H;++h){
        Mat next(K,Vec(M));
        for(size_t i=0;i<K;++i)for(size_t m=0;m<M;++m){
            double resources=(1+r)*a[m]+x[i][0],best=-std::numeric_limits<double>::infinity(),bc=0,ba=0;
            for(size_t k=0;k<M;++k){
                double c=resources-a[k];if(c<=0)continue;
                double u=gamma==1?std::log(c):std::expm1((1-gamma)*std::log(c))/(1-gamma),ev=0;
                for(size_t j=0;j<K;++j)ev+=x[i][j+1]*v[j][k];
                double value=u+beta*ev;if(value>best){best=value;bc=c;ba=a[k];}
            }
            require(std::isfinite(best),"no feasible consumption");next[i][m]=best;
            out.rows.push_back({double(h),double(i+1),a[m],ba,bc,best});
        }
        v=next;
    }
    return out;
}

inline Report mccall_reservation_wage(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,4);double beta=p[0],b=p[1],tol=positive(p[2]);int cap=count(p[3],1,1000000);
    require(beta>0&&beta<1,"discount must be below one");
    double mass=0;for(const auto&r:x){require(r[1]>=0,"negative probability");mass+=r[1];}
    require(std::abs(mass-1)<1e-10,"probabilities must sum to one");
    auto F=[&](double R){double z=b;for(const auto&r:x)z+=beta*r[1]*std::max(r[0]/(1-beta),R);return z;};
    double R=b/(1-beta);for(int i=0;i<cap;++i){double z=F(R);if(std::abs(z-R)<=tol){R=z;break;}R=z;}
    double err=std::abs(F(R)-R);require(err<=tol,"fixed point not converged");
    Report out;out.header={"Wage","Probability","Accept","AcceptValue","RejectValue","ReservationWage","BellmanResidual"};
    for(const auto&r:x)out.rows.push_back({r[0],r[1],double(r[0]/(1-beta)>=R),r[0]/(1-beta),R,(1-beta)*R,err});
    return out;
}

inline Report dynamic_logit_replacement(const Mat& x,const Vec& p){
    validate(x,1,5,5);parameters(p,6);double beta=p[0],lo=p[1],hi=p[2],tol=positive(p[4]);int G=count(p[3],2,10000),cap=count(p[5],1,100000);
    require(beta>0&&beta<1&&lo>=0&&hi>=lo&&double(G)*cap*x.size()<=1e8,"invalid dynamic grid");
    size_t n=x.size();std::vector<size_t> k0,k1;
    for(const auto&r:x){require(r[0]>=0,"negative state cost");k0.push_back(count(r[1],1,int(n))-1);k1.push_back(count(r[2],1,int(n))-1);count(r[3],0,1000000000);count(r[4],0,1000000000);}
    double best=-std::numeric_limits<double>::infinity(),bc=0,be=0;Vec bv,bp;
    for(int g=0;g<G;++g){
        double c=lo+(hi-lo)*g/(G-1);Vec v(n);
        auto update=[&](const Vec&w){Vec z(n);for(size_t i=0;i<n;++i)z[i]=logsumexp({-x[i][0]+beta*w[k0[i]],-c+beta*w[k1[i]]});return z;};
        for(int it=0;it<cap;++it){Vec z=update(v);double err=0;for(size_t i=0;i<n;++i)err=std::max(err,std::abs(z[i]-v[i]));v=z;if(err<=tol)break;}
        Vec z=update(v),probs(n);double err=0,ll=0;
        for(size_t i=0;i<n;++i){
            err=std::max(err,std::abs(z[i]-v[i]));double a=-x[i][0]+beta*v[k0[i]],b=-c+beta*v[k1[i]],den=logsumexp({a,b});
            probs[i]=std::exp(b-den);ll+=x[i][3]*(a-den)+x[i][4]*(b-den);
        }
        require(err<=tol,"Bellman iteration did not converge");
        if(ll>best){best=ll;bc=c;be=err;bv=v;bp=probs;}
    }
    Report out;out.header={"State","Value","ReplaceProbability","KeepProbability","ReplacementCost","LogLikelihood","BellmanResidual"};
    for(size_t i=0;i<n;++i)out.rows.push_back({x[i][0],bv[i],bp[i],1-bp[i],bc,best,be});
    return out;
}

inline Report matching_free_entry_equilibrium(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,3);double A=positive(p[0]),alpha=p[1],r=positive(p[2]);
    require(alpha>0&&alpha<1,"invalid matching elasticity");
    Report out;out.header={"Tightness","JobFindingRate","VacancyFillingRate","Unemployment","JobValue","FreeEntryResidual"};
    for(const auto&z:x){
        double profit=positive(z[0]-z[1]),s=z[2],c=positive(z[3]);require(s>=0,"negative separation");
        double J=profit/(r+s),theta=std::pow(A*J/c,1/alpha),f=A*std::pow(theta,1-alpha),q=A*std::pow(theta,-alpha);
        out.rows.push_back({theta,f,q,s/(s+f),J,c-q*J});
    }
    return out;
}

inline Report ces_armington_trade_counterfactual(const Mat& x,const Vec& p){
    validate(x,1,5,5);parameters(p,2);double sigma=p[0],E=positive(p[1]);require(sigma>1,"elasticity must exceed one");
    Vec a,b;std::set<int> origins;double mass=0;
    for(const auto&r:x){
        require(origins.insert(count(r[0],1,1000000000)).second,"duplicate origin");
        double w=positive(r[2]),c=positive(r[1]);mass+=w;
        a.push_back(std::log(w)+(1-sigma)*std::log(c*positive(r[3])));
        b.push_back(std::log(w)+(1-sigma)*std::log(c*positive(r[4])));
    }
    require(std::abs(mass-1)<1e-10,"weights must sum to one");
    double z0=logsumexp(a),z1=logsumexp(b),P0=std::exp(z0/(1-sigma)),P1=std::exp(z1/(1-sigma));
    Report out;out.header={"Origin","BaselineShare","CounterfactualShare","BaselinePriceIndex","CounterfactualPriceIndex","RealExpenditureRatio","CounterfactualSpending"};
    for(size_t j=0;j<x.size();++j){double s=std::exp(b[j]-z1);out.rows.push_back({x[j][0],std::exp(a[j]-z0),s,P0,P1,P0/P1,E*s});}
    return out;
}

inline Report leontief_dual_price_system(const Mat& x,const Vec& p){
    size_t n=x.size();validate(x,1,3,34);parameters(p,0);require(x[0].size()==n+2&&n<=32,"square technology matrix required");
    Mat A(n,Vec(n)),At=A;Vec f(n),v(n);
    for(size_t i=0;i<n;++i){
        f[i]=x[i][n];v[i]=x[i][n+1];require(f[i]>=0&&v[i]>=0,"negative final demand or value added");
        for(size_t j=0;j<n;++j){require(x[i][j]>=0,"negative technical coefficient");A[i][j]=At[j][i]=x[i][j];}
    }
    for(const auto&r:At)require(sum(r)<1-1e-9,"column contraction condition failed");
    auto fixed=[&](const Mat&B,const Vec& b){
        Vec z=b;
        for(int it=0;it<100000;++it){
            Vec u=mul(B,z);double err=0;for(size_t i=0;i<n;++i){u[i]+=b[i];err=std::max(err,std::abs(u[i]-z[i]));}
            z=u;if(err<=1e-11*std::max(1.,inf_norm(z)))return z;
        }
        throw std::runtime_error("Leontief iteration cap");
    };
    Vec q=fixed(A,f),price=fixed(At,v),inter=mul(A,q),cost=mul(At,price);
    Report out;out.header={"Sector","GrossOutput","UnitPrice","IntermediateDemand","FinalDemand","ValueAdded","QuantityResidual","PriceResidual"};
    for(size_t i=0;i<n;++i)out.rows.push_back({double(i+1),q[i],price[i],inter[i],f[i],v[i],q[i]-inter[i]-f[i],price[i]-cost[i]-v[i]});
    return out;
}

inline Report cournot_capacity_equilibrium(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,3);double a=positive(p[0]),b=positive(p[1]),tol=positive(p[2]);
    double hi=0;for(const auto&r:x){require(r[0]>=0,"negative marginal cost");hi+=positive(r[1]);}
    auto q=[&](double Q,const Vec&r){return std::clamp((a-b*Q-r[0])/b,0.,r[1]);};
    double lo=0;
    for(int it=0;it<200;++it){double m=lo+(hi-lo)/2,s=0;for(const auto&r:x)s+=q(m,r);if(s>m)lo=m;else hi=m;}
    double Q=(lo+hi)/2;Vec qty;for(const auto&r:x)qty.push_back(q(Q,r));double price=a-b*sum(qty);
    require(std::abs(sum(qty)-Q)<=tol*std::max(1.,Q),"market residual");
    Report out;out.header={"Firm","Quantity","MarketPrice","Profit","MarginalProfit","KKTResidual"};
    for(size_t i=0;i<x.size();++i){
        double z=qty[i],g=price-x[i][0]-b*z,e=z<=tol?std::max(0.,g):z>=x[i][1]-tol?std::max(0.,-g):std::abs(g);
        out.rows.push_back({double(i+1),z,price,(price-x[i][0])*z,g,e});
    }
    return out;
}

inline Report ramsey_steady_state_transition(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,5);double alpha=p[0],delta=p[1],beta=p[2],gamma=positive(p[3]);int H=count(p[4],1,10000);
    require(alpha>0&&alpha<1&&delta>=0&&delta<=1&&beta>0&&beta<1,"invalid growth parameters");
    double ks=std::pow(alpha/(1/beta-1+delta),1/(1-alpha)),cs=positive(std::pow(ks,alpha)-delta*ks);
    Report out;out.header={"Scenario","Period","Capital","Consumption","Output","NextCapital","NextConsumption","SteadyCapital","SteadyConsumption"};
    for(size_t i=0;i<x.size();++i){
        double k=positive(x[i][0]),c=positive(x[i][1]);
        for(int t=0;t<H;++t){
            double y=std::pow(k,alpha),next=positive(y+(1-delta)*k-c),nc=positive(c*std::pow(beta*(alpha*std::pow(next,alpha-1)+1-delta),1/gamma));
            out.rows.push_back({double(i+1),double(t),k,c,y,next,nc,ks,cs});k=next;c=nc;
        }
    }
    return out;
}
} }
