#pragma once
#include "system_research_common.hpp"
namespace kline { namespace system_research {
inline Report ces_cost_demand(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,2);double sig=positive(p[0]),Y=positive(p[1]),mass=0,logc=0,z=1-sig;Vec a;for(auto&r:x){positive(r[0]);mass+=positive(r[1]);a.push_back(std::log(r[1])+z*std::log(r[0]));}require(std::abs(mass-1)<=1e-10,"CES shares must sum to one");if(std::abs(z)<1e-7){for(auto&r:x)logc+=r[1]*std::log(r[0]);}else{double h=*std::max_element(a.begin(),a.end()),s=0;for(double v:a)s+=std::exp(v-h);logc=(h+std::log(s))/z;}double c=positive(std::exp(logc)),total=Y*c,actual=0;Report o{{"Input","Price","Quantity","ExpenditureShare","UnitCost","TotalCost","ConservationResidual"},{}};for(std::size_t i=0;i<x.size();++i){auto&r=x[i];double share=r[1]*std::exp(z*(std::log(r[0])-logc)),q=total*share/r[0];actual+=q*r[0];o.rows.push_back({double(i+1),r[0],q,share,c,total,0});}for(auto&r:o.rows)r[6]=std::abs(actual-total);return o;
}

inline Report logit_market_share(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,1);double alpha=positive(p[0]),m=0;Vec z;for(auto&r:x){require(r[1]>=0,"Nonnegative price required");z.push_back(r[0]-alpha*r[1]);m=std::max(m,z.back());}double s=std::exp(-m);for(double v:z)s+=std::exp(v-m);double ld=m+std::log(s),outside=std::exp(-ld),mass=outside;Report o{{"Product","Share","OutsideShare","OwnElasticity","ConsumerSurplus","ProbabilityResidual"},{}};for(std::size_t j=0;j<x.size();++j){double q=std::exp(z[j]-ld);mass+=q;o.rows.push_back({double(j+1),q,outside,-alpha*x[j][1]*(1-q),ld/alpha,0});}for(auto&r:o.rows)r[5]=std::abs(mass-1);return o;
}

inline Report solow_transition(const Mat& x,const Vec& p){
    validate(x,1,5,5);parameters(p,2);double initial=positive(p[0]);auto H=count(p[1],0,10000);require(double(H+1)*x.size()<=1e6,"Solow output limit");Report o{{"Scenario","Period","Capital","Output","Consumption","SteadyCapital","CapitalGap"},{}};for(std::size_t i=0;i<x.size();++i){auto&r=x[i];double s=r[0],d=r[1],n=r[2],g=r[3],a=r[4],G=(1+n)*(1+g);require(s>0&&s<1&&d>=0&&d<=1&&n>=0&&g>=0&&a>0&&a<1,"Invalid Solow parameters");double star=positive(std::pow(s/positive(G-1+d),1/(1-a))),k=initial;for(std::size_t t=0;t<=H;++t){double y=positive(std::pow(k,a));o.rows.push_back({double(i+1),double(t),k,y,(1-s)*y,star,k-star});k=positive(((1-d)*k+s*y)/G);}}return o;
}

inline Report two_period_consumption(const Mat& x,const Vec& p){
    validate(x,1,6,6);parameters(p,0);Report o{{"Consumption1","Consumption2","Saving","EulerResidual","ConstraintBinds","Utility"},{}};for(auto&r:x){double y=r[0],z=r[1],R=positive(1+r[2]),b=positive(r[3]),g=positive(r[4]),lower=r[5],a=positive(std::exp(std::log(b*R)/g)),s0=(a*y-z)/(R+a),s=std::max(s0,lower),c=positive(y-s),d=positive(z+R*s);auto u=[&](double x){return g==1?std::log(x):std::expm1((1-g)*std::log(x))/(1-g);};double e=std::pow(c,-g)-b*R*std::pow(d,-g);o.rows.push_back({c,d,s,e,double(s==lower),u(c)+b*u(d)});}return o;
}

inline Report input_output_carbon(const Mat& x,const Vec& p){
    auto c=validate(x,1,3,34),d=x.size();require(c==d+2&&d<=32,"Square input output system required");parameters(p,0);Mat A;Vec e,f;for(auto&r:x){A.emplace_back(r.begin(),r.begin()+d);require(r[d]>=0&&r[d+1]>=0,"Nonnegative intensity and demand required");e.push_back(r[d]);f.push_back(r[d+1]);}Mat At(d,Vec(d));for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<d;++j){require(A[i][j]>=0,"Nonnegative coefficients required");At[j][i]=A[i][j];}for(auto&r:At)require(sum(r)<1,"Column sum must be below one");auto solve=[&](const Mat&M,const Vec&b){Vec v=b;for(int k=0;k<10000;++k){Vec z=mul(M,v);for(std::size_t j=0;j<d;++j)z[j]+=b[j];Vec gap=z;for(std::size_t j=0;j<d;++j)gap[j]-=v[j];if(inf_norm(gap)<=1e-12*std::max(1.,inf_norm(z)))return v;v=z;}throw std::runtime_error("Input output iteration did not converge");};Vec v=solve(A,f),h=solve(At,e);double total=dot(v,e),other=dot(h,f);Report o{{"Sector","Output","DirectFootprint","EmbodiedIntensity","FinalDemandFootprint","TotalFootprint","ConservationResidual"},{}};for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),v[j],v[j]*e[j],h[j],h[j]*f[j],total,std::abs(total-other)});return o;
}

inline Report market_clearing_tatonnement(const Mat& x,const Vec& p){
    auto d=validate(x,1,2,33)-1;parameters(p,0);require(x.size()==d,"Square market system required");Mat H;Vec b;for(auto&r:x){H.emplace_back(r.begin(),r.end()-1);b.push_back(r.back());}Vec w=solve_spd(H,b),z=mul(H,w);for(std::size_t j=0;j<d;++j){positive(w[j]);z[j]=b[j]-z[j];}Report o{{"Market","Price","ExcessDemand","ResidualNorm"},{}};for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),w[j],z[j],inf_norm(z)});return o;
}
} }
