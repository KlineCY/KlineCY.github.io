#pragma once
#include "learning.hpp"
#include <vector>
namespace kline {
struct OriginSplit{
    int train_start=0,train_end=0,test_start=0,test_end=0;
};
inline std::vector<OriginSplit> rolling_origin_splits(
    int observations,int initial_train,int test_length,
    int step,int gap=0,int max_train=0){
    require(initial_train>=3&&test_length>=1&&step>=1&&gap>=0&&
            observations>=initial_train+gap+test_length&&
            max_train>=0,"Invalid rolling origin parameters");
    std::vector<OriginSplit> splits;
    for(int end=initial_train;end+gap+test_length<=observations;end+=step){
        int start=max_train?std::max(0,end-max_train):0;
        require(end-start>=3,"Short training segment");
        splits.push_back({start,end,end+gap,end+gap+test_length});
    }
    return splits;
}
inline std::vector<double> rolling_origin_ridge_errors(
    const Matrix& x,const std::vector<double>& y,
    const std::vector<OriginSplit>& splits,double penalty){
    check_feature_table(x,y);
    require(penalty>0&&!splits.empty(),"Invalid rolling origin Ridge penalty");
    std::vector<double> errors;
    for(const auto& segment:splits){
        require(segment.train_start>=0&&segment.train_end<=segment.test_start&&
                segment.test_start<segment.test_end&&
                segment.test_end<=int(x.size()),
                "Rolling origin split leaks future data");
        Matrix training(x.begin()+segment.train_start,
                        x.begin()+segment.train_end);
        std::vector<double> labels(y.begin()+segment.train_start,
                                   y.begin()+segment.train_end);
        auto fit=fit_ridge(training,labels,penalty);
        for(int i=segment.test_start;i<segment.test_end;i++)
            errors.push_back(y[i]-ridge_predict(fit,x[i]));
    }
    return errors;
}
} // namespace kline
