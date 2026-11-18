#pragma once
#include "logistic_irls.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct FtrlLogistic{
    std::vector<double> z,n;
    double alpha=0.1,beta=1,l1=0,l2=0;
};
inline FtrlLogistic new_ftrl_logistic(int features,double alpha,
                                     double beta,double l1,double l2){
    require(features>0&&std::isfinite(alpha)&&alpha>0&&
            std::isfinite(beta)&&beta>=0&&
            std::isfinite(l1)&&l1>=0&&
            std::isfinite(l2)&&l2>=0,
            "Invalid FTRL logistic hyperparameters");
    return {std::vector<double>(features+1,0),
            std::vector<double>(features+1,0),alpha,beta,l1,l2};
}
inline std::vector<double> ftrl_weights(const FtrlLogistic& m){
    std::vector<double> result(m.z.size(),0);
    for(int j=0;j<int(result.size());j++){
        double penalty=j==0?0:m.l1,regularizer=j==0?0:m.l2;
        if(std::fabs(m.z[j])>penalty){
            double sign=m.z[j]>0?1:-1;
            result[j]=-(m.z[j]-sign*penalty)/
                ((m.beta+std::sqrt(m.n[j]))/m.alpha+regularizer);
        }
    }
    return result;
}
inline double ftrl_predict(const FtrlLogistic& m,
                            const std::vector<double>& x){
    check_finite(x);
    require(x.size()+1==m.z.size(),"FTRL feature count mismatch");
    auto w=ftrl_weights(m);double score=w[0];
    for(int j=0;j<int(x.size());j++)score+=w[j+1]*x[j];
    return stable_sigmoid(score);
}
inline double ftrl_online_update(FtrlLogistic& m,
                    const std::vector<double>& x,int outcome){
    require(outcome==0||outcome==1,"FTRL requires binary target");
    double probability=ftrl_predict(m,x);
    auto weights=ftrl_weights(m);
    for(int j=0;j<int(m.z.size());j++){
        double feature=j==0?1:x[j-1];
        double gradient=(probability-outcome)*feature;
        double next=m.n[j]+gradient*gradient;
        double sigma=(std::sqrt(next)-std::sqrt(m.n[j]))/m.alpha;
        m.z[j]+=gradient-sigma*weights[j];
        m.n[j]=next;
    }
    return probability;
}
} // namespace kline
