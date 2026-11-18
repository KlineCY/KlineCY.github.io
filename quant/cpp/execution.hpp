#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

namespace kline {

struct Quote {
    double bid = 0;
    double ask = 0;
    double bid_size = 0;
    double ask_size = 0;
};

inline void check_quote(const Quote& quote) {
    require(std::isfinite(quote.bid) && std::isfinite(quote.ask) &&
            quote.bid > 0 && quote.ask >= quote.bid &&
            std::isfinite(quote.bid_size) && std::isfinite(quote.ask_size) &&
            quote.bid_size >= 0 && quote.ask_size >= 0,
            "Invalid best bid/ask quote");
}

inline double quoted_spread(const Quote& quote) {
    check_quote(quote);
    return quote.ask - quote.bid;
}

inline double relative_quoted_spread_bps(const Quote& quote) {
    check_quote(quote);
    double midpoint = (quote.bid + quote.ask) / 2;
    return (quote.ask - quote.bid) / midpoint * 10000;
}

inline double microprice(const Quote& quote) {
    check_quote(quote);
    double total = quote.bid_size + quote.ask_size;
    require(total > 0, "Microprice requires positive combined displayed quantity");
    return (quote.ask * quote.bid_size + quote.bid * quote.ask_size) / total;
}

inline double effective_spread_bps(const Quote& quote, double fill,
                                    int side) {
    check_quote(quote);
    require(std::isfinite(fill) && fill > 0 && (side == 1 || side == -1),
            "Invalid fill price or trade sign");
    double midpoint = (quote.bid + quote.ask) / 2;
    return 2.0 * side * (fill - midpoint) / midpoint * 10000;
}

struct ExecutionFill {
    double price = 0;
    double quantity = 0;
};

inline double vwap(const std::vector<ExecutionFill>& fills) {
    require(!fills.empty(), "VWAP requires fills");
    double notional = 0, quantity = 0;
    for (const auto& fill : fills) {
        require(fill.price > 0 && fill.quantity > 0 &&
                std::isfinite(fill.price) && std::isfinite(fill.quantity),
                "Invalid execution fill");
        notional += fill.price * fill.quantity;
        quantity += fill.quantity;
    }
    return notional / quantity;
}

inline double execution_shortfall_bps(double arrival_price,
                                      const std::vector<ExecutionFill>& fills,
                                      int side) {
    require(arrival_price > 0 && std::isfinite(arrival_price) &&
            (side == 1 || side == -1), "Invalid arrival price or direction");
    return side * (vwap(fills) - arrival_price) / arrival_price * 10000;
}

inline double realized_spread_bps(const Quote& quote, double fill,
                                   double future_midpoint, int side) {
    check_quote(quote);
    require(future_midpoint > 0 && std::isfinite(future_midpoint),
            "Invalid future midpoint");
    require(fill > 0 && std::isfinite(fill) && (side == 1 || side == -1),
            "Invalid fill price or trade direction");
    // Relative to the midpoint after the trade; distinct from effective spread.
    return 2 * side * (fill - future_midpoint) /
           ((quote.bid + quote.ask) / 2) * 10000;
}

inline double fill_ratio(double filled_quantity, double requested_quantity) {
    require(std::isfinite(filled_quantity) && std::isfinite(requested_quantity)
            && requested_quantity > 0 && filled_quantity >= 0 &&
            filled_quantity <= requested_quantity,
            "Fill quantity must be inside [0, requested]");
    return filled_quantity / requested_quantity;
}

inline double executed_participation(double executed_quantity, double market_volume) {
    require(executed_quantity >= 0 && std::isfinite(executed_quantity) &&
            market_volume > 0 && std::isfinite(market_volume),
            "Invalid participation input");
    return executed_quantity / market_volume;
}

inline std::vector<double> volume_weighted_schedule(const std::vector<double>& forecast_volume,
                                                      double total_order) {
    check_finite(forecast_volume);
    require(total_order >= 0 && std::isfinite(total_order), "Invalid parent order");
    double total_volume = 0;
    for (double x : forecast_volume) {
        require(x >= 0, "Forecast volumes must be nonnegative");
        total_volume += x;
    }
    require(total_volume > 0, "Cannot split order with zero forecast volume");
    std::vector<double> result;
    for (double x : forecast_volume) result.push_back(total_order * x / total_volume);
    return result;
}

inline std::vector<double> pov_schedule(const std::vector<double>& forecast_volume,
                                         double total_order, double participation) {
    check_finite(forecast_volume);
    require(total_order >= 0 && std::isfinite(total_order) &&
            participation >= 0 && participation <= 1,
            "Invalid order or POV rate");
    std::vector<double> result;
    double remaining = total_order;
    for (double volume : forecast_volume) {
        require(volume >= 0, "Market volume must be nonnegative");
        double child = std::min(remaining, participation * volume);
        result.push_back(child);
        remaining -= child;
    }
    return result;
}

inline double square_root_impact_bps(double daily_volatility,
                                      double participation_fraction,
                                      double impact_coefficient) {
    require(daily_volatility >= 0 && participation_fraction >= 0 &&
            impact_coefficient >= 0 && std::isfinite(daily_volatility) &&
            std::isfinite(participation_fraction) && std::isfinite(impact_coefficient),
            "Invalid market impact inputs");
    // Participation is order quantity / daily quantity, not a percentage in 0-100.
    return 10000 * impact_coefficient * daily_volatility *
           std::sqrt(participation_fraction);
}

}  // namespace kline
