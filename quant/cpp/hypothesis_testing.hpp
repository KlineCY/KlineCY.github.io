#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct TStatistic{
    double estimate=0,standard_error=0,t_value=0;
};
inline TStatistic welch_t_statistic(const std::vector<double>& a,
                                    const std::vector<double>& b){
    check_finite(a);check_finite(b);
    require(a.size()>=2&&b.size()>=2,"Welch test needs two observations per group");
    double va=variance(a)/a.size(),vb=variance(b)/b.size();
    double se=std::sqrt(va+vb);
    require(se>0,"Welch statistic undefined for zero variance");
    return {mean(a)-mean(b),se,(mean(a)-mean(b))/se};
}
inline TStatistic paired_t_statistic(const std::vector<double>& a,
                                     const std::vector<double>& b){
    check_finite(a);check_finite(b);
    require(a.size()==b.size()&&a.size()>=2,"Paired test shape mismatch");
    std::vector<double> d(a.size());
    for(int i=0;i<int(a.size());i++)d[i]=a[i]-b[i];
    double se=std::sqrt(variance(d)/d.size());
    require(se>0,"Paired t undefined for zero differences variance");
    return {mean(d),se,mean(d)/se};
}
inline double population_standard_error(const std::vector<double>& data){
    check_finite(data);
    require(data.size()>=2,"Standard error needs two observations");
    return std::sqrt(variance(data)/data.size());
}
} // namespace kline
