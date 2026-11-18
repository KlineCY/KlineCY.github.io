#pragma once
#include "stats.hpp"
#include <map>
#include <cmath>
namespace kline {
struct BookReplay{
    std::map<double,double> bids,asks;
};
inline void apply_orderbook_level(BookReplay& book,bool is_bid,
             double price,double new_available){
    require(std::isfinite(price)&&price>0&&
            std::isfinite(new_available)&&new_available>=0,
            "Invalid order book level update");
    auto& side=is_bid?book.bids:book.asks;
    if(new_available==0)side.erase(price);
    else side[price]=new_available;
}
inline double book_best_bid(const BookReplay& book){
    require(!book.bids.empty(),"Bid side empty");
    return book.bids.rbegin()->first;
}
inline double book_best_ask(const BookReplay& book){
    require(!book.asks.empty(),"Ask side empty");
    return book.asks.begin()->first;
}
inline void validate_uncrossed_book(const BookReplay& book){
    if(!book.bids.empty()&&!book.asks.empty())
        require(book_best_bid(book)<book_best_ask(book),
                "Book quotes are locked or crossed");
}
inline double total_visible_book_notional(const BookReplay& book){
    double notional=0;
    for(const auto& [price,size]:book.bids)notional+=price*size;
    for(const auto& [price,size]:book.asks)notional+=price*size;
    return notional;
}
} // namespace kline
