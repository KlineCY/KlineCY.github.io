#pragma once
#include "information_theory.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline std::vector<double> portfolio_gross_probabilities(
    const std::vector<double>& weights){
    check_finite(weights);
    double gross=0;
    for(double w:weights)gross+=std::fabs(w);
    require(gross>0,"Portfolio exposure is zero");
    std::vector<double> probs;
    for(double w:weights)probs.push_back(std::fabs(w)/gross);
    return probs;
}
inline double holdings_entropy(const std::vector<double>& weights){
    return shannon_entropy(portfolio_gross_probabilities(weights));
}
inline double holdings_effective_number(const std::vector<double>& weights){
    return effective_state_count(portfolio_gross_probabilities(weights));
}
inline double herfindahl_exposure(const std::vector<double>& weights){
    auto probs=portfolio_gross_probabilities(weights);
    double sum=0;for(double p:probs)sum+=p*p;
    return sum;
}
} // namespace kline
