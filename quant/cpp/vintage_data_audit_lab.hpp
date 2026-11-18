#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline double vintage_revision_rmse(
 const std::vector<double>& initialVintage,const std::vector<double>& revised){
 check_finite(initialVintage);check_finite(revised);
 require(initialVintage.size()==revised.size(),"Revision vintage rows do not align");
 double squared=0;
 for(std::size_t i=0;i<initialVintage.size();++i)
  squared+=std::pow(revised[i]-initialVintage[i],2);
 return std::sqrt(squared/initialVintage.size());
}
inline double average_publication_delay(
 const std::vector<double>& referenceTime,const std::vector<double>& releasedTime){
 check_finite(referenceTime);check_finite(releasedTime);
 require(referenceTime.size()==releasedTime.size(),"Release events misaligned");
 double lag=0;
 for(std::size_t i=0;i<referenceTime.size();++i){
  require(releasedTime[i]>=referenceTime[i],"Release cannot predate reference time");
  lag+=releasedTime[i]-referenceTime[i];
 }
 return lag/referenceTime.size();
}
inline double stale_quote_fraction(
 const std::vector<double>& quoteTimestamps,double allowedGap){
 check_finite(quoteTimestamps);
 require(quoteTimestamps.size()>=2&&std::isfinite(allowedGap)&&allowedGap>=0,
         "Stale quote requires ordered timestamp gaps");
 int stale=0;
 for(std::size_t i=1;i<quoteTimestamps.size();++i){
  require(quoteTimestamps[i]>quoteTimestamps[i-1],
          "Quote timestamps must be strictly increasing");
  stale+=quoteTimestamps[i]-quoteTimestamps[i-1]>allowedGap;
 }
 return double(stale)/(quoteTimestamps.size()-1);
}
inline double invalid_bid_ask_fraction(
 const std::vector<double>& bids,const std::vector<double>& asks){
 check_finite(bids);check_finite(asks);
 require(bids.size()==asks.size(),"Bid ask events misaligned");
 int invalid=0;
 for(std::size_t i=0;i<bids.size();++i)
   invalid+=(bids[i]<=0||asks[i]<=0||bids[i]>=asks[i]);
 return double(invalid)/bids.size();
}
}
