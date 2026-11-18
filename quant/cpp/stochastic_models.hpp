#pragma once
#include "portfolio.hpp"
#include "numerical.hpp"
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

namespace kline {

inline Matrix cholesky_psd(const Matrix& covariance, double tolerance=1e-12) {
    check_covariance(covariance);
    require(std::isfinite(tolerance) && tolerance > 0,
            "Invalid Cholesky tolerance");
    int n=int(covariance.size());
    double scale=0;
    for (int i=0;i<n;i++) scale=std::max(scale,covariance[i][i]);
    double threshold=std::max(1e-30,scale*tolerance);
    Matrix lower(n,std::vector<double>(n,0));
    for (int i=0;i<n;i++) {
        double pivot=covariance[i][i];
        for (int j=0;j<i;j++) pivot-=lower[i][j]*lower[i][j];
        require(pivot>=-threshold,"Indefinite covariance matrix");
        lower[i][i]=std::sqrt(std::max(0.0,pivot));
        for (int j=i+1;j<n;j++) {
            double remaining=covariance[j][i];
            for (int k=0;k<i;k++)
                remaining-=lower[j][k]*lower[i][k];
            if (lower[i][i] > std::sqrt(threshold))
                lower[j][i]=remaining/lower[i][i];
            else require(std::fabs(remaining)<=threshold,
                         "Degenerate covariance has inconsistent cross terms");
        }
    }
    return lower;
}

inline Matrix gaussian_correlated_paths(
    const std::vector<double>& starts,
    const std::vector<double>& annual_drifts,
    const Matrix& annual_covariance,
    int steps,double dt,unsigned int seed) {
    check_finite(starts);
    check_finite(annual_drifts);
    require(starts.size()==annual_drifts.size() &&
            starts.size()==annual_covariance.size() && steps>0 &&
            std::isfinite(dt) && dt>0,
            "Invalid multivariate simulation inputs");
    for (double v : starts) require(v>0,"Initial prices must be positive");
    Matrix lower=cholesky_psd(annual_covariance);
    int n=int(starts.size());
    std::mt19937 rng(seed);
    std::normal_distribution<double> normal(0,1);
    Matrix paths(steps+1,std::vector<double>(n));
    paths[0]=starts;
    for (int t=1;t<=steps;t++) {
        std::vector<double> z(n);
        for (double& v : z) v=normal(rng);
        for (int i=0;i<n;i++) {
            double shock=0;
            for (int j=0;j<=i;j++) shock+=lower[i][j]*z[j];
            double previous=paths[t-1][i];
            paths[t][i]=previous*std::exp(
                (annual_drifts[i]-0.5*annual_covariance[i][i])*dt+
                shock*std::sqrt(dt));
            require(std::isfinite(paths[t][i]) && paths[t][i]>0,
                    "Stochastic simulation underflow or overflow");
        }
    }
    return paths;
}

inline double exact_ou_step(double current,double mean_level,
                            double mean_reversion,double diffusion,
                            double dt,double normal_shock) {
    require(std::isfinite(current) && std::isfinite(mean_level) &&
            std::isfinite(mean_reversion) && mean_reversion >= 0 &&
            std::isfinite(diffusion) && diffusion >= 0 &&
            std::isfinite(dt) && dt>0 && std::isfinite(normal_shock),
            "Invalid OU process parameters");
    if (mean_reversion==0)
        return current+diffusion*std::sqrt(dt)*normal_shock;
    double decay=std::exp(-mean_reversion*dt);
    double variance=diffusion*diffusion*
                    (-std::expm1(-2*mean_reversion*dt))/(2*mean_reversion);
    return mean_level+(current-mean_level)*decay+
           std::sqrt(variance)*normal_shock;
}

} // namespace kline
