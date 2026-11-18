#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline double hayashi_yoshida_covariance(
 const std::vector<double>& startA,const std::vector<double>& endA,
 const std::vector<double>& returnA,const std::vector<double>& startB,
 const std::vector<double>& endB,const std::vector<double>& returnB) {
 check_finite(startA);check_finite(endA);check_finite(returnA);
 check_finite(startB);check_finite(endB);check_finite(returnB);
 require(startA.size()==endA.size()&&startA.size()==returnA.size()&&
         startB.size()==endB.size()&&startB.size()==returnB.size(),
         "Timestamp and asynchronous returns disagree");
 for(std::size_t i=0;i<startA.size();++i)
   require(startA[i]<endA[i]&&(i==0||startA[i]>=endA[i-1]),
           "Intervals A must be disjoint, sorted and positive");
 for(std::size_t i=0;i<startB.size();++i)
   require(startB[i]<endB[i]&&(i==0||startB[i]>=endB[i-1]),
           "Intervals B must be disjoint, sorted and positive");
 double sum=0;std::size_t i=0,j=0;
 while(i<startA.size()&&j<startB.size()) {
   if(std::min(endA[i],endB[j])>std::max(startA[i],startB[j]))
      sum+=returnA[i]*returnB[j];
   if(endA[i]<=endB[j])++i; else ++j;
 }
 return sum;
}
inline double realized_quarticity(const std::vector<double>& logReturns) {
 check_finite(logReturns);
 double sum=0;
 for(double r:logReturns)sum+=r*r*r*r;
 return double(logReturns.size())/3.0*sum;
}
inline double one_lag_bartlett_realized_kernel(const std::vector<double>& logReturns){
 check_finite(logReturns);
 require(logReturns.size()>=2,"Two returns required for realized kernel");
 double gamma0=0,gamma1=0;
 for(double r:logReturns)gamma0+=r*r;
 for(std::size_t i=1;i<logReturns.size();++i)
   gamma1+=logReturns[i]*logReturns[i-1];
 return gamma0+gamma1;
}
inline double truncated_realized_variance(
 const std::vector<double>& logReturns,double threshold) {
 check_finite(logReturns);
 require(std::isfinite(threshold)&&threshold>0,
         "Truncation threshold must be positive");
 double sum=0;
 for(double r:logReturns)if(std::abs(r)<=threshold)sum+=r*r;
 return sum;
}
}
