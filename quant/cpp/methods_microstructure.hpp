#pragma once
#include "evidence_common.hpp"
namespace kline { namespace methods_microstructure {
using namespace evidence;
inline Report quote_time_weighted(const Mat& x,const Vec& p){
    validate(x,1,3,3);
    parameters(p,1);
    double end=p[0],T=positive(end-x[0][0]),a=0,b=0;
    for(std::size_t i=0;
    i<x.size();
    ++i){auto&r=x[i];
    positive(r[1]);
    require(r[2]>=r[1],"Crossed quote");
    double h=positive((i+1<x.size()?x[i+1][0]:end)-r[0]);
    a+=h*(r[2]-r[1]);
    b+=h*(r[1]+r[2])/2;
    }return scalar({"Duration","WeightedSpread","WeightedMid"},{T,a/T,b/T});
    
}

inline Report signed_response(const Mat& x,const Vec& p){
    validate(x,2,2,2);
    parameters(p,1);
    auto h=count(p[0],1,x.size()-1);
    require(double(h)*x.size()<=2e7,"Work limit");
    for(auto&r:x){require(r[0]==1||r[0]==-1,"Invalid side");
    positive(r[1]);
    }Report o{{"Lag","Pairs","MeanSignedResponse"},{}};
    for(std::size_t j=1;
    j<=h;
    ++j){double s=0;
    for(std::size_t i=0;
    i+j<x.size();
    ++i)s+=x[i][0]*(x[i+j][1]-x[i][1]);
    o.rows.push_back({double(j),double(x.size()-j),s/(x.size()-j)});
    }return o;
    
}

inline Report depth_walk(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,1);
    double rem=positive(p[0]),n=0,c=0;
    Report o{{"Level","Fill","Remaining","CumulativeVWAP"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){auto&r=x[i];
    positive(r[0]);
    positive(r[1]);
    require(i==0||r[0]>x[i-1][0],"Increasing ask prices required");
    double q=std::min(rem,r[1]);
    rem-=q;
    n+=q;
    c+=q*r[0];
    o.rows.push_back({double(i+1),q,rem,c/n});
    }return o;
    
}

inline Report depth_concentration(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,0);
    double a=0,b=0;
    for(auto&r:x){require(r[0]>=0&&r[1]>=0,"Negative depth");
    a+=r[0];
    b+=r[1];
    }positive(a);
    positive(b);
    double u=0,v=0;
    for(auto&r:x){u+=(r[0]/a)*(r[0]/a);
    v+=(r[1]/b)*(r[1]/b);
    }return scalar({"BidHHI","AskHHI","BidEffectiveLevels","AskEffectiveLevels","TotalImbalance"},{u,v,1/u,1/v,(a-b)/(a+b)});
    
}
} }
