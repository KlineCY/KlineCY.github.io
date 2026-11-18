#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
struct NormalityMoments{double skewness=0,excess_kurtosis=0,jarque_bera=0;};
inline NormalityMoments normality_moments(const std::vector<double>& sample){
    check_finite(sample);
    require(sample.size()>=4,"Normality diagnostics need four observations");
    double mu=mean(sample),m2=0,m3=0,m4=0;
    for(double value:sample){
        double d=value-mu;
        m2+=d*d;m3+=d*d*d;m4+=d*d*d*d;
    }
    double n=double(sample.size());
    m2/=n;m3/=n;m4/=n;
    require(m2>0,"Normality moments undefined for constant series");
    NormalityMoments result;
    result.skewness=m3/std::pow(m2,1.5);
    result.excess_kurtosis=m4/(m2*m2)-3;
    result.jarque_bera=n*(result.skewness*result.skewness+
                        result.excess_kurtosis*result.excess_kurtosis/4)/6;
    return result;
}
} // namespace kline
