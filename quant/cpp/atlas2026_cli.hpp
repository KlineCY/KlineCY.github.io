#pragma once
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
#include "io.hpp"
#include <iomanip>
#include <ostream>
#include <string>
namespace kline {
inline bool run_atlas2026_cli(const std::string& command,int argc,char** argv,std::ostream& out){
 if(command=="atlas-tail-tail-frequency"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::tail_frequency(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-tail-mean-excess"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::mean_excess(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-tail-conditional-tail-mean"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::conditional_tail_mean(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-tail-exceedance-second-moment"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::exceedance_second_moment(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-tail-weighted-expected-shortfall"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::weighted_expected_shortfall(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-robust-huber-objective"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::huber_objective(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-robust-pseudo-huber"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::pseudo_huber(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-robust-cauchy-objective"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::cauchy_objective(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-robust-scaled-log-cosh"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::scaled_log_cosh(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-robust-tukey-biweight"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::tukey_biweight(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-macro-after-tax-real-growth"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::after_tax_real_growth(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-macro-real-rate-gap"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::real_rate_gap(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-macro-taylor-zero-target"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::taylor_zero_target(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-macro-gdp-deflator-level"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::gdp_deflator_level(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-macro-cpi-deflated-wage"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::cpi_deflated_wage(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-forecast-wape-guarded"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::wape_guarded(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-forecast-symmetric-mape"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::symmetric_mape(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-forecast-mean-log-cosh-error"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::mean_log_cosh_error(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-forecast-epsilon-tube-mae"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::epsilon_tube_mae(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-forecast-threshold-class-hit"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::threshold_class_hit(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-dependence-downside-cohit"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::downside_cohit(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-dependence-conditional-downside"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::conditional_downside(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-dependence-upside-cohit"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::upside_cohit(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-dependence-conditional-downside-beta"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::conditional_downside_beta(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-dependence-downside-relative-spread"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::downside_relative_spread(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-execution-participation-headroom"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::participation_headroom(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-execution-overcap-window-share"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::overcap_window_share(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-execution-exceeding-market-volume"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::exceeding_market_volume(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-execution-peak-participation-utilization"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::peak_participation_utilization(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-execution-participation-target-deviation"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::participation_target_deviation(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-signals-signal-hit-rate"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::signal_hit_rate(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-signals-signal-conditional-payoff"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::signal_conditional_payoff(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-signals-signal-signed-return"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::signal_signed_return(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-signals-signal-top-bottom-spread"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::signal_top_bottom_spread(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-signals-signal-ic-excess"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::signal_ic_excess(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-cashflows-cashflow-present-value"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::cashflow_present_value(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-cashflows-cashflow-discounted-duration"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::cashflow_discounted_duration(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-cashflows-cashflow-time-dispersion"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::cashflow_time_dispersion(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-cashflows-cashflow-coverage-ratio"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::cashflow_coverage_ratio(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-cashflows-cashflow-discounted-payback"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::cashflow_discounted_payback(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-wealth-relative-log-wealth"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::relative_log_wealth(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-wealth-relative-wealth-max-drawdown"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::relative_wealth_max_drawdown(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-wealth-excess-return-hurdle-hit"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::excess_return_hurdle_hit(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-wealth-relative-volatility-drag"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::relative_volatility_drag(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-wealth-relative-terminal-wealth"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::relative_terminal_wealth(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-credit-portfolio-expected-credit-loss"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::portfolio_expected_credit_loss(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-credit-independent-credit-loss-variance"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::independent_credit_loss_variance(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-credit-high-lgd-expected-defaults"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::high_lgd_expected_defaults(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-credit-expected-credit-recovery"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::expected_credit_recovery(a,b,scalar)<<"\n";
  return true;
 }
 if(command=="atlas-credit-independent-credit-loss-sd"){
  require(argc==6,"Atlas command requires CSV, two columns and a scalar parameter");
  const auto data=read_csv(argv[2]);
  const auto a=data.numbers(argv[3]),b=data.numbers(argv[4]);
  const double scalar=parse_number(argv[5]);
  out<<"Value,"<<std::setprecision(16)<<atlas2026::independent_credit_loss_sd(a,b,scalar)<<"\n";
  return true;
 }
 return false;
}
inline void atlas2026_usage(std::ostream& out){
 out<<"Atlas research: atlas-AREA-METHOD CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-tail-tail-frequency CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-tail-mean-excess CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-tail-conditional-tail-mean CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-tail-exceedance-second-moment CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-tail-weighted-expected-shortfall CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-robust-huber-objective CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-robust-pseudo-huber CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-robust-cauchy-objective CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-robust-scaled-log-cosh CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-robust-tukey-biweight CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-macro-after-tax-real-growth CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-macro-real-rate-gap CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-macro-taylor-zero-target CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-macro-gdp-deflator-level CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-macro-cpi-deflated-wage CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-forecast-wape-guarded CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-forecast-symmetric-mape CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-forecast-mean-log-cosh-error CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-forecast-epsilon-tube-mae CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-forecast-threshold-class-hit CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-dependence-downside-cohit CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-dependence-conditional-downside CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-dependence-upside-cohit CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-dependence-conditional-downside-beta CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-dependence-downside-relative-spread CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-execution-participation-headroom CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-execution-overcap-window-share CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-execution-exceeding-market-volume CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-execution-peak-participation-utilization CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-execution-participation-target-deviation CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-signals-signal-hit-rate CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-signals-signal-conditional-payoff CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-signals-signal-signed-return CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-signals-signal-top-bottom-spread CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-signals-signal-ic-excess CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-cashflows-cashflow-present-value CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-cashflows-cashflow-discounted-duration CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-cashflows-cashflow-time-dispersion CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-cashflows-cashflow-coverage-ratio CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-cashflows-cashflow-discounted-payback CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-wealth-relative-log-wealth CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-wealth-relative-wealth-max-drawdown CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-wealth-excess-return-hurdle-hit CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-wealth-relative-volatility-drag CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-wealth-relative-terminal-wealth CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-credit-portfolio-expected-credit-loss CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-credit-independent-credit-loss-variance CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-credit-high-lgd-expected-defaults CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-credit-expected-credit-recovery CSV X_COLUMN Y_COLUMN PARAMETER\n";
 out<<"  atlas-credit-independent-credit-loss-sd CSV X_COLUMN Y_COLUMN PARAMETER\n";
}
}
