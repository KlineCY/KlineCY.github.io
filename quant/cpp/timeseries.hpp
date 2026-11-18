#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace kline {

inline std::vector<double> exponential_mean(const std::vector<double>& values,
                                            double alpha) {
    check_finite(values);
    require(std::isfinite(alpha) && alpha > 0 && alpha <= 1,
            "EMA alpha must belong to (0,1]");
    std::vector<double> result(values.size());
    result[0] = values[0];
    for (int i = 1; i < int(values.size()); i++)
        result[i] = alpha*values[i] + (1-alpha)*result[i-1];
    return result;
}

inline std::vector<double> ewma_conditional_variance(
    const std::vector<double>& returns, double decay,
    double initial_variance) {
    check_finite(returns);
    require(std::isfinite(decay) && decay >= 0 && decay < 1 &&
            std::isfinite(initial_variance) && initial_variance >= 0,
            "Invalid EWMA variance parameters");
    std::vector<double> result;
    double h = initial_variance;
    for (double r : returns) {
        result.push_back(h);
        h = decay*h + (1-decay)*r*r;
        require(std::isfinite(h), "EWMA conditional variance overflow");
    }
    return result;
}

struct AR1 {
    double intercept = 0;
    double slope = 0;
    double residual_variance = 0;
};

inline AR1 fit_ar1(const std::vector<double>& values) {
    check_finite(values);
    require(values.size() >= 4, "AR(1) needs at least four observations");
    std::vector<double> lag(values.begin(), values.end()-1);
    std::vector<double> now(values.begin()+1, values.end());
    double denominator = variance(lag, false);
    require(denominator > 0, "Constant AR(1) predictor");
    AR1 result;
    result.slope = covariance(lag, now, false)/denominator;
    result.intercept = mean(now)-result.slope*mean(lag);
    double sum = 0;
    for (int i = 0; i < int(lag.size()); i++) {
        double error = now[i]-result.intercept-result.slope*lag[i];
        sum += error*error;
    }
    result.residual_variance = sum/lag.size();
    return result;
}

inline double ar1_forecast(const AR1& model, double last_observation) {
    require(std::isfinite(last_observation) && std::isfinite(model.slope) &&
            std::isfinite(model.intercept), "Invalid AR forecast inputs");
    return model.intercept+model.slope*last_observation;
}

inline std::vector<double> garch11_forecast_variances(
    const std::vector<double>& residuals, double omega,
    double alpha, double beta, double initial_variance) {
    check_finite(residuals);
    require(std::isfinite(omega) && omega > 0 &&
            std::isfinite(alpha) && std::isfinite(beta) &&
            alpha >= 0 && beta >= 0 && alpha+beta < 1 &&
            std::isfinite(initial_variance) && initial_variance > 0,
            "GARCH(1,1) parameters must be positive and stationary");
    std::vector<double> forecasts;
    double h = initial_variance;
    for (double error : residuals) {
        forecasts.push_back(h);
        h = omega + alpha*error*error + beta*h;
        require(std::isfinite(h) && h > 0, "GARCH overflow");
    }
    return forecasts;
}

inline std::vector<double> rolling_annualized_volatility(
    const std::vector<double>& returns, int window, int periods=252) {
    check_finite(returns);
    require(window >= 2 && window <= int(returns.size()) && periods > 0,
            "Invalid annualized volatility window");
    std::vector<double> result;
    for (int t = window; t <= int(returns.size()); t++) {
        std::vector<double> sample(returns.begin()+t-window,
                                   returns.begin()+t);
        result.push_back(std::sqrt(variance(sample))*std::sqrt(double(periods)));
    }
    return result;
}

inline double half_life_ar1(double slope) {
    require(std::isfinite(slope) && slope > 0 && slope < 1,
            "AR(1) half-life requires 0 < slope < 1");
    return std::log(0.5)/std::log(slope);
}

} // namespace kline
