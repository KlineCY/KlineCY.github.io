#pragma once
#include "atlas2026_common.hpp"
namespace kline { namespace atlas2026 {
inline double downside_cohit(const Values& x,const Values& y,double p){valid(x,y,p);int hits=0;for(std::size_t i=0;i<x.size();++i)hits+=(x[i]<p&&y[i]<p);return double(hits)/x.size();}
inline double conditional_downside(const Values& x,const Values& y,double p){valid(x,y,p);int cond=0,hits=0;for(std::size_t i=0;i<x.size();++i)if(x[i]<p){++cond;hits+=y[i]<p;}require(cond>0,"No downside conditioning observations");return double(hits)/cond;}
inline double upside_cohit(const Values& x,const Values& y,double p){valid(x,y,p);int hits=0;for(std::size_t i=0;i<x.size();++i)hits+=(x[i]>p&&y[i]>p);return double(hits)/x.size();}
inline double conditional_downside_beta(const Values& x,const Values& y,double p){valid(x,y,p);double sx=0,sy=0;int n=0;for(std::size_t i=0;i<x.size();++i)if(x[i]<p){sx+=x[i];sy+=y[i];++n;}require(n>=2,"Downside beta needs two observations");double mx=sx/n,my=sy/n,top=0,bot=0;for(std::size_t i=0;i<x.size();++i)if(x[i]<p){top+=(x[i]-mx)*(y[i]-my);bot+=(x[i]-mx)*(x[i]-mx);}require(bot>0,"Downside beta has no factor variation");return top/bot;}
inline double downside_relative_spread(const Values& x,const Values& y,double p){valid(x,y,p);double sum=0;int n=0;for(std::size_t i=0;i<x.size();++i)if(x[i]<p){sum+=y[i]-x[i];++n;}require(n>0,"No downside return observations");return sum/n;}
} }
