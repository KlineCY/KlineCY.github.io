#pragma once
#include "atlas2026_common.hpp"
namespace kline { namespace atlas2026 {
inline double tail_frequency(const Values& x,const Values& y,double p){valid(x,y,p);const double sw=positive_weights(y);double hit=0;for(std::size_t i=0;i<x.size();++i)if(x[i]>p)hit+=y[i];return hit/sw;}
inline double mean_excess(const Values& x,const Values& y,double p){valid(x,y,p);positive_weights(y);double a=0,b=0;for(std::size_t i=0;i<x.size();++i)if(x[i]>p){a+=y[i]*(x[i]-p);b+=y[i];}require(b>0,"No observations above threshold");return a/b;}
inline double conditional_tail_mean(const Values& x,const Values& y,double p){valid(x,y,p);positive_weights(y);double a=0,b=0;for(std::size_t i=0;i<x.size();++i)if(x[i]>p){a+=y[i]*x[i];b+=y[i];}require(b>0,"No tail observations");return a/b;}
inline double exceedance_second_moment(const Values& x,const Values& y,double p){valid(x,y,p);const double sw=positive_weights(y);double s=0;for(std::size_t i=0;i<x.size();++i){double z=std::max(0.0,x[i]-p);s+=y[i]*z*z;}return s/sw;}
inline double weighted_expected_shortfall(const Values& x,const Values& y,double p){valid(x,y,p);require(p>0&&p<1,"Expected shortfall confidence must be in (0,1)");double mass=(1-p)*positive_weights(y),remain=mass,sum=0;std::vector<std::pair<double,double>> s;for(std::size_t i=0;i<x.size();++i)s.push_back({x[i],y[i]});std::sort(s.begin(),s.end(),[](const auto& a,const auto& b){return a.first>b.first;});for(const auto& z:s){double take=std::min(remain,z.second);sum+=take*z.first;remain-=take;if(remain<=0)break;}return sum/mass;}
} }
