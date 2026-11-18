#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
inline double level_one_microprice(
    double best_bid,double best_ask,double bid_quantity,double ask_quantity){
    require(std::isfinite(best_bid)&&std::isfinite(best_ask)&&
            best_bid>0&&best_ask>best_bid&&
            std::isfinite(bid_quantity)&&bid_quantity>=0&&
            std::isfinite(ask_quantity)&&ask_quantity>=0&&
            bid_quantity+ask_quantity>0,"Invalid level one quotes");
    return (best_ask*bid_quantity+best_bid*ask_quantity)/
           (bid_quantity+ask_quantity);
}
inline double depth_imbalance(double bid_quantity,double ask_quantity){
    require(std::isfinite(bid_quantity)&&bid_quantity>=0&&
            std::isfinite(ask_quantity)&&ask_quantity>=0&&
            bid_quantity+ask_quantity>0,"Invalid market depth");
    return (bid_quantity-ask_quantity)/(bid_quantity+ask_quantity);
}
} // namespace kline
