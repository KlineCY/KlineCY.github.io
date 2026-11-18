#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace kline {

inline std::vector<double> simple_momentum(const std::vector<double>& prices,
                                            int lookback, int skip = 0) {
    check_finite(prices);
    require(lookback >= 1 && skip >= 0 && lookback+skip < int(prices.size()),
            "Invalid momentum windows");
    for (double price : prices) require(price > 0, "Prices must be positive");
    std::vector<double> result;
    for (int t = lookback+skip; t < int(prices.size()); t++)
        result.push_back(prices[t-skip]/prices[t-skip-lookback]-1);
    return result;
}

inline std::vector<double> relative_strength_index(
    const std::vector<double>& close, int period = 14) {
    check_finite(close);
    require(period > 0 && int(close.size()) > period, "Invalid RSI length");
    for (double price : close) require(price > 0, "RSI needs positive close");
    double gain = 0, loss = 0;
    for (int t = 1; t <= period; t++) {
        double delta = close[t]-close[t-1];
        gain += std::max(0.0, delta)/period;
        loss += std::max(0.0, -delta)/period;
    }
    auto score = [](double up, double down) {
        return up+down == 0 ? 50.0 : 100.0*up/(up+down);
    };
    std::vector<double> result = {score(gain, loss)};
    for (int t = period+1; t < int(close.size()); t++) {
        double delta = close[t]-close[t-1];
        gain = (gain*(period-1)+std::max(0.0, delta))/period;
        loss = (loss*(period-1)+std::max(0.0, -delta))/period;
        result.push_back(score(gain, loss));
    }
    return result;
}

inline std::vector<double> average_true_range(
    const std::vector<double>& high, const std::vector<double>& low,
    const std::vector<double>& close, int period = 14) {
    check_finite(high);
    check_finite(low);
    check_finite(close);
    require(high.size() == low.size() && high.size() == close.size() &&
            period > 0 && int(close.size()) >= period,
            "ATR requires aligned bars and valid period");
    std::vector<double> tr(close.size());
    for (int t = 0; t < int(close.size()); t++) {
        require(high[t] >= low[t] && low[t] >= 0 &&
                close[t] >= low[t] && close[t] <= high[t],
                "Invalid high-low-close bar");
        tr[t] = high[t]-low[t];
        if (t > 0)
            tr[t] = std::max(tr[t], std::max(
                std::fabs(high[t]-close[t-1]),
                std::fabs(low[t]-close[t-1])));
    }
    double average = 0;
    for (int t = 0; t < period; t++) average += tr[t]/period;
    std::vector<double> result = {average};
    for (int t = period; t < int(close.size()); t++) {
        average = ((period-1)*average+tr[t])/period;
        result.push_back(average);
    }
    return result;
}

inline std::vector<double> rolling_breakout_position(
    const std::vector<double>& prices, int lookback) {
    check_finite(prices);
    require(lookback > 0 && lookback < int(prices.size()), "Invalid breakout lookback");
    std::vector<double> result;
    for (int t = lookback; t < int(prices.size()); t++) {
        auto first = prices.begin()+t-lookback;
        auto last = prices.begin()+t;
        double low = *std::min_element(first, last);
        double high = *std::max_element(first, last);
        result.push_back(high > low ? (prices[t]-low)/(high-low) : 0.5);
    }
    return result;
}

} // namespace kline
