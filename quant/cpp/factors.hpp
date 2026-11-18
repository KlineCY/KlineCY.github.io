#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace kline {

inline std::vector<double> winsorize(const std::vector<double>& factor,
                                     double lower_probability = 0.01,
                                     double upper_probability = 0.99) {
    check_finite(factor);
    require(lower_probability >= 0 && upper_probability <= 1 &&
            lower_probability < upper_probability, "Invalid winsorization quantiles");
    double low = quantile(factor, lower_probability);
    double high = quantile(factor, upper_probability);
    std::vector<double> result = factor;
    for (double& x : result) x = std::max(low, std::min(high, x));
    return result;
}

inline std::vector<double> zscore(const std::vector<double>& data) {
    double center = mean(data), standard = std::sqrt(variance(data, false));
    require(standard > 0, "Cannot standardize a constant factor");
    std::vector<double> result;
    for (double x : data) result.push_back((x - center) / standard);
    return result;
}

inline std::vector<double> group_neutralize(const std::vector<double>& factor,
                                             const std::vector<int>& group) {
    check_finite(factor);
    require(group.size() == factor.size(), "Group and factor lengths differ");
    std::map<int, std::pair<double, int>> means;
    for (int i = 0; i < int(factor.size()); i++) {
        means[group[i]].first += factor[i];
        means[group[i]].second++;
    }
    std::vector<double> result;
    for (int i = 0; i < int(factor.size()); i++) {
        auto item = means.at(group[i]);
        result.push_back(factor[i] - item.first / item.second);
    }
    return result;
}

inline std::vector<double> residualize_factor(const std::vector<double>& factor,
                                               const std::vector<double>& control) {
    check_finite(factor);
    check_finite(control);
    require(factor.size() == control.size(), "Controls and factor lengths differ");
    double variance_control = variance(control, false);
    require(variance_control > 0, "Control factor is constant");
    double beta = covariance(factor, control, false) / variance_control;
    double intercept = mean(factor) - beta * mean(control);
    std::vector<double> residuals;
    for (int i = 0; i < int(factor.size()); i++)
        residuals.push_back(factor[i] - intercept - beta * control[i]);
    return residuals;
}

inline double quantile_long_short_spread(const std::vector<double>& scores,
                                         const std::vector<double>& next_returns,
                                         int side_count) {
    check_finite(scores);
    check_finite(next_returns);
    int n = int(scores.size());
    require(n == int(next_returns.size()) && side_count > 0 &&
            side_count * 2 <= n, "Invalid quantile portfolio sizes");
    std::vector<int> order(n);
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(),
                     [&](int a, int b) { return scores[a] < scores[b]; });
    double spread = 0;
    for (int i = 0; i < side_count; i++) {
        spread += next_returns[order[n - 1 - i]] / side_count;
        spread -= next_returns[order[i]] / side_count;
    }
    return spread;
}

inline std::vector<double> information_coefficients(
    const std::vector<std::vector<double>>& factor_by_date,
    const std::vector<std::vector<double>>& forward_returns_by_date,
    bool rank_ic = true) {
    require(!factor_by_date.empty() &&
            factor_by_date.size() == forward_returns_by_date.size(),
            "Factor and return histories must have aligned dates");
    std::vector<double> result;
    for (int t = 0; t < int(factor_by_date.size()); t++) {
        require(factor_by_date[t].size() == forward_returns_by_date[t].size()
                && factor_by_date[t].size() >= 2, "Invalid per-date cross section");
        result.push_back(sample_information_coefficient(
            factor_by_date[t], forward_returns_by_date[t], rank_ic));
    }
    return result;
}

inline double ic_information_ratio(const std::vector<double>& ic_history) {
    double standard = std::sqrt(variance(ic_history));
    require(standard > 0, "ICIR needs nonconstant IC history");
    return mean(ic_history) / standard;
}

inline double feature_decay(const std::vector<double>& factor,
                             const std::vector<double>& future_factor,
                             bool rank = true) {
    return rank ? spearman(factor, future_factor)
                : correlation(factor, future_factor);
}

inline double factor_coverage(const std::vector<double>& values,
                              const std::vector<bool>& valid) {
    require(!valid.empty() && values.size() == valid.size(),
            "Coverage requires equally sized masks and values");
    int count = 0;
    for (int i = 0; i < int(values.size()); i++)
        if (valid[i] && std::isfinite(values[i])) count++;
    return double(count) / valid.size();
}

}  // namespace kline
