#pragma once
#include "evidence_common.hpp"
namespace kline { namespace methods_rates {
using namespace evidence;
inline Report duration_immunization(const Mat& x,const Vec& p){
    validate(x,2,2,2);
    parameters(p,2);
    require(x.size()==2,"Two assets required");
    double L=positive(p[0]),d=p[1],a=x[0][1],b=x[1][1];
    require(a>=0&&b>a&&d>=a&&d<=b,"Target must lie between durations");
    double v=L*(b-d)/(b-a);
    return {{"Asset","Units","MarketValue","DurationExposure"},{{1,v/positive(x[0][0]),v,v*a},{2,(L-v)/positive(x[1][0]),L-v,(L-v)*b}}};
    
}

inline Report bond_holding_return(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,4);
    double a=positive(p[0]),b=p[1],h=positive(p[2]),r=p[3],c=0;
    require(b>=0,"Negative sale price");
    for(auto&z:x){require(z[0]>0&&z[0]<=h&&z[1]>=0,"Invalid coupon schedule");
    c+=z[1]*std::exp(r*(h-z[0]));
    }return scalar({"InitialPrice","ReinvestedCoupon","SalePrice","HoldingReturn"},{a,c,b,(b+c)/a-1});
    
}

inline Report fisher_breakeven(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,0);
    Report o{{"Nominal","Real","ExactInflation","LinearApproximation"},{}};
    for(auto&r:x){positive(1+r[0]);
    positive(1+r[1]);
    o.rows.push_back({r[0],r[1],(1+r[0])/(1+r[1])-1,r[0]-r[1]});
    }return o;
    
}

inline Report discount_forward_roll(const Mat& x,const Vec& p){
    validate(x,1,4,4);
    parameters(p,0);
    Report o{{"Start","End","ForwardDiscount","SimpleForward"},{}};
    for(auto&r:x){require(r[0]>=0&&r[1]>r[0],"Invalid tenor");
    double a=positive(r[2]),b=positive(r[3]);
    o.rows.push_back({r[0],r[1],b/a,(a/b-1)/(r[1]-r[0])});
    }return o;
    
}
} }
