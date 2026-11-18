#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace kline {

struct Drawdown {
    double worst = 0;
    int longest_underwater = 0;
    int current_underwater = 0;
};

inline Drawdown drawdown_from_equity(const std::vector<double>& equity) {
    check_finite(equity);
    double peak = equity.front();
    require(peak > 0, "Equity must be positive");
    Drawdown result;
    for (double value : equity) {
        require(value > 0, "Equity must be positive");
        peak = std::max(peak, value);
        double underwater = value / peak - 1;
        result.worst = std::min(result.worst, underwater);
        if (underwater < -1e-12) result.current_underwater++;
        else result.current_underwater = 0;
        result.longest_underwater = std::max(result.longest_underwater,
                                             result.current_underwater);
    }
    return result;
}

inline double historical_var(const std::vector<double>& losses, double confidence) {
    require(confidence > 0 && confidence < 1, "VaR confidence must be inside (0,1)");
    return quantile(losses, confidence);
}

inline double historical_expected_shortfall(std::vector<double> losses, double confidence) {
    check_finite(losses);
    require(confidence > 0 && confidence < 1, "ES confidence must be inside (0,1)");
    std::sort(losses.begin(), losses.end(), std::greater<double>());
    double tail = (1 - confidence) * losses.size();
    double remaining = tail, sum = 0;
    // Fractional boundary observations matter when (1-alpha)*n is not an integer.
    for (double loss : losses) {
        double mass = std::min(1.0, remaining);
        if (mass <= 0) break;
        sum += mass * loss;
        remaining -= mass;
    }
    return sum / tail;
}

inline double downside_deviation(const std::vector<double>& returns,
                                 double minimum_acceptable = 0) {
    check_finite(returns);
    double sum = 0;
    for (double r : returns)
        sum += std::pow(std::min(0.0, r - minimum_acceptable), 2);
    return std::sqrt(sum / returns.size());
}

inline double annual_volatility(const std::vector<double>& returns, int periods = 252) {
    require(periods > 0, "Periods must be positive");
    return std::sqrt(variance(returns)) * std::sqrt(double(periods));
}

inline double sharpe_ratio(const std::vector<double>& returns,
                           double risk_free_per_period = 0, int periods = 252) {
    double stdev = std::sqrt(variance(returns));
    require(stdev > 0 && periods > 0, "Sharpe needs variability and positive periods");
    return (mean(returns) - risk_free_per_period) / stdev * std::sqrt(double(periods));
}

inline double sortino_ratio(const std::vector<double>& returns,
                            double minimum_acceptable = 0, int periods = 252) {
    require(periods > 0, "Periods must be positive");
    double downside = downside_deviation(returns, minimum_acceptable);
    require(downside > 0, "Sortino needs downside deviation");
    return (mean(returns) - minimum_acceptable) / downside * std::sqrt(double(periods));
}

inline double compounded_return(const std::vector<double>& returns) {
    check_finite(returns);
    double growth = 1;
    for (double r : returns) {
        require(r > -1, "A simple return cannot be less than or equal to -100%");
        growth *= 1 + r;
    }
    return growth - 1;
}

inline double cagr(const std::vector<double>& returns, int periods = 252) {
    require(periods > 0, "Periods must be positive");
    return std::pow(1 + compounded_return(returns), double(periods) / returns.size()) - 1;
}

struct RelativePerformance {
    double beta = 0;
    double alpha_per_period = 0;
    double tracking_error_annual = 0;
    double information_ratio = 0;
};

inline RelativePerformance relative_performance(const std::vector<double>& strategy,
                                                const std::vector<double>& benchmark,
                                                int periods = 252) {
    check_finite(strategy);
    check_finite(benchmark);
    require(strategy.size() == benchmark.size() && strategy.size() >= 2 && periods > 0,
            "Return series must match and contain at least two observations");
    double vb = variance(benchmark);
    require(vb > 0, "Benchmark variance cannot be zero");
    RelativePerformance result;
    result.beta = covariance(strategy, benchmark) / vb;
    result.alpha_per_period = mean(strategy) - result.beta * mean(benchmark);
    std::vector<double> active;
    for (int t = 0; t < int(strategy.size()); t++)
        active.push_back(strategy[t] - benchmark[t]);
    result.tracking_error_annual = annual_volatility(active, periods);
    if (result.tracking_error_annual > 0)
        result.information_ratio = mean(active) * periods / result.tracking_error_annual;
    return result;
}

inline double stress_pnl(const std::vector<double>& weights,
                         const std::vector<double>& scenario) {
    check_finite(weights);
    check_finite(scenario);
    require(weights.size() == scenario.size(), "Stress weights and shocks do not match");
    return std::inner_product(weights.begin(), weights.end(), scenario.begin(), 0.0);
}

inline double parametric_normal_var(double mean_loss, double std_loss,
                                   double confidence, double z_score) {
    require(confidence > 0 && confidence < 1 && std_loss >= 0,
            "Invalid normal VaR inputs");
    // z_score must be the quantile of N(0,1) at the supplied confidence.
    require(z_score >= 0, "Positive confidence quantile required");
    return mean_loss + std_loss * z_score;
}

}  // namespace kline
