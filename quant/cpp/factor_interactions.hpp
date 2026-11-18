#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline std::vector<double> interaction_features(
    const std::vector<double>& x,bool include_square=false){
    check_finite(x);
    std::vector<double> result=x;
    for(int i=0;i<int(x.size());i++)
        for(int j=i+(include_square?0:1);j<int(x.size());j++)
            result.push_back(x[i]*x[j]);
    return result;
}
inline double cross_sectional_cosine_similarity(
    const std::vector<double>& a,const std::vector<double>& b){
    check_finite(a);check_finite(b);
    require(a.size()==b.size(),"Cosine vectors mismatch");
    double numerator=0,aa=0,bb=0;
    for(int i=0;i<int(a.size());i++){
        numerator+=a[i]*b[i];aa+=a[i]*a[i];bb+=b[i]*b[i];
    }
    require(aa>0&&bb>0,"Cosine zero vector");
    return numerator/std::sqrt(aa*bb);
}
inline double factor_spread_signal(const std::vector<double>& exposures,
                                   const std::vector<double>& factor){
    check_finite(exposures);check_finite(factor);
    require(exposures.size()==factor.size(),"Factor linear shape");
    return std::inner_product(exposures.begin(),exposures.end(),
                              factor.begin(),0.0);
}
} // namespace kline
