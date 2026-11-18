#pragma once
#include "atlas2026_common.hpp"
namespace kline { namespace atlas2026 {
inline double huber_objective(const Values& x,const Values& y,double p){valid(x,y,p);require(p>0,"Huber threshold must be positive");double s=0;for(std::size_t i=0;i<x.size();++i){double a=std::abs(x[i]-y[i]);s+=a<=p?a*a/2:p*(a-p/2);}return s/x.size();}
inline double pseudo_huber(const Values& x,const Values& y,double p){valid(x,y,p);require(p>0,"Pseudo-Huber scale must be positive");double s=0;for(std::size_t i=0;i<x.size();++i){double z=(x[i]-y[i])/p;s+=p*p*(std::hypot(1.0,z)-1);}return s/x.size();}
inline double cauchy_objective(const Values& x,const Values& y,double p){valid(x,y,p);require(p>0,"Cauchy scale must be positive");double s=0;for(std::size_t i=0;i<x.size();++i){double z=(x[i]-y[i])/p;s+=p*p*std::log1p(z*z)/2;}return s/x.size();}
inline double scaled_log_cosh(const Values& x,const Values& y,double p){valid(x,y,p);require(p>0,"Log-cosh scale must be positive");double s=0;for(std::size_t i=0;i<x.size();++i){double a=std::abs((x[i]-y[i])/p);s+=p*p*(a+std::log1p(std::exp(-2*a))-std::log(2.0));}return s/x.size();}
inline double tukey_biweight(const Values& x,const Values& y,double p){valid(x,y,p);require(p>0,"Tukey threshold must be positive");double s=0;for(std::size_t i=0;i<x.size();++i){double z=std::abs(x[i]-y[i])/p;if(z>=1)s+=p*p/6;else{double q=1-z*z;s+=p*p*(1-q*q*q)/6;}}return s/x.size();}
} }
