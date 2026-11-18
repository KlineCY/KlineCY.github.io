#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace kline {

struct ZeroCurve {
    std::vector<double> tenors;
    std::vector<double> zero_rates;
};

inline void check_zero_curve(const ZeroCurve& curve) {
    check_finite(curve.tenors);
    check_finite(curve.zero_rates);
    require(curve.tenors.size() == curve.zero_rates.size() &&
            curve.tenors.front() > 0,"Invalid zero curve dimensions");
    for (int i = 1; i < int(curve.tenors.size()); i++)
        require(curve.tenors[i] > curve.tenors[i-1],
                "Zero curve tenors must be strictly increasing");
}

inline double interpolate_zero_rate(const ZeroCurve& curve, double years) {
    check_zero_curve(curve);
    require(std::isfinite(years) && years >= 0, "Invalid zero rate maturity");
    if (years <= curve.tenors.front()) return curve.zero_rates.front();
    if (years >= curve.tenors.back()) return curve.zero_rates.back();
    auto it = std::upper_bound(curve.tenors.begin(),curve.tenors.end(),years);
    int j = int(it-curve.tenors.begin());
    double t0 = curve.tenors[j-1], t1 = curve.tenors[j];
    return curve.zero_rates[j-1]+
           (curve.zero_rates[j]-curve.zero_rates[j-1])*(years-t0)/(t1-t0);
}

inline double curve_discount(const ZeroCurve& curve,double years) {
    return std::exp(-years*interpolate_zero_rate(curve,years));
}

inline double present_value_cashflows(
    const ZeroCurve& curve,const std::vector<double>& times,
    const std::vector<double>& cashflows) {
    check_finite(times);
    check_finite(cashflows);
    require(times.size() == cashflows.size(),
            "Cashflow dates and amounts differ");
    double value = 0;
    for (int i = 0; i < int(times.size()); i++) {
        require(times[i] > 0,"Cashflow must occur in the future");
        value += cashflows[i]*curve_discount(curve,times[i]);
    }
    require(std::isfinite(value),"Cashflow present value overflow");
    return value;
}

inline double par_swap_rate(const ZeroCurve& curve,
                            const std::vector<double>& payment_times) {
    check_finite(payment_times);
    double annuity = 0, previous = 0;
    for (double t : payment_times) {
        require(t > previous,"Swap payment times must increase");
        annuity += (t-previous)*curve_discount(curve,t);
        previous = t;
    }
    require(annuity > 0,"Swap annuity must be positive");
    return (1-curve_discount(curve,payment_times.back()))/annuity;
}

inline std::vector<double> key_rate_dv01(
    const ZeroCurve& curve,const std::vector<double>& cashflow_times,
    const std::vector<double>& cashflows) {
    check_zero_curve(curve);
    present_value_cashflows(curve,cashflow_times,cashflows);
    std::vector<double> result;
    for (int i = 0; i < int(curve.zero_rates.size()); i++) {
        ZeroCurve up=curve,down=curve;
        up.zero_rates[i] += 1e-4;
        down.zero_rates[i] -= 1e-4;
        double price_up = present_value_cashflows(up,cashflow_times,cashflows);
        double price_down = present_value_cashflows(down,cashflow_times,cashflows);
        result.push_back((price_down-price_up)/2);
    }
    return result;
}

} // namespace kline
