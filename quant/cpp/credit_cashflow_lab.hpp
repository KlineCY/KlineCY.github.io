#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>

namespace kline {
inline double bond_accrued_coupon(double face,double annualCoupon,
                                  int paymentsPerYear,int daysElapsed,int daysInPeriod){
    require(std::isfinite(face)&&face>0&&std::isfinite(annualCoupon)&&annualCoupon>=0&&
            paymentsPerYear>=1&&paymentsPerYear<=365&&daysInPeriod>0&&
            daysElapsed>=0&&daysElapsed<=daysInPeriod,"Invalid bond accrual inputs");
    return face*annualCoupon/paymentsPerYear*daysElapsed/daysInPeriod;
}
inline double overnight_compounded_discount(const std::vector<double>& overnightRates,
                                            const std::vector<double>& accrualYears){
    check_finite(overnightRates);check_finite(accrualYears);
    require(overnightRates.size()==accrualYears.size(),
            "Overnight rates and day count fractions must align");
    double logAccrual=0;
    for(std::size_t i=0;i<overnightRates.size();++i){
        require(accrualYears[i]>0&&accrualYears[i]<=1&&
                1+overnightRates[i]*accrualYears[i]>0,
                "Invalid overnight compounding factor");
        logAccrual+=std::log1p(overnightRates[i]*accrualYears[i]);
    }
    require(std::isfinite(logAccrual),"Overnight accumulation overflow");
    return std::exp(-logAccrual);
}
inline double merton_structural_default_probability(double assetValue,double debtDue,
                                                      double assetDrift,double assetVol,
                                                      double years){
    require(std::isfinite(assetValue)&&assetValue>0&&
            std::isfinite(debtDue)&&debtDue>0&&std::isfinite(assetDrift)&&
            std::isfinite(assetVol)&&assetVol>0&&std::isfinite(years)&&
            years>0,"Invalid structural credit inputs");
    const double d2=(std::log(assetValue/debtDue)+
        (assetDrift-0.5*assetVol*assetVol)*years)/(assetVol*std::sqrt(years));
    return 0.5*std::erfc(d2/std::sqrt(2.0));
}
inline double risky_zero_coupon_recovery_of_par(double face,double rate,
                                               double hazard,double recovery,double years){
    require(std::isfinite(face)&&face>0&&std::isfinite(rate)&&rate>=0&&
            std::isfinite(hazard)&&hazard>=0&&std::isfinite(recovery)&&
            recovery>=0&&recovery<=1&&std::isfinite(years)&&years>=0,
            "Invalid risky zero-coupon terms");
    const double sum=rate+hazard;
    const double undiscountedRecovery=sum==0?years:-std::expm1(-sum*years)/sum;
    return face*(std::exp(-sum*years)+recovery*hazard*undiscountedRecovery);
}
} // namespace kline
