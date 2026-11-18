#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct MonteCarloEstimate{
    double mean=0,standard_error=0,lower95=0,upper95=0;
};
inline MonteCarloEstimate monte_carlo_confidence(
    const std::vector<double>& payoffs){
    check_finite(payoffs);
    require(payoffs.size()>=2,"Monte Carlo needs independent draws");
    MonteCarloEstimate out;
    out.mean=mean(payoffs);
    out.standard_error=std::sqrt(variance(payoffs)/payoffs.size());
    out.lower95=out.mean-1.959963984540054*out.standard_error;
    out.upper95=out.mean+1.959963984540054*out.standard_error;
    return out;
}
inline double antithetic_call_payoff(double spot,double strike,double rate,
    double volatility,double time,double normal){
    require(std::isfinite(spot)&&spot>0&&std::isfinite(strike)&&strike>0&&
            std::isfinite(rate)&&std::isfinite(volatility)&&volatility>=0&&
            std::isfinite(time)&&time>0&&std::isfinite(normal),
            "Invalid antithetic option parameters");
    auto payoff=[&](double z){
        double terminal=spot*std::exp((rate-0.5*volatility*volatility)*time+
                    volatility*std::sqrt(time)*z);
        return std::max(0.0,terminal-strike);
    };
    return 0.5*std::exp(-rate*time)*(payoff(normal)+payoff(-normal));
}
} // namespace kline
