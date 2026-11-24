#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_executiontheory {
using namespace evidence;
inline Report execution_kkt(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,1);double Q=positive(p[0]),rem=Q,s=0;Vec inv,grad(x.size(),0);
    for(auto&r:x){require(r[0]>=0&&r[2]>=0,"Negative trade or risk coefficient");positive(r[1]);s+=r[0];rem-=r[0];inv.push_back(rem);}require(std::abs(s-Q)<=1e-10*Q,"Trade quantity must reconcile");
    double tail=0,mu=0,n=0;
    for(std::size_t i=x.size();i-->0;){tail+=x[i][2]*inv[i];grad[i]=2*x[i][1]*x[i][0]-2*tail;if(x[i][0]>0){mu+=grad[i];++n;}}
    mu/=positive(n);Report o{{"Period","Trade","Remaining","TemporaryCost","RiskCost","Gradient","KKTResidual"},{}};
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];double g=grad[i],e=r[0]>0?std::abs(g-mu):std::max(0.,mu-g);o.rows.push_back({double(i+1),r[0],inv[i],r[1]*r[0]*r[0],r[2]*inv[i]*inv[i],g,e});}return o;
}

inline Report competing_execution_hazards(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,1);require(p[0]>=0,"Negative horizon");Report o{{"FillProbability","CancelProbability","NoEventProbability","ExpectedFirstEventTime"},{}};
    for(auto&r:x){require(r[0]>=0&&r[1]>=0,"Negative hazard");double s=positive(r[0]+r[1]),a=-std::expm1(-s*p[0]);o.rows.push_back({r[0]/s*a,r[1]/s*a,std::exp(-s*p[0]),1/s});}return o;
}

inline Report queue_cancel_bounds(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,2);require(p[0]>=0,"Negative initial queue");double q=positive(p[1]),lo=p[0],hi=p[0],fmin=0,fmax=0;Report o{{"Period","LowerAhead","UpperAhead","MinCumulativeFill","MaxCumulativeFill"},{}};
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];double v=r[0],c=r[1];require(v>=0&&c>=0,"Negative queue event");lo=std::max(0.,lo-c);fmax+=std::min(q-fmax,std::max(0.,v-lo));fmin+=std::min(q-fmin,std::max(0.,v-hi));lo=std::max(0.,lo-v);hi=std::max(0.,hi-v);o.rows.push_back({double(i+1),lo,hi,fmin,fmax});}return o;
}

inline Report quote_bayesian_selection(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,2);require(p[1]>p[0],"High value must exceed low");double L=p[0],H=p[1];Report o{{"PriorMid","Bid","Ask","Spread","PosteriorHighBuy","PosteriorHighSell"},{}};
    for(auto&r:x){double a=r[0],m=r[1];require(a>0&&a<1&&m>=0&&m<=1,"Invalid prior or informed fraction");double pb=a*(1+m)/(a*(1+m)+(1-a)*(1-m)),ps=a*(1-m)/(a*(1-m)+(1-a)*(1+m));double bid=L+(H-L)*ps,ask=L+(H-L)*pb;o.rows.push_back({L+(H-L)*a,bid,ask,ask-bid,pb,ps});}return o;
}
} }
