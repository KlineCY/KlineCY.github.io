#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
namespace kline {
inline double sabr_lognormal_volatility(
    double forward,double strike,double maturity,
    double alpha,double beta,double rho,double nu){
    require(std::isfinite(forward)&&forward>0&&std::isfinite(strike)&&strike>0&&
      std::isfinite(maturity)&&maturity>=0&&std::isfinite(alpha)&&alpha>0&&
      std::isfinite(beta)&&beta>=0&&beta<=1&&
      std::isfinite(rho)&&std::fabs(rho)<1&&
      std::isfinite(nu)&&nu>=0,"Invalid SABR parameters");
    double one_minus=1-beta;
    double fk=forward*strike;
    double logfk=std::log(forward/strike);
    double log2=logfk*logfk;
    double factor=std::pow(fk,0.5*one_minus);
    double denominator=factor*(1+one_minus*one_minus*log2/24+
                    std::pow(one_minus,4)*log2*log2/1920);
    double z=(nu/alpha)*factor*logfk;
    double ratio=1;
    if(std::fabs(z)>1e-7&&nu>0){
        double x=std::log((std::sqrt(1-2*rho*z+z*z)+z-rho)/(1-rho));
        require(std::fabs(x)>1e-14,"Invalid SABR z/x");
        ratio=z/x;
    }else if(nu>0)ratio=1-0.5*rho*z+
                                (2-3*rho*rho)*z*z/12;
    double adjustment=1+maturity*(
        one_minus*one_minus*alpha*alpha/(24*std::pow(fk,one_minus))+
        rho*beta*nu*alpha/(4*factor)+
        (2-3*rho*rho)*nu*nu/24);
    return alpha/denominator*ratio*adjustment;
}
inline double sabr_atm_volatility(double forward,double maturity,double alpha,
                                  double beta,double rho,double nu){
    return sabr_lognormal_volatility(forward,forward,maturity,
                                    alpha,beta,rho,nu);
}
} // namespace kline
