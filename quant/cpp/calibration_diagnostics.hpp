#pragma once
#include "metrics_models.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline double expected_calibration_error(const std::vector<double>& predicted,
    const std::vector<int>& labels,int bins=10){
    check_finite(predicted);
    require(predicted.size()==labels.size()&&bins>=2&&bins<=1000,
            "Invalid calibration sample");
    std::vector<double> prediction(bins,0),outcome(bins,0);
    std::vector<int> count(bins,0);
    for(int i=0;i<int(predicted.size());i++){
        require(predicted[i]>=0&&predicted[i]<=1&&
                (labels[i]==0||labels[i]==1),"Invalid calibration probability");
        int bin=std::min(bins-1,int(predicted[i]*bins));
        prediction[bin]+=predicted[i];outcome[bin]+=labels[i];count[bin]++;
    }
    double error=0;
    for(int b=0;b<bins;b++)if(count[b]>0)
        error+=double(count[b])/predicted.size()*
           std::fabs(prediction[b]/count[b]-outcome[b]/count[b]);
    return error;
}
inline double binary_brier(const std::vector<double>& predicted,
                           const std::vector<int>& labels){
    check_finite(predicted);
    require(predicted.size()==labels.size(),"Brier dimensions mismatch");
    double total=0;
    for(int i=0;i<int(predicted.size());i++){
        require(predicted[i]>=0&&predicted[i]<=1&&
                (labels[i]==0||labels[i]==1),"Invalid Brier inputs");
        total+=std::pow(predicted[i]-labels[i],2);
    }
    return total/predicted.size();
}
} // namespace kline
