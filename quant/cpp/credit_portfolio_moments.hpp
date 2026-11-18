#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double independent_credit_loss_variance(
   const std::vector<double>& exposures,
   const std::vector<double>& default_probabilities,
   const std::vector<double>& loss_given_default){
    check_finite(exposures);check_finite(default_probabilities);
    check_finite(loss_given_default);
    require(exposures.size()==default_probabilities.size()&&
            exposures.size()==loss_given_default.size(),
            "Credit portfolio loss variance shape");
    double total=0;
    for(int i=0;i<int(exposures.size());i++){
        require(exposures[i]>=0&&default_probabilities[i]>=0&&
                default_probabilities[i]<=1&&loss_given_default[i]>=0&&
                loss_given_default[i]<=1,"Invalid Bernoulli credit inputs");
        double size=exposures[i]*loss_given_default[i];
        total+=size*size*default_probabilities[i]*
               (1-default_probabilities[i]);
    }
    return total;
}
inline double independent_credit_expected_loss(
   const std::vector<double>& exposures,
   const std::vector<double>& default_probabilities,
   const std::vector<double>& loss_given_default){
    check_finite(exposures);check_finite(default_probabilities);
    check_finite(loss_given_default);
    require(exposures.size()==default_probabilities.size()&&
            exposures.size()==loss_given_default.size(),
            "Credit expected loss dimensions");
    double total=0;
    for(int i=0;i<int(exposures.size());i++){
        require(exposures[i]>=0&&default_probabilities[i]>=0&&
                default_probabilities[i]<=1&&loss_given_default[i]>=0&&
                loss_given_default[i]<=1,"Invalid Bernoulli credit input");
        total+=exposures[i]*default_probabilities[i]*loss_given_default[i];
    }
    return total;
}
} // namespace kline
