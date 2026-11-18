#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline double roll_effective_spread(const std::vector<double>& transactionPrice){
 check_finite(transactionPrice);
 require(transactionPrice.size()>=4,"Roll estimate needs four transaction prices");
 std::vector<double> difference;
 for(std::size_t i=1;i<transactionPrice.size();++i)
   difference.push_back(transactionPrice[i]-transactionPrice[i-1]);
 const double cov=covariance(
   std::vector<double>(difference.begin()+1,difference.end()),
   std::vector<double>(difference.begin(),difference.end()-1),false);
 return cov<0?2*std::sqrt(-cov):0;
}
inline double amihud_illiquidity(
 const std::vector<double>& simpleReturns,const std::vector<double>& dollarVolume){
 check_finite(simpleReturns);check_finite(dollarVolume);
 require(simpleReturns.size()==dollarVolume.size(),"Return/volume rows differ");
 double sum=0;
 for(std::size_t i=0;i<simpleReturns.size();++i){
   require(dollarVolume[i]>0,"Dollar volume must be positive");
   sum+=std::abs(simpleReturns[i])/dollarVolume[i];
 }
 return sum/simpleReturns.size();
}
inline double kyle_price_impact_slope(
 const std::vector<double>& signedVolume,const std::vector<double>& priceChange){
 check_finite(signedVolume);check_finite(priceChange);
 require(signedVolume.size()==priceChange.size()&&signedVolume.size()>=3,
         "Kyle regression needs at least three aligned observations");
 const double mu=mean(signedVolume),mv=mean(priceChange);
 double numerator=0,denominator=0;
 for(std::size_t i=0;i<signedVolume.size();++i){
   numerator+=(signedVolume[i]-mu)*(priceChange[i]-mv);
   denominator+=(signedVolume[i]-mu)*(signedVolume[i]-mu);
 }
 require(denominator>0,"Kyle impact requires variation in signed volume");
 return numerator/denominator;
}
inline double lag_one_order_sign_correlation(const std::vector<double>& signs){
 check_finite(signs);require(signs.size()>=3,"Order signs require three trades");
 for(double s:signs)require(s==-1||s==1,"Trade signs must be -1 or +1");
 const std::vector<double> a(signs.begin()+1,signs.end());
 const std::vector<double> b(signs.begin(),signs.end()-1);
 const double varA=variance(a,false),varB=variance(b,false);
 require(varA>0&&varB>0,"Both lagged signs require variation");
 return covariance(a,b,false)/std::sqrt(varA*varB);
}
}
