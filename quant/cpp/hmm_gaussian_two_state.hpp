#pragma once
#include "state_models.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct GaussianHmmPath{
    std::vector<std::vector<double>> filtered;
    double log_likelihood=0;
};
inline GaussianHmmPath gaussian_hmm_two_state(
    const std::vector<double>& observations,
    const std::vector<double>& initial,const Matrix& transition,
    const std::vector<double>& state_means,
    const std::vector<double>& state_std){
    check_finite(observations);check_finite(initial);
    check_finite(state_means);check_finite(state_std);
    require(initial.size()==2&&state_means.size()==2&&
            state_std.size()==2&&std::fabs(initial[0]+initial[1]-1)<1e-10,
            "Two-state HMM dimensions");
    for(double sigma:state_std)require(sigma>0,"Invalid Gaussian HMM sigma");
    GaussianHmmPath result;auto probabilities=initial;
    for(double observation:observations){
        std::vector<double> likelihood(2);
        for(int j=0;j<2;j++){
            double z=(observation-state_means[j])/state_std[j];
            likelihood[j]=std::exp(-0.5*z*z)/(state_std[j]*std::sqrt(2*std::acos(-1.0)));
        }
        double predictive=0;
        for(int i=0;i<2;i++)
            for(int j=0;j<2;j++)predictive+=
                probabilities[i]*transition[i][j]*likelihood[j];
        require(predictive>0&&std::isfinite(predictive),
                "HMM emission probability underflow");
        result.log_likelihood+=std::log(predictive);
        probabilities=markov_filter_step(probabilities,transition,likelihood);
        result.filtered.push_back(probabilities);
    }
    return result;
}
} // namespace kline
