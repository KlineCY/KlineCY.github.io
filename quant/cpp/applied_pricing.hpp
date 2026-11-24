#pragma once
#include "evidence_common.hpp"
namespace kline { namespace applied_pricing {
using namespace evidence;
inline Report dividend_forward(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,3);
    double s=positive(p[0]),r=p[1],T=positive(p[2]),pv=0;
    for(auto&z:x){require(z[0]>0&&z[0]<=T&&z[1]>=0,"Invalid dividend schedule");
    pv+=z[1]*std::exp(-r*z[0]);
    }double f=positive(s-pv);
    return scalar({"DividendPV","PrepaidForward","Forward"},{pv,f,f*std::exp(r*T)});
    
}

inline Report option_box(const Mat& x,const Vec& p){
    validate(x,2,3,3);
    parameters(p,0);
    require(x.size()==2&&x[0][0]>0&&x[1][0]>x[0][0],"Two increasing strikes required");
    for(auto&r:x)require(r[1]>=0&&r[2]>=0,"Negative option price");
    double b=x[0][1]-x[1][1]+x[1][2]-x[0][2],k=x[1][0]-x[0][0];
    return scalar({"BoxPrice","TerminalPayoff","ImpliedDiscount"},{positive(b),k,b/k});
    
}

inline Report implied_dividend(const Mat& x,const Vec& p){
    validate(x,1,3,3);
    parameters(p,2);
    double s=positive(p[0]),D=positive(p[1]);
    Report o{{"Strike","ImpliedDividendPV","PrepaidForward"},{}};
    for(auto&r:x){positive(r[0]);
    require(r[1]>=0&&r[2]>=0,"Negative option price");
    double f=r[1]-r[2]+D*r[0];
    o.rows.push_back({r[0],s-f,f});
    }return o;
    
}

inline Report geometric_asian(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,6);
    double s=positive(p[0]),k=positive(p[1]),r=p[2],q=p[3],sigma=positive(p[4]),T=positive(p[5]),n=x.size(),a=0,v=0;
    for(std::size_t i=0;
    i<x.size();
    ++i){double t=x[i][0];
    require(t>0&&t<=T&&(i==0||t>x[i-1][0]),"Increasing observation times required");
    a+=t/n;
    v+=t*(2*(n-double(i))-1)/(n*n);
    }v*=sigma*sigma;
    double m=std::log(s)+(r-q-.5*sigma*sigma)*a,d2=(m-std::log(k))/std::sqrt(positive(v)),d1=d2+std::sqrt(v),f=std::exp(m+v/2),D=std::exp(-r*T);
    return scalar({"LogMean","LogVariance","Call","Put"},{m,v,D*(f*normal(d1)-k*normal(d2)),D*(k*normal(-d2)-f*normal(-d1))});
    
}
} }
