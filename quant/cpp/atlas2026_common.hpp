#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <vector>
namespace kline {
namespace atlas2026 {
using Values=std::vector<double>;
inline void valid(const Values& x,const Values& y,double p){
 check_finite(x);check_finite(y);
 require(x.size()==y.size()&&std::isfinite(p),"Data rows and parameter must be finite and aligned");
}
inline double positive_weights(const Values& y){
 double sum=0;
 for(double w:y){require(w>=0,"Weights must be nonnegative");sum+=w;}
 require(sum>0&&std::isfinite(sum),"Positive finite total weight required");
 return sum;
}
inline void nonnegative(const Values& x){
 for(double z:x)require(z>=0,"Inputs must be nonnegative");
}
}
}
