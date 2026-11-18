#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace kline {
inline std::vector<double> daily_reset_leveraged_nav(const std::vector<double>& benchmarkReturns,
                                                     double leverage,double annualFee,
                                                     int periodsPerYear=252){
    check_finite(benchmarkReturns);
    require(std::isfinite(leverage)&&std::abs(leverage)<=10&&
            std::isfinite(annualFee)&&annualFee>=0&&annualFee<=1&&
            periodsPerYear>0,"Invalid daily-reset fund inputs");
    double nav=1;std::vector<double> path{nav};
    for(double r:benchmarkReturns){
        const double growth=1+leverage*r-annualFee/periodsPerYear;
        require(growth>0&&std::isfinite(growth),
                "Daily reset portfolio has exhausted capital");
        nav*=growth;require(std::isfinite(nav)&&nav>0,"Fund NAV overflow");
        path.push_back(nav);
    }
    return path;
}
struct DrawdownTailRisk{double quantile=0,expectedTail=0;};
inline DrawdownTailRisk simulated_drawdown_tail(
    const std::vector<std::vector<double>>& simulatedNav,double confidence){
    require(!simulatedNav.empty()&&std::isfinite(confidence)&&
            confidence>0&&confidence<1,"Invalid drawdown simulation inputs");
    std::vector<double> draws;
    for(const auto& nav:simulatedNav){
        check_finite(nav);require(nav.size()>=2,"Scenario must contain two NAV observations");
        double peak=nav.front(),worst=0;
        for(double x:nav){
            require(x>0,"NAV must be strictly positive");
            peak=std::max(peak,x);worst=std::max(worst,1-x/peak);
        }
        draws.push_back(worst);
    }
    std::sort(draws.begin(),draws.end());
    const std::size_t index=std::max(std::size_t(1),
        std::size_t(std::ceil(confidence*draws.size())))-1;
    const double q=draws[index],tail=(1-confidence)*draws.size();
    double sum=0,left=tail;
    for(auto it=draws.rbegin();it!=draws.rend();++it){
        const double weight=std::min(1.0,left);
        if(weight<=0)break;
        sum+=weight*(*it);left-=weight;
    }
    return {q,sum/tail};
}
struct MarginCall{double equity=0,maintenance=0,deficiency=0;};
inline MarginCall position_margin_call(double originalEquity,double signedShares,
                                        double entryPrice,double markPrice,
                                        double maintenanceFraction){
    require(std::isfinite(originalEquity)&&originalEquity>=0&&
            std::isfinite(signedShares)&&std::isfinite(entryPrice)&&entryPrice>0&&
            std::isfinite(markPrice)&&markPrice>0&&
            std::isfinite(maintenanceFraction)&&maintenanceFraction>=0&&
            maintenanceFraction<=1,"Invalid margin parameters");
    const double equity=originalEquity+signedShares*(markPrice-entryPrice);
    const double required=std::abs(signedShares)*markPrice*maintenanceFraction;
    return {equity,required,std::max(0.0,required-equity)};
}
inline double long_only_diversification_ratio(
    const std::vector<double>& weights,
    const std::vector<std::vector<double>>& covariance){
    check_finite(weights);
    require(weights.size()>=2&&covariance.size()==weights.size(),
            "Diversification requires multiple aligned assets");
    const std::size_t n=weights.size();
    for(std::size_t i=0;i<n;++i)
        require(covariance[i].size()==n,"Covariance matrix must be square");
    double sumWeight=0,weightedVol=0,variance=0;
    for(std::size_t i=0;i<n;++i){
        require(weights[i]>=0&&covariance[i].size()==n,
                "Long-only covariance has invalid dimensions");
        sumWeight+=weights[i];
        require(std::isfinite(covariance[i][i])&&covariance[i][i]>=0,
                "Covariance diagonal must be nonnegative");
        weightedVol+=weights[i]*std::sqrt(covariance[i][i]);
        for(std::size_t j=0;j<n;++j){
            require(std::isfinite(covariance[i][j])&&
                    std::abs(covariance[i][j]-covariance[j][i])<1e-8,
                    "Covariance matrix must be symmetric");
            require(std::abs(covariance[i][j])<=
                    std::sqrt(covariance[i][i]*covariance[j][j])+1e-8,
                    "Covariance violates pairwise Cauchy bound");
            variance+=weights[i]*weights[j]*covariance[i][j];
        }
    }
    require(std::abs(sumWeight-1)<1e-8&&variance>0,
            "Long-only weights or portfolio variance invalid");
    return weightedVol/std::sqrt(variance);
}
} // namespace kline
