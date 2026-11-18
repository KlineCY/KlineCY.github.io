#pragma once
#include "cholesky_numerics.hpp"
#include "learning.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct LogisticFit{std::vector<double> beta;double log_loss=0;int iterations=0;};
inline double stable_sigmoid(double z){
    if(z>=0)return 1/(1+std::exp(-z));
    double e=std::exp(z);return e/(1+e);
}
inline double logistic_objective(const Matrix& x,const std::vector<int>& y,
      const std::vector<double>& beta,double ridge){
    int n=int(x.size()),p=int(beta.size());
    double loss=0;
    for(int i=0;i<n;i++){
        double z=beta[0];for(int j=1;j<p;j++)z+=beta[j]*x[i][j-1];
        loss+=std::max(0.0,z)+std::log1p(std::exp(-std::fabs(z)))-y[i]*z;
    }
    for(int j=1;j<p;j++)loss+=0.5*ridge*beta[j]*beta[j];
    return loss/n;
}
inline LogisticFit fit_logistic_irls(const Matrix& x,const std::vector<int>& labels,
              double ridge=0.01,int max_iterations=100){
    require(!x.empty()&&x.size()==labels.size(),"Logistic data shape");
    int n=int(x.size()),features=int(x[0].size()),p=features+1;
    require(features>0&&n>=p&&std::isfinite(ridge)&&ridge>0&&
            max_iterations>0,"Invalid IRLS parameters");
    for(const auto& row:x){require(int(row.size())==features,"Logit ragged features");check_finite(row);}
    for(int y:labels)require(y==0||y==1,"Logistic labels must be binary");
    std::vector<double> beta(p,0);
    double objective=logistic_objective(x,labels,beta,ridge);
    for(int step=0;step<max_iterations;step++){
        Matrix h(p,std::vector<double>(p,0));
        std::vector<double> gradient(p,0);
        for(int i=0;i<n;i++){
            std::vector<double> v={1};v.insert(v.end(),x[i].begin(),x[i].end());
            double z=0;for(int j=0;j<p;j++)z+=beta[j]*v[j];
            double probability=stable_sigmoid(z),weight=probability*(1-probability);
            for(int j=0;j<p;j++){
                gradient[j]+=(probability-labels[i])*v[j]/n;
                for(int k=0;k<=j;k++)h[j][k]+=weight*v[j]*v[k]/n;
            }
        }
        for(int j=0;j<p;j++){
            if(j>0){gradient[j]+=ridge*beta[j]/n;h[j][j]+=ridge/n;}
            for(int k=0;k<j;k++)h[k][j]=h[j][k];
        }
        h[0][0]+=1e-10;
        auto delta=solve_cholesky(h,gradient);
        double shrink=1;
        std::vector<double> next(p);
        bool accepted=false;
        for(int ls=0;ls<32;ls++){
            for(int j=0;j<p;j++)next[j]=beta[j]-shrink*delta[j];
            double candidate=logistic_objective(x,labels,next,ridge);
            if(std::isfinite(candidate)&&candidate<=objective+1e-12){
                objective=candidate;accepted=true;break;
            }
            shrink*=0.5;
        }
        require(accepted,"Logistic IRLS line search failed");
        double update=0;
        for(int j=0;j<p;j++)update=std::max(update,std::fabs(next[j]-beta[j]));
        beta=next;
        if(update<1e-9)return {beta,objective,step+1};
    }
    return {beta,objective,max_iterations};
}
inline double logistic_probability(const LogisticFit& m,
                                   const std::vector<double>& row){
    check_finite(row);
    require(row.size()+1==m.beta.size(),"Logistic prediction dimension");
    double z=m.beta[0];for(int j=0;j<int(row.size());j++)z+=row[j]*m.beta[j+1];
    return stable_sigmoid(z);
}
} // namespace kline
