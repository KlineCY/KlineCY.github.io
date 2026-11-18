#pragma once
#include "probability_models.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct GeneralizedPareto{
    double shape=0,scale=1;
};
inline double gpd_survival(double excess,GeneralizedPareto p){
    require(std::isfinite(excess)&&excess>=0&&
            std::isfinite(p.shape)&&std::isfinite(p.scale)&&p.scale>0,
            "GPD parameters");
    if(std::fabs(p.shape)<1e-10)return std::exp(-excess/p.scale);
    double base=1+p.shape*excess/p.scale;
    return base<=0?0:std::pow(base,-1/p.shape);
}
inline double gpd_quantile(double probability,GeneralizedPareto p){
    require(std::isfinite(probability)&&probability>=0&&probability<1&&
            std::isfinite(p.shape)&&std::isfinite(p.scale)&&p.scale>0,
            "GPD quantile parameters");
    if(std::fabs(p.shape)<1e-10)return -p.scale*std::log1p(-probability);
    return p.scale*std::expm1(-p.shape*std::log1p(-probability))/p.shape;
}
inline double hill_tail_index(std::vector<double> losses,int upper_count){
    check_finite(losses);
    require(upper_count>=2&&upper_count<int(losses.size()),
            "Hill estimator needs threshold and exceedances");
    for(double v:losses)require(v>0,"Hill estimator positive losses only");
    std::sort(losses.begin(),losses.end(),std::greater<double>());
    double threshold=losses[upper_count];
    double mean_log=0;
    for(int i=0;i<upper_count;i++)
        mean_log+=std::log(losses[i]/threshold)/upper_count;
    require(mean_log>0,"No positive Hill tail variation");
    return 1/mean_log;
}
inline double gaussian_expected_shortfall(double mean_loss,double loss_sigma,
                                           double confidence){
    require(std::isfinite(mean_loss)&&std::isfinite(loss_sigma)&&loss_sigma>=0&&
            std::isfinite(confidence)&&confidence>0&&confidence<1,
            "Gaussian ES parameters");
    double z=normal_inverse_cdf(confidence);
    return mean_loss+loss_sigma*standard_normal_pdf(z)/(1-confidence);
}
} // namespace kline
