#pragma once
#include "evidence_common.hpp"
namespace kline { namespace methods_dataengineering {
using namespace evidence;
inline Report sequence_gap(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,0);
    double h=0;
    Report o{{"Row","Sequence","GapDetected","DuplicateOrLate","HighWatermark"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){double v=x[i][0];
    require(v>=0&&v<=9007199254740991.&&v==std::floor(v),"Sequence must be exact integer");
    double g=i?std::max(v-h-1,0.):0,l=i?double(v<=h):0;
    h=i?std::max(h,v):v;
    o.rows.push_back({double(i+1),v,g,l,h});
    }return o;
    
}

inline Report asof_staleness(const Mat& x,const Vec& p){
    validate(x,1,3,3);
    parameters(p,1);
    require(p[0]>=0,"Negative age limit");
    Report o{{"DecisionTime","Age","Accepted","Value"},{}};
    for(auto&r:x){double a=r[0]-r[1];
    require(a>=0,"Future information detected");
    o.rows.push_back({r[0],a,double(a<=p[0]),r[2]});
    }return o;
    
}

inline Report latency_tail(const Mat& x,const Vec& p){
    validate(x,1,3,3);
    parameters(p,1);
    require(p[0]>0&&p[0]<=1,"Invalid quantile");
    double a=0,b=0;
    Vec v;
    for(auto&r:x){require(r[1]>=r[0]&&r[2]>=r[1],"Invalid event chronology");
    a+=r[1]-r[0];
    b+=r[2]-r[1];
    v.push_back(r[2]-r[0]);
    }std::sort(v.begin(),v.end());
    auto n=x.size(),k=std::size_t(std::ceil(p[0]*n))-1;
    return scalar({"MeanOutbound","MeanInbound","MeanRoundTrip","RoundTripQuantile","Maximum"},{a/n,b/n,(a+b)/n,v[k],v.back()});
    
}

inline Report position_cash_reconcile(const Mat& x,const Vec& p){
    validate(x,1,4,4);
    parameters(p,1);
    double c=p[0],q=0;
    Report o{{"Trade","Position","Cash","MarkedEquity","TradeMarkImpact"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){auto&r=x[i];
    positive(r[1]);
    positive(r[3]);
    q+=r[0];
    c-=r[0]*r[1]+r[2];
    o.rows.push_back({double(i+1),q,c,c+q*r[3],r[0]*(r[3]-r[1])-r[2]});
    }return o;
    
}
} }
