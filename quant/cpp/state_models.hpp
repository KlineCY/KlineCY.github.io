#pragma once
#include "portfolio.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace kline {
struct ScalarKalman {
    double state=0;
    double variance=1;
};

inline ScalarKalman kalman_update(ScalarKalman prior,double observation,
                                  double process_variance,
                                  double observation_variance) {
    require(std::isfinite(prior.state)&&std::isfinite(prior.variance)&&
            prior.variance>=0&&std::isfinite(observation)&&
            std::isfinite(process_variance)&&process_variance>=0&&
            std::isfinite(observation_variance)&&observation_variance>0,
            "Invalid scalar Kalman parameters");
    double forecast=prior.variance+process_variance;
    double gain=forecast/(forecast+observation_variance);
    return {prior.state+gain*(observation-prior.state),
            (1-gain)*forecast};
}

inline std::vector<ScalarKalman> kalman_filter(
    const std::vector<double>& observations,ScalarKalman initial,
    double process_variance,double observation_variance) {
    check_finite(observations);
    std::vector<ScalarKalman> result;
    for(double value:observations){
        initial=kalman_update(initial,value,process_variance,observation_variance);
        result.push_back(initial);
    }
    return result;
}

inline std::vector<double> markov_filter_step(
    const std::vector<double>& prior,const Matrix& transition,
    const std::vector<double>& likelihood) {
    check_finite(prior);check_finite(likelihood);
    int n=int(prior.size());
    require(int(likelihood.size())==n&&int(transition.size())==n,
            "Markov state dimensions differ");
    std::vector<double> predicted(n,0);
    double prior_sum=0;
    for(int i=0;i<n;i++){
        require(int(transition[i].size())==n,"Transition must be square");
        double row_sum=0;
        for(int j=0;j<n;j++){
            double v=transition[i][j];
            require(std::isfinite(v)&&v>=0,"Invalid transition probability");
            row_sum+=v;predicted[j]+=prior[i]*v;
        }
        require(std::fabs(row_sum-1)<1e-9,"Transition rows must sum to one");
        require(prior[i]>=0&&likelihood[i]>=0,
                "State probabilities and likelihood must be nonnegative");
        prior_sum+=prior[i];
    }
    require(std::fabs(prior_sum-1)<1e-9,"Prior probabilities must sum to one");
    double total=0;
    for(int j=0;j<n;j++){
        predicted[j]*=likelihood[j];
        total+=predicted[j];
    }
    require(total>0&&std::isfinite(total),"All state likelihoods are zero");
    for(double& v:predicted)v/=total;
    return predicted;
}

inline std::vector<std::vector<double>> markov_forward_filter(
    std::vector<double> prior,const Matrix& transition,
    const std::vector<std::vector<double>>& emission_likelihoods) {
    require(!emission_likelihoods.empty(),"State filter needs observations");
    std::vector<std::vector<double>> path;
    for(const auto& likelihood:emission_likelihoods){
        prior=markov_filter_step(prior,transition,likelihood);
        path.push_back(prior);
    }
    return path;
}
} // namespace kline
