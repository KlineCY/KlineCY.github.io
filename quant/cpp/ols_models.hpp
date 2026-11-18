#pragma once
#include "learning.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace kline {
struct OLSResult {
    std::vector<double> coefficients;
    std::vector<double> fitted;
    std::vector<double> residuals;
    double r_squared=0;
    double mse=0;
};

inline OLSResult ordinary_least_squares(const Matrix& features,
                                         const std::vector<double>& outcomes) {
    check_feature_table(features,outcomes);
    int n=int(features.size()), p=int(features.front().size())+1;
    require(n>p,"OLS needs more rows than coefficients");
    Matrix columns(p,std::vector<double>(n,1));
    for(int j=1;j<p;j++)
        for(int i=0;i<n;i++)columns[j][i]=features[i][j-1];
    Matrix q(p,std::vector<double>(n,0));
    Matrix r(p,std::vector<double>(p,0));
    for(int j=0;j<p;j++){
        auto remaining=columns[j];
        for(int pass=0;pass<2;pass++){
            for(int k=0;k<j;k++){
                double value=std::inner_product(q[k].begin(),q[k].end(),
                                                remaining.begin(),0.0);
                r[k][j]+=value;
                for(int i=0;i<n;i++)remaining[i]-=value*q[k][i];
            }
        }
        double squared=std::inner_product(remaining.begin(),remaining.end(),
                                          remaining.begin(),0.0);
        double norm=std::sqrt(squared);
        require(norm>1e-11,"OLS has linearly dependent predictors");
        r[j][j]=norm;
        for(int i=0;i<n;i++)q[j][i]=remaining[i]/norm;
    }
    std::vector<double> rhs(p);
    for(int j=0;j<p;j++)
        rhs[j]=std::inner_product(q[j].begin(),q[j].end(),outcomes.begin(),0.0);
    std::vector<double> beta(p,0);
    for(int j=p-1;j>=0;j--){
        double value=rhs[j];
        for(int k=j+1;k<p;k++)value-=r[j][k]*beta[k];
        beta[j]=value/r[j][j];
    }
    OLSResult result;
    result.coefficients=beta;
    double total=0,residual_sum=0,center=mean(outcomes);
    for(int i=0;i<n;i++){
        double prediction=beta[0];
        for(int j=1;j<p;j++)prediction+=beta[j]*features[i][j-1];
        result.fitted.push_back(prediction);
        result.residuals.push_back(outcomes[i]-prediction);
        residual_sum+=std::pow(outcomes[i]-prediction,2);
        total+=std::pow(outcomes[i]-center,2);
    }
    result.mse=residual_sum/(n-p);
    result.r_squared=total>0?1-residual_sum/total:
                     (residual_sum<1e-20?1:0);
    return result;
}

inline double ols_predict(const OLSResult& model,
                          const std::vector<double>& features) {
    check_finite(features);
    require(features.size()+1==model.coefficients.size(),
            "OLS prediction dimension mismatch");
    double value=model.coefficients[0];
    for(int j=0;j<int(features.size());j++)
        value+=model.coefficients[j+1]*features[j];
    return value;
}

inline double durbin_watson(const std::vector<double>& residuals) {
    check_finite(residuals);
    require(residuals.size()>=2,"Durbin-Watson needs two errors");
    double sum=0,denominator=0;
    for(double e:residuals)denominator+=e*e;
    require(denominator>0,"Durbin-Watson undefined for exact fit");
    for(int t=1;t<int(residuals.size());t++)
        sum+=std::pow(residuals[t]-residuals[t-1],2);
    return sum/denominator;
}

inline std::vector<double> expanding_ols_forecast(
    const std::vector<double>& values,int min_train) {
    check_finite(values);
    require(min_train>=4 && min_train<int(values.size()),
            "Invalid expanding AR forecast start");
    std::vector<double> predictions;
    for(int t=min_train;t<int(values.size());t++){
        Matrix lag;
        std::vector<double> next;
        for(int j=1;j<t;j++){
            lag.push_back({values[j-1]});
            next.push_back(values[j]);
        }
        auto fit=ordinary_least_squares(lag,next);
        predictions.push_back(ols_predict(fit,{values[t-1]}));
    }
    return predictions;
}
} // namespace kline
