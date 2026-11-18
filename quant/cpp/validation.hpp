#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace kline {

struct TimeFold {
    int train_begin = 0;
    int train_end = 0; // Exclusive
    int test_begin = 0;
    int test_end = 0; // Exclusive
};

inline std::vector<TimeFold> walk_forward_folds(
    int observations, int training_window, int testing_window,
    int embargo = 0, bool expanding = false) {
    require(observations >= 1 && training_window >= 2 && testing_window >= 1
            && embargo >= 0, "Invalid walk-forward window parameters");
    std::vector<TimeFold> result;
    for (int train_end = training_window;
         train_end + embargo + testing_window <= observations;
         train_end += testing_window) {
        int first = expanding ? 0 : train_end - training_window;
        result.push_back({first, train_end, train_end + embargo,
                          train_end + embargo + testing_window});
    }
    return result;
}

inline std::vector<int> moving_block_resample(int observations, int block_length,
                                               unsigned int seed) {
    require(observations >= 2 && block_length > 0 &&
            block_length <= observations, "Invalid bootstrap block parameters");
    std::mt19937 random(seed);
    std::uniform_int_distribution<int> start(0, observations - block_length);
    std::vector<int> indices;
    while (int(indices.size()) < observations) {
        int beginning = start(random);
        for (int offset = 0; offset < block_length &&
             int(indices.size()) < observations; offset++) {
            indices.push_back(beginning + offset);
        }
    }
    return indices;
}

inline std::vector<double> block_bootstrap_means(const std::vector<double>& returns,
                                                   int block_length,
                                                   int replicates, unsigned int seed) {
    check_finite(returns);
    require(replicates > 0 && replicates <= 100000, "Invalid bootstrap replicates");
    std::vector<double> result;
    for (int r = 0; r < replicates; r++) {
        auto indices = moving_block_resample(int(returns.size()), block_length, seed + r);
        double sum = 0;
        for (int id : indices) sum += returns[id];
        result.push_back(sum / indices.size());
    }
    return result;
}

inline double bonferroni_adjusted_p(double raw_p, int number_of_tests) {
    require(raw_p >= 0 && raw_p <= 1 && number_of_tests > 0,
            "Invalid p-value or test count");
    return std::min(1.0, raw_p * number_of_tests);
}

inline std::vector<bool> benjamini_hochberg(const std::vector<double>& p_values,
                                             double false_discovery_rate = 0.05) {
    check_finite(p_values);
    require(false_discovery_rate > 0 && false_discovery_rate < 1,
            "Invalid target FDR");
    int m = int(p_values.size());
    std::vector<int> order(m);
    std::iota(order.begin(), order.end(), 0);
    for (double p : p_values) require(p >= 0 && p <= 1, "Invalid individual p-value");
    std::stable_sort(order.begin(), order.end(),
                     [&](int a, int b) { return p_values[a] < p_values[b]; });
    int k = 0;
    for (int i = 0; i < m; i++)
        if (p_values[order[i]] <= false_discovery_rate * (i + 1) / m) k = i + 1;
    std::vector<bool> rejected(m, false);
    for (int i = 0; i < k; i++) rejected[order[i]] = true;
    return rejected;
}

inline double brier_score(const std::vector<double>& probability,
                          const std::vector<int>& binary_label) {
    check_finite(probability);
    require(binary_label.size() == probability.size(), "Labels must align with predictions");
    double result = 0;
    for (int i = 0; i < int(probability.size()); i++) {
        require(probability[i] >= 0 && probability[i] <= 1 &&
                (binary_label[i] == 0 || binary_label[i] == 1),
                "Brier requires probabilities and binary labels");
        result += std::pow(probability[i] - binary_label[i], 2);
    }
    return result / probability.size();
}

struct CalibrationBin {
    int observations = 0;
    double predicted_frequency = 0;
    double observed_frequency = 0;
};

inline std::vector<CalibrationBin> reliability_bins(
    const std::vector<double>& probabilities, const std::vector<int>& labels,
    int bins = 10) {
    check_finite(probabilities);
    require(probabilities.size() == labels.size() && bins >= 1 && bins <= 100,
            "Invalid calibration bin inputs");
    std::vector<CalibrationBin> results(bins);
    for (int i = 0; i < int(probabilities.size()); i++) {
        require(probabilities[i] >= 0 && probabilities[i] <= 1 &&
                (labels[i] == 0 || labels[i] == 1), "Invalid probability or label");
        int index = std::min(bins - 1, int(probabilities[i] * bins));
        results[index].observations++;
        results[index].predicted_frequency += probabilities[i];
        results[index].observed_frequency += labels[i];
    }
    for (auto& bin : results) {
        if (bin.observations > 0) {
            bin.predicted_frequency /= bin.observations;
            bin.observed_frequency /= bin.observations;
        }
    }
    return results;
}

}  // namespace kline
