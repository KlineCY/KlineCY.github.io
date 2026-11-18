#pragma once
#include "learning.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>
namespace kline {
struct RegressionStump{
    int feature=0;
    double split=0,left=0,right=0;
    double training_loss=0;
};
inline RegressionStump fit_regression_stump(const Matrix& x,
                                            const std::vector<double>& y){
    check_feature_table(x,y);
    require(x.size()>=3,"Decision stump needs three observations");
    int n=int(x.size()),p=int(x.front().size());
    RegressionStump best;best.training_loss=std::numeric_limits<double>::infinity();
    for(int j=0;j<p;j++){
        std::vector<std::pair<double,double>> rows;
        for(int i=0;i<n;i++)rows.push_back({x[i][j],y[i]});
        std::sort(rows.begin(),rows.end());
        double total=0,squares=0,left=0,leftsq=0;
        for(const auto& row:rows){total+=row.second;squares+=row.second*row.second;}
        for(int i=1;i<n;i++){
            left+=rows[i-1].second;leftsq+=rows[i-1].second*rows[i-1].second;
            if(rows[i-1].first>=rows[i].first)continue;
            double right=total-left,rightsq=squares-leftsq;
            double loss=leftsq-left*left/i+rightsq-right*right/(n-i);
            if(loss<best.training_loss){
                best={j,0.5*rows[i-1].first+0.5*rows[i].first,
                      left/i,right/(n-i),std::max(0.0,loss)};
            }
        }
    }
    require(std::isfinite(best.training_loss),"Stump has no separable feature");
    return best;
}
inline double stump_predict(const RegressionStump& stump,
                             const std::vector<double>& features){
    check_finite(features);
    require(stump.feature>=0&&stump.feature<int(features.size()),
            "Stump feature missing");
    return features[stump.feature]<stump.split?stump.left:stump.right;
}
inline double stump_training_rmse(const RegressionStump& stump,
                                  const Matrix& x,const std::vector<double>& y){
    check_feature_table(x,y);
    double s=0;
    for(int i=0;i<int(x.size());i++)
        s+=std::pow(stump_predict(stump,x[i])-y[i],2);
    return std::sqrt(s/x.size());
}
} // namespace kline
