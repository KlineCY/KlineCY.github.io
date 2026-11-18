#pragma once
#include "atlas2026_common.hpp"
namespace kline { namespace atlas2026 {
inline double portfolio_expected_credit_loss(const Values& x,const Values& y,double p){valid(x,y,p);require(p>=0,"Exposure must be nonnegative");double s=0;for(std::size_t i=0;i<x.size();++i){require(x[i]>=0&&x[i]<=1&&y[i]>=0&&y[i]<=1,"Invalid PD/LGD");s+=p*x[i]*y[i];}return s;}
inline double independent_credit_loss_variance(const Values& x,const Values& y,double p){valid(x,y,p);require(p>=0,"Exposure must be nonnegative");double s=0;for(std::size_t i=0;i<x.size();++i){require(x[i]>=0&&x[i]<=1&&y[i]>=0&&y[i]<=1,"Invalid PD/LGD");double v=p*y[i];s+=v*v*x[i]*(1-x[i]);}return s;}
inline double high_lgd_expected_defaults(const Values& x,const Values& y,double p){valid(x,y,p);require(p>=0&&p<=1,"LGD selection threshold invalid");double s=0;for(std::size_t i=0;i<x.size();++i){require(x[i]>=0&&x[i]<=1&&y[i]>=0&&y[i]<=1,"Invalid PD/LGD");if(y[i]>p)s+=x[i];}return s;}
inline double expected_credit_recovery(const Values& x,const Values& y,double p){valid(x,y,p);require(p>=0,"Exposure must be nonnegative");double s=0;for(std::size_t i=0;i<x.size();++i){require(x[i]>=0&&x[i]<=1&&y[i]>=0&&y[i]<=1,"Invalid PD/LGD");s+=p*x[i]*(1-y[i]);}return s;}
inline double independent_credit_loss_sd(const Values& x,const Values& y,double p){valid(x,y,p);require(p>=0,"Exposure must be nonnegative");double s=0;for(std::size_t i=0;i<x.size();++i){require(x[i]>=0&&x[i]<=1&&y[i]>=0&&y[i]<=1,"Invalid PD/LGD");double v=p*y[i];s+=v*v*x[i]*(1-x[i]);}return std::sqrt(s);}
} }
