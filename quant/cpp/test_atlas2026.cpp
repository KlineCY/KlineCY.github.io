#include "atlas2026_tail.hpp"
#include "atlas2026_robust.hpp"
#include "atlas2026_macro.hpp"
#include "atlas2026_forecast.hpp"
#include "atlas2026_dependence.hpp"
#include "atlas2026_execution.hpp"
#include "atlas2026_signals.hpp"
#include "atlas2026_cashflows.hpp"
#include "atlas2026_wealth.hpp"
#include "atlas2026_credit.hpp"
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace kline;
void near(double actual,double target,double epsilon){
 if(!std::isfinite(actual)||std::abs(actual-target)>epsilon)
    throw std::runtime_error("Atlas 2026 mathematical identity did not hold");
}
template<class F>void rejects(F f){
 bool rejected=false;
 try{f();}catch(const std::invalid_argument&){rejected=true;}
 if(!rejected)throw std::runtime_error("Atlas 2026 accepted invalid input");
}
int main(){
  near(atlas2026::tail_frequency({1,2,3,4},{1,1,1,1},2.5),0.5,1e-7);
  rejects([]{atlas2026::tail_frequency({}, {1},2.5);});
  rejects([]{atlas2026::tail_frequency({1,2,3,4},{1,1,1,1},std::numeric_limits<double>::infinity());});
  near(atlas2026::mean_excess({1,2,3,4},{1,1,1,1},2.5),1,1e-7);
  rejects([]{atlas2026::mean_excess({}, {1},2.5);});
  rejects([]{atlas2026::mean_excess({1,2,3,4},{1,1,1,1},std::numeric_limits<double>::infinity());});
  near(atlas2026::conditional_tail_mean({1,2,3,4},{1,1,1,1},2.5),3.5,1e-7);
  rejects([]{atlas2026::conditional_tail_mean({}, {1},2.5);});
  rejects([]{atlas2026::conditional_tail_mean({1,2,3,4},{1,1,1,1},std::numeric_limits<double>::infinity());});
  near(atlas2026::exceedance_second_moment({1,2,3,4},{1,1,1,1},2.5),0.625,1e-7);
  rejects([]{atlas2026::exceedance_second_moment({}, {1},2.5);});
  rejects([]{atlas2026::exceedance_second_moment({1,2,3,4},{1,1,1,1},std::numeric_limits<double>::infinity());});
  near(atlas2026::weighted_expected_shortfall({1,2,3,4},{1,1,1,1},0.5),3.5,1e-7);
  rejects([]{atlas2026::weighted_expected_shortfall({}, {1},0.5);});
  rejects([]{atlas2026::weighted_expected_shortfall({1,2,3,4},{1,1,1,1},std::numeric_limits<double>::infinity());});
  near(atlas2026::huber_objective({0,2},{0,0},1),0.75,1e-7);
  rejects([]{atlas2026::huber_objective({}, {1},1);});
  rejects([]{atlas2026::huber_objective({0,2},{0,0},std::numeric_limits<double>::infinity());});
  near(atlas2026::pseudo_huber({0,2},{0,0},1),0.6180339887498949,1e-7);
  rejects([]{atlas2026::pseudo_huber({}, {1},1);});
  rejects([]{atlas2026::pseudo_huber({0,2},{0,0},std::numeric_limits<double>::infinity());});
  near(atlas2026::cauchy_objective({0,2},{0,0},1),0.40235947810852507,1e-7);
  rejects([]{atlas2026::cauchy_objective({}, {1},1);});
  rejects([]{atlas2026::cauchy_objective({0,2},{0,0},std::numeric_limits<double>::infinity());});
  near(atlas2026::scaled_log_cosh({0,1},{0,0},1),0.21689041524151356,1e-7);
  rejects([]{atlas2026::scaled_log_cosh({}, {1},1);});
  rejects([]{atlas2026::scaled_log_cosh({0,1},{0,0},std::numeric_limits<double>::infinity());});
  near(atlas2026::tukey_biweight({0,2},{0,0},1),0.08333333333333333,1e-7);
  rejects([]{atlas2026::tukey_biweight({}, {1},1);});
  rejects([]{atlas2026::tukey_biweight({0,2},{0,0},std::numeric_limits<double>::infinity());});
  near(atlas2026::after_tax_real_growth({0.1},{0.02},0.1),0.06862745098039214,1e-7);
  rejects([]{atlas2026::after_tax_real_growth({}, {1},0.1);});
  rejects([]{atlas2026::after_tax_real_growth({0.1},{0.02},std::numeric_limits<double>::infinity());});
  near(atlas2026::real_rate_gap({0.1},{0.02},0.02),0.05843137254901977,1e-7);
  rejects([]{atlas2026::real_rate_gap({}, {1},0.02);});
  rejects([]{atlas2026::real_rate_gap({0.1},{0.02},std::numeric_limits<double>::infinity());});
  near(atlas2026::taylor_zero_target({0.02},{0.01},0.02),0.055,1e-7);
  rejects([]{atlas2026::taylor_zero_target({}, {1},0.02);});
  rejects([]{atlas2026::taylor_zero_target({0.02},{0.01},std::numeric_limits<double>::infinity());});
  near(atlas2026::gdp_deflator_level({120},{100},100),120,1e-7);
  rejects([]{atlas2026::gdp_deflator_level({}, {1},100);});
  rejects([]{atlas2026::gdp_deflator_level({120},{100},std::numeric_limits<double>::infinity());});
  near(atlas2026::cpi_deflated_wage({110},{110},100),100,1e-7);
  rejects([]{atlas2026::cpi_deflated_wage({}, {1},100);});
  rejects([]{atlas2026::cpi_deflated_wage({110},{110},std::numeric_limits<double>::infinity());});
  near(atlas2026::wape_guarded({1,3},{2,2},1),0.5,1e-7);
  rejects([]{atlas2026::wape_guarded({}, {1},1);});
  rejects([]{atlas2026::wape_guarded({1,3},{2,2},std::numeric_limits<double>::infinity());});
  near(atlas2026::symmetric_mape({1,3},{2,2},1),0.6666666666666666,1e-7);
  rejects([]{atlas2026::symmetric_mape({}, {1},1);});
  rejects([]{atlas2026::symmetric_mape({1,3},{2,2},std::numeric_limits<double>::infinity());});
  near(atlas2026::mean_log_cosh_error({0,1},{0,0},1),0.21689041524151356,1e-7);
  rejects([]{atlas2026::mean_log_cosh_error({}, {1},1);});
  rejects([]{atlas2026::mean_log_cosh_error({0,1},{0,0},std::numeric_limits<double>::infinity());});
  near(atlas2026::epsilon_tube_mae({1,3},{2,2},0.5),0.5,1e-7);
  rejects([]{atlas2026::epsilon_tube_mae({}, {1},0.5);});
  rejects([]{atlas2026::epsilon_tube_mae({1,3},{2,2},std::numeric_limits<double>::infinity());});
  near(atlas2026::threshold_class_hit({1,3},{2,2},2.5),0.5,1e-7);
  rejects([]{atlas2026::threshold_class_hit({}, {1},2.5);});
  rejects([]{atlas2026::threshold_class_hit({1,3},{2,2},std::numeric_limits<double>::infinity());});
  near(atlas2026::downside_cohit({-2,-1,1,2},{-3,1,-1,3},0),0.25,1e-7);
  rejects([]{atlas2026::downside_cohit({}, {1},0);});
  rejects([]{atlas2026::downside_cohit({-2,-1,1,2},{-3,1,-1,3},std::numeric_limits<double>::infinity());});
  near(atlas2026::conditional_downside({-2,-1,1,2},{-3,1,-1,3},0),0.5,1e-7);
  rejects([]{atlas2026::conditional_downside({}, {1},0);});
  rejects([]{atlas2026::conditional_downside({-2,-1,1,2},{-3,1,-1,3},std::numeric_limits<double>::infinity());});
  near(atlas2026::upside_cohit({-2,-1,1,2},{-3,1,-1,3},0),0.25,1e-7);
  rejects([]{atlas2026::upside_cohit({}, {1},0);});
  rejects([]{atlas2026::upside_cohit({-2,-1,1,2},{-3,1,-1,3},std::numeric_limits<double>::infinity());});
  near(atlas2026::conditional_downside_beta({-2,-1,1,2},{-3,1,-1,3},0),4,1e-7);
  rejects([]{atlas2026::conditional_downside_beta({}, {1},0);});
  rejects([]{atlas2026::conditional_downside_beta({-2,-1,1,2},{-3,1,-1,3},std::numeric_limits<double>::infinity());});
  near(atlas2026::downside_relative_spread({-2,-1,1,2},{-3,1,-1,3},0),0.5,1e-7);
  rejects([]{atlas2026::downside_relative_spread({}, {1},0);});
  rejects([]{atlas2026::downside_relative_spread({-2,-1,1,2},{-3,1,-1,3},std::numeric_limits<double>::infinity());});
  near(atlas2026::participation_headroom({1,3,1,2},{10,10,5,10},0.2),0,1e-7);
  rejects([]{atlas2026::participation_headroom({}, {1},0.2);});
  rejects([]{atlas2026::participation_headroom({1,3,1,2},{10,10,5,10},std::numeric_limits<double>::infinity());});
  near(atlas2026::overcap_window_share({1,3,1,2},{10,10,5,10},0.2),0.25,1e-7);
  rejects([]{atlas2026::overcap_window_share({}, {1},0.2);});
  rejects([]{atlas2026::overcap_window_share({1,3,1,2},{10,10,5,10},std::numeric_limits<double>::infinity());});
  near(atlas2026::exceeding_market_volume({1,3,1,2},{10,10,5,10},0.2),1,1e-7);
  rejects([]{atlas2026::exceeding_market_volume({}, {1},0.2);});
  rejects([]{atlas2026::exceeding_market_volume({1,3,1,2},{10,10,5,10},std::numeric_limits<double>::infinity());});
  near(atlas2026::peak_participation_utilization({1,3,1,2},{10,10,5,10},0.2),1.5,1e-7);
  rejects([]{atlas2026::peak_participation_utilization({}, {1},0.2);});
  rejects([]{atlas2026::peak_participation_utilization({1,3,1,2},{10,10,5,10},std::numeric_limits<double>::infinity());});
  near(atlas2026::participation_target_deviation({1,3,1,2},{10,10,5,10},0.2),0.005,1e-7);
  rejects([]{atlas2026::participation_target_deviation({}, {1},0.2);});
  rejects([]{atlas2026::participation_target_deviation({1,3,1,2},{10,10,5,10},std::numeric_limits<double>::infinity());});
  near(atlas2026::signal_hit_rate({1,2,3,4},{-1,1,2,3},2.5),1,1e-7);
  rejects([]{atlas2026::signal_hit_rate({}, {1},2.5);});
  rejects([]{atlas2026::signal_hit_rate({1,2,3,4},{-1,1,2,3},std::numeric_limits<double>::infinity());});
  near(atlas2026::signal_conditional_payoff({1,2,3,4},{-1,1,2,3},2.5),2.5,1e-7);
  rejects([]{atlas2026::signal_conditional_payoff({}, {1},2.5);});
  rejects([]{atlas2026::signal_conditional_payoff({1,2,3,4},{-1,1,2,3},std::numeric_limits<double>::infinity());});
  near(atlas2026::signal_signed_return({1,2,3,4},{-1,1,2,3},2.5),1.25,1e-7);
  rejects([]{atlas2026::signal_signed_return({}, {1},2.5);});
  rejects([]{atlas2026::signal_signed_return({1,2,3,4},{-1,1,2,3},std::numeric_limits<double>::infinity());});
  near(atlas2026::signal_top_bottom_spread({1,2,3,4},{-1,1,2,3},0.25),4,1e-7);
  rejects([]{atlas2026::signal_top_bottom_spread({}, {1},0.25);});
  rejects([]{atlas2026::signal_top_bottom_spread({1,2,3,4},{-1,1,2,3},std::numeric_limits<double>::infinity());});
  near(atlas2026::signal_ic_excess({1,2,3,4},{1,2,3,4},0.1),0.9,1e-7);
  rejects([]{atlas2026::signal_ic_excess({}, {1},0.1);});
  rejects([]{atlas2026::signal_ic_excess({1,2,3,4},{1,2,3,4},std::numeric_limits<double>::infinity());});
  near(atlas2026::cashflow_present_value({1,2},{10,20},0),30,1e-7);
  rejects([]{atlas2026::cashflow_present_value({}, {1},0);});
  rejects([]{atlas2026::cashflow_present_value({1,2},{10,20},std::numeric_limits<double>::infinity());});
  near(atlas2026::cashflow_discounted_duration({1,2},{10,20},0),1.6666666666666667,1e-7);
  rejects([]{atlas2026::cashflow_discounted_duration({}, {1},0);});
  rejects([]{atlas2026::cashflow_discounted_duration({1,2},{10,20},std::numeric_limits<double>::infinity());});
  near(atlas2026::cashflow_time_dispersion({1,2},{10,20},0),0.4714045207910317,1e-7);
  rejects([]{atlas2026::cashflow_time_dispersion({}, {1},0);});
  rejects([]{atlas2026::cashflow_time_dispersion({1,2},{10,20},std::numeric_limits<double>::infinity());});
  near(atlas2026::cashflow_coverage_ratio({0,1,2},{-10,5,10},0),1.5,1e-7);
  rejects([]{atlas2026::cashflow_coverage_ratio({}, {1},0);});
  rejects([]{atlas2026::cashflow_coverage_ratio({0,1,2},{-10,5,10},std::numeric_limits<double>::infinity());});
  near(atlas2026::cashflow_discounted_payback({0,1,2},{-10,5,10},0),1.5,1e-7);
  rejects([]{atlas2026::cashflow_discounted_payback({}, {1},0);});
  rejects([]{atlas2026::cashflow_discounted_payback({0,1,2},{-10,5,10},std::numeric_limits<double>::infinity());});
  near(atlas2026::relative_log_wealth({0.1,-0.1},{0,0},0),-0.01005033585350145,1e-7);
  rejects([]{atlas2026::relative_log_wealth({}, {1},0);});
  rejects([]{atlas2026::relative_log_wealth({0.1,-0.1},{0,0},std::numeric_limits<double>::infinity());});
  near(atlas2026::relative_wealth_max_drawdown({0.1,-0.1},{0,0},0),0.1,1e-7);
  rejects([]{atlas2026::relative_wealth_max_drawdown({}, {1},0);});
  rejects([]{atlas2026::relative_wealth_max_drawdown({0.1,-0.1},{0,0},std::numeric_limits<double>::infinity());});
  near(atlas2026::excess_return_hurdle_hit({0.1,-0.1},{0,0},0.05),0.5,1e-7);
  rejects([]{atlas2026::excess_return_hurdle_hit({}, {1},0.05);});
  rejects([]{atlas2026::excess_return_hurdle_hit({0.1,-0.1},{0,0},std::numeric_limits<double>::infinity());});
  near(atlas2026::relative_volatility_drag({0.1,-0.1},{0,0},0),0.005012562893380035,1e-7);
  rejects([]{atlas2026::relative_volatility_drag({}, {1},0);});
  rejects([]{atlas2026::relative_volatility_drag({0.1,-0.1},{0,0},std::numeric_limits<double>::infinity());});
  near(atlas2026::relative_terminal_wealth({0.1,-0.1},{0,0},0),0.99,1e-7);
  rejects([]{atlas2026::relative_terminal_wealth({}, {1},0);});
  rejects([]{atlas2026::relative_terminal_wealth({0.1,-0.1},{0,0},std::numeric_limits<double>::infinity());});
  near(atlas2026::portfolio_expected_credit_loss({0.1,0.2},{0.5,0.25},100),10,1e-7);
  rejects([]{atlas2026::portfolio_expected_credit_loss({}, {1},100);});
  rejects([]{atlas2026::portfolio_expected_credit_loss({0.1,0.2},{0.5,0.25},std::numeric_limits<double>::infinity());});
  near(atlas2026::independent_credit_loss_variance({0.1,0.2},{0.5,0.25},100),325,1e-7);
  rejects([]{atlas2026::independent_credit_loss_variance({}, {1},100);});
  rejects([]{atlas2026::independent_credit_loss_variance({0.1,0.2},{0.5,0.25},std::numeric_limits<double>::infinity());});
  near(atlas2026::high_lgd_expected_defaults({0.1,0.2},{0.5,0.25},0.3),0.1,1e-7);
  rejects([]{atlas2026::high_lgd_expected_defaults({}, {1},0.3);});
  rejects([]{atlas2026::high_lgd_expected_defaults({0.1,0.2},{0.5,0.25},std::numeric_limits<double>::infinity());});
  near(atlas2026::expected_credit_recovery({0.1,0.2},{0.5,0.25},100),20,1e-7);
  rejects([]{atlas2026::expected_credit_recovery({}, {1},100);});
  rejects([]{atlas2026::expected_credit_recovery({0.1,0.2},{0.5,0.25},std::numeric_limits<double>::infinity());});
  near(atlas2026::independent_credit_loss_sd({0.1,0.2},{0.5,0.25},100),18.027756377319946,1e-7);
  rejects([]{atlas2026::independent_credit_loss_sd({}, {1},100);});
  rejects([]{atlas2026::independent_credit_loss_sd({0.1,0.2},{0.5,0.25},std::numeric_limits<double>::infinity());});
 std::cout<<"PASS: 50 mathematical research estimators and 100 invalid-input guards\n";
}
