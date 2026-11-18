#pragma once
#include "orderbook_replay.hpp"
#include <algorithm>
#include <cmath>
namespace kline {
struct MarketSweep{double filled=0,remaining=0,vwap=0,notional=0;};
inline MarketSweep sweep_visible_orderbook(
    const BookReplay& book,bool buy,double desired_quantity){
    require(std::isfinite(desired_quantity)&&desired_quantity>=0,
            "Invalid market order quantity");
    validate_uncrossed_book(book);
    MarketSweep result;result.remaining=desired_quantity;
    if(buy){
        for(const auto& [price,size]:book.asks){
            double filled=std::min(size,result.remaining);
            result.notional+=filled*price;result.filled+=filled;
            result.remaining-=filled;
            if(result.remaining<=1e-12)break;
        }
    }else{
        for(auto it=book.bids.rbegin();it!=book.bids.rend();++it){
            double filled=std::min(it->second,result.remaining);
            result.notional+=filled*it->first;result.filled+=filled;
            result.remaining-=filled;
            if(result.remaining<=1e-12)break;
        }
    }
    if(result.filled>0)result.vwap=result.notional/result.filled;
    return result;
}
} // namespace kline
