#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline double shannon_entropy(const std::vector<double>& probabilities){
    check_finite(probabilities);double sum=0,entropy=0;
    for(double p:probabilities){
        require(p>=0,"Negative entropy mass");sum+=p;
        if(p>0)entropy-=p*std::log(p);
    }
    require(std::fabs(sum-1)<1e-9,"Probabilities do not sum to one");
    return entropy;
}
inline double kullback_leibler(const std::vector<double>& p,
                               const std::vector<double>& q){
    check_finite(p);check_finite(q);
    require(p.size()==q.size(),"KL dimension mismatch");
    double total_p=0,total_q=0,value=0;
    for(int i=0;i<int(p.size());i++){
        require(p[i]>=0&&q[i]>=0,"Invalid KL mass");
        total_p+=p[i];total_q+=q[i];
        if(p[i]>0){
            require(q[i]>0,"KL divergence infinite");
            value+=p[i]*std::log(p[i]/q[i]);
        }
    }
    require(std::fabs(total_p-1)<1e-9&&std::fabs(total_q-1)<1e-9,
            "KL masses need normalization");
    return std::max(0.0,value);
}
inline double binary_entropy(double probability){
    require(std::isfinite(probability)&&probability>=0&&probability<=1,
            "Invalid binary entropy probability");
    return probability==0||probability==1?0:
       -probability*std::log(probability)-
       (1-probability)*std::log1p(-probability);
}
inline double effective_state_count(const std::vector<double>& masses){
    return std::exp(shannon_entropy(masses));
}
} // namespace kline
