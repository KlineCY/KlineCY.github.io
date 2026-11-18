#pragma once
#include "learning.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace kline {

inline double soft_threshold(double value, double penalty) {
    require(std::isfinite(value) && std::isfinite(penalty) && penalty >= 0,
            "Invalid soft threshold");
    return value > penalty ? value-penalty :
           value < -penalty ? value+penalty : 0;
}

inline RidgeModel fit_lasso(const Matrix& x, const std::vector<double>& y,
                           double lambda, int iterations = 5000) {
    check_feature_table(x, y);
    require(std::isfinite(lambda) && lambda >= 0 && iterations > 0,
            "Invalid Lasso penalty or iteration count");
    int n = int(x.size()), p = int(x[0].size());
    RidgeModel model;
    model.intercept = mean(y);
    model.coefficients.assign(p, 0);
    model.feature_mean.assign(p, 0);
    model.feature_scale.assign(p, 1);
    Matrix standardized(n, std::vector<double>(p));
    std::vector<double> norm(p, 0);
    for (int j = 0; j < p; j++) {
        for (int t = 0; t < n; t++) model.feature_mean[j] += x[t][j]/n;
        double variance_sum = 0;
        for (int t = 0; t < n; t++)
            variance_sum += std::pow(x[t][j]-model.feature_mean[j],2)/n;
        model.feature_scale[j] = variance_sum > 0 ? std::sqrt(variance_sum):1;
        for (int t = 0; t < n; t++) {
            standardized[t][j] =
                (x[t][j]-model.feature_mean[j])/model.feature_scale[j];
            norm[j] += standardized[t][j]*standardized[t][j]/n;
        }
    }
    std::vector<double> residual(n);
    for (int t = 0; t < n; t++) residual[t] = y[t]-model.intercept;
    for (int iter = 0; iter < iterations; iter++) {
        double largest_change = 0;
        for (int j = 0; j < p; j++) {
            if (norm[j] == 0) continue;
            double correlation = 0;
            for (int t = 0; t < n; t++)
                correlation += standardized[t][j] *
                               (residual[t]+standardized[t][j]*model.coefficients[j])/n;
            double next = soft_threshold(correlation,lambda)/norm[j];
            double change = next-model.coefficients[j];
            for (int t = 0; t < n; t++) residual[t] -= standardized[t][j]*change;
            model.coefficients[j] = next;
            largest_change = std::max(largest_change,std::fabs(change));
        }
        if (largest_change < 1e-11) break;
    }
    return model;
}

inline double sigmoid(double x) {
    if (x >= 0) return 1/(1+std::exp(-x));
    double e = std::exp(x);
    return e/(1+e);
}

struct LogisticModel {
    RidgeModel linear;
};

inline LogisticModel fit_logistic(const Matrix& x, const std::vector<int>& labels,
                                 double lambda, int iterations = 4000) {
    require(!x.empty() && x.size() == labels.size(),
            "Logistic features and labels do not align");
    std::vector<double> numeric;
    for (int label : labels) {
        require(label == 0 || label == 1, "Logistic label must be binary");
        numeric.push_back(label);
    }
    check_feature_table(x, numeric);
    require(std::isfinite(lambda) && lambda >= 0 && iterations > 0,
            "Invalid logistic penalty or iteration count");
    int n = int(x.size()), p = int(x[0].size());
    LogisticModel result;
    result.linear.coefficients.assign(p,0);
    result.linear.feature_mean.assign(p,0);
    result.linear.feature_scale.assign(p,1);
    Matrix z(n,std::vector<double>(p));
    double target_rate = mean(numeric);
    result.linear.intercept = std::log(
        std::max(1e-6,target_rate)/std::max(1e-6,1-target_rate));
    for (int j = 0; j < p; j++) {
        for (int t = 0; t < n; t++) result.linear.feature_mean[j] += x[t][j]/n;
        double second = 0;
        for (int t = 0; t < n; t++)
            second += std::pow(x[t][j]-result.linear.feature_mean[j],2)/n;
        result.linear.feature_scale[j] = second > 0 ? std::sqrt(second):1;
        for (int t = 0; t < n; t++)
            z[t][j] = (x[t][j]-result.linear.feature_mean[j])/
                       result.linear.feature_scale[j];
    }
    double step = 1.0/(1.0+0.25*p+lambda);
    for (int iter = 0; iter < iterations; iter++) {
        double ga = 0;
        std::vector<double> gb(p,0);
        for (int t = 0; t < n; t++) {
            double score = result.linear.intercept;
            for (int j = 0; j < p; j++)
                score += result.linear.coefficients[j]*z[t][j];
            double diff = sigmoid(score)-labels[t];
            ga += diff/n;
            for (int j = 0; j < p; j++) gb[j] += diff*z[t][j]/n;
        }
        double change = std::fabs(step*ga);
        result.linear.intercept -= step*ga;
        for (int j = 0; j < p; j++) {
            gb[j] += lambda*result.linear.coefficients[j];
            change = std::max(change,std::fabs(step*gb[j]));
            result.linear.coefficients[j] -= step*gb[j];
        }
        if (change < 1e-10) break;
    }
    return result;
}

inline double logistic_predict_probability(const LogisticModel& model,
                                           const std::vector<double>& features) {
    return sigmoid(ridge_predict(model.linear,features));
}

inline double logistic_log_loss(const LogisticModel& model,
                                const Matrix& features,
                                const std::vector<int>& labels) {
    require(!features.empty() && features.size() == labels.size(),
            "Invalid logistic loss sample");
    double loss = 0;
    for (int t = 0; t < int(features.size()); t++) {
        require(labels[t] == 0 || labels[t] == 1,"Invalid logistic label");
        double p = logistic_predict_probability(model,features[t]);
        p = std::max(1e-15,std::min(1-1e-15,p));
        loss -= labels[t]*std::log(p)+(1-labels[t])*std::log1p(-p);
    }
    return loss/features.size();
}

} // namespace kline
