#pragma once
#include "covariance_regularization.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>
namespace kline {
inline double cluster_inverse_variance(const Matrix& cov,
                                        const std::vector<int>& ids){
    double sum=0;
    for(int id:ids){
        require(id>=0&&id<int(cov.size())&&cov[id][id]>0,
                "Hierarchical portfolio needs positive variances");
        sum+=1/cov[id][id];
    }
    double variance=0;
    for(int i:ids)for(int j:ids)
        variance+=(1/cov[i][i])*(1/cov[j][j])*
                     cov[i][j]/(sum*sum);
    return variance;
}
inline std::vector<double> hierarchical_risk_parity(const Matrix& cov){
    check_covariance(cov);
    int n=int(cov.size());
    require(n>=2,"HRP requires multiple assets");
    auto corr=covariance_correlation(cov);
    std::vector<std::vector<int>> clusters(n);
    for(int i=0;i<n;i++){
        require(cov[i][i]>0,"HRP requires positive variances");
        clusters[i]={i};
    }
    while(clusters.size()>1){
        double best=std::numeric_limits<double>::infinity();
        int p=-1,q=-1;
        for(int i=0;i<int(clusters.size());i++)
            for(int j=i+1;j<int(clusters.size());j++){
                double distance=0;
                for(int a:clusters[i])for(int b:clusters[j])
                    distance+=std::sqrt(std::max(0.0,(1-corr[a][b])*0.5));
                distance/=clusters[i].size()*clusters[j].size();
                if(distance<best){best=distance;p=i;q=j;}
            }
        require(p>=0,"Failed to form HRP cluster");
        clusters[p].insert(clusters[p].end(),
                           clusters[q].begin(),clusters[q].end());
        clusters.erase(clusters.begin()+q);
    }
    std::vector<int> order=clusters[0];
    std::vector<double> weights(n,1);
    std::vector<std::vector<int>> queue={order};
    while(!queue.empty()){
        std::vector<std::vector<int>> next;
        for(const auto& group:queue){
            if(group.size()<2)continue;
            int middle=int(group.size())/2;
            std::vector<int> left(group.begin(),group.begin()+middle);
            std::vector<int> right(group.begin()+middle,group.end());
            double a=cluster_inverse_variance(cov,left);
            double b=cluster_inverse_variance(cov,right);
            require(a+b>0,"Invalid HRP cluster variance");
            double left_fraction=b/(a+b);
            for(int id:left)weights[id]*=left_fraction;
            for(int id:right)weights[id]*=1-left_fraction;
            next.push_back(left);next.push_back(right);
        }
        queue=next;
    }
    double sum=std::accumulate(weights.begin(),weights.end(),0.0);
    for(double& weight:weights)weight/=sum;
    return weights;
}
} // namespace kline
