#pragma once
#include "probability_models.hpp"
#include <cmath>
#include <random>
#include <vector>
namespace kline {
inline double vasicek_conditional_default_rate(double probability,
    double correlation,double systemic_shock){
    require(probability>0&&probability<1&&std::isfinite(probability)&&
            correlation>=0&&correlation<1&&std::isfinite(correlation)&&
            std::isfinite(systemic_shock),"Invalid one-factor credit inputs");
    double threshold=normal_inverse_cdf(probability);
    return standard_normal_cdf(
        (threshold-std::sqrt(correlation)*systemic_shock)/
        std::sqrt(1-correlation));
}
inline std::vector<double> one_factor_credit_losses(
    const std::vector<double>& exposures,const std::vector<double>& pd,
    const std::vector<double>& lgd,double correlation,
    int scenarios,unsigned int seed){
    check_finite(exposures);check_finite(pd);check_finite(lgd);
    require(exposures.size()==pd.size()&&pd.size()==lgd.size()&&
            scenarios>0&&scenarios<=1000000&&correlation>=0&&correlation<1,
            "Invalid credit simulation");
    int n=int(pd.size());
    for(int i=0;i<n;i++)
        require(exposures[i]>=0&&pd[i]>0&&pd[i]<1&&lgd[i]>=0&&lgd[i]<=1,
                "Invalid obligor exposure or credit probability");
    std::mt19937 rng(seed);
    std::normal_distribution<double> normal(0,1);
    std::vector<double> result;
    for(int s=0;s<scenarios;s++){
        double systemic=normal(rng),loss=0;
        for(int i=0;i<n;i++){
            double latent=std::sqrt(correlation)*systemic+
                std::sqrt(1-correlation)*normal(rng);
            if(latent<normal_inverse_cdf(pd[i]))loss+=exposures[i]*lgd[i];
        }
        result.push_back(loss);
    }
    return result;
}
} // namespace kline
