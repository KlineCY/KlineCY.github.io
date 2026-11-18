#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace kline {

struct PortfolioPeriod {
    int index = 0;
    double beginning_equity = 0;
    double traded_notional = 0;
    double trading_cost = 0;
    double gross_return = 0;
    double net_return = 0;
    double ending_equity = 0;
};

inline void check_long_only_weights(const std::vector<double>& weights) {
    check_finite(weights);
    double total = 0;
    for (double w : weights) {
        require(w >= 0 && w <= 1, "Long-only weights must be within [0,1]");
        total += w;
    }
    require(total <= 1 + 1e-10, "Long-only weights exceed available equity");
}

inline double post_cost_equity(double equity, const std::vector<double>& old_weights,
                               const std::vector<double>& new_weights, double fee) {
    require(equity > 0 && std::isfinite(equity) &&
            fee >= 0 && std::isfinite(fee), "Invalid equity or trading cost rate");
    require(old_weights.size() == new_weights.size(),
            "Rebalance dimensions differ");
    check_long_only_weights(old_weights);
    check_long_only_weights(new_weights);
    if (fee == 0) return equity;
    double low = 0, high = equity;
    for (int k = 0; k < 80; k++) {
        double after_cost = (low + high) * 0.5;
        double traded = 0;
        for (int i = 0; i < int(new_weights.size()); i++)
            traded += std::fabs(new_weights[i] * after_cost -
                               old_weights[i] * equity);
        if (after_cost + fee * traded <= equity) low = after_cost;
        else high = after_cost;
    }
    require(low > 0, "Trading cost exhausted portfolio equity");
    return low;
}

inline std::vector<PortfolioPeriod> simulate_long_only_rebalance(
    const std::vector<std::vector<double>>& target_weights,
    const std::vector<std::vector<double>>& subsequent_returns,
    double cost_bps, double cash_return = 0, double starting_equity = 1) {

    require(!target_weights.empty() &&
            target_weights.size() == subsequent_returns.size(),
            "Target weights and subsequent returns must have aligned dates");
    require(std::isfinite(cost_bps) && cost_bps >= 0 &&
            std::isfinite(cash_return) && cash_return > -1 &&
            std::isfinite(starting_equity) && starting_equity > 0,
            "Invalid simulation fee, cash return, or starting capital");
    int p = int(target_weights.front().size());
    require(p > 0, "Portfolio requires assets");
    std::vector<double> before(p, 0);
    double equity = starting_equity;
    std::vector<PortfolioPeriod> result;
    for (int t = 0; t < int(target_weights.size()); t++) {
        const auto& weights = target_weights[t];
        const auto& returns = subsequent_returns[t];
        require(int(weights.size()) == p && int(returns.size()) == p,
                "Portfolio observations must be rectangular");
        check_long_only_weights(weights);
        check_finite(returns);
        for (double r : returns)
            require(r >= -1, "Asset simple return below minus 100 percent");

        double after_cost = post_cost_equity(equity, before, weights,
                                            cost_bps / 10000.0);
        double traded = 0;
        for (int i = 0; i < p; i++)
            traded += std::fabs(weights[i] * after_cost - before[i] * equity);
        double charge = equity - after_cost;
        double invested = std::accumulate(weights.begin(), weights.end(), 0.0);
        double gross = (1 - invested) * cash_return;
        for (int i = 0; i < p; i++) gross += weights[i] * returns[i];
        double end_equity = after_cost * (1 + gross);
        require(std::isfinite(end_equity) && end_equity > 0,
                "Portfolio equity was exhausted or overflowed");
        result.push_back({t, equity, traded, charge, gross,
                          end_equity / equity - 1, end_equity});
        for (int i = 0; i < p; i++)
            before[i] = weights[i] * (1 + returns[i]) / (1 + gross);
        equity = end_equity;
    }
    return result;
}

inline std::vector<double> equity_curve_from_periods(
    const std::vector<PortfolioPeriod>& periods) {
    require(!periods.empty(), "No portfolio periods");
    std::vector<double> equity = {periods.front().beginning_equity};
    for (int i = 0; i < int(periods.size()); i++) {
        require(std::isfinite(periods[i].ending_equity) &&
                periods[i].ending_equity > 0, "Nonpositive portfolio equity");
        if (i > 0)
            require(std::fabs(periods[i].beginning_equity - equity.back()) <=
                    1e-9 * std::max(1.0, std::fabs(equity.back())),
                    "Portfolio period equity is not continuous");
        equity.push_back(periods[i].ending_equity);
    }
    return equity;
}

} // namespace kline
