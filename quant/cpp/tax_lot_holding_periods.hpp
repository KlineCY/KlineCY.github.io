#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
struct TaxLot{int acquired_day=0;double shares=0,cost_per_share=0;};
inline double unrealized_tax_lot_profit(
    const std::vector<TaxLot>& lots,double current_price){
    require(std::isfinite(current_price)&&current_price>=0,
            "Invalid tax lot market price");
    double result=0;
    for(auto lot:lots){
        require(std::isfinite(lot.shares)&&lot.shares>=0&&
                std::isfinite(lot.cost_per_share)&&lot.cost_per_share>=0,
                "Invalid tax lot shares or basis");
        result+=lot.shares*(current_price-lot.cost_per_share);
    }
    return result;
}
inline double held_more_than_days(const std::vector<TaxLot>& lots,
                                  int current_day,int threshold_days){
    require(threshold_days>=0,"Negative holding threshold");
    double shares=0;
    for(auto lot:lots){
        require(lot.acquired_day<=current_day&&lot.shares>=0,
                "Invalid tax lot acquisition date");
        if(current_day-lot.acquired_day>threshold_days)shares+=lot.shares;
    }
    return shares;
}
} // namespace kline
