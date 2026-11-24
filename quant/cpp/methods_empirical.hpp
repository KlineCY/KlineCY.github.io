#pragma once
#include "evidence_common.hpp"
namespace kline { namespace methods_empirical {
using namespace evidence;
inline Report event_car(const Mat& x,const Vec& p){
    validate(x,1,3,3);
    parameters(p,2);
    double s=0;
    Report o{{"RelativeDay","ExpectedReturn","AbnormalReturn","CAR"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){auto&r=x[i];
    require(i==0||r[0]>x[i-1][0],"Increasing event time required");
    double a=p[0]+p[1]*r[2],e=r[1]-a;
    s+=e;
    o.rows.push_back({r[0],a,e,s});
    }return o;
    
}

inline Report paired_randomization(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,0);
    auto n=x.size();
    require(n<=20&&double(std::uint64_t(1)<<n)*n<=3e7,"Exact enumeration work limit");
    Vec d;
    double s=0,scale=0;
    for(auto&r:x){d.push_back(r[0]-r[1]);
    s+=d.back();
    scale+=std::abs(d.back());
    }auto M=std::uint64_t(1)<<n;
    double hit=0;
    for(std::uint64_t z=0;
    z<M;
    ++z){double t=0;
    for(std::size_t i=0;
    i<n;
    ++i)t+=(z&(std::uint64_t(1)<<i)?1:-1)*d[i];
    if(std::abs(t)+1e-12*scale>=std::abs(s))++hit;
    }return scalar({"Pairs","MeanDifference","TwoSidedP","Assignments"},{double(n),s/n,hit/M,double(M)});
    
}

inline Report ipw_balance(const Mat& x,const Vec& p){
    validate(x,2,3,3);
    parameters(p,0);
    double n1=0,n0=0,a=0,b=0,w1=0,w0=0,c=0,d=0;
    for(auto&r:x){require((r[0]==0||r[0]==1)&&r[1]>0&&r[1]<1,"Invalid treatment or propensity");
    if(r[0]){++n1;
    a+=r[2];
    w1+=1/r[1];
    c+=r[2]/r[1];
    }else{++n0;
    b+=r[2];
    w0+=1/(1-r[1]);
    d+=r[2]/(1-r[1]);
    }}positive(n1);
    positive(n0);
    return scalar({"TreatedMean","ControlMean","RawDifference","WeightedDifference"},{c/w1,d/w0,a/n1-b/n0,c/w1-d/w0});
    
}

inline Report placebo_rank(const Mat& x,const Vec& p){
    validate(x,2,1,1);
    parameters(p,0);
    double z=std::abs(x[0][0]),h=0;
    for(std::size_t i=1;
    i<x.size();
    ++i)if(std::abs(x[i][0])>=z)++h;
    return scalar({"ObservedAbs","Placebos","Exceedances","RankP"},{z,double(x.size()-1),h,(1+h)/x.size()});
    
}
} }
