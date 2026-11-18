#pragma once
#include "portfolio.hpp"
#include <algorithm>
#include <cmath>
namespace kline {
struct PoissonFit{std::vector<double> coefficients;};
inline PoissonFit fit_poisson_glm_sgd(
    const Matrix& features,const std::vector<double>& counts,
    double learning_rate,int epochs,double l2=0.001){
    require(features.size()==counts.size()&&!features.empty()&&
            std::isfinite(learning_rate)&&learning_rate>0&&
            epochs>=1&&epochs<=100000&&std::isfinite(l2)&&l2>=0,
            "Invalid Poisson GLM optimizer inputs");
    check_finite(counts);
    int p=int(features.front().size());
    require(p>0,"Poisson GLM requires at least one feature");
    for(const auto& row:features){
        check_finite(row);
        require(int(row.size())==p,"Poisson regression feature shape");
    }
    double average=mean(counts);
    for(double c:counts)require(c>=0&&std::floor(c)==c,
                                "Poisson target requires count outcomes");
    std::vector<double> beta(p+1,0);
    beta[0]=std::log(std::max(0.001,average));
    for(int epoch=0;epoch<epochs;epoch++){
        std::vector<double> gradient(p+1,0);
        for(int t=0;t<int(counts.size());t++){
            double linear=beta[0];
            for(int j=0;j<p;j++)linear+=beta[j+1]*features[t][j];
            double expected=std::exp(std::clamp(linear,-20.0,20.0));
            double error=std::clamp(expected-counts[t],-100.0,100.0)/
                         counts.size();
            gradient[0]+=error;
            for(int j=0;j<p;j++)gradient[j+1]+=error*features[t][j];
        }
        double eta=learning_rate/std::sqrt(epoch+1.0);
        for(int j=0;j<p+1;j++)
            beta[j]-=eta*(gradient[j]+(j?l2*beta[j]:0));
    }
    return {beta};
}
inline double predict_poisson_glm(const PoissonFit& model,
                                  const std::vector<double>& feature){
    check_finite(feature);
    require(feature.size()+1==model.coefficients.size(),
            "Poisson prediction dimensionality mismatch");
    double score=model.coefficients[0];
    for(int j=0;j<int(feature.size());j++)
        score+=feature[j]*model.coefficients[j+1];
    return std::exp(std::clamp(score,-20.0,20.0));
}
} // namespace kline
