#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
namespace kline {
struct FundFeePeriod{double before_fee=0,management_fee=0,performance_fee=0;
    double after_fee=0,high_watermark_per_share=0;};
inline FundFeePeriod charge_fund_fees(
    double previous_nav_per_share,double gross_return,
    double previous_high_watermark,double management_fee_period,
    double incentive_rate){
    require(std::isfinite(previous_nav_per_share)&&previous_nav_per_share>0&&
            std::isfinite(gross_return)&&gross_return>-1&&
            std::isfinite(previous_high_watermark)&&previous_high_watermark>0&&
            std::isfinite(management_fee_period)&&management_fee_period>=0&&
            management_fee_period<1&&std::isfinite(incentive_rate)&&
            incentive_rate>=0&&incentive_rate<=1,"Invalid fund fee parameters");
    double before=previous_nav_per_share*(1+gross_return);
    double mgmt=before*management_fee_period;
    double pre_incentive=before-mgmt;
    double incentive=std::max(0.0,pre_incentive-previous_high_watermark)*
                     incentive_rate;
    double net=pre_incentive-incentive;
    return {before,mgmt,incentive,net,std::max(previous_high_watermark,net)};
}
} // namespace kline
