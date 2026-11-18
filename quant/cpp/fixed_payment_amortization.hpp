#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct PaymentRecord{int period=0;double interest=0,principal=0,balance=0;};
inline std::vector<PaymentRecord> fixed_rate_amortization(
    double principal,double rate_per_period,int periods){
    require(std::isfinite(principal)&&principal>0&&
            std::isfinite(rate_per_period)&&rate_per_period>=0&&
            periods>=1&&periods<=10000,"Invalid fixed loan schedule");
    double payment=rate_per_period==0?principal/periods:
         principal*rate_per_period/
         (1-std::pow(1+rate_per_period,-periods));
    double balance=principal;
    std::vector<PaymentRecord> schedule;
    for(int i=1;i<=periods;i++){
        double interest=balance*rate_per_period;
        double principal_payment=std::min(balance,payment-interest);
        if(i==periods)principal_payment=balance;
        balance=std::max(0.0,balance-principal_payment);
        schedule.push_back({i,interest,principal_payment,balance});
    }
    return schedule;
}
} // namespace kline
