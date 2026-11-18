#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline double quoted_spread_basis_points(
 const std::vector<double>& bid,const std::vector<double>& ask){
 check_finite(bid);check_finite(ask);
 require(bid.size()==ask.size(),"Quote sides do not align");
 double total=0;
 for(std::size_t i=0;i<bid.size();++i){
  require(bid[i]>0&&ask[i]>bid[i],"Valid non-crossed quotes required");
  total+=20000.0*(ask[i]-bid[i])/(ask[i]+bid[i]);
 }
 return total/bid.size();
}
inline double liquidity_replenishment_ratio(
 const std::vector<double>& executed,const std::vector<double>& replenished){
 check_finite(executed);check_finite(replenished);
 require(executed.size()==replenished.size(),"Liquidity flow rows differ");
 double out=0,in=0;
 for(std::size_t i=0;i<executed.size();++i){
  require(executed[i]>=0&&replenished[i]>=0,
          "Signed trades are invalid for total replenishment");
  out+=executed[i];in+=replenished[i];
 }
 require(out>0,"Replenishment requires executed volume");
 return in/out;
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
