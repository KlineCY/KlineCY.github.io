#pragma once
#include "portfolio.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace kline {

inline std::vector<double> solve_linear_system(Matrix a, std::vector<double> b) {
    int n = int(a.size());
    require(n > 0 && int(b.size()) == n, "Linear system dimension mismatch");
    check_finite(b);
    for (const auto& row : a) {
        require(int(row.size()) == n, "Linear system matrix must be square");
        check_finite(row);
    }
    for (int k = 0; k < n; k++) {
        int pivot = k;
        for (int i = k + 1; i < n; i++)
            if (std::fabs(a[i][k]) > std::fabs(a[pivot][k])) pivot = i;
        double scale = 0;
        for (int j = k; j < n; j++) scale = std::max(scale, std::fabs(a[pivot][j]));
        require(scale > 0 && std::fabs(a[pivot][k]) > 1e-12 * scale,
                "Linear system is singular or numerically unstable");
        std::swap(a[pivot], a[k]);
        std::swap(b[pivot], b[k]);
        for (int i = k + 1; i < n; i++) {
            double factor = a[i][k] / a[k][k];
            a[i][k] = 0;
            for (int j = k + 1; j < n; j++) a[i][j] -= factor * a[k][j];
            b[i] -= factor * b[k];
        }
    }
    std::vector<double> x(n, 0);
    for (int i = n - 1; i >= 0; i--) {
        double sum = b[i];
        for (int j = i + 1; j < n; j++) sum -= a[i][j] * x[j];
        x[i] = sum / a[i][i];
        require(std::isfinite(x[i]), "Linear system produced nonfinite solution");
    }
    return x;
}

inline void check_feature_table(const Matrix& x, const std::vector<double>& y) {
    require(!x.empty() && x.size() == y.size(),
            "Feature rows and labels must have the same nonzero length");
    check_finite(y);
    int columns = int(x.front().size());
    require(columns > 0, "At least one predictor is required");
    for (const auto& row : x) {
        require(int(row.size()) == columns, "Features must form a rectangular table");
        check_finite(row);
    }
}

struct RidgeModel {
    double intercept = 0;
    std::vector<double> coefficients;
    std::vector<double> feature_mean;
    std::vector<double> feature_scale;
};

inline RidgeModel fit_ridge(const Matrix& x, const std::vector<double>& y,
                            double lambda) {
    check_feature_table(x, y);
    require(x.size() >= 2 && std::isfinite(lambda) && lambda >= 0,
            "Ridge needs two observations and nonnegative penalty");
    int n = int(x.size()), p = int(x.front().size());
    RidgeModel model;
    model.intercept = mean(y);
    model.coefficients.assign(p, 0);
    model.feature_mean.resize(p);
    model.feature_scale.resize(p);
    for (int j = 0; j < p; j++) {
        double sum = 0;
        for (const auto& row : x) sum += row[j];
        double center = sum / n, square = 0;
        for (const auto& row : x) square += (row[j] - center) * (row[j] - center);
        model.feature_mean[j] = center;
        model.feature_scale[j] = square > 0 ? std::sqrt(square / n) : 1.0;
    }

    Matrix gram(p, std::vector<double>(p, 0));
    std::vector<double> right(p, 0);
    for (int i = 0; i < n; i++) {
        std::vector<double> standardized(p);
        for (int j = 0; j < p; j++)
            standardized[j] = (x[i][j] - model.feature_mean[j]) / model.feature_scale[j];
        for (int j = 0; j < p; j++) {
            right[j] += standardized[j] * (y[i] - model.intercept) / n;
            for (int k = 0; k < p; k++)
                gram[j][k] += standardized[j] * standardized[k] / n;
        }
    }
    for (int j = 0; j < p; j++) gram[j][j] += lambda;
    model.coefficients = solve_linear_system(gram, right);
    return model;
}

inline double ridge_predict(const RidgeModel& model, const std::vector<double>& row) {
    require(!row.empty() && row.size() == model.coefficients.size() &&
            row.size() == model.feature_mean.size() &&
            row.size() == model.feature_scale.size(), "Predictor dimensions do not match");
    check_finite(row);
    double value = model.intercept;
    for (int j = 0; j < int(row.size()); j++)
        value += model.coefficients[j] *
                 (row[j] - model.feature_mean[j]) / model.feature_scale[j];
    require(std::isfinite(value), "Prediction is nonfinite");
    return value;
}

