#pragma once
#include "trade_fill_reconciliation.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct ShortfallBreakdown{
    double arrival_cost=0,execution_cost=0,opportunity_cost=0,
           commissions=0,total=0;
};
inline ShortfallBreakdown implementation_shortfall(
    double intended_signed_shares,double arrival_price,
    double last_price,const std::vector<Fill>& actual_fills){
    require(std::isfinite(intended_signed_shares)&&
            std::isfinite(arrival_price)&&arrival_price>0&&
            std::isfinite(last_price)&&last_price>0,
            "Invalid implementation shortfall benchmark");
    double direction=intended_signed_shares>=0?1:-1;
    double ordered=std::fabs(intended_signed_shares),executed=0;
    ShortfallBreakdown result;
    for(const auto& fill:actual_fills){
        require(std::isfinite(fill.signed_shares)&&
                std::isfinite(fill.price)&&fill.price>0&&
                std::isfinite(fill.commission)&&fill.commission>=0&&
                direction*fill.signed_shares>=0,
                "Implementation shortfall execution side mismatch");
        executed+=std::fabs(fill.signed_shares);
        result.execution_cost+=direction*fill.signed_shares*
                               direction*(fill.price-arrival_price);
        result.commissions+=fill.commission;
    }
    require(executed<=ordered+1e-10,"Fills exceed intended order quantity");
    result.opportunity_cost=(ordered-executed)*direction*
                            (last_price-arrival_price);
    result.total=result.execution_cost+result.opportunity_cost+
                 result.commissions;
    return result;
}
} // namespace kline
