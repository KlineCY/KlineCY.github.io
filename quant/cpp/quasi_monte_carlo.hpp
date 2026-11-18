#pragma once
#include "probability_models.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double radical_inverse(unsigned int index,unsigned int base){
    require(base>=2&&base<=100,"Invalid Halton base");
    double value=0,factor=1.0/base;
    while(index){
        value+=(index%base)*factor;
        index/=base;factor/=base;
    }
    return value;
}
inline std::vector<std::vector<double>> halton_points(int observations,
    const std::vector<unsigned int>& bases,int skip=1){
    require(observations>0&&observations<=100000&&skip>=1&&
            !bases.empty(),"Invalid Halton configuration");
    std::vector<std::vector<double>> result(
        observations,std::vector<double>(bases.size()));
    for(int i=0;i<observations;i++)
        for(int j=0;j<int(bases.size());j++)
            result[i][j]=radical_inverse(unsigned(i+skip),bases[j]);
    return result;
}
inline std::vector<double> halton_normal_samples(int observations,
    unsigned int base=2){
    auto points=halton_points(observations,{base});
    std::vector<double> normal;
    for(const auto& row:points)
        normal.push_back(normal_inverse_cdf(row[0]));
    return normal;
}
} // namespace kline
