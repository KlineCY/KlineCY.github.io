#pragma once
#include "portfolio.hpp"
#include "allocation.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace kline {

inline std::vector<double> project_capped_simplex(
    const std::vector<double>& values, double cap) {
    check_finite(values);
    require(std::isfinite(cap) && cap > 0 && cap <= 1 &&
            cap*values.size() >= 1-1e-12,"Infeasible capped portfolio simplex");
    double low = *std::min_element(values.begin(),values.end())-cap;
    double high = *std::max_element(values.begin(),values.end());
    for (int iter = 0; iter < 100; iter++) {
        double threshold = (low+high)/2, sum = 0;
        for (double v : values) sum += std::max(0.0,std::min(cap,v-threshold));
        if (sum > 1) low = threshold;
        else high = threshold;
    }
    std::vector<double> result;
    for (double v : values) result.push_back(std::max(0.0,std::min(cap,v-(low+high)/2)));
    return result;
}

inline std::vector<double> risk_budget_weights(
    const Matrix& covariance, const std::vector<double>& budget,
    int iterations = 3000) {
    require_positive_semidefinite(covariance);
    check_finite(budget);
    int n = int(budget.size());
    require(n == int(covariance.size()) && iterations > 0,
            "Risk budget dimensions or iteration count are invalid");
    double total_budget = std::accumulate(budget.begin(),budget.end(),0.0);
    require(total_budget > 0,"Total risk budget must be positive");
    for (int i = 0; i < n; i++) {
        require(budget[i] > 0 && covariance[i][i] > 0,
                "Risk budget and asset variance must be positive");
    }
    std::vector<double> x(n);
    for (int i = 0; i < n; i++) x[i] = std::sqrt(budget[i]/total_budget /
                                                   covariance[i][i]);
    for (int iter = 0; iter < iterations; iter++) {
        double changed = 0;
        for (int i = 0; i < n; i++) {
            double cross = 0;
            for (int j = 0; j < n; j++) if (j != i)
                cross += covariance[i][j]*x[j];
            double b = budget[i]/total_budget;
            double updated = 2*b/(std::sqrt(cross*cross+4*covariance[i][i]*b)+cross);
            require(std::isfinite(updated) && updated > 0,
                    "Risk budget optimization is numerically invalid");
            changed = std::max(changed,std::fabs(updated-x[i])/
                                      std::max(1.0,x[i]));
            x[i] = updated;
        }
        if (changed < 1e-11) break;
    }
    double sum = std::accumulate(x.begin(),x.end(),0.0);
    require(sum > 0 && std::isfinite(sum),"Risk budget normalization failed");
    for (double& value : x) value /= sum;
    double variance = portfolio_variance(covariance,x);
    require(variance > 0,"Risk budget solution has no variance");
    return x;
}

inline std::vector<double> historical_es_contributions(
    const std::vector<double>& weights,
    const std::vector<std::vector<double>>& scenario_returns,
    double confidence) {
    check_finite(weights);
    require(!scenario_returns.empty() && std::isfinite(confidence) &&
            confidence > 0 && confidence < 1,
            "Invalid ES confidence or scenario matrix");
    int n = int(weights.size());
    std::vector<std::pair<double,int>> losses;
    for (int t = 0; t < int(scenario_returns.size()); t++) {
        const auto& row = scenario_returns[t];
        require(int(row.size()) == n,"ES scenario has inconsistent assets");
        check_finite(row);
        double loss = -std::inner_product(weights.begin(),weights.end(),row.begin(),0.0);
        losses.emplace_back(loss,t);
    }
    std::sort(losses.begin(),losses.end(),
        [](const auto& a,const auto& b){return a.first>b.first;});
    double tail = (1-confidence)*losses.size(), remaining = tail;
    std::vector<double> result(n,0);
    for (const auto& item : losses) {
        if (remaining <= 0) break;
        double mass = std::min(1.0,remaining);
        for (int j = 0; j < n; j++)
            result[j] -= mass*weights[j]*scenario_returns[item.second][j]/tail;
        remaining -= mass;
    }
    return result;
}

inline double portfolio_effective_number(const std::vector<double>& weights) {
    check_finite(weights);
    double sum = 0, squares = 0;
    for (double w : weights) {sum += std::fabs(w);squares += w*w;}
    require(sum > 0 && squares > 0,"Effective holdings require nonzero exposure");
    return sum*sum/squares;
}

} // namespace kline
