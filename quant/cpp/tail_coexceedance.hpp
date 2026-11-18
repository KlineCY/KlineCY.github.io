#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
namespace kline {
inline double lower_tail_conditional_exceedance(
   const std::vector<double>& x,const std::vector<double>& y,double tail){
    check_finite(x);check_finite(y);
    require(x.size()==y.size()&&x.size()>=8&&
            std::isfinite(tail)&&tail>0&&tail<0.5,
            "Invalid empirical lower tail dependence inputs");
    double cut_x=quantile(x,tail),cut_y=quantile(y,tail);
    int conditioning=0,joint=0;
    for(int i=0;i<int(x.size());i++){
        if(x[i]<=cut_x){
            conditioning++;
            if(y[i]<=cut_y)joint++;
        }
    }
    require(conditioning>0,"No tail conditioning observations");
    return double(joint)/conditioning;
}
inline double upper_tail_conditional_exceedance(
   const std::vector<double>& x,const std::vector<double>& y,double tail){
    check_finite(x);check_finite(y);
    require(x.size()==y.size()&&x.size()>=8&&
            std::isfinite(tail)&&tail>0&&tail<0.5,
            "Invalid empirical upper tail dependence inputs");
    double cut_x=quantile(x,1-tail),cut_y=quantile(y,1-tail);
    int conditioning=0,joint=0;
    for(int i=0;i<int(x.size());i++){
        if(x[i]>=cut_x){
            conditioning++;
            if(y[i]>=cut_y)joint++;
        }
    }
    require(conditioning>0,"No upper conditioning observations");
    return double(joint)/conditioning;
}
} // namespace kline
