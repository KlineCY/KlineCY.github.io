#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
struct BetaBernoulliPosterior{double alpha=0,beta=0,mean=0,variance=0;};
inline BetaBernoulliPosterior beta_binomial_update(
    int successes,int failures,double alpha_prior=1,double beta_prior=1){
    require(successes>=0&&failures>=0&&
            std::isfinite(alpha_prior)&&alpha_prior>0&&
            std::isfinite(beta_prior)&&beta_prior>0,
            "Invalid Beta Bernoulli sample and prior");
    double a=alpha_prior+successes,b=beta_prior+failures;
    return {a,b,a/(a+b),a*b/((a+b)*(a+b)*(a+b+1))};
}
inline double beta_binomial_predictive_log_score(
    bool observed_success,const BetaBernoulliPosterior& posterior){
    require(posterior.mean>0&&posterior.mean<1,
            "Invalid Bernoulli predictive probability");
    return std::log(observed_success?posterior.mean:1-posterior.mean);
}
} // namespace kline
