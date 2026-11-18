#pragma once
#include "monte_carlo_precision.hpp"
#include <cmath>
#include <random>
#include <vector>
namespace kline {
inline MonteCarloEstimate down_and_out_call_monte_carlo(
    double spot,double strike,double barrier,double risk_free,
    double volatility,double years,int monitoring_steps,int paths,
    unsigned int seed){
    require(std::isfinite(spot)&&spot>0&&std::isfinite(strike)&&strike>0&&
            std::isfinite(barrier)&&barrier>0&&barrier<spot&&
            std::isfinite(risk_free)&&std::isfinite(volatility)&&
            volatility>=0&&std::isfinite(years)&&years>0&&
            monitoring_steps>=1&&monitoring_steps<=10000&&
            paths>=2&&paths<=1000000,
            "Invalid discretely monitored barrier call");
    std::mt19937 rng(seed);std::normal_distribution<double> normal(0,1);
    double dt=years/monitoring_steps;
    double drift=(risk_free-0.5*volatility*volatility)*dt;
    double diffusion=volatility*std::sqrt(dt);
    std::vector<double> payoffs;
    for(int p=0;p<paths;p++){
        double price=spot;bool knocked=false;
        for(int t=0;t<monitoring_steps;t++){
            price*=std::exp(drift+diffusion*normal(rng));
            if(price<=barrier)knocked=true;
        }
        payoffs.push_back(knocked?0:
            std::exp(-risk_free*years)*std::max(0.0,price-strike));
    }
    return monte_carlo_confidence(payoffs);
}
} // namespace kline
