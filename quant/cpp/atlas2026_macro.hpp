#pragma once
#include "atlas2026_common.hpp"
namespace kline { namespace atlas2026 {
inline double after_tax_real_growth(const Values& x,const Values& y,double p){valid(x,y,p);require(p>=0&&p<=1,"Tax fraction must be within [0,1]");double z=1;for(std::size_t i=0;i<x.size();++i){require(1+(1-p)*x[i]>0&&1+y[i]>0,"Invalid gross return or price growth");z*=(1+(1-p)*x[i])/(1+y[i]);}return z-1;}
inline double real_rate_gap(const Values& x,const Values& y,double p){valid(x,y,p);double sum=0;for(std::size_t i=0;i<x.size();++i){require(1+y[i]>0,"Inflation gross factor must be positive");sum+=(1+x[i])/(1+y[i])-1-p;}return sum/x.size();}
inline double taylor_zero_target(const Values& x,const Values& y,double p){valid(x,y,p);double sum=0;for(std::size_t i=0;i<x.size();++i)sum+=p+1.5*x[i]+.5*y[i];return sum/x.size();}
inline double gdp_deflator_level(const Values& x,const Values& y,double p){valid(x,y,p);require(p>0,"Price index reference must be positive");double sum=0;for(std::size_t i=0;i<x.size();++i){require(y[i]>0&&x[i]>=0,"GDP output must be valid");sum+=p*x[i]/y[i];}return sum/x.size();}
inline double cpi_deflated_wage(const Values& x,const Values& y,double p){valid(x,y,p);require(p>0,"CPI base must be positive");double sum=0;for(std::size_t i=0;i<x.size();++i){require(y[i]>0,"CPI index must be positive");sum+=p*x[i]/y[i];}return sum/x.size();}
} }
