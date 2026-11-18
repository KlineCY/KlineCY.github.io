#pragma once
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace kline {

inline void require(bool condition, const char* message) {
    if (!condition) throw std::invalid_argument(message);
}

inline void check_finite(const std::vector<double>& data) {
    require(!data.empty(), "The input series is empty");
    for (double x : data) require(std::isfinite(x), "Nonfinite input");
}

inline double mean(const std::vector<double>& data) {
    check_finite(data);
    return std::accumulate(data.begin(), data.end(), 0.0) / data.size();
}

inline double variance(const std::vector<double>& data, bool sample = true) {
    check_finite(data);
    if (sample) require(data.size() >= 2, "Sample variance needs two observations");
    double center = mean(data), sum = 0;
    for (double x : data) sum += (x - center) * (x - center);
    return sum / (data.size() - (sample ? 1 : 0));
}

inline double covariance(const std::vector<double>& a, const std::vector<double>& b,
                         bool sample = true) {
    check_finite(a);
    check_finite(b);
    require(a.size() == b.size(), "Covariance lengths differ");
    if (sample) require(a.size() >= 2, "Sample covariance needs two observations");
    double ma = mean(a), mb = mean(b), sum = 0;
    for (int i = 0; i < int(a.size()); i++) sum += (a[i] - ma) * (b[i] - mb);
    return sum / (a.size() - (sample ? 1 : 0));
}

inline double correlation(const std::vector<double>& a, const std::vector<double>& b) {
    double va = variance(a), vb = variance(b);
    require(va > 0 && vb > 0, "Correlation needs nonconstant inputs");
    return covariance(a, b) / std::sqrt(va * vb);
}

inline double quantile(std::vector<double> data, double probability) {
    check_finite(data);
    require(probability >= 0 && probability <= 1, "Quantile probability must be in [0,1]");
    std::sort(data.begin(), data.end());
    double index = probability * (data.size() - 1);
    int low = int(std::floor(index));
    int high = int(std::ceil(index));
    return data[low] + (index - low) * (data[high] - data[low]);
}

inline std::vector<double> ranks(const std::vector<double>& data) {
    check_finite(data);
    std::vector<int> order(data.size());
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(),
                     [&](int a, int b) { return data[a] < data[b]; });
    std::vector<double> result(data.size());
    int left = 0;
    while (left < int(order.size())) {
        int right = left + 1;
        while (right < int(order.size()) && data[order[right]] == data[order[left]])
            right++;
        double rank = 0.5 * (left + right - 1) + 1;
        for (int i = left; i < right; i++) result[order[i]] = rank;
        left = right;
    }
    return result;
}

inline double spearman(const std::vector<double>& x, const std::vector<double>& y) {
    require(x.size() == y.size(), "Rank correlation lengths differ");
    return correlation(ranks(x), ranks(y));
}

inline double hac_mean_standard_error(const std::vector<double>& data, int lags) {
    check_finite(data);
    require(data.size() >= 2 && lags >= 0 && lags < int(data.size()),
            "HAC lag count must be between zero and n-1");
    int n = int(data.size());
    double center = mean(data);
    double long_run = 0;
    for (int t = 0; t < n; t++) long_run += (data[t] - center) * (data[t] - center) / n;
    for (int lag = 1; lag <= lags; lag++) {
        double gamma = 0;
        for (int t = lag; t < n; t++)
            gamma += (data[t] - center) * (data[t - lag] - center) / n;
        long_run += 2 * (1.0 - double(lag) / (lags + 1)) * gamma;
    }
    return std::sqrt(std::max(0.0, long_run / n));
}

inline std::vector<double> rolling_mean(const std::vector<double>& data, int window) {
    check_finite(data);
    require(window > 0 && window <= int(data.size()), "Invalid rolling window");
    std::vector<double> output;
    double sum = 0;
    for (int i = 0; i < int(data.size()); i++) {
        sum += data[i];
        if (i >= window) sum -= data[i - window];
        if (i >= window - 1) output.push_back(sum / window);
    }
    return output;
}

inline std::vector<double> log_returns(const std::vector<double>& prices) {
    check_finite(prices);
    require(prices.size() >= 2, "Need at least two prices");
    std::vector<double> result;
    for (int t = 1; t < int(prices.size()); t++) {
        require(prices[t] > 0 && prices[t - 1] > 0, "Prices must be positive");
        result.push_back(std::log(prices[t] / prices[t - 1]));
    }
    return result;
}

inline double sample_information_coefficient(const std::vector<double>& factor,
                                             const std::vector<double>& future_return,
                                             bool rank_ic = true) {
    return rank_ic ? spearman(factor, future_return) : correlation(factor, future_return);
}

}  // namespace kline
