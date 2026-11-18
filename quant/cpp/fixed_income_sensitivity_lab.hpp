#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double continuously_compounded_forward_rate(
 double discountStart,double discountEnd,double startYears,double endYears){
 require(std::isfinite(discountStart)&&discountStart>0&&
         std::isfinite(discountEnd)&&discountEnd>0&&
         std::isfinite(startYears)&&startYears>=0&&
         std::isfinite(endYears)&&endYears>startYears,
         "Invalid dates or discount factors");
 return std::log(discountStart/discountEnd)/(endYears-startYears);
}
inline double zero_coupon_dv01(double face,double rate,double maturity){
 require(std::isfinite(face)&&face>0&&std::isfinite(rate)&&
         std::isfinite(maturity)&&maturity>=0,
         "Invalid zero-coupon bond");
 const double pv=face*std::exp(-rate*maturity);
 return pv*(1-std::exp(-0.0001*maturity));
}
inline double cashflow_macaulay_duration(
 const std::vector<double>& time,const std::vector<double>& cash,
 double annualContinuouslyCompoundedYield){
 check_finite(time);check_finite(cash);
 require(time.size()==cash.size()&&std::isfinite(annualContinuouslyCompoundedYield),
         "Cash-flow schedule does not align");
 double pv=0,weighted=0;
 for(std::size_t i=0;i<time.size();++i){
   require(time[i]>0&&cash[i]>=0&&(i==0||time[i]>time[i-1]),
           "Cash flows need sorted positive payment times");
   const double present=cash[i]*std::exp(-annualContinuouslyCompoundedYield*time[i]);
   pv+=present;weighted+=time[i]*present;
 }
 require(pv>0&&std::isfinite(pv),"Cash-flow present value must be positive");
 return weighted/pv;
}
inline double cashflow_continuous_convexity(
 const std::vector<double>& time,const std::vector<double>& cash,double yield){
 check_finite(time);check_finite(cash);
 require(time.size()==cash.size()&&std::isfinite(yield),
         "Invalid continuous convexity data");
 double pv=0,weighted=0;
 for(std::size_t i=0;i<time.size();++i){
   require(time[i]>0&&cash[i]>=0&&(i==0||time[i]>time[i-1]),
           "Cash flow schedule must increase");
   const double present=cash[i]*std::exp(-yield*time[i]);
   pv+=present;weighted+=time[i]*time[i]*present;
 }
 require(pv>0&&std::isfinite(pv),"Cash-flow PV invalid");
 return weighted/pv;
}
}
