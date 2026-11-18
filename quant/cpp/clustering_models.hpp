#pragma once
#include "learning.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <vector>

namespace kline {
struct KMeansResult {
    Matrix centers;
    std::vector<int> labels;
    double inertia=0;
};

inline double squared_distance(const std::vector<double>& a,
                               const std::vector<double>& b) {
    check_finite(a);check_finite(b);
    require(a.size()==b.size(),"Distance dimensions differ");
    double sum=0;
    for(int j=0;j<int(a.size());j++)sum+=std::pow(a[j]-b[j],2);
    return sum;
}

inline KMeansResult fit_kmeans(const Matrix& observations,int clusters,
                               int iterations=300) {
    require(!observations.empty() && iterations>0 && clusters>0 &&
            clusters<=int(observations.size()),"Invalid K-means dimensions");
    int n=int(observations.size()),p=int(observations[0].size());
    require(p>0,"K-means needs features");
    for(const auto& row:observations){
        require(int(row.size())==p,"K-means feature count changed");
        check_finite(row);
    }
    KMeansResult result;
    result.centers.push_back(observations[0]);
    while(int(result.centers.size())<clusters){
        int farthest=-1;
        double distance=-1;
        for(int i=0;i<n;i++){
            double nearest=std::numeric_limits<double>::infinity();
            for(const auto& center:result.centers)
                nearest=std::min(nearest,squared_distance(observations[i],center));
            if(nearest>distance){distance=nearest;farthest=i;}
        }
        require(distance>0,"K-means requires distinct initial centers");
        result.centers.push_back(observations[farthest]);
    }
    result.labels.assign(n,-1);
    for(int iter=0;iter<iterations;iter++){
        bool changed=false;
        for(int i=0;i<n;i++){
            int best=0;
            double distance=squared_distance(observations[i],result.centers[0]);
            for(int j=1;j<clusters;j++){
                double d=squared_distance(observations[i],result.centers[j]);
                if(d<distance){distance=d;best=j;}
            }
            if(result.labels[i]!=best){changed=true;result.labels[i]=best;}
        }
        Matrix centers(clusters,std::vector<double>(p,0));
        std::vector<int> counts(clusters,0);
        for(int i=0;i<n;i++){
            counts[result.labels[i]]++;
            for(int j=0;j<p;j++)centers[result.labels[i]][j]+=observations[i][j];
        }
        for(int cluster=0;cluster<clusters;cluster++){
            if(counts[cluster]==0)continue;
            for(int j=0;j<p;j++)centers[cluster][j]/=counts[cluster];
            result.centers[cluster]=centers[cluster];
        }
        if(!changed)break;
    }
    result.inertia=0;
    for(int i=0;i<n;i++)
        result.inertia+=squared_distance(observations[i],
                                         result.centers[result.labels[i]]);
    return result;
}

inline std::vector<double> distance_to_centers(
    const std::vector<double>& observation,const Matrix& centers) {
    require(!centers.empty(),"No cluster centers");
    std::vector<double> result;
    for(const auto& center:centers)
        result.push_back(std::sqrt(squared_distance(observation,center)));
    return result;
}
} // namespace kline
