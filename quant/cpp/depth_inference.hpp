#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_inference {
using namespace evidence;
inline Report paired_student(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,0);Vec d;for(auto&r:x)d.push_back(r[0]-r[1]);double m=mean(d),s=0;for(double z:d)s+=(z-m)*(z-m);
    double n=d.size(),se=std::sqrt(positive(s)/(n-1)/n);
    return scalar({"Pairs","MeanDifference","StandardError","TStatistic","DegreesFreedom"},{n,m,se,m/se,n-1});
}

inline Report equivalence_tost(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,2);double d=positive(p[0]),z=positive(p[1]);Report o{{"Estimate","Lower","Upper","Equivalent"},{}};
    for(auto&r:x){double w=z*positive(r[1]),a=r[0]-w,b=r[0]+w;o.rows.push_back({r[0],a,b,double(a>-d&&b<d)});}return o;
}

inline Report survey_hajek(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,1);auto N=count(p[0],x.size(),1000000000);double y=0,n=0;
    for(auto&r:x){require(r[1]>0&&r[1]<=1,"Invalid inclusion probability");y+=r[0]/r[1];n+=1/r[1];}
    return scalar({"HTTotal","HTPopulation","KnownPopulationMean","HajekMean"},{y,n,y/N,y/n});
}

inline Report fisher_exact(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,0);require(x.size()==2,"Two rows required");
    double a=count(x[0][0],0,100000),b=count(x[0][1],0,100000),c=count(x[1][0],0,100000),d=count(x[1][1],0,100000);
    double N=a+b+c+d,r=a+b,k=a+c;require(r>0&&r<N&&k>0&&k<N,"Degenerate margins");
    auto lc=[](double n,double j){return std::lgamma(n+1)-std::lgamma(j+1)-std::lgamma(n-j+1);};
    auto lp=[&](double z){return lc(k,z)+lc(N-k,r-z)-lc(N,r);};
    double lo=std::max(0.,r-(N-k)),hi=std::min(r,k),obs=lp(a),v=0;
    for(double z=lo;z<=hi;++z){double t=lp(z);if(t<=obs+1e-10)v+=std::exp(t);}
    return scalar({"ObservedProbability","TwoSidedP","SupportSize"},{std::exp(obs),std::min(v,1.),hi-lo+1});
}
} }
