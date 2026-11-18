#pragma once
#include "credit_survival_curve.hpp"
#include <algorithm>
#include <cmath>
namespace kline {
struct CDSLegs{double premium_annuity=0,default_protection=0,fair_spread=0;};
inline CDSLegs cds_discrete_legs(
    const HazardCurve& curve,double maturity,double rate,
    double recovery,int intervals_per_year=4){
    require(std::isfinite(maturity)&&maturity>0&&std::isfinite(rate)&&
            std::isfinite(recovery)&&recovery>=0&&recovery<1&&
            intervals_per_year>=1&&intervals_per_year<=365,
            "Invalid CDS protection contract");
    int n=int(std::ceil(maturity*intervals_per_year));
    require(n>0&&n<=200000,"Excessive CDS time grid");
    double annuity=0,protection=0,prior=1,previous_time=0;
    for(int k=1;k<=n;k++){
        double t=std::min(maturity,k/double(intervals_per_year));
        double delta=t-previous_time;
        double s=survival_probability(curve,t);
        annuity+=delta*std::exp(-rate*t)*s;
        protection+=(prior-s)*std::exp(-rate*(previous_time+t)/2)*(1-recovery);
        prior=s;previous_time=t;
    }
    require(annuity>0,"Nonpositive CDS premium annuity");
    return {annuity,protection,protection/annuity};
}
} // namespace kline