inline double ridge_training_mse(const RidgeModel& model, const Matrix& x,
                                 const std::vector<double>& y) {
    check_feature_table(x, y);
    double error = 0;
    for (int i = 0; i < int(x.size()); i++) {
        double difference = ridge_predict(model, x[i]) - y[i];
        error += difference * difference;
    }
    return error / x.size();
}

struct ForwardPrediction {
    int index = 0;
    int fitted_at = 0;
    int training_rows = 0;
    double prediction = 0;
};

inline std::vector<ForwardPrediction> forward_ridge(
    const Matrix& features, const std::vector<double>& labels,
    const std::vector<int>& label_available_at, int initial_train,
    int retrain_every, int training_window, int embargo, double lambda,
    bool expanding = false) {

    check_feature_table(features, labels);
    int n = int(features.size());
    require(int(label_available_at.size()) == n &&
            initial_train >= 2 && initial_train < n && retrain_every > 0 &&
            training_window >= 2 && embargo >= 0 && embargo < initial_train,
            "Invalid forward validation configuration");
    for (int i = 0; i < n; i++)
        require(label_available_at[i] > i && label_available_at[i] <= n + 1000000,
                "Label maturity must follow its feature date");

    std::vector<ForwardPrediction> result;
    for (int decision = initial_train; decision < n; decision += retrain_every) {
        int cutoff = decision - embargo;
        int begin = expanding ? 0 : std::max(0, cutoff - training_window);
        Matrix train_x;
        std::vector<double> train_y;
        for (int i = begin; i < cutoff; i++) {
            if (label_available_at[i] > cutoff) continue;
            train_x.push_back(features[i]);
            train_y.push_back(labels[i]);
        }
        require(train_y.size() >= 2, "Fewer than two mature training labels");
        auto model = fit_ridge(train_x, train_y, lambda);
        for (int i = decision; i < n && i < decision + retrain_every; i++)
            result.push_back({i, decision, int(train_y.size()),
                              ridge_predict(model, features[i])});
    }
    return result;
}

struct ShrunkCovariance {
    Matrix covariance;
    double intensity = 0;
};

inline ShrunkCovariance ledoit_wolf_identity(
    const std::vector<std::vector<double>>& asset_returns) {

    require(!asset_returns.empty(), "At least one asset is required");
    int p = int(asset_returns.size());
    int n = int(asset_returns.front().size());
    require(n >= 2, "Covariance shrinkage needs at least two dates");
    std::vector<double> centers(p);
    for (int i = 0; i < p; i++) {
        require(int(asset_returns[i].size()) == n, "Asset dates must align");
        centers[i] = mean(asset_returns[i]);
    }
    Matrix centered(p, std::vector<double>(n));
    for (int i = 0; i < p; i++)
        for (int t = 0; t < n; t++)
            centered[i][t] = asset_returns[i][t] - centers[i];

    Matrix sample(p, std::vector<double>(p, 0));
    for (int i = 0; i < p; i++)
        for (int j = 0; j <= i; j++) {
            double value = 0;
            for (int t = 0; t < n; t++)
                value += centered[i][t] * centered[j][t] / n;
            sample[i][j] = value;
            sample[j][i] = value;
        }
    double average_variance = 0;
    for (int i = 0; i < p; i++) average_variance += sample[i][i] / p;
    double target_distance = 0, sampling_variance = 0;
    for (int i = 0; i < p; i++)
        for (int j = 0; j < p; j++) {
            double target = (i == j ? average_variance : 0);
            double difference = sample[i][j] - target;
            target_distance += difference * difference;
            for (int t = 0; t < n; t++) {
                double residual = centered[i][t] * centered[j][t] - sample[i][j];
                sampling_variance += residual * residual / (double(n) * n);
            }
        }
    require(std::isfinite(target_distance) && std::isfinite(sampling_variance),
            "Covariance magnitude exceeds floating-point range");
    double intensity = target_distance > 0
        ? std::min(1.0, sampling_variance / target_distance) : 1.0;
    Matrix result(p, std::vector<double>(p));
    for (int i = 0; i < p; i++)
        for (int j = 0; j < p; j++)
            result[i][j] = (1 - intensity) * sample[i][j] +
                           (i == j ? intensity * average_variance : 0);
    check_covariance(result);
    return {result, intensity};
}

} // namespace kline
