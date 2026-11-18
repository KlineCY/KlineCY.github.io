#pragma once
#include "curve_models.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double nelson_siegel_zero(double maturity,double level,
    double slope,double curvature,double decay){
    require(std::isfinite(maturity)&&maturity>=0&&
            std::isfinite(level)&&std::isfinite(slope)&&
            std::isfinite(curvature)&&std::isfinite(decay)&&decay>0,
            "Invalid Nelson-Siegel zero inputs");
    if(maturity==0)return level+slope;
    double x=maturity/decay;
    double loading=-std::expm1(-x)/x;
    return level+slope*loading+
           curvature*(loading-std::exp(-x));
}
inline ZeroCurve nelson_siegel_curve(const std::vector<double>& times,
    double level,double slope,double curvature,double decay){
    check_finite(times);
    std::vector<double> rates;
    for(int i=0;i<int(times.size());i++){
        require(times[i]>0&&(i==0||times[i]>times[i-1]),
                "Nelson Siegel tenor order");
        rates.push_back(nelson_siegel_zero(times[i],level,slope,
                                           curvature,decay));
    }
    return {times,rates};
}
inline double curve_slope(const ZeroCurve& curve,double short_term,
                           double long_term){
    require(long_term>short_term,"Yield curve slope tenors");
    return interpolate_zero_rate(curve,long_term)-
           interpolate_zero_rate(curve,short_term);
}
} // namespace kline
