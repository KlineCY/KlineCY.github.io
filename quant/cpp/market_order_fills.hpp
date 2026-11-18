#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct DepthLevel{double price=0,available=0;};
struct MarketFill{double filled=0,remaining=0,average_price=0,notional=0;};
inline MarketFill sweep_market_order(
    const std::vector<DepthLevel>& opposite_side,double quantity,
    bool buying){
    require(std::isfinite(quantity)&&quantity>0&&
            !opposite_side.empty(),"Invalid market order quantity or depth");
    for(int j=0;j<int(opposite_side.size());j++){
        auto level=opposite_side[j];
        require(std::isfinite(level.price)&&level.price>0&&
                std::isfinite(level.available)&&level.available>=0,
                "Invalid level price or quantity");
        if(j>0)
            require(buying?level.price>=opposite_side[j-1].price:
                   level.price<=opposite_side[j-1].price,
                   "Liquidity levels are not sorted by execution priority");
    }
    MarketFill result;
    result.remaining=quantity;
    for(const auto& level:opposite_side){
        double executed=std::min(result.remaining,level.available);
        result.filled+=executed;
        result.notional+=executed*level.price;
        result.remaining-=executed;
        if(result.remaining<=1e-10)break;
    }
    if(result.filled>0)result.average_price=result.notional/result.filled;
    return result;
}
inline double market_order_slippage(
    const MarketFill& fill,double arrival_mid,bool buying){
    require(std::isfinite(arrival_mid)&&arrival_mid>0&&fill.filled>0,
            "Invalid slippage reference");
    return (buying?1.0:-1.0)*(fill.average_price/arrival_mid-1);
}
} // namespace kline
