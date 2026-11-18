#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace kline {

inline double standard_normal_cdf(double x) {
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

inline double standard_normal_pdf(double x) {
    const double pi = std::acos(-1.0);
    return std::exp(-x * x / 2) / std::sqrt(2 * pi);
}

struct OptionGreeks {
    double value = 0;
    double delta = 0;
    double gamma = 0;
    double vega = 0;
    double theta_per_year = 0;
    double rho = 0;
};

inline OptionGreeks black_scholes(double spot, double strike, double rate,
                                 double dividend_yield, double volatility,
                                 double maturity, bool call = true) {
    require(spot > 0 && strike > 0 && volatility > 0 && maturity > 0 &&
                std::isfinite(rate) && std::isfinite(dividend_yield),
            "Black-Scholes inputs must be valid and positive where required");
    double root = std::sqrt(maturity);
    double d1 = (std::log(spot / strike) +
                 (rate - dividend_yield + volatility * volatility / 2) * maturity)
                / (volatility * root);
    double d2 = d1 - volatility * root;
    double discounted_spot = spot * std::exp(-dividend_yield * maturity);
    double discounted_strike = strike * std::exp(-rate * maturity);
    double density = standard_normal_pdf(d1);
    OptionGreeks result;
    result.gamma = std::exp(-dividend_yield * maturity) * density /
                   (spot * volatility * root);
    result.vega = discounted_spot * density * root;
    double diffusion = -discounted_spot * density * volatility / (2 * root);
    if (call) {
        result.value = discounted_spot * standard_normal_cdf(d1) -
                       discounted_strike * standard_normal_cdf(d2);
        result.delta = std::exp(-dividend_yield * maturity) * standard_normal_cdf(d1);
        result.theta_per_year = diffusion +
            dividend_yield * discounted_spot * standard_normal_cdf(d1) -
            rate * discounted_strike * standard_normal_cdf(d2);
        result.rho = maturity * discounted_strike * standard_normal_cdf(d2);
    } else {
        result.value = discounted_strike * standard_normal_cdf(-d2) -
                       discounted_spot * standard_normal_cdf(-d1);
        result.delta = std::exp(-dividend_yield * maturity) *
                       (standard_normal_cdf(d1) - 1);
        result.theta_per_year = diffusion -
            dividend_yield * discounted_spot * standard_normal_cdf(-d1) +
            rate * discounted_strike * standard_normal_cdf(-d2);
        result.rho = -maturity * discounted_strike * standard_normal_cdf(-d2);
    }
    return result;
}

inline double option_implied_volatility(double market_price, double spot,
                                        double strike, double rate,
                                        double dividend_yield, double maturity,
                                        bool call = true) {
    require(market_price > 0 && maturity > 0, "Price and maturity must be positive");
    double discount_s = spot * std::exp(-dividend_yield * maturity);
    double discount_k = strike * std::exp(-rate * maturity);
    double lower = call ? std::max(0.0, discount_s - discount_k)
                        : std::max(0.0, discount_k - discount_s);
    double upper = call ? discount_s : discount_k;
    require(market_price > lower && market_price < upper,
            "Option price violates strict European no-arbitrage bounds");
    double left = 1e-8, right = 0.5;
    while (right < 20 &&
           black_scholes(spot, strike, rate, dividend_yield, right, maturity, call).value
           < market_price) {
        right *= 2;
    }
    require(right < 20, "Could not bracket implied volatility");
    for (int i = 0; i < 100; i++) {
        double mid = (left + right) / 2;
        double price = black_scholes(spot, strike, rate, dividend_yield,
                                    mid, maturity, call).value;
        if (price < market_price) left = mid;
        else right = mid;
    }
    return (left + right) / 2;
}

inline double put_call_parity_residual(double call, double put, double spot,
                                        double strike, double rate,
                                        double dividend_yield, double maturity) {
    require(spot > 0 && strike > 0 && maturity >= 0, "Invalid parity inputs");
    return call - put - spot * std::exp(-dividend_yield * maturity) +
           strike * std::exp(-rate * maturity);
}

inline double american_binomial(double spot, double strike, double rate,
                                 double dividend_yield, double volatility,
                                 double maturity, int steps, bool call = false) {
    require(spot > 0 && strike > 0 && volatility > 0 && maturity > 0,
            "Invalid binomial inputs");
    require(steps >= 1 && steps <= 10000, "Binomial steps outside [1,10000]");
    double dt = maturity / steps;
    double u = std::exp(volatility * std::sqrt(dt));
    double d = 1 / u;
    double growth = std::exp((rate - dividend_yield) * dt);
    double probability = (growth - d) / (u - d);
    require(probability >= 0 && probability <= 1,
            "Binomial risk-neutral transition probability is invalid");
    double discount = std::exp(-rate * dt);
    std::vector<double> value(steps + 1);
    for (int j = 0; j <= steps; j++) {
        double final_spot = spot * std::pow(u, j) * std::pow(d, steps - j);
        value[j] = call ? std::max(0.0, final_spot - strike)
                        : std::max(0.0, strike - final_spot);
    }
    for (int step = steps - 1; step >= 0; step--) {
        for (int j = 0; j <= step; j++) {
            double exercise_spot = spot * std::pow(u, j) * std::pow(d, step - j);
            double exercise = call ? std::max(0.0, exercise_spot - strike)
                                   : std::max(0.0, strike - exercise_spot);
            double continuation = discount *
                (probability * value[j + 1] + (1 - probability) * value[j]);
            value[j] = std::max(exercise, continuation);
        }
    }
    return value[0];
}

struct BondRisk {
    double price = 0;
    double macaulay_duration = 0;
    double modified_duration = 0;
    double dv01 = 0;
    double convexity = 0;
};

inline BondRisk fixed_coupon_bond(double face, double annual_coupon_rate,
                                   double annual_yield, int years,
                                   int coupons_per_year = 2) {
    require(face > 0 && annual_coupon_rate >= 0 && years > 0 &&
            coupons_per_year > 0 && coupons_per_year <= 12,
            "Invalid fixed-rate bond inputs");
    double y = annual_yield / coupons_per_year;
    require(y > -1, "Periodic yield is below -100%");
    int n = years * coupons_per_year;
    double coupon = face * annual_coupon_rate / coupons_per_year;
    double weighted = 0, second = 0;
    BondRisk result;
    for (int k = 1; k <= n; k++) {
        double cf = coupon + (k == n ? face : 0);
        double discount = std::pow(1 + y, -k);
        double present_value = cf * discount;
        double t = double(k) / coupons_per_year;
        result.price += present_value;
        weighted += present_value * t;
        second += present_value * k * (k + 1) /
                  (coupons_per_year * coupons_per_year *
                   (1 + y) * (1 + y));
    }
    result.macaulay_duration = weighted / result.price;
    result.modified_duration = result.macaulay_duration / (1 + y);
    result.dv01 = result.price * result.modified_duration * 0.0001;
    result.convexity = second / result.price;
    return result;
}

}  // namespace kline
