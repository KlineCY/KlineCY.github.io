#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline double best_quote_order_flow(double bid_old,double bid_new,
    double bid_size_old,double bid_size_new,double ask_old,double ask_new,
    double ask_size_old,double ask_size_new){
    require(std::isfinite(bid_old)&&std::isfinite(bid_new)&&
            std::isfinite(ask_old)&&std::isfinite(ask_new)&&
            bid_old>0&&bid_new>0&&ask_old>0&&ask_new>0&&
            bid_old<ask_old&&bid_new<ask_new&&
            std::isfinite(bid_size_old)&&std::isfinite(bid_size_new)&&
            std::isfinite(ask_size_old)&&std::isfinite(ask_size_new)&&
            bid_size_old>=0&&bid_size_new>=0&&
            ask_size_old>=0&&ask_size_new>=0,
            "Invalid quote OFI inputs");
    double bid=(bid_new>=bid_old?bid_size_new:0)-
               (bid_new<=bid_old?bid_size_old:0);
    double ask=(ask_new<=ask_old?ask_size_new:0)-
               (ask_new>=ask_old?ask_size_old:0);
    return bid-ask;
}
inline double weighted_order_flow(
    const std::vector<double>& signed_sizes,
    const std::vector<double>& time_weights){
    check_finite(signed_sizes);check_finite(time_weights);
    require(signed_sizes.size()==time_weights.size(),
            "OFI time weights mismatch");
    double total=0;
    for(int i=0;i<int(signed_sizes.size());i++){
        require(time_weights[i]>=0,"Negative OFI weight");
        total+=signed_sizes[i]*time_weights[i];
    }
    return total;
}
} // namespace kline
