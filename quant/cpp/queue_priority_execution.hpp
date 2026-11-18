#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
struct QueuePriorityFill{
    double queue_ahead=0,own_remaining=0,own_filled=0;
};
inline QueuePriorityFill consume_fifo_queue(
    QueuePriorityFill state,double marketable_shares){
    require(std::isfinite(state.queue_ahead)&&state.queue_ahead>=0&&
            std::isfinite(state.own_remaining)&&state.own_remaining>=0&&
            std::isfinite(state.own_filled)&&state.own_filled>=0&&
            std::isfinite(marketable_shares)&&marketable_shares>=0,
            "Invalid FIFO queue state");
    double consumed_ahead=std::min(state.queue_ahead,marketable_shares);
    state.queue_ahead-=consumed_ahead;marketable_shares-=consumed_ahead;
    double self=std::min(state.own_remaining,marketable_shares);
    state.own_filled+=self;state.own_remaining-=self;
    return state;
}
} // namespace kline
