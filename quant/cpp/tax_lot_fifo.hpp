#pragma once
#include "stats.hpp"
#include <cmath>
#include <deque>
namespace kline {
struct TaxLot{double shares=0,cost_per_share=0;};
struct FifoLedger{
    std::deque<TaxLot> lots;
    double realized_pnl=0;
    double remaining_shares=0;
    double remaining_basis=0;
};
inline void fifo_buy(FifoLedger& ledger,double shares,double price,double commission=0){
    require(std::isfinite(shares)&&shares>0&&std::isfinite(price)&&price>=0&&
            std::isfinite(commission)&&commission>=0,"Invalid FIFO buy");
    ledger.lots.push_back({shares,price+commission/shares});
    ledger.remaining_shares+=shares;
    ledger.remaining_basis+=shares*price+commission;
}
inline void fifo_sell(FifoLedger& ledger,double shares,double price,double commission=0){
    require(std::isfinite(shares)&&shares>0&&
            shares<=ledger.remaining_shares+1e-9&&
            std::isfinite(price)&&price>=0&&
            std::isfinite(commission)&&commission>=0,
            "Invalid FIFO sale");
    double remain=shares,realized=shares*price-commission;
    while(remain>1e-10){
        require(!ledger.lots.empty(),"FIFO inventory exhausted");
        auto& lot=ledger.lots.front();
        double used=std::min(remain,lot.shares);
        realized-=used*lot.cost_per_share;
        ledger.remaining_basis-=used*lot.cost_per_share;
        lot.shares-=used;remain-=used;
        if(lot.shares<1e-10)ledger.lots.pop_front();
    }
    ledger.remaining_shares-=shares;
    if(ledger.remaining_shares<1e-9){
        ledger.remaining_shares=0;ledger.remaining_basis=0;ledger.lots.clear();
    }
    ledger.realized_pnl+=realized;
}
} // namespace kline
