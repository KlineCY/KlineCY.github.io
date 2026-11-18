#pragma once
#include "atlas2026_common.hpp"
namespace kline { namespace atlas2026 {
inline double participation_headroom(const Values& x,const Values& y,double p){valid(x,y,p);require(p>0&&p<=1,"Participation cap must be in (0,1]");double q=0,v=0;for(std::size_t i=0;i<x.size();++i){require(y[i]>0&&x[i]>=0&&x[i]<=y[i],"Invalid child and market volume");q+=x[i];v+=y[i];}return p-q/v;}
inline double overcap_window_share(const Values& x,const Values& y,double p){valid(x,y,p);require(p>0&&p<=1,"Participation cap must be in (0,1]");int bad=0;for(std::size_t i=0;i<x.size();++i){require(y[i]>0&&x[i]>=0&&x[i]<=y[i],"Invalid trading volumes");bad+=x[i]/y[i]>p;}return double(bad)/x.size();}
inline double exceeding_market_volume(const Values& x,const Values& y,double p){valid(x,y,p);require(p>0&&p<=1,"Participation cap must be in (0,1]");double excess=0;for(std::size_t i=0;i<x.size();++i){require(y[i]>0&&x[i]>=0&&x[i]<=y[i],"Invalid volumes");excess+=std::max(0.0,x[i]-p*y[i]);}return excess;}
inline double peak_participation_utilization(const Values& x,const Values& y,double p){valid(x,y,p);require(p>0&&p<=1,"Participation cap must be in (0,1]");double u=0;for(std::size_t i=0;i<x.size();++i){require(y[i]>0&&x[i]>=0&&x[i]<=y[i],"Invalid volumes");u=std::max(u,x[i]/y[i]);}return u/p;}
inline double participation_target_deviation(const Values& x,const Values& y,double p){valid(x,y,p);require(p>=0&&p<=1,"Participation target must be in [0,1]");double s=0;for(std::size_t i=0;i<x.size();++i){require(y[i]>0&&x[i]>=0&&x[i]<=y[i],"Invalid volumes");double d=x[i]/y[i]-p;s+=d*d;}return s/x.size();}
} }
