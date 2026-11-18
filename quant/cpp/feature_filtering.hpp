#pragma once
#include "learning.hpp"
#include <algorithm>
#include <numeric>
#include <vector>
namespace kline {
inline std::vector<int> select_top_abs_correlation(const Matrix& x,
                     const std::vector<double>& y,int count){
    check_feature_table(x,y);
    int p=int(x[0].size());
    require(count>=1&&count<=p,"Invalid feature selection count");
    std::vector<std::pair<double,int>> scored;
    for(int j=0;j<p;j++){
        std::vector<double> column;
        for(const auto& row:x)column.push_back(row[j]);
        double score=0;
        if(variance(column,false)>0&&variance(y,false)>0)
            score=std::fabs(correlation(column,y));
        scored.push_back({-score,j});
    }
    std::stable_sort(scored.begin(),scored.end());
    std::vector<int> selected;
    for(int i=0;i<count;i++)selected.push_back(scored[i].second);
    return selected;
}
inline Matrix subset_feature_columns(const Matrix& x,
                                     const std::vector<int>& selection){
    require(!x.empty()&&!selection.empty(),"Empty selected feature design");
    Matrix out;
    for(const auto& row:x){
        check_finite(row);std::vector<double> subset;
        for(int id:selection){
            require(id>=0&&id<int(row.size()),"Feature index outside design");
            subset.push_back(row[id]);
        }
        out.push_back(subset);
    }
    return out;
}
} // namespace kline
