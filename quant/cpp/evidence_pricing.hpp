#pragma once
#include "evidence_common.hpp"
namespace kline { namespace evidence {
inline Report hj_bound(const Mat& x,const Vec& p){
    validate(x,2,1,32);parameters(p,1);double a=positive(p[0]);
    Vec mu=means(x),b=solve_spd(moment(x,true),mu);
    double h=std::sqrt(std::max(0.0,dot(mu,b))),lo=std::numeric_limits<double>::infinity(),hi=-lo,neg=0;
    for(const auto& r:x){double m=a*(1-dot(b,r)+dot(b,mu));lo=std::min(lo,m);hi=std::max(hi,m);if(m<0)++neg;}
    return scalar({"MaximumSharpe","MinimumSDFSD","MeanSDF","MaximumSDF","MinimumSDF","NegativeCount"},{h,a*h,a,hi,lo,neg});
}

inline Report minimum_norm_sdf(const Mat& x,const Vec& p){
    auto d=validate(x,2,1,32);parameters(p,d);
    Vec b=solve_spd(moment(x,false),p);Report o{{"State","SDF"},{}};
    for(std::size_t t=0;t<x.size();++t)o.rows.push_back({double(t),dot(x[t],b)});
    return o;
}

inline Report sdf_pricing_errors(const Mat& x,const Vec& p){
    auto d=validate(x,1,2,33)-1;parameters(p,d);Report o{{"Asset","ObservedPrice","ModelPrice","PricingError"},{}};
    for(std::size_t j=0;j<d;++j){double m=0;for(const auto& r:x)m+=r[0]*r[j+1]/x.size();o.rows.push_back({double(j+1),p[j],m,m-p[j]});}
    return o;
}

inline Report grs_joint_alpha(const Mat& x,const Vec& p){
    auto d=validate(x,5,2,32);parameters(p,1);auto k=count(p[0],1,d-1),n=d-k,T=x.size();
    require(T>n+k,"Insufficient GRS degrees of freedom");
    Mat f,ret;
    for(const auto& r:x){f.emplace_back(r.begin(),r.begin()+k);ret.emplace_back(r.begin()+k,r.end());}
    Vec mf=means(f),mr=means(ret),a(n);Mat C=moment(f,true),E(T,Vec(n));
    for(std::size_t j=0;j<n;++j){
        Vec z(k,0);for(std::size_t t=0;t<T;++t)for(std::size_t i=0;i<k;++i)z[i]+=(f[t][i]-mf[i])*(ret[t][j]-mr[j])/T;
        Vec beta=solve_spd(C,z);a[j]=mr[j]-dot(beta,mf);
        for(std::size_t t=0;t<T;++t)E[t][j]=ret[t][j]-a[j]-dot(beta,f[t]);
    }
    double h=dot(mf,solve_spd(C,mf)),v=dot(a,solve_spd(moment(E,false),a));
    return scalar({"Observations","Assets","Factors","GRS","NumeratorDF","DenominatorDF","AlphaQuadratic","FactorSharpeSquared"},{double(T),double(n),double(k),double(T-n-k)/n*v/(1+h),double(n),double(T-n-k),v,h});
}
} }
