#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
struct ConformalInterval{double lower=0,upper=0,radius=0;};
inline ConformalInterval split_conformal_interval(
    const std::vector<double>& calibration_prediction,
    const std::vector<double>& calibration_target,
    double new_prediction,double miscoverage){
    check_finite(calibration_prediction);check_finite(calibration_target);
    require(calibration_prediction.size()==calibration_target.size()&&
            std::isfinite(new_prediction)&&std::isfinite(miscoverage)&&
            miscoverage>0&&miscoverage<1,
            "Invalid split conformal inputs");
    int n=int(calibration_target.size());
    int rank=int(std::ceil((n+1)*(1-miscoverage)));
    require(rank>=1&&rank<=n,
            "Finite-sample conformal quantile requires more calibration data");
    std::vector<double> residual;
    for(int i=0;i<n;i++)
        residual.push_back(std::fabs(calibration_target[i]-
                                     calibration_prediction[i]));
    std::sort(residual.begin(),residual.end());
    double radius=residual[rank-1];
    return {new_prediction-radius,new_prediction+radius,radius};
}
inline double empirical_conformal_coverage(
    const std::vector<double>& center,
    const std::vector<double>& radius,
    const std::vector<double>& realized){
    check_finite(center);check_finite(radius);check_finite(realized);
    require(center.size()==radius.size()&&center.size()==realized.size(),
            "Conformal coverage dimension");
    int covered=0;
    for(int i=0;i<int(center.size());i++){
        require(radius[i]>=0,"Negative interval halfwidth");
        if(std::fabs(realized[i]-center[i])<=radius[i])covered++;
    }
    return double(covered)/center.size();
}
} // namespace kline
