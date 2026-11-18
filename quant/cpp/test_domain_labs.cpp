#include "high_frequency_inference_lab.hpp"
#include "causal_robustness_lab.hpp"
#include "market_quality_lab.hpp"
#include "fixed_income_sensitivity_lab.hpp"
#include "classification_audit_lab.hpp"
#include "scenario_risk_attribution_lab.hpp"
#include "vintage_data_audit_lab.hpp"
#include "options_surface_diagnostics_lab.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace kline;
inline void near_domain(double a,double b,double tol=1e-8) {
 if(!std::isfinite(a)||std::abs(a-b)>tol)
  throw std::runtime_error("Crossdomain numerical identity failed");
}
template<class F>inline void rejected_domain(F f) {
 bool rejected=false;
 try{f();}catch(const std::invalid_argument&){rejected=true;}
 if(!rejected)throw std::runtime_error("Crossdomain invalid input was accepted");
}
int main(){
 near_domain(hayashi_yoshida_covariance({0,1},{1,2},{.1,.2},
      {0,.5},{.5,2},{.3,.4}),.1*.3+.1*.4+.2*.4);
 near_domain(realized_quarticity({1,2}),2./3*(1+16));
 near_domain(one_lag_bartlett_realized_kernel({1,2,3}),14+8);
 near_domain(truncated_realized_variance({.1,.3,-.2},.2),.05);
 rejected_domain([]{hayashi_yoshida_covariance({1},{0},{1},{0},{1},{1});});
 rejected_domain([]{realized_quarticity({});});
 rejected_domain([]{one_lag_bartlett_realized_kernel({1});});
 rejected_domain([]{truncated_realized_variance({1},0);});

 const std::vector<double> t={0,0,1,1},p={.5,.5,.5,.5},y={0,2,4,6};
 near_domain(horvitz_thompson_ate(t,y,p),4);
 near_domain(augmented_ipw_ate(t,y,p,{1,1,1,1},{5,5,5,5}),4);
 near_domain(standardized_mean_difference({0,2,4,6},t),4/std::sqrt(2));
 near_domain(ipw_effective_sample_size(t,p),4);
 rejected_domain([]{horvitz_thompson_ate({1,1,1,1},{1,2,3,4},{.5,.5,.5,.5});});
 rejected_domain([]{augmented_ipw_ate({0,0,1,1},{0,1,2,3},{0,.5,.5,.5},{0,0,0,0},{1,1,1,1});});
 rejected_domain([]{standardized_mean_difference({1,1,1,1},{0,0,1,1});});
 rejected_domain([]{ipw_effective_sample_size({0,0,1,1},{.5,.5,1,.5});});

 near_domain(quoted_spread_basis_points({99,99},{101,101}),20000.0*2/200);
 near_domain(liquidity_replenishment_ratio({1,3},{2,2}),1);
 near_domain(kyle_price_impact_slope({1,2,3},{3,5,7}),2);
 near_domain(lag_one_order_sign_correlation({1,1,-1,-1,1,1}),.16666666666666666,1e-8);
 rejected_domain([]{quoted_spread_basis_points({100},{99});});
 rejected_domain([]{liquidity_replenishment_ratio({0,0},{1,2});});
 rejected_domain([]{kyle_price_impact_slope({1,1,1},{1,2,3});});
 rejected_domain([]{lag_one_order_sign_correlation({1,1,1});});

 near_domain(continuously_compounded_forward_rate(1,std::exp(-.05),0,1),.05);
 near_domain(zero_coupon_dv01(100,0,1),100*(1-std::exp(-.0001)));
 near_domain(cashflow_macaulay_duration({1,2},{0,100},.02),2);
 near_domain(cashflow_continuous_convexity({1,2},{0,100},.02),4);
 rejected_domain([]{continuously_compounded_forward_rate(1,1,2,1);});
 rejected_domain([]{zero_coupon_dv01(0,.02,1);});
 rejected_domain([]{cashflow_macaulay_duration({2,1},{10,10},.02);});
 rejected_domain([]{cashflow_continuous_convexity({1,2},{0,0},.02);});

 near_domain(matthews_correlation({1,1,0,0},{1,0,1,0}),0);
 near_domain(macro_f1_score({0,1,2},{0,1,2},3),1);
 near_domain(balanced_multiclass_accuracy({0,1,2},{0,1,2},3),1);
 near_domain(multiclass_brier_score({{1,0,0},{0,1,0}},{0,1}),0);
 rejected_domain([]{matthews_correlation({1,1},{1,1});});
 rejected_domain([]{macro_f1_score({0,1},{0,2},2);});
 rejected_domain([]{balanced_multiclass_accuracy({0,0,1,1},{0,1,0,1},3);});
 rejected_domain([]{multiclass_brier_score({{.1,.2,.3}},{0});});

 auto er=euler_volatility_contributions({.5,.5},{{1,0},{0,1}});
 near_domain(er[0]+er[1],std::sqrt(.5));
 auto es=scenario_expected_shortfall_contributions({{1,0},{0,2}}, {.5,.5},.5);
 near_domain(es[0],0);near_domain(es[1],1);
 near_domain(quadratic_factor_stress_pnl({1,2},{.1,.2},{{1,0},{0,1}}),.525);
 near_domain(probability_weighted_scenario_loss({2,4},{.25,.75}),3.5);
 rejected_domain([]{euler_volatility_contributions({.5,.5},{{1,0},{0}});});
 rejected_domain([]{scenario_expected_shortfall_contributions({{1,0},{0,2}},{.5},.5);});
 rejected_domain([]{quadratic_factor_stress_pnl({1,2},{.1,.2},{{1}});});
 rejected_domain([]{probability_weighted_scenario_loss({1,2},{.1,.1});});

 near_domain(vintage_revision_rmse({1,2},{2,2}),std::sqrt(.5));
 near_domain(average_publication_delay({1,3},{2,5}),1.5);
 near_domain(stale_quote_fraction({0,1,4,5},1.5),1./3);
 near_domain(invalid_bid_ask_fraction({1,2,5},{2,1,5}),2./3);
 rejected_domain([]{vintage_revision_rmse({1},{1,2});});
 rejected_domain([]{average_publication_delay({2},{1});});
 rejected_domain([]{stale_quote_fraction({2,1},1);});
 rejected_domain([]{invalid_bid_ask_fraction({1},{1,2});});

 near_domain(black_scholes_cash_digital_call(100,100,0,.0,.2,1),
              .5*std::erfc(.1/std::sqrt(2.0)));
 const double vanna=black_scholes_vanna_call(100,100,0,0,.2,1);
 const double volga=black_scholes_volga_call(100,100,0,0,.2,1);
 if(!(vanna>0&&volga<0))throw std::runtime_error("Vanna and volga sign");
 near_domain(annualized_forward_variance(1,.2,2,.3),.14);
 rejected_domain([]{black_scholes_cash_digital_call(0,100,0,0,.2,1);});
 rejected_domain([]{black_scholes_vanna_call(100,100,0,0,0,1);});
 rejected_domain([]{black_scholes_volga_call(100,100,0,0,.2,0);});
 rejected_domain([]{annualized_forward_variance(1,.3,2,.1);});
 std::cout<<"PASS: 32 research functions, independent reference identities and invalid-input tests\n";
}
