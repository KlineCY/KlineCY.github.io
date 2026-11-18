#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double quoted_relative_spread(double bid,double ask){
    require(std::isfinite(bid)&&bid>0&&std::isfinite(ask)&&ask>bid,
            "Invalid quoted spread");
    return 2*(ask-bid)/(ask+bid);
}
inline double effective_spread(double execution_price,double midpoint,
                                int trade_side){
    require(std::isfinite(execution_price)&&execution_price>0&&
            std::isfinite(midpoint)&&midpoint>0&&
            (trade_side==1||trade_side==-1),"Invalid effective spread");
    return 2*trade_side*(execution_price-midpoint)/midpoint;
}
inline double realized_spread(double trade_price,double future_midpoint,
                               int trade_side){
    require(std::isfinite(trade_price)&&trade_price>0&&
            std::isfinite(future_midpoint)&&future_midpoint>0&&
            (trade_side==1||trade_side==-1),"Invalid realized spread");
    return 2*trade_side*(trade_price-future_midpoint)/future_midpoint;
}
} // namespace kline
