#pragma once
#include "portfolio.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace kline {

struct EigenDecomposition {
    std::vector<double> values;
    Matrix vectors;
};

inline EigenDecomposition symmetric_eigen(const Matrix& matrix,
                                          int max_sweeps = 120,
                                          double tolerance = 1e-12) {
    check_covariance(matrix);
    require(max_sweeps > 0 && tolerance > 0 && std::isfinite(tolerance),
            "Invalid Jacobi eigen parameters");
    int n = int(matrix.size());
    Matrix a = matrix, vectors(n, std::vector<double>(n, 0));
    for (int i = 0; i < n; i++) vectors[i][i] = 1;
    double scale = 0;
    for (const auto& row : matrix)
        for (double v : row) scale = std::max(scale, std::fabs(v));
    for (int sweep = 0; sweep < max_sweeps && scale > 0; sweep++) {
        double off = 0;
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
                off = std::max(off, std::fabs(a[i][j]));
        if (off <= tolerance * scale) break;
        for (int p = 0; p < n; p++)
            for (int q = p + 1; q < n; q++) {
                if (std::fabs(a[p][q]) <= tolerance * scale) continue;
                double tau = (a[q][q] - a[p][p]) / (2 * a[p][q]);
                double t = (tau >= 0 ? 1.0 : -1.0) /
                           (std::fabs(tau) + std::sqrt(1 + tau * tau));
                double c = 1 / std::sqrt(1 + t*t), s = t*c;
                double app = a[p][p], aqq = a[q][q], apq = a[p][q];
                a[p][p] = app - t*apq;
                a[q][q] = aqq + t*apq;
                a[p][q] = a[q][p] = 0;
                for (int k = 0; k < n; k++) {
                    if (k != p && k != q) {
                        double akp = a[k][p], akq = a[k][q];
                        a[k][p] = a[p][k] = c*akp - s*akq;
                        a[k][q] = a[q][k] = s*akp + c*akq;
                    }
                    double vp = vectors[k][p], vq = vectors[k][q];
                    vectors[k][p] = c*vp - s*vq;
                    vectors[k][q] = s*vp + c*vq;
                }
            }
    }
    double off = 0;
    for (int p = 0; p < n; p++)
        for (int q = p + 1; q < n; q++)
            off = std::max(off, std::fabs(a[p][q]));
    require(off <= 10*tolerance*std::max(scale, 1e-30),
            "Jacobi eigensolver did not converge");
    std::vector<int> order(n);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(),
              [&](int x, int y) { return a[x][x] > a[y][y]; });
    EigenDecomposition result;
    result.values.resize(n);
    result.vectors.assign(n, std::vector<double>(n));
    for (int j = 0; j < n; j++) {
        result.values[j] = a[order[j]][order[j]];
        for (int i = 0; i < n; i++)
            result.vectors[i][j] = vectors[i][order[j]];
        int pivot = 0;
        for (int i = 1; i < n; i++)
            if (std::fabs(result.vectors[i][j]) >
                std::fabs(result.vectors[pivot][j])) pivot = i;
        if (result.vectors[pivot][j] < 0)
            for (int i = 0; i < n; i++)
                result.vectors[i][j] = -result.vectors[i][j];
    }
    return result;
}

struct PCAResult {
    std::vector<double> means;
    std::vector<double> eigenvalues;
    Matrix loadings;
    Matrix scores;
};

inline PCAResult principal_components(const Matrix& observations,
                                       int components) {
    require(observations.size() >= 2, "PCA needs at least two observations");
    int n = int(observations.size()), p = int(observations.front().size());
    require(p > 0 && components > 0 && components <= p,
            "Invalid PCA component count");
    for (const auto& row : observations) {
        require(int(row.size()) == p, "PCA input must be rectangular");
        check_finite(row);
    }
    PCAResult result;
    result.means.assign(p, 0);
    for (const auto& row : observations)
        for (int j = 0; j < p; j++) result.means[j] += row[j]/n;
    Matrix centered = observations;
    for (auto& row : centered)
        for (int j = 0; j < p; j++) row[j] -= result.means[j];
    Matrix covariance(p, std::vector<double>(p, 0));
    for (int i = 0; i < p; i++)
        for (int j = 0; j <= i; j++) {
            double value = 0;
            for (const auto& row : centered) value += row[i]*row[j];
            covariance[i][j] = covariance[j][i] = value/(n-1);
        }
    auto eigen = symmetric_eigen(covariance);
    result.eigenvalues = eigen.values;
    result.loadings.assign(p, std::vector<double>(components));
    result.scores.assign(n, std::vector<double>(components, 0));
    for (int i = 0; i < p; i++)
        for (int k = 0; k < components; k++)
            result.loadings[i][k] = eigen.vectors[i][k];
    for (int t = 0; t < n; t++)
        for (int k = 0; k < components; k++)
            for (int j = 0; j < p; j++)
                result.scores[t][k] += centered[t][j]*result.loadings[j][k];
    return result;
}

inline double condition_number_symmetric(const Matrix& matrix,
                                          double tolerance = 1e-12) {
    auto eigen = symmetric_eigen(matrix);
    double highest = 0, lowest = std::numeric_limits<double>::infinity();
    for (double v : eigen.values) {
        double magnitude = std::fabs(v);
        highest = std::max(highest, magnitude);
        lowest = std::min(lowest, magnitude);
    }
    return highest == 0 || lowest <= tolerance * highest
        ? std::numeric_limits<double>::infinity() : highest / lowest;
}

} // namespace kline
