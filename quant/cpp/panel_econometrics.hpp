#pragma once
#include "ols_models.hpp"
#include <algorithm>
#include <map>
#include <vector>
namespace kline {
inline OLSResult entity_fixed_effects(
    const Matrix& features,const std::vector<double>& outcomes,
    const std::vector<int>& entity_ids){
    check_feature_table(features,outcomes);
    require(entity_ids.size()==outcomes.size(),
            "Entity panel identities differ");
    int n=int(features.size()),p=int(features.front().size());
    std::map<int,std::vector<int>> groups;
    for(int i=0;i<n;i++)groups[entity_ids[i]].push_back(i);
    require(int(groups.size())>=2,"Panel requires multiple entities");
    Matrix x=features;
    std::vector<double> y=outcomes;
    for(const auto& group:groups){
        const auto& ids=group.second;
        require(ids.size()>=2,"Each entity needs repeated observations");
        double ym=0;std::vector<double> xm(p,0);
        for(int index:ids){
            ym+=outcomes[index]/ids.size();
            for(int j=0;j<p;j++)xm[j]+=features[index][j]/ids.size();
        }
        for(int index:ids){
            y[index]-=ym;
            for(int j=0;j<p;j++)x[index][j]-=xm[j];
        }
    }
    auto result=ordinary_least_squares(x,y);
    return result;
}
inline double panel_within_r_squared(
    const Matrix& x,const std::vector<double>& y,
    const std::vector<int>& entity_ids){
    return entity_fixed_effects(x,y,entity_ids).r_squared;
}
} // namespace kline
