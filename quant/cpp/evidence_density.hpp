#pragma once
#include "evidence_common.hpp"
namespace kline { namespace evidence {
inline Report energy_score(const Mat& x,const Vec& p){
    auto d=validate(x,3,1,32);parameters(p,0);auto M=x.size()-1;
    require(double(M)*M*d<=5e7,"Energy score work limit exceeded");
    double a=0,b=0;
    for(std::size_t i=1;i<x.size();++i){a+=distance(x[0],x[i])/M;for(std::size_t j=1;j<i;++j)b+=2*distance(x[i],x[j])/M/M;}
    return scalar({"MeanDistance","PairDistance","EnergyScore","Scenarios"},{a,b,a-.5*b,double(M)});
}

inline Report variogram_score(const Mat& x,const Vec& p){
    auto d=validate(x,2,2,32);parameters(p,1);require(p[0]>0&&p[0]<2,"Variogram exponent must lie in (0,2)");
    require(double(x.size())*d*d<=5e7,"Variogram work limit exceeded");
    Report o{{"AssetI","AssetJ","ObservedMoment","ForecastMoment","Score"},{}};
    for(std::size_t i=0;i<d;++i)for(std::size_t j=i+1;j<d;++j){
        double a=std::pow(std::abs(x[0][i]-x[0][j]),p[0]),b=0;
        for(std::size_t t=1;t<x.size();++t)b+=std::pow(std::abs(x[t][i]-x[t][j]),p[0])/(x.size()-1);
        o.rows.push_back({double(i+1),double(j+1),a,b,(a-b)*(a-b)});
    }
    return o;
}

inline Report quantile_rearrangement(const Mat& x,const Vec& p){
    auto d=validate(x,1,3,64)-1;parameters(p,d);
    for(std::size_t j=0;j<d;++j)require(p[j]>0&&p[j]<1&&(j==0||p[j]>p[j-1]),"Increasing interior quantile levels required");
    Report o{{"Period","CrossingPairs","OriginalPinball","RearrangedPinball","LossReduction"},{}};
    auto loss=[](double y,double q,double a){double e=y-q;return e*(a-double(e<0));};
    for(std::size_t t=0;t<x.size();++t){
        Vec q(x[t].begin()+1,x[t].end());double n=0,a=0,b=0;
        for(std::size_t i=0;i<d;++i){a+=loss(x[t][0],q[i],p[i])/d;for(std::size_t j=i+1;j<d;++j)if(q[i]>q[j])++n;}
        std::sort(q.begin(),q.end());for(std::size_t i=0;i<d;++i)b+=loss(x[t][0],q[i],p[i])/d;
        o.rows.push_back({double(t),n,a,b,a-b});
    }
    return o;
}

inline Report ensemble_rank(const Mat& x,const Vec& p){
    auto d=validate(x,1,2,64);parameters(p,0);Vec h(d,0);
    for(const auto& r:x){
        std::size_t a=0,b=0;
        for(std::size_t j=1;j<d;++j){if(r[j]<r[0])++a;else if(r[j]==r[0])++b;}
        for(std::size_t k=a;k<=a+b;++k)h[k]+=1.0/(b+1);
    }
    Report o{{"ZeroBasedRank","OneBasedRank","Count","Frequency"},{}};
    for(std::size_t k=0;k<d;++k)o.rows.push_back({double(k),double(k+1),h[k],h[k]/x.size()});
    return o;
}
} }
