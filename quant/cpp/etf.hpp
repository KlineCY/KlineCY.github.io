#pragma once
#include "risk.hpp"
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace kline {

inline double nav_premium_discount(double traded_price, double nav) {
    require(traded_price > 0 && nav > 0 &&
            std::isfinite(traded_price) && std::isfinite(nav),
            "ETF price and NAV must be positive");
    return traded_price / nav - 1;
}

inline double tracking_difference_annualized(const std::vector<double>& fund_returns,
                                               const std::vector<double>& index_returns,
                                               int periods = 252) {
    check_finite(fund_returns);
    check_finite(index_returns);
    require(fund_returns.size() == index_returns.size() && periods > 0,
            "Tracking difference needs matched returns and positive period count");
    return (mean(fund_returns) - mean(index_returns)) * periods;
}

inline double tracking_error_annualized(const std::vector<double>& fund_returns,
                                         const std::vector<double>& index_returns,
                                         int periods = 252) {
    check_finite(fund_returns);
    check_finite(index_returns);
    require(fund_returns.size() == index_returns.size() &&
            fund_returns.size() >= 2 && periods > 0,
            "Tracking error needs at least two paired observations");
    std::vector<double> differences;
    for (int i = 0; i < int(fund_returns.size()); i++)
        differences.push_back(fund_returns[i] - index_returns[i]);
    return std::sqrt(variance(differences)) * std::sqrt(double(periods));
}

inline double daily_expense_drag(double annual_expense_ratio, int days_per_year = 252) {
    require(annual_expense_ratio >= 0 && annual_expense_ratio < 1 &&
            days_per_year > 0, "Invalid ETF expense ratio or day count");
    return 1 - std::pow(1 - annual_expense_ratio, 1.0 / days_per_year);
}

inline double gross_exposure(const std::vector<double>& weights) {
    check_finite(weights);
    double sum = 0;
    for (double x : weights) sum += std::fabs(x);
    return sum;
}

inline double net_exposure(const std::vector<double>& weights) {
    return std::accumulate(weights.begin(), weights.end(), 0.0);
}

inline double leverage_multiple(const std::vector<double>& weights) {
    return gross_exposure(weights);
}

inline double volatility_target_multiplier(double realized_volatility_annual,
                                            double target_volatility_annual,
                                            double max_gross_exposure) {
    require(realized_volatility_annual > 0 && target_volatility_annual >= 0 &&
            max_gross_exposure >= 0 && std::isfinite(realized_volatility_annual) &&
            std::isfinite(target_volatility_annual) &&
            std::isfinite(max_gross_exposure),
            "Invalid volatility targeting input");
    return std::min(target_volatility_annual / realized_volatility_annual,
                    max_gross_exposure);
}

inline double total_return_with_cash_distribution(double prior_price,
                                                   double end_price,
                                                   double cash_distribution) {
    require(prior_price > 0 && end_price > 0 && cash_distribution >= 0 &&
            std::isfinite(prior_price) && std::isfinite(end_price) &&
            std::isfinite(cash_distribution),
            "Invalid price or cash distribution");
    return (end_price + cash_distribution) / prior_price - 1;
}

inline double exchange_traded_fund_liquidity_cost_bps(double quoted_spread_bps,
                                                       double commission_bps,
                                                       double estimated_impact_bps) {
    require(quoted_spread_bps >= 0 && commission_bps >= 0 &&
            estimated_impact_bps >= 0 && std::isfinite(quoted_spread_bps) &&
            std::isfinite(commission_bps) && std::isfinite(estimated_impact_bps),
            "ETF trading cost inputs must be nonnegative");
    // Crossing once from a midpoint costs roughly half the quoted spread.
    return quoted_spread_bps / 2 + commission_bps + estimated_impact_bps;
}

}  // namespace kline
