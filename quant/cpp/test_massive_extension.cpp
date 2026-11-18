#include "variance_ratio_diagnostics.hpp"
#include "ljung_box_diagnostics.hpp"
#include "normality_diagnostics.hpp"
#include "divergence_measures.hpp"
#include "tail_coexceedance.hpp"
#include "hac_mean_inference.hpp"
#include "kendall_rank_dependence.hpp"
#include "forecast_loss_tests.hpp"
#include "drawdown_duration.hpp"
#include "markov_transition_estimation.hpp"
#include "kaplan_meier_survival.hpp"
#include "high_watermark_fees.hpp"
#include "cash_flow_time_weighted.hpp"
#include "capacity_profitability.hpp"
#include "covariance_diagonal_shrinkage.hpp"
#include "long_only_risk_parity.hpp"
#include "portfolio_rebalance_bands.hpp"
#include "kelly_drawdown_bounds.hpp"
#include "portfolio_concentration.hpp"
#include "fixed_payment_amortization.hpp"
#include "factor_information_ratio.hpp"
#include "tax_lot_holding_periods.hpp"
#include "european_parity_bounds.hpp"
#include "american_binomial_tree.hpp"
#include "black76_futures_greeks.hpp"
#include "bachelier_normal_options.hpp"
#include "bridge_barrier_crossing.hpp"
#include "implied_volatility_solver.hpp"
#include "option_vertical_no_arbitrage.hpp"
#include "dupire_local_variance.hpp"
#include "cds_protection_legs.hpp"
#include "fx_triangular_consistency.hpp"
#include "futures_daily_settlement.hpp"
#include "poisson_glm_sgd.hpp"
#include "multiclass_log_loss.hpp"
#include "probability_reliability_bins.hpp"
#include "prediction_interval_coverage.hpp"
#include "online_moments_state.hpp"
#include "regression_prediction_metrics.hpp"
#include "binary_decision_utility.hpp"
#include "orderbook_market_sweep.hpp"
#include "vwap_execution_benchmark.hpp"
#include "trade_arrival_durations.hpp"
#include "volume_bars.hpp"
#include "inventory_carry_risk.hpp"
#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace std;
int count_checks=0;
void ensure(bool ok,const char* reason){count_checks++;if(!ok)throw runtime_error(reason);}
void near(double x,double expected,double tolerance,const char* reason){
    ensure(isfinite(x)&&fabs(x-expected)<=tolerance,reason);
}
void invalid(const function<void()>& function,const char* reason){
    bool threw=false;try{function();}catch(const exception&){threw=true;}
    ensure(threw,reason);
}
int main(){
try{
    using namespace kline;
    vector<double> x={1,-1,2,-2,1.5,-1.5,2.5,-2.5,1.2,-1.2,
                      2.1,-2.1,0.8,-0.8,1.7,-1.7,0.5,-0.5,1.1,-1.1};
    near(overlapping_variance_ratio({1,1,1,2,2,2,3,3,3,4,4,4},2),
         overlapping_variance_ratio({1,1,1,2,2,2,3,3,3,4,4,4},2),1e-14,
         "Variance ratio deterministic");
    ensure(overlapping_variance_ratio(x,2)>0,"Variance ratio positive");
    invalid([&]{overlapping_variance_ratio(x,11);},"Variance ratio excessive lags");
    near(ljung_box_q_statistic(x,2),ljung_box_q_statistic(x,2),1e-12,
         "Ljung Box deterministic lag sum");
    ensure(ljung_box_q_statistic(x,2)>0,"Ljung Box detects autocorrelation");
    auto normal=normality_moments({-2,-1,0,1,2});
    near(normal.skewness,0,1e-12,"Symmetric sample skew zero");
    ensure(normal.jarque_bera>=0,"Jarque Bera nonnegative");
    near(discrete_entropy({0.5,0.5}),log(2),1e-12,"Binary uniform entropy");
    near(kl_divergence({0.5,0.5},{0.5,0.5}),0,1e-12,
         "KL zero for equal mass");
    near(jensen_shannon_divergence({1,0},{0,1}),log(2),1e-12,
         "Jensen Shannon disjoint mass");
    invalid([]{kl_divergence({1,0},{0,1});},"KL zero reference rejected");
    auto monotone=vector<double>{1,2,3,4,5,6,7,8,9,10};
    near(lower_tail_conditional_exceedance(monotone,monotone,0.2),
         1,1e-12,"Perfect lower tail coexceedance");
    near(upper_tail_conditional_exceedance(monotone,monotone,0.2),
         1,1e-12,"Perfect upper tail coexceedance");
    auto hac=bartlett_hac_mean({1,2,3,4,5},0);
    near(hac.estimate,3,1e-12,"HAC mean");
    near(hac.long_run_variance,2,1e-12,"HAC lag zero population variance");
    near(hac.standard_error,sqrt(2.0/5),1e-12,"HAC mean standard error");
    near(kendall_tau_b({1,2,3,4},{4,3,2,1}),-1,1e-12,
         "Perfect reversed Kendall ranks");
    near(kendall_tau_b({1,2,3,4},{2,3,4,5}),1,1e-12,
         "Perfect aligned Kendall ranks");
    auto loss=squared_forecast_loss_comparison(
         {1,2,3,4,5},{1,2,3,4,5},{0,1,2,3,4},0);
    near(loss.average_difference,-1,1e-12,
         "Perfect forecast reduces squared loss by one");
    auto dd=drawdown_timing({100,110,99,88,110,111,100,90});
    near(dd.largest_fraction,0.2,1e-12,"Largest exact path drawdown");
    ensure(dd.longest_underwater>=2,"Drawdown duration counts periods");
    auto markov=categorical_transition_probabilities({0,1,1,0,1},2,0);
    near(markov[0][1],1,1e-12,"Markov state transition");
    near(markov[1][0],0.5,1e-12,"Markov second state return");
    invalid([]{categorical_transition_probabilities({0,0,0},2,0);},
            "Unobserved Markov rows without smoothing rejected");
    auto km=kaplan_meier({1,2,3},{1,0,1});
    near(km[0].survival,2.0/3,1e-12,"Kaplan Meier first event");
    near(km[1].survival,2.0/3,1e-12,"Censoring leaves survival constant");
    near(km[2].survival,0,1e-12,"Final KM event");
    auto fee=charge_fund_fees(100,0.2,100,0.01,0.2);
    near(fee.management_fee,1.2,1e-12,"Management fee accrual");
    near(fee.performance_fee,3.76,1e-12,"High water incentive charge");
    near(fee.after_fee,115.04,1e-12,"Net share value after all fees");
    auto nofee=charge_fund_fees(100,0,110,0,0.2);
    near(nofee.performance_fee,0,1e-12,"No incentive below watermark");
    near(period_time_weighted_return({100,110},{110,121}),0.21,1e-12,
         "Subperiod linked time weighted return");
    near(dietz_modified_return(100,130,{10},{0.5}),20.0/105,1e-12,
         "Modified Dietz with intraperiod contribution");
    auto economics=square_root_capacity_profit(100,0.1,1,0.001);
    near(economics.gross_profit,10,1e-12,"Gross capacity economics");
    near(economics.impact_cost,1,1e-12,"Nonlinear impact cost");
    near(economics.net_profit,9,1e-12,"Net capacity profitability");
    ensure(square_root_capacity_optimal_capital(0.1,1,0.001)>100,
           "Capacity optimum nontrivial scaling");
    auto cov=shrink_covariance_to_diagonal({{1,0.5},{0.5,4}},1);
    near(cov[0][1],0,1e-12,"Full diagonal covariance shrinkage");
    near(cov[1][1],4,1e-12,"Covariance diagonal preserved");
    near(covariance_average_variance({{1,0.5},{0.5,4}}),2.5,1e-12,
         "Average asset covariance diagonal");
    auto rp=positive_covariance_risk_parity({{1,0},{0,4}});
    near(rp[0],2.0/3,1e-7,"Risk parity inverse volatility first");
    near(rp[1],1.0/3,1e-7,"Risk parity inverse volatility second");
    auto drift=drifted_portfolio_weights({0.5,0.5},{0.1,-0.1});
    near(drift[0],0.55,1e-12,"Weight drift on rising asset");
    auto stayed=no_trade_band_rebalance({0.5,0.5},{0.51,0.49},0.02);
    near(stayed[0],0.5,1e-12,"No-trade band preserves holdings");
    auto traded=no_trade_band_rebalance({0.5,0.5},{0.6,0.4},0.02);
    near(traded[0],0.6,1e-12,"Band breach selects target weights");
    near(binary_kelly_fraction(0.6,1,1),0.2,1e-12,
         "Kelly symmetric gamble fraction");
    near(fractional_kelly_fraction(0.6,1,1,0.5),0.1,1e-12,
         "Half Kelly capital reduction");
    near(herfindahl_concentration({0.5,0.5}),0.5,1e-12,
         "HHI equal two asset concentration");
    near(effective_holdings_count({0.5,0.5}),2,1e-12,
         "Effective holding count inverse HHI");
    auto loan=fixed_rate_amortization(100,0,4);
    ensure(loan.size()==4,"Amortization cashflow records");
    near(loan[0].principal,25,1e-12,"Zero-rate principal amortization");
    near(loan.back().balance,0,1e-12,"Loan final balance extinguished");
    near(information_ratio_of_active_returns({0.01,0.02,0.03},4),4.0,1e-12,
         "Information ratio frequency scaling");
    near(upside_downside_capture({0.02,-0.01,0.04,-0.02},
         {0.01,-0.02,0.02,-0.04},true),2,1e-12,"Upside market capture");
    near(unrealized_tax_lot_profit({{1,2,90},{3,3,110}},100),-10,1e-12,
         "Tax lot unrealized PnL");
    near(held_more_than_days({{1,2,90},{3,3,110}},10,8),2,1e-12,
         "Long holding tenure shares");
    near(parity_forward_component(100,100,0,0,1),0,1e-12,
         "European parity flat financing");
    near(call_put_parity_deviation(12,12,100,100,0,0,1),0,1e-12,
         "Call and put parity balanced");
    double american=american_crr_option(100,100,0.05,0,0.2,1,200,false);
    ensure(american>5&&american<10,"American CRR put plausible");
    ensure(american_crr_option(90,100,0.05,0,0.2,1,200,false)>=10,
           "American option worth immediate exercise");
    auto black=black76_futures_option(100,100,0,0.2,1,true);
    near(black.value,100*(2*standard_normal_cdf(0.1)-1),1e-9,
         "Black 76 at the money forward");
    ensure(black.forward_delta>0&&black.vega>0,
           "Black 76 forward sensitivity and vega");
    auto normaloption=bachelier_option(100,100,0,10,1,true);
    near(normaloption.value,10/sqrt(2*acos(-1.0)),1e-9,
         "Bachelier normal option ATM");
    near(normaloption.delta_forward,0.5,1e-12,"Bachelier ATM delta");
    ensure(log_brownian_bridge_down_survival(100,100,90,0.2,1)>0,
           "Brownian bridge nonzero survival");
    near(log_brownian_bridge_down_survival(100,89,90,0.2,1),0,1e-12,
         "Barrier touched gives zero survival");
    auto original=black_scholes(100,100,0.02,0.01,0.3,1,true);
    near(implied_black_scholes_volatility(original.value,100,100,
         0.02,0.01,1,true),0.3,1e-9,"Implied volatility inversion");
    near(maximum_call_vertical_violation({90,100,110},{14,10,5}),
         0,1e-12,"Monotone call quotes");
    ensure(maximum_call_vertical_violation({90,100},{3,4})>0,
           "Invalid rising call price identified");
    near(maximum_call_convexity_violation({90,100,110},{12,8,4}),
         0,1e-12,"Linear call strike price convexity");
    near(dupire_local_volatility_squared(100,10,2,0,0.04,0,0),
         0.01,1e-12,"Dupire local variance numerical identity");
    auto cds=cds_discrete_legs({{1,2},{0.02,0.02}},2,0,0.4,4);
    ensure(cds.fair_spread>0.01&&cds.fair_spread<0.02,
           "CDS flat hazard credit premium");
    near(triangular_log_mispricing(2,3,6),0,1e-12,
         "Triangular FX parity exact");
    near(cross_rate_multiplication(2,3),6,1e-12,"Triangular cross quotation");
    auto settle=futures_daily_settlement({100,110,90},2,10,300,200);
    near(settle[0].variation_margin,200,1e-12,
         "Futures initial daily variation collateral");
    near(settle[1].equity,100,1e-12,"Futures collateral below maintenance");
    near(settle[1].maintenance_shortfall,100,1e-12,
         "Variation margin shortfall");
    Matrix zeros={{0},{0},{0},{0},{0}};
    auto poisson=fit_poisson_glm_sgd(zeros,{2,2,2,2,2},0.01,20);
    near(predict_poisson_glm(poisson,{0}),2,1e-9,
         "Poisson intercept for constant event intensity");
    near(multiclass_negative_log_likelihood({{0.2,0.8},{0.8,0.2}},
         {1,0}),-log(0.8),1e-12,"Multiclass proper logarithmic loss");
    auto reliability=binary_reliability_bins({0.1,0.2,0.9,0.95},{0,0,1,1},2);
    ensure(reliability[0].count==2,"Low probability bin has two points");
    near(reliability[1].event_rate,1,1e-12,"High probability event calibration");
    auto interval=interval_coverage_report({2,4,6},{1,4,7},{3,5,8});
    near(interval.empirical_coverage,2.0/3,1e-12,
         "Interval inclusion includes boundaries");
    near(interval.average_width,4.0/3,1e-12,"Interval mean width");
    OnlineMomentsState running;
    for(double y:{1.0,2.0,3.0,4.0})running=online_moment_update(running,y);
    near(running.mean_value,2.5,1e-12,"Welford online mean");
    near(online_moment_sample_variance(running),5.0/3,1e-12,
         "Welford sample variance");
    auto metrics=out_of_sample_regression_metrics({1,2,3},{1,3,2});
    near(metrics.mae,2.0/3,1e-12,"Out of sample mean absolute error");
    near(metrics.rmse,sqrt(2.0/3),1e-12,"Out of sample RMSE");
    near(metrics.mean_bias,0,1e-12,"Out of sample forecast bias");
    near(optimal_probability_threshold(2,1,0.2),0.4,1e-12,
         "Trade economic decision threshold");
    near(expected_trade_utility(0.4,2,1,0.2),0,1e-12,
         "Zero expected net trading utility at threshold");
    BookReplay book;apply_orderbook_level(book,true,99,3);
    apply_orderbook_level(book,true,98,5);
    apply_orderbook_level(book,false,101,2);
    apply_orderbook_level(book,false,102,6);
    auto sweep=sweep_visible_orderbook(book,true,5);
    near(sweep.filled,5,1e-12,"Visible book sweep fills five units");
    near(sweep.vwap,(2*101+3*102)/5.0,1e-12,
         "Visible orderbook average execution price");
    near(sweep.remaining,0,1e-12,"Orderbook full execution remaining zero");
    near(market_volume_weighted_price({100,110},{1,3}),107.5,1e-12,
         "Exchange-reported VWAP benchmark");
    near(vwap_slippage_fraction(110,100,true),0.1,1e-12,
         "Buyer VWAP slippage sign convention");
    auto durations=trade_arrival_duration_stats({0,2,4,6,8});
    near(durations.average_seconds,2,1e-12,
         "Equal-arrival trade intensity duration");
    near(durations.coefficient_of_variation,0,1e-12,
         "Constant trading intervals CV");
    auto bars=fixed_volume_bars({100,110},{4,5},3);
    ensure(bars.size()==3,"Volume bars split large prints exactly");
    near(bars[0].vwap,100,1e-12,"First full bar price");
    near(bars[1].vwap,(100+2*110)/3.0,1e-12,
         "Volume bar split across trades");
    near(bars[2].vwap,110,1e-12,"Third full volume bar");
    near(inventory_variance_penalty(10,0.2,100,1,0.1),200,1e-12,
         "Inventory exposure variance penalty");
    near(inventory_mark_to_market_pnl(10,100,102),20,1e-12,
         "Inventory drift PnL");
    invalid([]{normality_moments({1,1,1,1});},
            "Constant series normality diagnostics undefined");
    invalid([]{jensen_shannon_divergence({0.8,0.8},{0.5,0.5});},
            "Invalid probability normalization");
    invalid([]{binary_kelly_fraction(1.2,1,1);},
            "Invalid binary win probability");
    invalid([]{charge_fund_fees(100,0,100,-0.1,0.2);},
            "Invalid negative fund fee");
    invalid([]{american_crr_option(100,100,2,0,0.01,1,1,false);},
            "CRR probability outside one rejected");
    invalid([]{implied_black_scholes_volatility(200,100,100,0,0,1,true);},
            "Option price exceeding spot rejected");
    invalid([]{multiclass_negative_log_likelihood({{0.6,0.6}},{0});},
            "Invalid class mass rejected");
    invalid([]{fixed_volume_bars({100},{1},0);},
            "Zero volume bucket size rejected");
    invalid([]{kaplan_meier({1,2},{1,2});},
            "Censor indicator outside binary rejected");
    invalid([]{futures_daily_settlement({100},1,1,100,50);},
            "Short futures settlement path rejected");
    cout<<"PASS: "<<count_checks<<" newly integrated market, statistics and portfolio assertions\n";
}catch(const exception& error){
    cerr<<"FAIL: "<<error.what()<<"\n";return 1;
}
}
