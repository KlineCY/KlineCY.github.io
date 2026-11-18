#pragma once
#include "atlas2026_common.hpp"
namespace kline { namespace atlas2026 {
inline double wape_guarded(const Values& x,const Values& y,double p){valid(x,y,p);require(p>0,"WAPE floor must be positive");double a=0,b=0;for(std::size_t i=0;i<x.size();++i){a+=std::abs(x[i]-y[i]);b+=std::abs(y[i]);}return a/std::max(b,p);}
inline double symmetric_mape(const Values& x,const Values& y,double p){valid(x,y,p);require(p>0,"sMAPE floor must be positive");double s=0;for(std::size_t i=0;i<x.size();++i)s+=2*std::abs(x[i]-y[i])/std::max(std::abs(x[i])+std::abs(y[i]),p);return s/x.size();}
inline double mean_log_cosh_error(const Values& x,const Values& y,double p){valid(x,y,p);require(p>0,"Log-cosh scale must be positive");double s=0;for(std::size_t i=0;i<x.size();++i){double a=std::abs((x[i]-y[i])/p);s+=p*p*(a+std::log1p(std::exp(-2*a))-std::log(2.0));}return s/x.size();}
inline double epsilon_tube_mae(const Values& x,const Values& y,double p){valid(x,y,p);require(p>=0,"Epsilon tolerance must be nonnegative");double s=0;for(std::size_t i=0;i<x.size();++i)s+=std::max(0.0,std::abs(x[i]-y[i])-p);return s/x.size();}
inline double threshold_class_hit(const Values& x,const Values& y,double p){valid(x,y,p);int ok=0;for(std::size_t i=0;i<x.size();++i)ok+=((x[i]>p)==(y[i]>p));return double(ok)/x.size();}
} }
