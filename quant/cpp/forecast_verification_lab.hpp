#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <utility>
#include <vector>

namespace kline {
inline double pinball_mean_loss(const std::vector<double>& observed,
                                const std::vector<double>& forecast,double quantileLevel){
    check_finite(observed);check_finite(forecast);
    require(observed.size()==forecast.size()&&quantileLevel>0&&
            quantileLevel<1,"Invalid quantile forecast inputs");
    double sum=0;
    for(std::size_t i=0;i<observed.size();++i){
        const double error=observed[i]-forecast[i];
        sum+=error>=0?quantileLevel*error:(quantileLevel-1)*error;
    }
    return sum/observed.size();
}
inline double gaussian_crps_mean(const std::vector<double>& observed,
                                const std::vector<double>& predictedMean,
                                const std::vector<double>& predictedSd){
    check_finite(observed);check_finite(predictedMean);check_finite(predictedSd);
    require(observed.size()==predictedMean.size()&&
            observed.size()==predictedSd.size(),"Gaussian CRPS columns must align");
    const double pi=std::acos(-1.0),sqrtTwo=std::sqrt(2.0);
    double sum=0;
    for(std::size_t i=0;i<observed.size();++i){
        const double sd=predictedSd[i];require(sd>0,"Predictive SD must be positive");
        const double z=(observed[i]-predictedMean[i])/sd;
        const double cdf=0.5*std::erfc(-z/sqrtTwo);
        const double pdf=std::exp(-0.5*z*z)/std::sqrt(2*pi);
        sum+=sd*(z*(2*cdf-1)+2*pdf-1/std::sqrt(pi));
    }
    return sum/observed.size();
}
inline double average_precision_with_ties(const std::vector<double>& scores,
                                           const std::vector<double>& labels){
    check_finite(scores);check_finite(labels);
    require(scores.size()==labels.size(),"Precision columns must align");
    std::vector<std::pair<double,int>> pairs;int positive=0;
    for(std::size_t i=0;i<scores.size();++i){
        require(labels[i]==0||labels[i]==1,"Precision labels must be binary");
        positive+=int(labels[i]);pairs.push_back({scores[i],int(labels[i])});
    }
    require(positive>0,"Average precision needs a positive label");
    std::sort(pairs.begin(),pairs.end(),[](const auto& a,const auto& b){
        return a.first>b.first;
    });
    double area=0;int tp=0;
    for(std::size_t i=0;i<pairs.size();){
        std::size_t j=i;int groupPos=0;
        while(j<pairs.size()&&pairs[j].first==pairs[i].first){
            groupPos+=pairs[j].second;++j;
        }
        tp+=groupPos;
        area+=double(groupPos)/positive*(double(tp)/j);
        i=j;
    }
    return area;
}
inline double ordinal_ranked_probability_score(
    const std::vector<std::vector<double>>& probabilities,
    const std::vector<int>& outcomes){
    require(!probabilities.empty()&&probabilities.size()==outcomes.size(),
            "Categorical probability rows must align");
    const std::size_t groups=probabilities.front().size();
    require(groups>=2,"Ranked probability score needs two classes");
    double total=0;
    for(std::size_t i=0;i<probabilities.size();++i){
        require(probabilities[i].size()==groups&&outcomes[i]>=0&&
                std::size_t(outcomes[i])<groups,"Invalid ranked class");
        double cumulative=0,sum=0,loss=0;
        for(std::size_t j=0;j<groups;++j){
            const double probability=probabilities[i][j];
            require(std::isfinite(probability)&&probability>=0&&probability<=1,
                    "Invalid categorical probability");
            sum+=probability;cumulative+=probability;
            if(j+1<groups){
                const double target=outcomes[i]<=int(j)?1.0:0.0;
                loss+=(cumulative-target)*(cumulative-target);
            }
        }
        require(std::abs(sum-1)<1e-8,"Class probabilities must sum to one");
        total+=loss/(groups-1);
    }
    return total/probabilities.size();
}
} // namespace kline
