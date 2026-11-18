#pragma once
#include "derivatives.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>
namespace kline {
inline double normal_inverse_cdf(double probability){
    require(std::isfinite(probability)&&probability>0&&probability<1,
            "Normal quantile probability must be interior");
    double lo=-1,hi=1;
    while(standard_normal_cdf(lo)>probability)lo*=2;
    while(standard_normal_cdf(hi)<probability)hi*=2;
    for(int i=0;i<140;i++){
        double mid=(lo+hi)/2;
        if(standard_normal_cdf(mid)<probability)lo=mid;else hi=mid;
    }
    return (lo+hi)/2;
}
inline double student_t_density(double value,double degrees){
    require(std::isfinite(value)&&std::isfinite(degrees)&&degrees>0,
            "Student t parameters");
    double half=(degrees+1)/2;
    double log_density=std::lgamma(half)-std::lgamma(degrees/2)-
       0.5*std::log(degrees*std::acos(-1.0))-
       half*std::log1p(value*value/degrees);
    return std::exp(log_density);
}
inline double poisson_probability(int count,double intensity){
    require(count>=0&&std::isfinite(intensity)&&intensity>=0,
            "Poisson parameters");
    if(intensity==0)return count==0?1.0:0.0;
    return std::exp(count*std::log(intensity)-intensity-std::lgamma(count+1.0));
}
inline double lognormal_density(double value,double log_mean,double log_std){
    require(value>0&&std::isfinite(value)&&std::isfinite(log_mean)&&
            std::isfinite(log_std)&&log_std>0,"Lognormal parameters");
    double z=(std::log(value)-log_mean)/log_std;
    return standard_normal_pdf(z)/(value*log_std);
}
inline double normal_interval_probability(double lower,double upper,
                                           double mean_value,double stddev){
    require(std::isfinite(lower)&&std::isfinite(upper)&&lower<=upper&&
            std::isfinite(mean_value)&&std::isfinite(stddev)&&stddev>0,
            "Normal interval parameters");
    return standard_normal_cdf((upper-mean_value)/stddev)-
           standard_normal_cdf((lower-mean_value)/stddev);
}
} // namespace kline
