#pragma once
#include "atlas2026_common.hpp"
namespace kline { namespace atlas2026 {
inline double signal_hit_rate(const Values& x,const Values& y,double p){valid(x,y,p);double n=0,k=0;for(std::size_t i=0;i<x.size();++i)if(x[i]>p){++n;k+=y[i]>0;}require(n>0,"No active trading signals");return k/n;}
inline double signal_conditional_payoff(const Values& x,const Values& y,double p){valid(x,y,p);double n=0,s=0;for(std::size_t i=0;i<x.size();++i)if(x[i]>p){++n;s+=y[i];}require(n>0,"No active trading signals");return s/n;}
inline double signal_signed_return(const Values& x,const Values& y,double p){valid(x,y,p);double s=0;for(std::size_t i=0;i<x.size();++i)s+=(x[i]>p?1.0:-1.0)*y[i];return s/x.size();}
inline double signal_top_bottom_spread(const Values& x,const Values& y,double p){valid(x,y,p);require(p>0&&p<.5,"Top-bottom fraction must be in (0,0.5)");std::vector<std::pair<double,double>> a;for(std::size_t i=0;i<x.size();++i)a.push_back({x[i],y[i]});std::stable_sort(a.begin(),a.end(),[](auto u,auto v){return u.first<v.first;});std::size_t k=std::size_t(std::ceil(p*a.size()));require(k>0&&2*k<=a.size(),"Insufficient observations for disjoint tails");double lo=0,hi=0;for(std::size_t i=0;i<k;++i){lo+=a[i].second;hi+=a[a.size()-1-i].second;}return (hi-lo)/k;}
inline double signal_ic_excess(const Values& x,const Values& y,double p){valid(x,y,p);return correlation(x,y)-p;}
} }
