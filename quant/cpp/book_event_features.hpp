#pragma once
#include "microprice_and_imbalance.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct BookSnapshot{
    double bid=0,ask=0,bid_size=0,ask_size=0;
};
inline double best_level_order_flow_imbalance(
    const BookSnapshot& previous,const BookSnapshot& current){
    require(previous.bid>0&&previous.ask>previous.bid&&
            current.bid>0&&current.ask>current.bid&&
            previous.bid_size>=0&&previous.ask_size>=0&&
            current.bid_size>=0&&current.ask_size>=0,
            "Invalid book event snapshots");
    double buy=(current.bid>=previous.bid?current.bid_size:0)-
               (current.bid<=previous.bid?previous.bid_size:0);
    double sell=(current.ask<=previous.ask?current.ask_size:0)-
                (current.ask>=previous.ask?previous.ask_size:0);
    return buy-sell;
}
inline double microprice_deviation(const BookSnapshot& snapshot){
    double micro=level_one_microprice(snapshot.bid,snapshot.ask,
                                      snapshot.bid_size,snapshot.ask_size);
    return micro-0.5*(snapshot.bid+snapshot.ask);
}
} // namespace kline
