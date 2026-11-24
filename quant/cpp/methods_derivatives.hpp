#pragma once
#include "evidence_common.hpp"
namespace kline { namespace methods_derivatives {
using namespace evidence;
inline Report margrabe_exchange(const Mat& x,const Vec& p){
    validate(x,1,5,5);
    parameters(p,3);
    double T=positive(p[0]),q1=p[1],q2=p[2];
    Report o{{"EffectiveVol","ExchangeCall"},{}};
    for(auto&r:x){double a=positive(r[0]),b=positive(r[1]),u=r[2],v=r[3],c=r[4];
    require(u>=0&&v>=0&&std::abs(c)<=1,"Invalid volatility or correlation");
    double vol=std::sqrt((u-v)*(u-v)+2*u*v*(1-c)),f=a*std::exp(-q1*T),g=b*std::exp(-q2*T),z=vol*std::sqrt(T),price=std::max(f-g,0.);
    if(z>0){double d=std::log(f/g)/z+.5*z;
    price=f*normal(d)-g*normal(d-z);
    }o.rows.push_back({vol,price});
    }return o;
    
}

inline Report forward_start(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,4);
    double S=positive(p[0]),r=p[1],q=p[2],vol=positive(p[3]);
    Report o{{"RemainingVol","Call"},{}};
    for(auto&a:x){double t=a[0],T=a[1];
    require(t>=0&&T>t,"Invalid exercise times");
    double h=T-t,z=vol*std::sqrt(h),d=(r-q+.5*vol*vol)*h/z;
    o.rows.push_back({z,S*std::exp(-q*t)*(std::exp(-q*h)*normal(d)-std::exp(-r*h)*normal(d-z))});
    }return o;
    
}

inline Report quanto_expectation(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,6);
    double s=positive(p[0]),rf=p[1],q=p[2],u=p[3],v=p[4],rho=p[5];
    require(u>=0&&v>=0&&std::abs(rho)<=1,"Invalid vol or correlation");
    double a=rf-q-rho*u*v;
    Report o{{"Horizon","AdjustedDrift","ExpectedForeignPrice"},{}};
    for(auto&r:x){require(r[0]>=0,"Negative horizon");
    o.rows.push_back({r[0],a,s*std::exp(a*r[0])});
    }return o;
    
}

inline Report power_payoff(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,5);
    double S=positive(p[0]),r=p[1],q=p[2],v=p[3],T=positive(p[4]);
    require(v>=0,"Negative volatility");
    Report o{{"Power","DiscountedMoment"},{}};
    for(auto&z:x){double a=z[0];
    o.rows.push_back({a,std::exp(a*std::log(S)+(-r+a*(r-q)+.5*a*(a-1)*v*v)*T)});
    }return o;
    
}
} }
