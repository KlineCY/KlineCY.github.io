#pragma once
#include "atlas2026_common.hpp"
namespace kline { namespace atlas2026 {
inline double relative_log_wealth(const Values& x,const Values& y,double p){valid(x,y,p);require(p>=0&&p<1,"Period cost fraction invalid");double z=0;for(std::size_t i=0;i<x.size();++i){require(1+x[i]-p>0&&1+y[i]>0,"Nonpositive gross wealth factor");z+=std::log((1+x[i]-p)/(1+y[i]));}return z;}
inline double relative_wealth_max_drawdown(const Values& x,const Values& y,double p){valid(x,y,p);require(p>=0&&p<1,"Period cost invalid");double nav=1,peak=1,worst=0;for(std::size_t i=0;i<x.size();++i){require(1+x[i]-p>0&&1+y[i]>0,"Invalid relative wealth factors");nav*=(1+x[i]-p)/(1+y[i]);peak=std::max(peak,nav);worst=std::max(worst,1-nav/peak);}return worst;}
inline double excess_return_hurdle_hit(const Values& x,const Values& y,double p){valid(x,y,p);int n=0;for(std::size_t i=0;i<x.size();++i)n+=(x[i]-y[i]>p);return double(n)/x.size();}
inline double relative_volatility_drag(const Values& x,const Values& y,double p){valid(x,y,p);require(p>=0&&p<1,"Period cost invalid");double ar=0,lg=0;for(std::size_t i=0;i<x.size();++i){require(1+x[i]-p>0&&1+y[i]>0,"Nonpositive wealth growth");ar+=x[i]-y[i];lg+=std::log((1+x[i]-p)/(1+y[i]));}return ar/x.size()-(std::exp(lg/x.size())-1);}
inline double relative_terminal_wealth(const Values& x,const Values& y,double p){valid(x,y,p);require(p>=0&&p<1,"Period cost invalid");double wealth=1;for(std::size_t i=0;i<x.size();++i){require(1+x[i]-p>0&&1+y[i]>0,"Nonpositive growth factors");wealth*=(1+x[i]-p)/(1+y[i]);}return wealth;}
} }
