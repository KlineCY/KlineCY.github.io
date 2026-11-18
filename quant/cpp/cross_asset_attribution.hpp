#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct AllocationAttribution{
    double market=0,selection=0,trading=0,fees=0,total=0;
};
inline AllocationAttribution decompose_single_period(
    const std::vector<double>& initial_weights,
    const std::vector<double>& final_weights,
    const std::vector<double>& asset_returns,
    double transaction_fee_fraction){
    check_finite(initial_weights);check_finite(final_weights);
    check_finite(asset_returns);
    require(initial_weights.size()==final_weights.size()&&
            initial_weights.size()==asset_returns.size()&&
            std::isfinite(transaction_fee_fraction)&&transaction_fee_fraction>=0,
            "Invalid single period attribution");
    AllocationAttribution result;
    for(int i=0;i<int(asset_returns.size());i++){
        result.market+=initial_weights[i]*asset_returns[i];
        result.trading+=(final_weights[i]-initial_weights[i])*asset_returns[i];
        result.fees+=std::fabs(final_weights[i]-initial_weights[i])*
                     transaction_fee_fraction;
    }
    result.total=result.market+result.trading-result.fees;
    return result;
}
inline double linear_factor_attribution(const std::vector<double>& exposures,
                                        const std::vector<double>& factor_returns,
                                        double specific){
    check_finite(exposures);check_finite(factor_returns);
    require(exposures.size()==factor_returns.size()&&std::isfinite(specific),
            "Factor attribution shape");
    return std::inner_product(exposures.begin(),exposures.end(),
                              factor_returns.begin(),specific);
}
} // namespace kline
