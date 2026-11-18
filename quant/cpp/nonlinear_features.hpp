#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline std::vector<double> polynomial_features(
    const std::vector<double>& features,int degree){
    check_finite(features);
    require(degree>=1&&degree<=5&&features.size()<=20,
            "Polynomial feature degree or width invalid");
    std::vector<double> out=features;
    if(degree>=2)
        for(int i=0;i<int(features.size());i++)
            for(int j=i;j<int(features.size());j++)
                out.push_back(features[i]*features[j]);
    if(degree>=3)
        for(int d=3;d<=degree;d++)
            for(double value:features)out.push_back(std::pow(value,d));
    return out;
}
inline std::vector<double> radial_basis_features(
    const std::vector<double>& x,const std::vector<std::vector<double>>& centers,
    double bandwidth){
    check_finite(x);
    require(!centers.empty()&&std::isfinite(bandwidth)&&bandwidth>0,
            "Invalid RBF bandwidth or centers");
    std::vector<double> result;
    for(const auto& center:centers){
        check_finite(center);
        require(center.size()==x.size(),"RBF input dimension");
        double distance=0;
        for(int i=0;i<int(x.size());i++)distance+=std::pow(x[i]-center[i],2);
        result.push_back(std::exp(-distance/(2*bandwidth*bandwidth)));
    }
    return result;
}
inline double tanh_activation(double value){
    require(std::isfinite(value),"Nonfinite tanh input");
    return std::tanh(value);
}
} // namespace kline
