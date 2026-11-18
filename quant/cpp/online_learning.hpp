#pragma once
#include "portfolio.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct RecursiveLeastSquares{
    std::vector<double> coefficients;
    Matrix inverse_covariance;
};
inline RecursiveLeastSquares initialize_rls(int features,double ridge=1){
    require(features>0&&features<=500&&std::isfinite(ridge)&&ridge>0,
            "RLS initialization");
    RecursiveLeastSquares s;
    s.coefficients.assign(features,0);
    s.inverse_covariance.assign(features,std::vector<double>(features,0));
    for(int i=0;i<features;i++)s.inverse_covariance[i][i]=1/ridge;
    return s;
}
inline double rls_predict(const RecursiveLeastSquares& s,
                          const std::vector<double>& features){
    check_finite(features);
    require(features.size()==s.coefficients.size(),"RLS feature mismatch");
    return std::inner_product(features.begin(),features.end(),
                              s.coefficients.begin(),0.0);
}
inline void rls_update(RecursiveLeastSquares& s,
                       const std::vector<double>& features,double target,
                       double forgetting=1){
    check_finite(features);
    require(std::isfinite(target)&&std::isfinite(forgetting)&&
            forgetting>0&&forgetting<=1&&
            features.size()==s.coefficients.size(),"Invalid RLS update");
    int n=int(features.size());
    std::vector<double> px(n,0);
    for(int i=0;i<n;i++){
        require(int(s.inverse_covariance[i].size())==n,"RLS covariance shape");
        for(int j=0;j<n;j++)px[i]+=s.inverse_covariance[i][j]*features[j];
    }
    double denom=forgetting;
    for(int i=0;i<n;i++)denom+=features[i]*px[i];
    require(denom>0&&std::isfinite(denom),"Invalid RLS innovation covariance");
    double error=target-rls_predict(s,features);
    for(int i=0;i<n;i++)s.coefficients[i]+=px[i]*error/denom;
    for(int i=0;i<n;i++)
        for(int j=0;j<n;j++)
            s.inverse_covariance[i][j]=(s.inverse_covariance[i][j]-
                px[i]*px[j]/denom)/forgetting;
}
} // namespace kline
