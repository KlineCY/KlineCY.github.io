#pragma once
#include "timeseries.hpp"
#include <cmath>
#include <limits>
#include <vector>
namespace kline {
struct GarchFit{
    double omega=0,alpha=0,beta=0,negative_log_likelihood=0;
    double next_variance=0;
};
inline GarchFit fit_garch_grid(const std::vector<double>& residuals,
                               int resolution=20){
    check_finite(residuals);
    require(residuals.size()>=12&&resolution>=4&&resolution<=100,
            "Invalid GARCH grid parameters");
    double unconditional=0;
    for(double r:residuals)unconditional+=r*r/residuals.size();
    require(unconditional>0,"GARCH needs nonzero variance");
    GarchFit best;best.negative_log_likelihood=
      std::numeric_limits<double>::infinity();
    for(int a=0;a<=resolution;a++)
      for(int b=0;b<=resolution;b++){
        double alpha=double(a)/resolution;
        double beta=double(b)/resolution;
        if(alpha+beta>=0.995)continue;
        double omega=unconditional*(1-alpha-beta);
        double h=unconditional,loss=0;
        for(double r:residuals){
            loss+=0.5*(std::log(2*std::acos(-1.0)*h)+r*r/h);
            h=omega+alpha*r*r+beta*h;
        }
        if(loss<best.negative_log_likelihood)
            best={omega,alpha,beta,loss,h};
    }
    require(std::isfinite(best.negative_log_likelihood),
            "No valid GARCH parameters");
    return best;
}
inline double garch_unconditional_variance(double omega,double alpha,double beta){
    require(std::isfinite(omega)&&omega>0&&
            std::isfinite(alpha)&&alpha>=0&&
            std::isfinite(beta)&&beta>=0&&alpha+beta<1,
            "Nonstationary GARCH");
    return omega/(1-alpha-beta);
}
inline double garch_forecast_horizon(double variance,double omega,double alpha,
                                     double beta,int steps){
    require(std::isfinite(variance)&&variance>=0&&
            steps>=1,"Invalid GARCH horizon");
    double unconditional=garch_unconditional_variance(omega,alpha,beta);
    return unconditional+std::pow(alpha+beta,steps)*(variance-unconditional);
}
} // namespace kline
