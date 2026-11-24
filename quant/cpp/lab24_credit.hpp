#pragma once
#include "lab24_common.hpp"

namespace kline { namespace lab24 {
inline Vec migration_two(const Mat& x,const Vec& p){
    inputs(x,p,3,4,3);require(x.size()==3,"Three rating states required");auto h=integer(p[0],1,100000);Vec v(p.begin()+1,p.end());
    for(double q:v)require(q>=0&&q<=1,"Invalid initial state probability");require(std::abs(sum(v)-1)<1e-10,"Initial probabilities must sum to one");
    for(const auto& r:x){for(double q:r)require(q>=0&&q<=1,"Invalid transition");require(std::abs(sum(r)-1)<1e-10,"Transition rows must sum to one");}
    require(x[2][0]==0&&x[2][1]==0&&x[2][2]==1,"Default state must be absorbing");
    for(std::size_t t=0;t<h;++t){Vec w(3,0);for(std::size_t i=0;i<3;++i)for(std::size_t j=0;j<3;++j)w[j]+=v[i]*x[i][j];v=std::move(w);}
    return checked(v);
}

inline Vec poisson_binomial(const Mat& x,const Vec& p){
    inputs(x,p,1,0);require(x.size()<=1000,"Exact convolution limited to 1000 debts");Vec d(x.size()+1,0);d[0]=1;std::size_t n=0;
    for(const auto& r:x){double q=r[0];require(q>=0&&q<=1,"Invalid PD");++n;for(std::size_t k=n;k>0;--k)d[k]=(1-q)*d[k]+q*d[k-1];d[0]*=1-q;}
    return checked(d);
}

inline Vec loss_convolution(const Mat& x,const Vec& p){
    inputs(x,p,2,1);require(p[0]>0&&p[0]<1,"Invalid confidence");require(x.size()<=1000,"Too many credit names");std::size_t maxloss=0;
    for(const auto& r:x){require(r[0]>=0&&r[0]<=1,"Invalid PD");maxloss+=integer(r[1],0,20000);require(maxloss<=20000,"Loss grid too large");}
    Vec d(maxloss+1,0);d[0]=1;std::size_t current=0;
    for(const auto& r:x){auto l=std::size_t(r[1]);double q=r[0];if(l==0)continue;current+=l;for(std::size_t k=current+1;k-->0;){d[k]=(1-q)*d[k]+(k>=l?q*d[k-l]:0);}}
    double el=0,cdf=0,var=0;bool found=false;for(std::size_t k=0;k<d.size();++k){el+=k*d[k];cdf+=d[k];if(!found&&cdf>=p[0]-1e-14){var=double(k);found=true;}}
    double mass=1-p[0],left=mass,tail=0;for(std::size_t k=d.size();k-->0&&left>0;){double take=std::min(left,d[k]);tail+=take*k;left-=take;}
    return checked({el,var,tail/mass});
}

inline Vec survival_hazard(const Mat& x,const Vec& p){
    inputs(x,p,2,0);double last=0,s=1;Vec out;
    for(const auto& r:x){require(r[0]>last&&r[1]>=0,"Invalid hazard interval");double e=-r[1]*(r[0]-last),d=s*(-std::expm1(e));s*=std::exp(e);out.push_back(s);out.push_back(d);last=r[0];}
    return checked(out);
}

inline Vec discrete_cds_spread(const Mat& x,const Vec& p){
    inputs(x,p,3,1);require(p[0]>=0&&p[0]<=1,"Invalid recovery");double t=0,s=1,a=0,b=0;
    for(const auto& r:x){require(r[0]>t&&r[1]>=0&&r[1]<=s,"Invalid survival schedule");positive(r[2]);a+=(1-p[0])*r[2]*(s-r[1]);b+=(r[0]-t)*r[2]*r[1];t=r[0];s=r[1];}
    return checked({a,b,a/positive(b)});
}
} }
