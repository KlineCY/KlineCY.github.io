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
            fee >= 0 && fee < 1 && std::isfinite(fee), "Invalid equity or trading cost rate");
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
    require(std::isfinite(cost_bps) && cost_bps >= 0 && cost_bps < 10000 &&
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


inline double total_gross_exposure(const std::vector<double>& weights) {
    check_finite(weights);
    double gross = 0;
    for (double w : weights) gross += std::fabs(w);
    return gross;
}

inline double signed_post_cost_equity(
    double equity, const std::vector<double>& old_weights,
    const std::vector<double>& target, double commission_fraction) {

    require(equity > 0 && std::isfinite(equity) &&
            std::isfinite(commission_fraction) && commission_fraction >= 0,
            "Invalid equity or trading commission");
    require(!target.empty() && target.size() == old_weights.size(),
            "Signed portfolio dimensions differ");
    double gross_target = total_gross_exposure(target);
    total_gross_exposure(old_weights);
    require(commission_fraction * gross_target < 1,
            "Commission and target gross exposure make cash equation unstable");
    if (commission_fraction == 0) return equity;
    double low = 0, high = equity;
    for (int k = 0; k < 80; k++) {
        double after_cost = (low + high) / 2;
        double traded = 0;
        for (int i = 0; i < int(target.size()); i++)
            traded += std::fabs(target[i] * after_cost - old_weights[i] * equity);
        if (after_cost + commission_fraction * traded <= equity)
            low = after_cost;
        else
            high = after_cost;
    }
    require(low > 0, "Commission exhausted equity");
    return low;
}

inline std::vector<PortfolioPeriod> simulate_long_short_rebalance(
    const std::vector<std::vector<double>>& target_weights,
    const std::vector<std::vector<double>>& subsequent_returns,
    double commission_bps, double cash_rate, double funding_rate,
    double short_borrow_rate, double max_gross_exposure,
    double starting_equity = 1) {

    require(!target_weights.empty() &&
            target_weights.size() == subsequent_returns.size(),
            "Trade targets and returns must have matching dates");
    require(std::isfinite(commission_bps) && commission_bps >= 0 &&
            std::isfinite(cash_rate) && cash_rate >= 0 &&
            std::isfinite(funding_rate) && funding_rate >= 0 &&
            std::isfinite(short_borrow_rate) && short_borrow_rate >= 0 &&
            std::isfinite(max_gross_exposure) && max_gross_exposure > 0 &&
            std::isfinite(starting_equity) && starting_equity > 0 &&
            commission_bps / 10000.0 * max_gross_exposure < 1,
            "Invalid financing, commission, or leverage parameters");

    int p = int(target_weights.front().size());
    require(p > 0, "Portfolio requires at least one asset");
    std::vector<double> previous_weights(p, 0);
    double equity = starting_equity;
    std::vector<PortfolioPeriod> result;
    for (int t = 0; t < int(target_weights.size()); t++) {
        const auto& weights = target_weights[t];
        const auto& returns = subsequent_returns[t];
        require(int(weights.size()) == p && int(returns.size()) == p,
                "Portfolio asset counts changed");
        check_finite(weights);
        check_finite(returns);
        require(total_gross_exposure(weights) <= max_gross_exposure + 1e-10,
                "Target exceeds permitted gross exposure");
        for (double r : returns)
            require(r >= -1, "Asset return is below minus 100 percent");

        double after_cost = signed_post_cost_equity(
            equity, previous_weights, weights, commission_bps / 10000.0);
        double traded = 0, long_exposure = 0, short_exposure = 0;
        double gross_return = 0;
        for (int i = 0; i < p; i++) {
            traded += std::fabs(weights[i] * after_cost -
                               previous_weights[i] * equity);
            long_exposure += std::max(weights[i], 0.0);
            short_exposure += std::max(-weights[i], 0.0);
            gross_return += weights[i] * returns[i];
        }

        double free_cash = std::max(0.0, 1.0 - long_exposure);
        double borrowed_cash = std::max(0.0, long_exposure - 1.0);
        double carrying_return = free_cash * cash_rate -
                                 borrowed_cash * funding_rate -
                                 short_exposure * short_borrow_rate;
        double return_before_trade_cost = gross_return + carrying_return;
        double ending_equity = after_cost * (1 + return_before_trade_cost);
        require(std::isfinite(ending_equity) && ending_equity > 0,
                "Short exposure or borrowing exhausted portfolio equity");
        result.push_back({t, equity, traded, equity - after_cost,
                          return_before_trade_cost,
                          ending_equity / equity - 1, ending_equity});
        for (int i = 0; i < p; i++)
            previous_weights[i] = weights[i] * (1 + returns[i]) /
                                  (1 + return_before_trade_cost);
        equity = ending_equity;
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
