#pragma once
#include "learning.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct AdamLinearFit{std::vector<double> coefficients;double training_mse=0;};
inline AdamLinearFit fit_adam_linear(const Matrix& x,const std::vector<double>& y,
                  int epochs=400,double step=0.05,double l2=0.0){
    check_feature_table(x,y);
    require(epochs>=1&&epochs<=100000&&std::isfinite(step)&&step>0&&
            std::isfinite(l2)&&l2>=0,"Invalid Adam learning hyperparameters");
    int n=int(x.size()),p=int(x[0].size())+1;
    std::vector<double> w(p,0),m(p,0),v(p,0);
    for(int epoch=1;epoch<=epochs;epoch++){
        std::vector<double> gradient(p,0);
        for(int i=0;i<n;i++){
            double predicted=w[0];
            for(int j=1;j<p;j++)predicted+=w[j]*x[i][j-1];
            double error=predicted-y[i];
            gradient[0]+=2*error/n;
            for(int j=1;j<p;j++)gradient[j]+=2*error*x[i][j-1]/n;
        }
        for(int j=0;j<p;j++){
            if(j>0)gradient[j]+=2*l2*w[j];
            m[j]=0.9*m[j]+0.1*gradient[j];
            v[j]=0.999*v[j]+0.001*gradient[j]*gradient[j];
            double corrected=m[j]/(1-std::pow(0.9,epoch));
            double variance=v[j]/(1-std::pow(0.999,epoch));
            w[j]-=step*corrected/(std::sqrt(variance)+1e-8);
        }
    }
    double mse=0;
    for(int i=0;i<n;i++){
        double prediction=w[0];
        for(int j=1;j<p;j++)prediction+=w[j]*x[i][j-1];
        mse+=std::pow(y[i]-prediction,2)/n;
    }
    return {w,mse};
}
inline double adam_linear_prediction(const AdamLinearFit& fit,
                                      const std::vector<double>& features){
    check_finite(features);
    require(features.size()+1==fit.coefficients.size(),"Adam prediction shape");
    double y=fit.coefficients[0];
    for(int j=0;j<int(features.size());j++)y+=fit.coefficients[j+1]*features[j];
    return y;
}
} // namespace kline
