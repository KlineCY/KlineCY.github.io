#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline void check_probability_vector(const std::vector<double>& p){
    check_finite(p);
    double sum=0;
    for(double value:p){
        require(value>=0,"Negative probability");
        sum+=value;
    }
    require(std::fabs(sum-1)<1e-9,"Probability masses must sum to one");
}
inline double discrete_entropy(const std::vector<double>& probabilities){
    check_probability_vector(probabilities);
    double result=0;
    for(double p:probabilities)if(p>0)result-=p*std::log(p);
    return result;
}
inline double kl_divergence(const std::vector<double>& p,const std::vector<double>& q){
    check_probability_vector(p);check_probability_vector(q);
    require(p.size()==q.size(),"KL distribution dimensions differ");
    double result=0;
    for(int i=0;i<int(p.size());i++){
        if(p[i]==0)continue;
        require(q[i]>0,"KL infinite due to zero reference mass");
        result+=p[i]*std::log(p[i]/q[i]);
    }
    return result;
}
inline double jensen_shannon_divergence(const std::vector<double>& p,
                                       const std::vector<double>& q){
    check_probability_vector(p);check_probability_vector(q);
    require(p.size()==q.size(),"JS distribution shape");
    std::vector<double> midpoint;
    for(int i=0;i<int(p.size());i++)midpoint.push_back((p[i]+q[i])/2);
    return (kl_divergence(p,midpoint)+kl_divergence(q,midpoint))/2;
}
} // namespace kline
