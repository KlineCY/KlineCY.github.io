#pragma once
#include "stats.hpp"
#include <cmath>
namespace kline {
struct ReservationQuotes{
    double reservation=0,bid=0,ask=0,spread=0;
};
inline ReservationQuotes avellaneda_stoikov_quotes(
    double midpoint,double inventory,double risk_aversion,
    double volatility,double remaining_time,double order_flow_decay){
    require(std::isfinite(midpoint)&&midpoint>0&&
            std::isfinite(inventory)&&std::isfinite(risk_aversion)&&
            risk_aversion>0&&std::isfinite(volatility)&&volatility>=0&&
            std::isfinite(remaining_time)&&remaining_time>=0&&
            std::isfinite(order_flow_decay)&&order_flow_decay>0,
            "Invalid market maker quote parameters");
    double risk_term=risk_aversion*volatility*volatility*remaining_time;
    double reservation=midpoint-inventory*risk_term;
    double spread=risk_term+
        2/risk_aversion*std::log1p(risk_aversion/order_flow_decay);
    return {reservation,reservation-spread/2,
            reservation+spread/2,spread};
}
} // namespace kline
