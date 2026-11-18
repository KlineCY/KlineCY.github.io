#include "range_volatility_lab.hpp"
#include "forecast_verification_lab.hpp"
#include "causal_inference_lab.hpp"
#include "credit_cashflow_lab.hpp"
#include "execution_dynamics_lab.hpp"
#include "portfolio_path_lab.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace kline;
void near(double observed,double expected,double tolerance=1e-8){
    if(!std::isfinite(observed)||std::abs(observed-expected)>tolerance)
        throw std::runtime_error("Empirical research numerical identity failed");
}
template<class F>void rejects(F f){
    bool caught=false;
    try{f();}catch(const std::invalid_argument&){caught=true;}
    if(!caught)throw std::runtime_error("Invalid empirical input was not rejected");
}
int main(){
    const double pi=std::acos(-1.0);
    near(parkinson_intraday_variance({100},{100}),0);
    near(parkinson_intraday_variance({110},{90}),
         std::pow(std::log(110.0/90),2)/(4*std::log(2.0)));
    near(garman_klass_intraday_variance({100},{110},{90},{100}),
         0.5*std::pow(std::log(110.0/90),2));
    near(rogers_satchell_intraday_variance({100},{110},{90},{100}),
         std::pow(std::log(1.1),2)+std::pow(std::log(.9),2));
    near(overnight_gap_second_moment({110},{100}),
         std::pow(std::log(1.1),2));
    rejects([]{parkinson_intraday_variance({-1},{1});});
    rejects([]{garman_klass_intraday_variance({100},{101},{90},{110});});
    rejects([]{overnight_gap_second_moment({1},{0});});

    near(pinball_mean_loss({1,2},{0,3},.5),.5);
    near(gaussian_crps_mean({0},{0},{1}),
         (std::sqrt(2)-1)/std::sqrt(pi));
    near(average_precision_with_ties({.9,.8,.8,.1},{1,0,1,0}),5.0/6);
    near(ordinal_ranked_probability_score({{1,0,0},{0,0,1}},{0,0}),.5);
    rejects([]{pinball_mean_loss({1},{1},0);});
    rejects([]{gaussian_crps_mean({1},{1},{0});});
    rejects([]{average_precision_with_ties({1,2},{0,0});});
    rejects([]{ordinal_ranked_probability_score({{.2,.3,.3}},{0});});

    const std::vector<double> x={-.8,-.6,-.4,-.2,0,.2,.4,.6};
    std::vector<double> y;
    for(double v:x)y.push_back((v<0?2:5)+3*v);
    const auto rd=regression_discontinuity_triangular(x,y,0,1);
    near(rd.treatmentEffect,3,1e-7);near(rd.leftIntercept,2,1e-7);
    near(rd.rightIntercept,5,1e-7);
    near(wald_instrumental_variable({0,0,1,1},{1,1,3,3},{3,3,7,7}),2);
    auto meta=inverse_variance_meta({1,3},{1,1});
    near(meta.effect,2);near(meta.standardError,std::sqrt(.5));
    near(meta.heterogeneityQ,2);near(meta.iSquared,.5);
    auto synth=synthetic_control_simplex({1,2,3},{{1,3},{2,2},{3,1}});
    near(synth.weights[0],1,1e-6);near(synth.weights[1],0,1e-6);
    near(synth.inSampleRmse,0,1e-6);
    rejects([]{regression_discontinuity_triangular({1,2,3},{1,2,3},0,1);});
    rejects([]{wald_instrumental_variable({0,0,1,1},{1,1,1,1},{3,3,7,7});});
    rejects([]{inverse_variance_meta({1,3},{1,0});});
    rejects([]{synthetic_control_simplex({1,2,3},{{0,0},{0,0},{0,0}});});

    near(bond_accrued_coupon(1000,.06,2,45,90),15);
    near(overnight_compounded_discount({.05,.04},{.5,.5}),1/(1.025*1.02));
    near(merton_structural_default_probability(100,100,.02,.2,1),.5);
    near(risky_zero_coupon_recovery_of_par(100,0,0,.4,2),100);
    near(risky_zero_coupon_recovery_of_par(100,0,.1,0,1),100*std::exp(-.1));
    rejects([]{bond_accrued_coupon(1000,.05,2,91,90);});
    rejects([]{overnight_compounded_discount({-4},{.5});});
    rejects([]{merton_structural_default_probability(100,100,0,0,1);});
    rejects([]{risky_zero_coupon_recovery_of_par(100,.02,.1,1.2,1);});

    near(poisson_queue_completion_probability(1,1,0,1),1-std::exp(-1));
    near(poisson_queue_completion_probability(0,1,0,1),0);
    auto impact=exponential_transient_impact({1,0,-1},2,.5);
    near(impact[0],2);near(impact[1],1);near(impact[2],-1.5);
    auto sig=realized_variance_signature({100,110,100},2);
    near(sig.coarse,0);if(sig.finest<=0)throw std::runtime_error("Positive fine RV required");
    auto bucket=fixed_volume_signed_imbalance({3,7},{1,-1},5);
    if(bucket.size()!=2)throw std::runtime_error("Volume bucket count");
    near(bucket[0],.2);near(bucket[1],1);
    rejects([]{poisson_queue_completion_probability(-1,1,0,1);});
    rejects([]{exponential_transient_impact({1},1,1.1);});
    rejects([]{realized_variance_signature({1,0,1},1);});
    rejects([]{fixed_volume_signed_imbalance({3},{1},0);});

    auto nav=daily_reset_leveraged_nav({.1,-.1},2,0);
    near(nav[0],1);near(nav[1],1.2);near(nav[2],.96);
    auto tail=simulated_drawdown_tail({{100,80,90},{100,90,100}},.5);
    near(tail.quantile,.1);near(tail.expectedTail,.2);
    auto margin=position_margin_call(50,10,100,90,.1);
    near(margin.equity,-50);near(margin.maintenance,90);
    near(margin.deficiency,140);
    near(long_only_diversification_ratio({.5,.5},{{1,0},{0,1}}),std::sqrt(2.0));
    rejects([]{daily_reset_leveraged_nav({-.6},2,0);});
    rejects([]{simulated_drawdown_tail({{100,0}},.9);});
    rejects([]{position_margin_call(10,2,100,-1,.2);});
    rejects([]{long_only_diversification_ratio({1.1,-.1},{{1,0},{0,1}});});
    rejects([]{long_only_diversification_ratio({.5,.5},{{1,0},{0}});});
    std::cout<<"PASS: 24 quantitative research algorithms and numerical boundary tests\n";
}
