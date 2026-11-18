#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <map>
#include <vector>
namespace kline {
struct LimitBook{
    std::map<double,double,std::greater<double>> bids;
    std::map<double,double> asks;
};
inline void update_limit_level(LimitBook& book,bool is_bid,
    double price,double quantity){
    require(std::isfinite(price)&&price>0&&std::isfinite(quantity)&&quantity>=0,
            "Invalid limit level");
    if(is_bid){
        if(quantity==0)book.bids.erase(price);else book.bids[price]=quantity;
    }else{
        if(quantity==0)book.asks.erase(price);else book.asks[price]=quantity;
    }
    if(!book.bids.empty()&&!book.asks.empty())
        require(book.bids.begin()->first<book.asks.begin()->first,
                "Crossed limit book");
}
inline double book_mid_price(const LimitBook& book){
    require(!book.bids.empty()&&!book.asks.empty(),"Two-sided quote required");
    return 0.5*(book.bids.begin()->first+book.asks.begin()->first);
}
inline double book_microprice(const LimitBook& book){
    require(!book.bids.empty()&&!book.asks.empty(),"Two-sided quote required");
    double bid=book.bids.begin()->first,ask=book.asks.begin()->first;
    double bq=book.bids.begin()->second,aq=book.asks.begin()->second;
    require(bq+aq>0,"No quoted volume");
    return (bid*aq+ask*bq)/(bq+aq);
}
inline double best_level_imbalance(const LimitBook& book){
    require(!book.bids.empty()&&!book.asks.empty(),"Two-sided quote required");
    double b=book.bids.begin()->second,a=book.asks.begin()->second;
    require(b+a>0,"No best level volume");
    return (b-a)/(b+a);
}
} // namespace kline
