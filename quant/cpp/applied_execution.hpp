#pragma once
#include "evidence_common.hpp"
namespace kline { namespace applied_execution {
using namespace evidence;
inline Report markout(const Mat& x,const Vec& p){
    auto n=validate(x,1,4,64);
    parameters(p,0);
    double q=0;
    for(auto&r:x){require(r[0]==1||r[0]==-1,"Invalid side");
    positive(r[1]);
    q+=positive(r[2]);
    for(std::size_t j=3;
    j<n;
    ++j)positive(r[j]);
    }Report o{{"Horizon","SignedMarkout","QuantityWeightedMarkout"},{}};
    for(std::size_t j=3;
    j<n;
    ++j){double v=0;
    for(auto&r:x)v+=r[0]*r[2]*(r[j]-r[1]);
    o.rows.push_back({double(j-2),v,v/q});
    }return o;
    
}

inline Report participation_cap(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,2);
    double rem=positive(p[0]),eta=p[1];
    require(eta>=0&&eta<=1,"Invalid participation");
    Report o{{"Period","MarketVolume","Fill","Remaining","Participation"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){double v=x[i][0];
    require(v>=0,"Negative volume");
    double q=std::min(rem,eta*v);
    rem-=q;
    o.rows.push_back({double(i+1),v,q,rem,v==0?0:q/v});
    }return o;
    
}

inline Report recovery(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,1);
    double b=p[0],d=positive(x[0][1]-b);
    require(b>=0,"Negative reference spread");
    Report o{{"Time","Spread","Recovery","HalfRecovered"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){auto&r=x[i];
    require(r[0]>=0&&r[1]>=0&&(i==0||r[0]>x[i-1][0]),"Invalid time or spread");
    double z=1-(r[1]-b)/d;
    o.rows.push_back({r[0],r[1],z,double(z>=.5)});
    }return o;
    
}

inline Report maker_taker(const Mat& x,const Vec& p){
    validate(x,1,5,5);
    parameters(p,0);
    Report o{{"Trade","GrossMarkout","Fee","NetMarkout"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){auto&r=x[i];
    require(r[0]==1||r[0]==-1,"Invalid side");
    positive(r[1]);
    positive(r[2]);
    positive(r[3]);
    double g=r[0]*(r[2]-r[1])*r[3],f=r[3]*r[4];
    o.rows.push_back({double(i+1),g,f,g-f});
    }return o;
    
}
} }
