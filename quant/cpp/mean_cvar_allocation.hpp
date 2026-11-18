#pragma once
#include "portfolio.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>
namespace kline {
inline double empirical_cvar_loss(const Matrix& scenario_returns,
        const std::vector<double>& weights,double confidence){
    require(!scenario_returns.empty()&&std::isfinite(confidence)&&
            confidence>0&&confidence<1,"Invalid CVaR confidence");
    check_finite(weights);
    std::vector<double> losses;
    for(const auto& scenario:scenario_returns){
        check_finite(scenario);
        require(scenario.size()==weights.size(),"Scenario-portfolio dimension");
        double loss=0;
        for(int i=0;i<int(weights.size());i++)loss-=weights[i]*scenario[i];
        losses.push_back(loss);
    }
    std::sort(losses.begin(),losses.end(),std::greater<double>());
    double tail_count=std::max(1.0,(1-confidence)*losses.size());
    int complete=int(std::floor(tail_count));
    double sum=0;
    for(int i=0;i<complete;i++)sum+=losses[i];
    if(complete<int(losses.size()))sum+=(tail_count-complete)*losses[complete];
    return sum/tail_count;
}
inline std::vector<double> minimum_cvar_long_only(const Matrix& scenarios,
            double confidence,int iterations=2000,double learning_rate=0.2){
    require(!scenarios.empty()&&!scenarios[0].empty()&&iterations>0&&
            std::isfinite(learning_rate)&&learning_rate>0&&
            confidence>0&&confidence<1,"Invalid CVaR optimizer");
    int n=int(scenarios[0].size());
    for(const auto& r:scenarios){
        check_finite(r);require(int(r.size())==n,"CVaR ragged scenarios");
    }
    std::vector<double> w(n,1.0/n),average(n,0);
    for(int step=1;step<=iterations;step++){
        std::vector<std::pair<double,int>> losses;
        for(int s=0;s<int(scenarios.size());s++){
            double loss=0;for(int j=0;j<n;j++)loss-=scenarios[s][j]*w[j];
            losses.push_back({loss,s});
        }
        std::stable_sort(losses.begin(),losses.end(),
                         [](auto& a,auto& b){return a.first>b.first;});
        double count=std::max(1.0,(1-confidence)*scenarios.size());
        std::vector<double> gradient(n,0);
        for(int k=0;k<int(losses.size())&&k<int(std::ceil(count));k++){
            double fraction=std::min(1.0,count-k)/count;
            for(int j=0;j<n;j++)gradient[j]-=
                fraction*scenarios[losses[k].second][j];
        }
        std::vector<double> next(n);
        for(int j=0;j<n;j++)next[j]=w[j]-learning_rate/std::sqrt(step)*gradient[j];
        w=project_simplex(next);
        for(int j=0;j<n;j++)average[j]+=w[j]/iterations;
    }
    return project_simplex(average);
}
} // namespace kline
