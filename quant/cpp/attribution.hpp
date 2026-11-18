#pragma once
#include "stats.hpp"
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace kline {

struct BrinsonAttribution {
    std::vector<double> allocation_by_sector;
    std::vector<double> selection_by_sector;
    std::vector<double> interaction_by_sector;
    double active_return = 0;
    double explained_active_return = 0;
};

inline BrinsonAttribution brinson_fachler_single_period(
    const std::vector<double>& portfolio_sector_weight,
    const std::vector<double>& benchmark_sector_weight,
    const std::vector<double>& portfolio_sector_return,
    const std::vector<double>& benchmark_sector_return) {
    check_finite(portfolio_sector_weight);
    check_finite(benchmark_sector_weight);
    check_finite(portfolio_sector_return);
    check_finite(benchmark_sector_return);
    int n = int(portfolio_sector_weight.size());
    require(int(benchmark_sector_weight.size()) == n &&
            int(portfolio_sector_return.size()) == n &&
            int(benchmark_sector_return.size()) == n,
            "Brinson sector vectors must align");
    double sum_p = 0, sum_b = 0, total_portfolio = 0, total_benchmark = 0;
    for (int i = 0; i < n; i++) {
        double wp = portfolio_sector_weight[i], wb = benchmark_sector_weight[i];
        require(wp >= 0 && wb >= 0, "Long-only sector weights must be nonnegative");
        sum_p += wp;
        sum_b += wb;
        total_portfolio += wp * portfolio_sector_return[i];
        total_benchmark += wb * benchmark_sector_return[i];
    }
    require(std::fabs(sum_p - 1) < 1e-9 && std::fabs(sum_b - 1) < 1e-9,
            "Sector weights must sum to one");
    BrinsonAttribution result;
    for (int i = 0; i < n; i++) {
        double wp = portfolio_sector_weight[i], wb = benchmark_sector_weight[i];
        double rp = portfolio_sector_return[i], rb = benchmark_sector_return[i];
        double allocation = (wp - wb) * (rb - total_benchmark);
        double selection = wb * (rp - rb);
        double interaction = (wp - wb) * (rp - rb);
        result.allocation_by_sector.push_back(allocation);
        result.selection_by_sector.push_back(selection);
        result.interaction_by_sector.push_back(interaction);
        result.explained_active_return += allocation + selection + interaction;
    }
    result.active_return = total_portfolio - total_benchmark;
    return result;
}

struct LinearFactorAttribution {
    std::vector<double> contribution_by_factor;
    double systematic_return = 0;
    double idiosyncratic_return = 0;
};

inline LinearFactorAttribution factor_return_attribution(
    const std::vector<double>& betas, const std::vector<double>& realized_factor_returns,
    double realized_asset_return) {
    check_finite(betas);
    check_finite(realized_factor_returns);
    require(betas.size() == realized_factor_returns.size() &&
            std::isfinite(realized_asset_return), "Factor attribution inputs must match");
    LinearFactorAttribution result;
    for (int i = 0; i < int(betas.size()); i++) {
        double contribution = betas[i] * realized_factor_returns[i];
        result.contribution_by_factor.push_back(contribution);
        result.systematic_return += contribution;
    }
    result.idiosyncratic_return = realized_asset_return - result.systematic_return;
    return result;
}

inline std::vector<double> asset_return_contributions(
    const std::vector<double>& beginning_weights,
    const std::vector<double>& asset_returns) {
    check_finite(beginning_weights);
    check_finite(asset_returns);
    require(beginning_weights.size() == asset_returns.size(),
            "Asset contribution vectors must match");
    std::vector<double> contributions(beginning_weights.size());
    for (int i = 0; i < int(beginning_weights.size()); i++)
        contributions[i] = beginning_weights[i] * asset_returns[i];
    return contributions;
}

inline double single_period_active_return(
    const std::vector<double>& active_contributions) {
    return std::accumulate(active_contributions.begin(),
                           active_contributions.end(), 0.0);
}

// Linking Brinson effects across periods requires geometric linking or an
// explicitly chosen multi-period methodology; this routine is one-period only.

}  // namespace kline
