#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>

namespace kline {
inline double survival_probability(double hazard,double years) {
    require(std::isfinite(hazard)&&hazard>=0&&
            std::isfinite(years)&&years>=0,"Invalid credit hazard or maturity");
    return std::exp(-hazard*years);
}

inline double default_probability(double hazard,double years) {
    return 1-survival_probability(hazard,years);
}

inline double continuous_cds_spread(double hazard,double recovery) {
    require(std::isfinite(hazard)&&hazard>=0&&
            std::isfinite(recovery)&&recovery>=0&&recovery<=1,
            "Invalid CDS credit parameters");
    return hazard*(1-recovery);
}

inline double implied_hazard_from_cds(double spread,double recovery) {
    require(std::isfinite(spread)&&spread>=0&&
            std::isfinite(recovery)&&recovery>=0&&recovery<1,
            "Invalid implied credit hazard inputs");
    return spread/(1-recovery);
}

inline double expected_credit_loss(double exposure,double probability,
                                   double loss_given_default) {
    require(std::isfinite(exposure)&&exposure>=0&&
            std::isfinite(probability)&&probability>=0&&probability<=1&&
            std::isfinite(loss_given_default)&&loss_given_default>=0&&
            loss_given_default<=1,"Invalid credit loss parameters");
    return exposure*probability*loss_given_default;
}

inline double risky_zero_coupon_price(double face,double years,double risk_free,
                                      double hazard,double recovery=0) {
    require(std::isfinite(face)&&face>0&&
            std::isfinite(years)&&years>=0&&std::isfinite(risk_free)&&
            std::isfinite(hazard)&&hazard>=0&&std::isfinite(recovery)&&
            recovery>=0&&recovery<=1,"Invalid risky zero-coupon inputs");
    double discount=std::exp(-risk_free*years);
    double survival=survival_probability(hazard,years);
    return face*discount*(survival+recovery*(1-survival));
}

inline double portfolio_expected_credit_loss(
    const std::vector<double>& exposures,
    const std::vector<double>& probabilities,
    const std::vector<double>& lgds) {
    check_finite(exposures);check_finite(probabilities);check_finite(lgds);
    require(exposures.size()==probabilities.size()&&
            exposures.size()==lgds.size(),"Credit portfolio arrays differ");
    double loss=0;
    for(int i=0;i<int(exposures.size());i++)
        loss+=expected_credit_loss(exposures[i],probabilities[i],lgds[i]);
    return loss;
}
} // namespace kline
