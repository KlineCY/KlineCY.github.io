#pragma once
#include "execution.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace kline {

struct BookLevel {
    double price = 0;
    double quantity = 0;
};

struct BookExecution {
    double requested = 0;
    double executed = 0;
    double unfilled = 0;
    double notional = 0;
    double average_price = 0;
};

inline BookExecution walk_order_book(
    const std::vector<BookLevel>& levels, double requested,
    bool buy=true) {
    require(!levels.empty() && std::isfinite(requested) && requested > 0,
            "Invalid market order size");
    double remaining=requested,notional=0,previous=0;
    for (int i = 0; i < int(levels.size()); i++) {
        const auto& level=levels[i];
        require(std::isfinite(level.price) && level.price > 0 &&
                std::isfinite(level.quantity) && level.quantity >= 0,
                "Invalid order-book level");
        if (i > 0) require(buy ? level.price >= previous :
                                   level.price <= previous,
                           "Levels must be ordered from best to worst");
        previous=level.price;
        double filled=std::min(remaining,level.quantity);
        notional+=filled*level.price;
        remaining-=filled;
    }
    BookExecution result;
    result.requested=requested;
    result.executed=requested-remaining;
    result.unfilled=remaining;
    result.notional=notional;
    result.average_price=result.executed > 0 ?
                         notional/result.executed : 0;
    return result;
}

inline std::vector<double> twap_schedule(double order_quantity, int slices) {
    require(std::isfinite(order_quantity) && order_quantity >= 0 &&
            slices > 0 && slices <= 100000,
            "TWAP requires nonnegative quantity and positive slices");
    return std::vector<double>(slices,order_quantity/slices);
}

inline std::vector<double> capped_volume_schedule(
    const std::vector<double>& capacity, double order_quantity) {
    check_finite(capacity);
    require(std::isfinite(order_quantity) && order_quantity >= 0,
            "Invalid volume-constrained order");
    double available=0;
    for (double value : capacity) {
        require(value >= 0, "Negative slice capacity");
        available+=value;
    }
    require(available+1e-12 >= order_quantity,
            "Insufficient total executable volume");
    std::vector<double> result(capacity.size(),0);
    if (order_quantity == 0) return result;
    double low=0, high=*std::max_element(capacity.begin(),capacity.end());
    for (int iter=0; iter<80; iter++) {
        double limit=(low+high)/2,sum=0;
        for (double v : capacity) sum+=std::min(v,limit);
        if (sum < order_quantity) low=limit;
        else high=limit;
    }
    for (int i=0; i<int(capacity.size());i++)
        result[i]=std::min(capacity[i],high);
    return result;
}

inline double passive_fill_quantity(double order_quantity,
                                    double visible_queue_ahead,
                                    double opposing_market_order_volume) {
    require(std::isfinite(order_quantity) && order_quantity >= 0 &&
            std::isfinite(visible_queue_ahead) && visible_queue_ahead >= 0 &&
            std::isfinite(opposing_market_order_volume) &&
            opposing_market_order_volume >= 0,
            "Invalid passive queue quantities");
    return std::min(order_quantity,
                    std::max(0.0,opposing_market_order_volume-
                                   visible_queue_ahead));
}

inline double signed_markout(double execution_price,
                             double future_midpoint, int side) {
    require(std::isfinite(execution_price) && execution_price > 0 &&
            std::isfinite(future_midpoint) && future_midpoint > 0 &&
            (side==1 || side==-1),"Invalid markout inputs");
    return side*(future_midpoint-execution_price);
}

} // namespace kline
