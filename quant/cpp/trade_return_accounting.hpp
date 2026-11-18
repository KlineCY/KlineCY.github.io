#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct TransactionBook{
    double cash=0,shares=0,realized_fees=0;
};
inline TransactionBook execute_cash_stock_trade(
    TransactionBook ledger,double signed_shares,
    double execution_price,double fee){
    require(std::isfinite(ledger.cash)&&std::isfinite(ledger.shares)&&
            std::isfinite(ledger.realized_fees)&&
            std::isfinite(signed_shares)&&
            std::isfinite(execution_price)&&execution_price>0&&
            std::isfinite(fee)&&fee>=0,"Invalid stock ledger trade");
    ledger.cash-=signed_shares*execution_price+fee;
    ledger.shares+=signed_shares;
    ledger.realized_fees+=fee;
    return ledger;
}
inline double mark_transaction_book(
    const TransactionBook& ledger,double mark_price){
    require(std::isfinite(mark_price)&&mark_price>0,
            "Invalid stock ledger mark");
    return ledger.cash+ledger.shares*mark_price;
}
} // namespace kline
