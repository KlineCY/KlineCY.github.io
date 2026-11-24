#pragma once
#include "evidence_common.hpp"
namespace kline { namespace identification {
using namespace evidence;
inline Report fieller_grid(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,4);
    double c=positive(p[0]),lo=p[1],hi=p[2];
    auto k=count(p[3],2,10001);require(lo<hi,"Invalid ratio grid");
    double n=double(x.size()),a=mean(column(x,0)),b=mean(column(x,1)),v=0,w=0,u=0;
    for(const auto& r:x){v+=(r[0]-a)*(r[0]-a);w+=(r[1]-b)*(r[1]-b);u+=(r[0]-a)*(r[1]-b);}
    v/=n*(n-1);w/=n*(n-1);u/=n*(n-1);
    require(v>0&&w>0,"Nondegenerate sample means required");
    double A=b*b-c*c*w,B=-2*a*b+2*c*c*u,C=a*a-c*c*v;
    Report out{{"Ratio","Quadratic","Accepted","A","B","C"},{}};
    for(std::size_t j=0;j<k;++j){
        double z=lo+(hi-lo)*double(j)/double(k-1),f=(A*z+B)*z+C;
        out.rows.push_back({z,f,double(f<=0),A,B,C});
    }
    return out;
}
inline Report anderson_rubin_grid(const Mat& x,const Vec& p){
    validate(x,4,3,3);parameters(p,4);
    double critical=positive(p[0]),lo=p[1],hi=p[2];
    auto k=count(p[3],2,10001);require(lo<hi&&double(k)*x.size()<=2e7,"Invalid or excessive AR grid");
    double n=double(x.size()),mz=mean(column(x,2)),my=mean(column(x,0)),mx=mean(column(x,1));
    double zz=0;for(const auto& r:x)zz+=(r[2]-mz)*(r[2]-mz);
    positive(zz);
    Report out{{"Coefficient","ReducedFormSlope","F","Accepted"},{}};
    for(std::size_t j=0;j<k;++j){
        double b=lo+(hi-lo)*double(j)/double(k-1),cross=0,sse=0;
        for(const auto& r:x)cross+=(r[2]-mz)*((r[0]-my)-b*(r[1]-mx));
        double slope=cross/zz;
        for(const auto& r:x){double e=(r[0]-my)-b*(r[1]-mx)-slope*(r[2]-mz);sse+=e*e;}
        positive(sse);double f=slope*slope*zz/(sse/(n-2));
        out.rows.push_back({b,slope,f,double(f<=critical)});
    }
    return out;
}
inline Report jackknife_ratio(const Mat& x,const Vec& p){
    validate(x,3,2,2);parameters(p,0);
    double n=double(x.size()),a=sum(column(x,0)),b=sum(column(x,1));
    require(b!=0,"Zero full-sample denominator");
    double full=a/b;Vec pseudo;
    for(const auto& r:x){
        require(b-r[1]!=0,"Zero leave-one-out denominator");
        pseudo.push_back(n*full-(n-1)*(a-r[0])/(b-r[1]));
    }
    double avg=mean(pseudo),ss=0;
    for(double v:pseudo)ss+=(v-avg)*(v-avg);
    return scalar({"FullRatio","BiasCorrectedRatio","JackknifeSE","EstimatedBias"},{full,avg,std::sqrt(ss/(n*(n-1))),full-avg});
}
inline Report exact_sign(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,1);
    std::size_t plus=0,minus=0;
    for(const auto& r:x){if(r[0]>p[0])++plus;else if(r[0]<p[0])++minus;}
    auto n=plus+minus;require(n>0&&n<=100000,"Invalid number of non-tied observations");
    auto k=std::min(plus,minus);
    double log_prob=std::lgamma(double(n)+1)-std::lgamma(double(k)+1)-std::lgamma(double(n-k)+1)-double(n)*std::log(2.0);
    double relative=1,term=1;
    for(std::size_t i=k;i>0;--i){term*=double(i)/double(n-i+1);relative+=term;}
    double log_p=std::min(0.0,std::log(2.0)+log_prob+std::log(relative));
    return scalar({"Positive","Negative","Ties","PValue","LogPValue"},{double(plus),double(minus),double(x.size()-n),std::exp(log_p),log_p});
}
} }
