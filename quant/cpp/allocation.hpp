#pragma once
#include "learning.hpp"
#include "portfolio.hpp"
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace kline {

inline void require_positive_semidefinite(const Matrix& covariance) {
    check_covariance(covariance);
    int n = int(covariance.size());
    double scale = 0;
    for (int i = 0; i < n; i++)
        scale = std::max(scale, covariance[i][i]);
    double tolerance = std::max(1e-30, scale * 1e-10);
    Matrix lower(n, std::vector<double>(n, 0));
    for (int i = 0; i < n; i++) {
        double pivot = covariance[i][i];
        for (int k = 0; k < i; k++) pivot -= lower[i][k] * lower[i][k];
        require(pivot >= -tolerance, "Covariance is not positive semidefinite");
        if (pivot <= tolerance) {
            for (int j = i + 1; j < n; j++) {
                double cross = covariance[j][i];
                for (int k = 0; k < i; k++)
                    cross -= lower[i][k] * lower[j][k];
                require(std::fabs(cross) <= tolerance,
                        "Covariance with a zero pivot has a nonzero cross term");
            }
            continue;
        }
        lower[i][i] = std::sqrt(pivot);
        for (int j = i + 1; j < n; j++) {
            double cross = covariance[j][i];
            for (int k = 0; k < i; k++)
                cross -= lower[j][k] * lower[i][k];
            lower[j][i] = cross / lower[i][i];
        }
    }
}

inline std::vector<double> black_litterman_posterior(
    const std::vector<double>& prior_mean, const Matrix& covariance,
    double tau, const Matrix& view_weights,
    const std::vector<double>& view_returns,
    const std::vector<double>& view_error_variances) {

    check_finite(prior_mean);
    require_positive_semidefinite(covariance);
    int p = int(prior_mean.size());
    require(int(covariance.size()) == p && std::isfinite(tau) && tau > 0,
            "Prior dimensions or tau are invalid");
    require(!view_weights.empty() &&
            view_weights.size() == view_returns.size() &&
            view_weights.size() == view_error_variances.size(),
            "Views, expected returns and uncertainty must align");
    check_finite(view_returns);
    check_finite(view_error_variances);
    int m = int(view_weights.size());
    for (int a = 0; a < m; a++) {
        require(int(view_weights[a].size()) == p,
                "View rows must match the number of assets");
        check_finite(view_weights[a]);
        require(view_error_variances[a] > 0,
                "View uncertainty variance must be positive");
    }
    Matrix system(m, std::vector<double>(m, 0));
    std::vector<double> surprise(m, 0);
    for (int a = 0; a < m; a++) {
        double prior_view = std::inner_product(
            view_weights[a].begin(), view_weights[a].end(),
            prior_mean.begin(), 0.0);
        surprise[a] = view_returns[a] - prior_view;
        for (int b = 0; b < m; b++) {
            double value = 0;
            for (int i = 0; i < p; i++)
                for (int j = 0; j < p; j++)
                    value += view_weights[a][i] * covariance[i][j] *
                             view_weights[b][j];
            system[a][b] = tau * value + (a == b ? view_error_variances[a] : 0);
        }
    }
    auto adjustment = solve_linear_system(system, surprise);
    std::vector<double> posterior = prior_mean;
    for (int i = 0; i < p; i++)
        for (int a = 0; a < m; a++)
            for (int j = 0; j < p; j++)
                posterior[i] += tau * covariance[i][j] *
                                view_weights[a][j] * adjustment[a];
    check_finite(posterior);
    return posterior;
}

struct AllocationResult {
    std::vector<double> weights;
    double expected_return = 0;
    double expected_volatility = 0;
    double one_way_turnover = 0;
    double objective = 0;
};

inline AllocationResult optimize_long_only(
    const std::vector<double>& expected_returns, const Matrix& covariance,
    const std::vector<double>& previous_weights, double risk_aversion,
    double turnover_penalty, int maximum_iterations = 5000) {

    check_finite(expected_returns);
    check_finite(previous_weights);
    require_positive_semidefinite(covariance);
    int n = int(expected_returns.size());
    require(int(covariance.size()) == n && int(previous_weights.size()) == n,
            "Expected returns, covariance and previous weights must align");
    require(std::isfinite(risk_aversion) && risk_aversion > 0 &&
            std::isfinite(turnover_penalty) && turnover_penalty >= 0 &&
            maximum_iterations > 0, "Invalid optimization parameters");
    for (double w : previous_weights)
        require(w >= 0 && w <= 1, "Previous weights must be nonnegative and bounded");
    require(std::accumulate(previous_weights.begin(), previous_weights.end(), 0.0)
            <= 1 + 1e-10, "Previous weights exceed one");

    double row_bound = 0;
    for (const auto& row : covariance) {
        double sum = 0;
        for (double value : row) sum += std::fabs(value);
        row_bound = std::max(row_bound, sum);
    }
    double lipschitz = risk_aversion * row_bound + turnover_penalty;
    std::vector<double> w = project_simplex(previous_weights);
    if (lipschitz == 0) {
        w.assign(n, 0);
        w[std::max_element(expected_returns.begin(),
                           expected_returns.end()) - expected_returns.begin()] = 1;
    } else {
        double step = 1.0 / lipschitz;
        for (int iteration = 0; iteration < maximum_iterations; iteration++) {
            auto gradient = covariance_times_weights(covariance, w);
            std::vector<double> next(n);
            for (int i = 0; i < n; i++)
                next[i] = w[i] - step *
                    (risk_aversion * gradient[i] - expected_returns[i] +
                     turnover_penalty * (w[i] - previous_weights[i]));
            next = project_simplex(next);
            double change = 0;
            for (int i = 0; i < n; i++)
                change = std::max(change, std::fabs(next[i] - w[i]));
            w = next;
            if (change < 1e-12) break;
        }
    }
    AllocationResult result;
    result.weights = w;
    result.expected_return = std::inner_product(
        w.begin(), w.end(), expected_returns.begin(), 0.0);
    result.expected_volatility = std::sqrt(portfolio_variance(covariance, w));
    double squared_change = 0;
    for (int i = 0; i < n; i++) {
        double delta = w[i] - previous_weights[i];
        squared_change += delta * delta;
    }
    result.one_way_turnover = one_way_turnover(previous_weights, w);
    result.objective = result.expected_return -
        0.5 * risk_aversion * result.expected_volatility * result.expected_volatility -
        0.5 * turnover_penalty * squared_change;
    return result;
}

} // namespace kline
