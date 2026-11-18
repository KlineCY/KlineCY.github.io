#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline std::vector<double> back_adjust_additive_futures(
    const std::vector<double>& raw_settlements,
    const std::vector<double>& old_contract_on_roll,
    const std::vector<double>& new_contract_on_roll,
    const std::vector<int>& roll_indices){
    check_finite(raw_settlements);
    require(old_contract_on_roll.size()==new_contract_on_roll.size()&&
            old_contract_on_roll.size()==roll_indices.size(),
            "Futures roll series dimensions");
    check_finite(old_contract_on_roll);check_finite(new_contract_on_roll);
    std::vector<double> adjusted=raw_settlements;
    double cumulative=0;
    for(int k=int(roll_indices.size())-1;k>=0;k--){
        int boundary=roll_indices[k];
        require(boundary>0&&boundary<int(raw_settlements.size())&&
                (k==0||roll_indices[k-1]<boundary),
                "Roll boundaries must be strictly ascending");
        cumulative+=new_contract_on_roll[k]-old_contract_on_roll[k];
        for(int t=0;t<boundary;t++)adjusted[t]+=new_contract_on_roll[k]-
                                                  old_contract_on_roll[k];
    }
    return adjusted;
}
inline std::vector<double> held_contract_futures_returns(
    const std::vector<double>& held_old,
    const std::vector<double>& held_next,
    const std::vector<double>& reference_notional){
    check_finite(held_old);check_finite(held_next);
    check_finite(reference_notional);
    require(held_old.size()==held_next.size()&&
            held_old.size()==reference_notional.size(),
            "Futures held return dimensions");
    std::vector<double> results;
    for(int i=0;i<int(held_old.size());i++){
        require(reference_notional[i]>0,"Futures return denominator");
        results.push_back((held_next[i]-held_old[i])/reference_notional[i]);
    }
    return results;
}
} // namespace kline
