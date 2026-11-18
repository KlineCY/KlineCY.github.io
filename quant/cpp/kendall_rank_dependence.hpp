#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
inline double kendall_tau_b(const std::vector<double>& x,const std::vector<double>& y){
    check_finite(x);check_finite(y);
    require(x.size()==y.size()&&x.size()>=2,
            "Kendall needs aligned paired observations");
    double concordant=0,discordant=0,ties_x=0,ties_y=0;
    for(int i=0;i<int(x.size());i++)
      for(int j=i+1;j<int(x.size());j++){
        double dx=x[i]-x[j],dy=y[i]-y[j];
        if(dx==0&&dy==0)continue;
        if(dx==0){ties_x++;continue;}
        if(dy==0){ties_y++;continue;}
        if(dx*dy>0)concordant++;
        else discordant++;
      }
    double denominator=std::sqrt((concordant+discordant+ties_x)*
                                 (concordant+discordant+ties_y));
    require(denominator>0,"Kendall undefined for constant ranks");
    return (concordant-discordant)/denominator;
}
} // namespace kline
