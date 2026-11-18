#pragma once
#include "regression_models.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace kline {

inline RidgeModel fit_elastic_net(const Matrix& x,const std::vector<double>& y,
                                  double l1,double l2,int iterations=5000) {
    check_feature_table(x,y);
    require(std::isfinite(l1)&&std::isfinite(l2)&&l1>=0&&l2>=0&&
            iterations>0,"Invalid elastic-net penalties");
    int n=int(x.size()),p=int(x[0].size());
    RidgeModel model;
    model.intercept=mean(y);
    model.coefficients.assign(p,0);
    model.feature_mean.assign(p,0);
    model.feature_scale.assign(p,1);
    Matrix z(n,std::vector<double>(p));
    std::vector<double> norms(p,0),residual(n);
    for(int t=0;t<n;t++)residual[t]=y[t]-model.intercept;
    for(int j=0;j<p;j++){
        for(int t=0;t<n;t++)model.feature_mean[j]+=x[t][j]/n;
        double square=0;
        for(int t=0;t<n;t++)
            square+=std::pow(x[t][j]-model.feature_mean[j],2)/n;
        model.feature_scale[j]=square>0?std::sqrt(square):1;
        for(int t=0;t<n;t++){
            z[t][j]=(x[t][j]-model.feature_mean[j])/model.feature_scale[j];
            norms[j]+=z[t][j]*z[t][j]/n;
        }
    }
    for(int iter=0;iter<iterations;iter++){
        double change=0;
        for(int j=0;j<p;j++){
            double correlation=0;
            for(int t=0;t<n;t++)
                correlation+=z[t][j]*(residual[t]+z[t][j]*model.coefficients[j])/n;
            double updated=(norms[j]+l2)>0?
                soft_threshold(correlation,l1)/(norms[j]+l2):0;
            double diff=updated-model.coefficients[j];
            model.coefficients[j]=updated;
            for(int t=0;t<n;t++)residual[t]-=z[t][j]*diff;
            change=std::max(change,std::fabs(diff));
        }
        if(change<1e-11)break;
    }
    return model;
}

inline std::vector<double> huber_weights(const std::vector<double>& errors,
                                          double threshold) {
    check_finite(errors);
    require(std::isfinite(threshold)&&threshold>0,"Invalid Huber threshold");
    std::vector<double> out;
    for(double e:errors)
        out.push_back(std::fabs(e)<=threshold?1:threshold/std::fabs(e));
    return out;
}

inline double huber_loss(const std::vector<double>& errors,double threshold) {
    check_finite(errors);
    require(std::isfinite(threshold)&&threshold>0,"Invalid Huber loss threshold");
    double total=0;
    for(double e:errors){
        double a=std::fabs(e);
        total+=a<=threshold?0.5*a*a:threshold*(a-0.5*threshold);
    }
    return total/errors.size();
}

inline double root_mean_squared_error(const std::vector<double>& actual,
                                      const std::vector<double>& predicted) {
    check_finite(actual);check_finite(predicted);
    require(actual.size()==predicted.size(),"RMSE dimensions differ");
    double sum=0;
    for(int i=0;i<int(actual.size());i++)
        sum+=std::pow(actual[i]-predicted[i],2);
    return std::sqrt(sum/actual.size());
}
} // namespace kline
