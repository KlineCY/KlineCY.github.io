#pragma once
#include "learning.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline std::vector<double> minimum_change_factor_neutral(
    const std::vector<double>& weights,const Matrix& constraint_rows,
    const std::vector<double>& targets){
    check_finite(weights);check_finite(targets);
    require(!constraint_rows.empty()&&constraint_rows.size()==targets.size(),
            "Invalid factor neutrality constraints");
    int n=int(weights.size()),k=int(constraint_rows.size());
    for(const auto& row:constraint_rows){
        check_finite(row);require(int(row.size())==n,
                                  "Factor constraint assets mismatch");
    }
    Matrix gram(k,std::vector<double>(k));
    std::vector<double> residual(k);
    for(int a=0;a<k;a++){
        residual[a]=-targets[a];
        for(int i=0;i<n;i++)residual[a]+=constraint_rows[a][i]*weights[i];
        for(int b=0;b<k;b++)
            for(int i=0;i<n;i++)
                gram[a][b]+=constraint_rows[a][i]*constraint_rows[b][i];
    }
    auto multiplier=solve_linear_system(gram,residual);
    std::vector<double> result=weights;
    for(int i=0;i<n;i++)
        for(int a=0;a<k;a++)
            result[i]-=constraint_rows[a][i]*multiplier[a];
    return result;
}
} // namespace kline
