#pragma once
#include "learning.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct GaussianNaiveBayes{
    Matrix means,variances;
    std::vector<double> priors;
};
inline GaussianNaiveBayes fit_gaussian_naive_bayes(
    const Matrix& x,const std::vector<int>& labels,double floor_variance=1e-8){
    require(!x.empty()&&x.size()==labels.size()&&
            std::isfinite(floor_variance)&&floor_variance>0,
            "Invalid naive Bayes fit");
    int n=int(x.size()),p=int(x[0].size());
    require(p>0,"Naive Bayes needs features");
    GaussianNaiveBayes m;
    m.means.assign(2,std::vector<double>(p,0));
    m.variances.assign(2,std::vector<double>(p,floor_variance));
    std::vector<int> counts(2,0);
    for(int i=0;i<n;i++){
        require((labels[i]==0||labels[i]==1)&&int(x[i].size())==p,
                "Naive Bayes input shape or label");
        check_finite(x[i]);
        int c=labels[i];counts[c]++;
        for(int j=0;j<p;j++)m.means[c][j]+=x[i][j];
    }
    require(counts[0]>0&&counts[1]>0,"Both classes required");
    for(int c=0;c<2;c++)
        for(int j=0;j<p;j++)m.means[c][j]/=counts[c];
    for(int i=0;i<n;i++)
        for(int j=0;j<p;j++)
            m.variances[labels[i]][j]+=
               std::pow(x[i][j]-m.means[labels[i]][j],2)/counts[labels[i]];
    m.priors={double(counts[0])/n,double(counts[1])/n};
    return m;
}
inline double gaussian_nb_probability(const GaussianNaiveBayes& m,
                                      const std::vector<double>& features){
    check_finite(features);
    require(m.means.size()==2&&features.size()==m.means[0].size(),
            "Naive Bayes prediction shape");
    double logscore[2]={std::log(m.priors[0]),std::log(m.priors[1])};
    for(int c=0;c<2;c++)
        for(int j=0;j<int(features.size());j++){
            double v=m.variances[c][j],d=features[j]-m.means[c][j];
            logscore[c]-=0.5*(std::log(2*std::acos(-1.0)*v)+d*d/v);
        }
    double z=logscore[1]-logscore[0];
    return z>=0?1/(1+std::exp(-z)):std::exp(z)/(1+std::exp(z));
}
} // namespace kline
