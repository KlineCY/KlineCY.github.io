#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace kline {

inline std::vector<double> geometric_brownian_path(double start,
                                                    double annual_drift,
                                                    double annual_volatility,
                                                    double years_per_step,
                                                    const std::vector<double>& normal_shocks) {
    require(start > 0 && annual_volatility >= 0 && years_per_step > 0 &&
            std::isfinite(start) && std::isfinite(annual_drift) &&
            std::isfinite(annual_volatility), "Invalid GBM model inputs");
    check_finite(normal_shocks);
    std::vector<double> path = {start};
    for (double z : normal_shocks) {
        double next = path.back() * std::exp(
            (annual_drift - 0.5 * annual_volatility * annual_volatility) * years_per_step +
             annual_volatility * std::sqrt(years_per_step) * z);
        require(std::isfinite(next) && next > 0, "GBM path overflow");
        path.push_back(next);
    }
    return path;
}

inline std::vector<double> ornstein_uhlenbeck_path(double start, double speed,
                                                    double long_run_mean,
                                                    double volatility, double step_years,
                                                    const std::vector<double>& shocks) {
    require(speed > 0 && volatility >= 0 && step_years > 0 &&
            std::isfinite(start) && std::isfinite(long_run_mean),
            "Invalid OU model inputs");
    check_finite(shocks);
    std::vector<double> path = {start};
    for (double z : shocks) {
        double next = path.back() + speed * (long_run_mean - path.back()) * step_years +
                      volatility * std::sqrt(step_years) * z;
        require(std::isfinite(next), "OU path overflow");
        path.push_back(next);
    }
    return path;
}

inline std::vector<double> cir_full_truncation_path(double start, double speed,
                                                     double long_run_mean,
                                                     double volatility, double step_years,
                                                     const std::vector<double>& shocks) {
    require(start >= 0 && speed > 0 && long_run_mean >= 0 && volatility >= 0 &&
            step_years > 0, "Invalid CIR simulation parameters");
    check_finite(shocks);
    std::vector<double> path = {start};
    for (double z : shocks) {
        double nonnegative = std::max(0.0, path.back());
        double next = path.back() +
            speed * (long_run_mean - nonnegative) * step_years +
            volatility * std::sqrt(nonnegative * step_years) * z;
        require(std::isfinite(next), "CIR path overflow");
        path.push_back(std::max(0.0, next));
    }
    return path;
}

struct MonteCarloPrice {
    double estimate = 0;
    double standard_error = 0;
    int simulated_pairs = 0;
};

inline MonteCarloPrice monte_carlo_european_option(
    double spot, double strike, double rate, double dividend, double volatility,
    double maturity, bool call, int pairs, unsigned int seed) {
    require(spot > 0 && strike > 0 && volatility > 0 && maturity > 0 &&
            pairs > 1 && pairs <= 5000000, "Invalid Monte Carlo option inputs");
    require(std::isfinite(rate) && std::isfinite(dividend), "Invalid carry assumptions");
    std::mt19937 engine(seed);
    std::normal_distribution<double> normal(0, 1);
    double drift = (rate - dividend - volatility * volatility / 2) * maturity;
    double vol = volatility * std::sqrt(maturity);
    double discount = std::exp(-rate * maturity);
    std::vector<double> pair_average;
    pair_average.reserve(pairs);
    for (int i = 0; i < pairs; i++) {
        double z = normal(engine);
        double plus = spot * std::exp(drift + vol * z);
        double minus = spot * std::exp(drift - vol * z);
        double payoff_plus = call ? std::max(0.0, plus - strike)
                                  : std::max(0.0, strike - plus);
        double payoff_minus = call ? std::max(0.0, minus - strike)
                                   : std::max(0.0, strike - minus);
        pair_average.push_back(discount * (payoff_plus + payoff_minus) / 2);
    }
    MonteCarloPrice result;
    result.estimate = mean(pair_average);
    result.standard_error = std::sqrt(variance(pair_average) / pairs);
    result.simulated_pairs = pairs;
    return result;
}

inline std::vector<double> markov_distribution_after_steps(
    const std::vector<std::vector<double>>& transition,
    std::vector<double> probability, int steps) {
    require(steps >= 0 && !transition.empty() &&
            transition.size() == probability.size(), "Invalid Markov model");
    check_finite(probability);
    double initial_total = 0;
    for (double x : probability) {
        require(x >= 0, "Negative state probability");
        initial_total += x;
    }
    require(std::fabs(initial_total - 1) <= 1e-10,
            "Initial state probabilities must sum to one");
    for (const auto& row : transition) {
        require(row.size() == probability.size(), "Transition must be square");
        double total = 0;
        for (double x : row) {
            require(std::isfinite(x) && x >= 0 && x <= 1,
                    "Transition probabilities must be within [0,1]");
            total += x;
        }
        require(std::fabs(total - 1) <= 1e-10, "Transition row must sum to one");
    }
    for (int step = 0; step < steps; step++) {
        std::vector<double> next(probability.size(), 0);
        for (int i = 0; i < int(probability.size()); i++)
            for (int j = 0; j < int(probability.size()); j++)
                next[j] += probability[i] * transition[i][j];
        probability = next;
    }
    return probability;
}

}  // namespace kline
