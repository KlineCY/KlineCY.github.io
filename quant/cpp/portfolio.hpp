#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace kline {

using Matrix = std::vector<std::vector<double>>;

inline void check_covariance(const Matrix& cov) {
    require(!cov.empty(), "Covariance matrix is empty");
    int n = int(cov.size());
    for (int i = 0; i < n; i++) {
        require(int(cov[i].size()) == n, "Covariance matrix must be square");
        for (int j = 0; j < n; j++) {
            require(std::isfinite(cov[i][j]), "Nonfinite covariance");
            if (i == j) require(cov[i][i] >= 0, "Negative covariance diagonal");
            if (j < i)
                require(std::fabs(cov[i][j] - cov[j][i]) < 1e-9,
                        "Covariance matrix must be symmetric");
        }
    }
}

inline Matrix sample_covariance_matrix(const std::vector<std::vector<double>>& asset_returns) {
    require(!asset_returns.empty(), "Asset return matrix is empty");
    int n = int(asset_returns.size());
    int observations = int(asset_returns[0].size());
    require(observations >= 2, "Covariance estimation needs at least two dates");
    for (const auto& series : asset_returns) {
        check_finite(series);
        require(int(series.size()) == observations, "Asset dates must be aligned");
    }
    Matrix cov(n, std::vector<double>(n, 0));
    for (int i = 0; i < n; i++)
        for (int j = 0; j <= i; j++) {
            cov[i][j] = covariance(asset_returns[i], asset_returns[j]);
            cov[j][i] = cov[i][j];
        }
    return cov;
}

inline Matrix shrink_to_diagonal(const Matrix& covariance, double intensity) {
    check_covariance(covariance);
    require(intensity >= 0 && intensity <= 1, "Shrinkage intensity must be in [0,1]");
    Matrix result = covariance;
    for (int i = 0; i < int(result.size()); i++)
        for (int j = 0; j < int(result.size()); j++)
            if (i != j) result[i][j] *= (1 - intensity);
    // Intensity is a supplied parameter, not an estimated Ledoit-Wolf coefficient.
    return result;
}

inline std::vector<double> covariance_times_weights(const Matrix& covariance,
                                                      const std::vector<double>& weights) {
    check_covariance(covariance);
    require(weights.size() == covariance.size(), "Weight and covariance sizes differ");
    check_finite(weights);
    std::vector<double> result(weights.size(), 0);
    for (int i = 0; i < int(weights.size()); i++)
        for (int j = 0; j < int(weights.size()); j++)
            result[i] += covariance[i][j] * weights[j];
    return result;
}

inline double portfolio_variance(const Matrix& covariance,
                                 const std::vector<double>& weights) {
    auto marginal = covariance_times_weights(covariance, weights);
    double result = std::inner_product(weights.begin(), weights.end(),
                                       marginal.begin(), 0.0);
    require(result >= -1e-10, "Covariance input yields negative portfolio variance");
    return std::max(0.0, result);
}

inline std::vector<double> volatility_risk_contributions(const Matrix& covariance,
                                                          const std::vector<double>& weights) {
    double volatility = std::sqrt(portfolio_variance(covariance, weights));
    require(volatility > 0, "Zero-volatility portfolio has undefined contributions");
    auto marginal = covariance_times_weights(covariance, weights);
    for (int i = 0; i < int(weights.size()); i++)
        marginal[i] = weights[i] * marginal[i] / volatility;
    return marginal;
}

inline std::vector<double> project_simplex(const std::vector<double>& input) {
    check_finite(input);
    std::vector<double> sorted = input;
    std::sort(sorted.begin(), sorted.end(), std::greater<double>());
    double sum = 0, threshold = 0;
    int active = 0;
    for (int i = 0; i < int(sorted.size()); i++) {
        sum += sorted[i];
        double candidate = (sum - 1) / (i + 1);
        if (sorted[i] > candidate) {
            active = i + 1;
            threshold = candidate;
        }
    }
    require(active > 0, "Projection could not find active assets");
    std::vector<double> result(input.size());
    for (int i = 0; i < int(input.size()); i++)
        result[i] = std::max(0.0, input[i] - threshold);
    return result;
}

inline std::vector<double> minimum_variance_long_only(const Matrix& covariance,
                                                       int iterations = 6000) {
    check_covariance(covariance);
    require(iterations > 0, "Iteration count must be positive");
    int n = int(covariance.size());
    std::vector<double> weights(n, 1.0 / n);
    double row_norm = 0;
    for (const auto& row : covariance) {
        double sum = 0;
        for (double x : row) sum += std::fabs(x);
        row_norm = std::max(row_norm, sum);
    }
    if (row_norm == 0) return weights;
    double step = 1.0 / (2.0 * row_norm);
    for (int iteration = 0; iteration < iterations; iteration++) {
        auto marginal = covariance_times_weights(covariance, weights);
        for (int i = 0; i < n; i++) weights[i] -= 2 * step * marginal[i];
        weights = project_simplex(weights);
    }
    return weights;
}

inline std::vector<double> inverse_volatility_weights(const Matrix& covariance) {
    check_covariance(covariance);
    std::vector<double> weights(covariance.size());
    double sum = 0;
    for (int i = 0; i < int(covariance.size()); i++) {
        require(covariance[i][i] > 0, "Inverse volatility needs positive variances");
        weights[i] = 1 / std::sqrt(covariance[i][i]);
        sum += weights[i];
    }
    for (double& w : weights) w /= sum;
    return weights;
}

inline double one_way_turnover(const std::vector<double>& before,
                               const std::vector<double>& target) {
    check_finite(before);
    check_finite(target);
    require(before.size() == target.size(), "Turnover vectors differ");
    double two_way = 0;
    for (int i = 0; i < int(before.size()); i++)
        two_way += std::fabs(target[i] - before[i]);
    return two_way / 2;
}

inline double trading_cost(const std::vector<double>& before,
                           const std::vector<double>& target,
                           double cost_bps_per_dollar) {
    require(cost_bps_per_dollar >= 0 && std::isfinite(cost_bps_per_dollar),
            "Trading cost rate must be finite and nonnegative");
    // Charge the absolute traded notional on both purchases and sales.
    return 2 * one_way_turnover(before, target) * cost_bps_per_dollar / 10000.0;
}

inline std::vector<double> drifted_weights(const std::vector<double>& target,
                                            const std::vector<double>& asset_returns) {
    check_finite(target);
    check_finite(asset_returns);
    require(target.size() == asset_returns.size(), "Weight and return sizes differ");
    double total = 0;
    std::vector<double> drifted(target.size());
    for (int i = 0; i < int(target.size()); i++) {
        require(asset_returns[i] > -1, "Asset lost more than 100%");
        drifted[i] = target[i] * (1 + asset_returns[i]);
        total += drifted[i];
    }
    require(std::fabs(total) > 1e-12, "Portfolio wealth is zero");
    for (double& x : drifted) x /= total;
    return drifted;
}

}  // namespace kline
