#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace kline {

struct PriceBar {
    std::string date;
    double open = 0;
    double high = 0;
    double low = 0;
    double close = 0;
    double volume = 0;
    double adjusted_close = 0;
    bool has_adjusted_close = false;
};

inline void validate_bars(const std::vector<PriceBar>& bars) {
    require(!bars.empty(), "No OHLCV bars");
    std::string previous;
    for (const auto& bar : bars) {
        require(bar.date.size() == 10 && bar.date[4] == '-' && bar.date[7] == '-',
                "Dates must be YYYY-MM-DD");
        require(bar.date > previous, "Dates must be unique and strictly increasing");
        previous = bar.date;
        for (double x : {bar.open, bar.high, bar.low, bar.close, bar.volume})
            require(std::isfinite(x), "Nonfinite OHLCV value");
        require(bar.open > 0 && bar.high > 0 && bar.low > 0 && bar.close > 0,
                "OHLC prices must be positive");
        require(bar.high >= std::max(bar.open, bar.close) &&
                bar.low <= std::min(bar.open, bar.close) && bar.high >= bar.low,
                "High/low prices contradict OHLC");
        require(bar.volume >= 0, "Negative volume");
        if (bar.has_adjusted_close)
            require(std::isfinite(bar.adjusted_close) && bar.adjusted_close > 0,
                    "Invalid adjusted close");
    }
}

inline std::vector<double> total_return_series(const std::vector<PriceBar>& bars) {
    validate_bars(bars);
    require(bars.size() >= 2, "Need at least two price observations");
    for (const auto& bar : bars)
        require(bar.has_adjusted_close,
                "Cannot compute adjusted total returns from unadjusted closes");
    std::vector<double> result;
    for (int t = 1; t < int(bars.size()); t++)
        result.push_back(bars[t].adjusted_close / bars[t - 1].adjusted_close - 1);
    return result;
}

struct EventRecord {
    std::string event_date;
    std::string published_at;
    std::string received_at;
    std::string revision_id;
    double value = 0;
};

inline std::string available_at(const EventRecord& event) {
    return std::max(event.published_at, event.received_at);
}

inline bool valid_date_time(const std::string& time) {
    return time.size() >= 10 && time[4] == '-' && time[7] == '-';
}

// Point-in-time join by observable availability time. The event date alone
// never proves information could have been used in a backtest.
inline std::vector<std::pair<bool, double>> asof_join(
    std::vector<EventRecord> events, const std::vector<std::string>& decisions) {
    for (const auto& e : events) {
        require(valid_date_time(e.event_date) && valid_date_time(e.published_at)
                && valid_date_time(e.received_at), "Malformed event date or timestamp");
        require(std::isfinite(e.value), "Nonfinite event value");
    }
    std::stable_sort(events.begin(), events.end(), [](const EventRecord& a,
                                                      const EventRecord& b) {
        return available_at(a) < available_at(b);
    });
    std::vector<std::pair<bool, double>> result;
    for (const auto& decision : decisions) {
        require(valid_date_time(decision), "Malformed decision timestamp");
        bool found = false;
        double last = 0;
        for (const auto& event : events) {
            if (available_at(event) > decision) break;
            last = event.value;
            found = true;
        }
        result.push_back({found, last});
    }
    return result;
}

struct TimeLabel {
    int start = 0;
    int end = 0;
};

// Both train and test include their start and end timestamps.
inline bool labels_overlap(const TimeLabel& a, const TimeLabel& b) {
    require(a.end >= a.start && b.end >= b.start, "Invalid label interval");
    return a.start <= b.end && b.start <= a.end;
}

inline std::vector<int> purge_training_labels(
    const std::vector<TimeLabel>& training, const std::vector<TimeLabel>& validation,
    int embargo_after_validation = 0) {
    require(embargo_after_validation >= 0, "Embargo cannot be negative");
    std::vector<int> kept;
    for (int i = 0; i < int(training.size()); i++) {
        require(training[i].end >= training[i].start, "Malformed training label");
        bool blocked = false;
        for (const auto& test : validation) {
            require(test.end >= test.start, "Malformed validation label");
            TimeLabel excluded = {test.start, test.end + embargo_after_validation};
            if (labels_overlap(training[i], excluded)) {
                blocked = true;
                break;
            }
        }
        if (!blocked) kept.push_back(i);
    }
    return kept;
}

inline std::vector<double> forward_simple_returns(const std::vector<double>& prices,
                                                   int horizon) {
    check_finite(prices);
    require(horizon > 0 && horizon < int(prices.size()), "Invalid target horizon");
    std::vector<double> labels;
    for (int t = 0; t + horizon < int(prices.size()); t++) {
        require(prices[t] > 0 && prices[t + horizon] > 0, "Prices must be positive");
        labels.push_back(prices[t + horizon] / prices[t] - 1);
    }
    return labels;
}

inline std::vector<std::string> common_dates(
    const std::map<std::string, std::vector<PriceBar>>& assets) {
    require(!assets.empty(), "No assets to align");
    std::map<std::string, int> frequency;
    for (const auto& item : assets) {
        validate_bars(item.second);
        for (const auto& bar : item.second) frequency[bar.date]++;
    }
    std::vector<std::string> result;
    for (const auto& item : frequency)
        if (item.second == int(assets.size())) result.push_back(item.first);
    return result;
}

}  // namespace kline
