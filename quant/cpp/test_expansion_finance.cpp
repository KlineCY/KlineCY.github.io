#include "sabr_models.hpp"
#include "option_risk_scenarios.hpp"
#include "bond_risk_models.hpp"
#include "credit_copula.hpp"
#include "swap_models.hpp"
#include "futures_margin_models.hpp"
#include "optimal_execution.hpp"
#include "order_book_events.hpp"
#include "cross_asset_attribution.hpp"
#include "event_labeling.hpp"
#include "cross_validation_extended.hpp"
#include "calibration_diagnostics.hpp"
#include "backtest_cashflows.hpp"
#include "market_data_alignment.hpp"
#include "corporate_actions.hpp"
#include "drawdown_analytics.hpp"
#include "risk_backtesting.hpp"
#include "factor_exposure_models.hpp"
#include "liquidity_stress.hpp"
#include "order_flow_imbalance.hpp"
#include "trade_sizing.hpp"
#include "jump_diffusion.hpp"
#include "portfolio_entropy.hpp"
#include "corporate_finance_models.hpp"
#include "yield_curve_shape.hpp"
#include <cmath>
#include <functional>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <vector>
using namespace std;
int checks=0;
void check(bool pass,const char* message){
    checks++;if(!pass)throw runtime_error(message);
}
void approx(double a,double b,double eps,const char* message){
    check(isfinite(a)&&fabs(a-b)<=eps,message);
}
void throws(const function<void()>& action,const char* message){
    bool failed=false;try{action();}catch(const exception&){failed=true;}
    check(failed,message);
}
void derivatives_suite(){
    double atm=kline::sabr_atm_volatility(100,1,0.2,1,0,0);
    approx(atm,0.2,1e-11,"SABR Black constant variance");
    check(kline::sabr_lognormal_volatility(100,110,1,0.2,1,0,0)>0,
          "SABR non-ATM positivity");
    double original=kline::black_scholes(100,100,0.02,0,0.2,1,true).value;
    double stressed=kline::black_scholes(101,100,0.02,0,0.2,1,true).value;
    approx(kline::reprice_option_shock(100,100,0.02,0,0.2,1,{1,0,0}),
           stressed-original,1e-12,"Nonlinear option revaluation");
    approx(kline::gamma_scalping_approx(0.1,0.2,0.04,1,100),
           0,1e-11,"Gamma neutral realized variance");
    throws([]{kline::sabr_atm_volatility(100,1,0.2,0,1,0.2);},
           "SABR correlation boundary");
    throws([]{kline::reprice_option_shock(100,100,0,0,0.2,1,{-101,0,0});},
           "Negative shocked stock");
}
void bonds_suite(){
    auto bond=kline::continuous_bond_risk({2},{100},0.05);
    approx(bond.price,100*exp(-0.1),1e-12,"Continuous zero coupon price");
    approx(bond.duration,2,1e-12,"Zero coupon duration");
    approx(bond.convexity,4,1e-12,"Zero coupon convexity");
    approx(bond.dv01,0.02*exp(-0.1),1e-12,"Zero coupon DV01");
    approx(kline::bond_shock_pnl_approx(100,2,4,0.01),
           -1.98,1e-12,"Convexity-corrected price shock");
    kline::ZeroCurve curve{{1,2,3},{0.05,0.05,0.05}};
    double fixed=kline::fixed_leg_present_value(curve,{1,2},0.05,100);
    approx(fixed,5*(exp(-0.05)+exp(-0.1)),1e-12,"Fixed-leg annuity");
    approx(kline::floating_leg_par_value(curve,2,100),
           100*(1-exp(-0.1)),1e-12,"Par floating leg");
    approx(kline::payer_swap_present_value(curve,{1,2},
           kline::par_swap_rate(curve,{1,2}),100),0,1e-11,
           "Par swap value at inception");
    approx(kline::forward_rate_from_discounts(curve,1,2),
           exp(0.05)-1,1e-12,"Annual simple forward rate");
    approx(kline::nelson_siegel_zero(0,0.03,-0.01,0.02,2),
           0.02,1e-12,"NS short end level plus slope");
    auto ns=kline::nelson_siegel_curve({1,2,3},0.03,0.0,0.0,2);
    approx(kline::curve_slope(ns,1,3),0,1e-12,
           "Flat Nelson Siegel curve");
    throws([]{kline::nelson_siegel_curve({2,1},0.03,0,0,2);},
           "Unordered curve tenors");
}
void credit_and_margin_suite(){
    approx(kline::vasicek_conditional_default_rate(0.1,0,5),
           0.1,1e-12,"Uncorrelated default probability");
    auto losses=kline::one_factor_credit_losses(
        {100,100},{0.05,0.05},{0.6,0.6},0.2,100,42);
    check(losses.size()==100,"Credit Monte Carlo scenario count");
    auto again=kline::one_factor_credit_losses(
        {100,100},{0.05,0.05},{0.6,0.6},0.2,100,42);
    check(losses==again,"Credit Monte Carlo seed reproducible");
    for(double loss:losses)
        check(loss==0||loss==60||loss==120,"Discrete default loss support");
    auto account=kline::open_margin_account(1000,2,100,10,0.2,0.1);
    approx(account.initial_margin,400,1e-12,"Initial futures collateral");
    approx(account.free_cash,600,1e-12,"Unencumbered funds");
    auto next=kline::mark_margin_account(account,2,100,101,10,0.2,0.1);
    approx(next.equity,1020,1e-12,"Futures variation margin");
    approx(next.cumulative_pnl,20,1e-12,"Cumulative settlement gains");
    check(!kline::margin_call(next),"Maintenance covered");
    throws([]{kline::open_margin_account(100,2,100,10,0.2,0.1);},
           "Insufficient initial collateral");
    throws([]{kline::mark_margin_account({100,0,0,0,0},2,100,-1,10,0.2,0.1);},
           "Invalid futures settlement");
}
void order_execution_suite(){
    auto schedule=kline::almgren_chriss_schedule(100,10,0.1,0.2,1);
    approx(accumulate(schedule.begin(),schedule.end(),0.0),100,1e-9,
           "Execution inventory conservation");
    check(schedule.front()>schedule.back(),"Risk averse front loaded");
    auto neutral=kline::almgren_chriss_schedule(100,10,0.1,0.2,0);
    approx(neutral[0],10,1e-12,"Risk neutral uniform schedule");
    approx(kline::quadratic_execution_cost({10,10},0.1,1),
           40,1e-12,"Square impact and commissions");
    kline::LimitBook book;
    kline::update_limit_level(book,true,99,10);
    kline::update_limit_level(book,false,101,20);
    approx(kline::book_mid_price(book),100,1e-12,"Orderbook midpoint");
    approx(kline::book_microprice(book),(99*20.0+101*10.0)/30,1e-12,
           "Size-aware microprice");
    approx(kline::best_level_imbalance(book),-1.0/3,1e-12,
           "Orderbook visible imbalance");
    kline::update_limit_level(book,true,99,0);
    check(book.bids.empty(),"Removing limit order level");
    throws([]{kline::LimitBook invalid;kline::update_limit_level(
            invalid,true,-1,10);},"Invalid price rejected");
    throws([]{kline::almgren_chriss_schedule(10,1,0,0.2,1);},
           "Execution impact coefficient positive");
    auto attrib=kline::decompose_single_period(
        {0.5,0.5},{0.6,0.4},{0.1,-0.1},0.01);
    approx(attrib.market,0,1e-12,"Benchmark hold attribution");
    approx(attrib.trading,0.02,1e-12,"Delta weight attribution");
    approx(attrib.fees,0.002,1e-12,"Transaction drag");
    approx(attrib.total,0.018,1e-12,"Attribution reconciliation");
    approx(kline::linear_factor_attribution({1,2},{0.1,0.2},0.01),
           0.51,1e-12,"Linear factor with specific return");
}
void validation_suite(){
    auto event=kline::label_triple_barrier({100,101,105,102,90},0,4,0.04,0.05);
    check(event.end==2&&event.direction==1,
          "First profit barrier hit");
    approx(event.realized_return,0.05,1e-12,"Barrier realized return");
    auto labels=kline::label_all_barriers({100,101,102,103,104,105},3,0.1,0.1);
    check(labels.size()==3,"Only fully mature bar label horizons");
    auto folds=kline::purged_kfold(
        {{0,2},{2,4},{4,6},{6,8},{8,10},{10,12}},3,1);
    check(folds.size()==3&&folds[0].test.size()==2,
          "Purged event grouping");
    check(folds[0].training.size()>0,"Valid training outside test");
    check(kline::expanding_train_endpoints(12,4,3,2)==
          vector<int>({4,7,10}),"Chronological expansion windows");
    approx(kline::expected_calibration_error(
        {0,0,1,1},{0,0,1,1},2),0,1e-12,
        "Perfect probability reliability");
    approx(kline::binary_brier({0,1},{0,1}),0,1e-12,
           "Perfect Brier loss");
    throws([]{kline::label_all_barriers({100,110},3,0.1,0.1);},
           "Insufficient full label maturity");
}
void cashflow_data_suite(){
    auto ledger=kline::time_weighted_equity(
        100,{0.1,0.1},{10,0});
    approx(ledger.ending_equity,132,1e-12,
           "External contribution applied post returns");
    approx(ledger.total_contribution,10,1e-12,
           "Total net contributions");
    approx(kline::time_weighted_growth({0.1,0.1}),1.21,1e-12,
           "TWR independent of contributions");
    approx(kline::money_weighted_irr({-100,110}),0.1,1e-12,
           "IRR single-period positive flow");
    vector<double> b;
    auto alignment=kline::aligned_intersection(
        {{1,10},{3,30},{5,50}},{{2,20},{3,33},{5,55}},b);
    check(alignment.size()==2&&b.size()==2,
          "Aligned common trading dates");
    approx(b[0],33,1e-12,"Date match values");
    auto asof=kline::asof_last_known(
        {{1,10},{3,30},{5,50}},{1,2,4,6});
    check(asof==vector<double>({10,10,30,50}),
          "Historical publication time map");
    throws([]{vector<double> x;kline::aligned_intersection(
        {{1,1},{1,2}},{{1,1}},x);},"Duplicate dates rejected");
}
void corporate_actions_suite(){
    approx(kline::stock_total_return(100,50,0,2),
           0,1e-12,"Split does not create value");
    approx(kline::stock_total_return(100,99,2),
           0.01,1e-12,"Dividend reinvested return");
    auto returns=kline::stock_total_returns(
        {100,50,52},{0,0,1},{1,2,1});
    approx(returns[0],0,1e-12,"Split corrected");
    approx(returns[1],0.06,1e-12,"Dividend period returns");
    approx(kline::rights_adjusted_theoretical_ex_price(100,80,0.5),
           140.0/1.5,1e-12,"Theoretical ex rights price");
    auto episodes=kline::drawdown_episodes({100,90,80,100,110,100});
    check(episodes.size()==2,"Complete and active drawdown episodes");
    check(episodes[0].peak==0&&episodes[0].trough==2&&
          episodes[0].recovery==3,"Peak trough recovery anchors");
    approx(episodes[0].deepest,-0.2,1e-12,"First maximum drawdown");
    approx(kline::max_drawdown_duration({100,90,80,100}),3,
           1e-12,"Peak to recovery duration");
}
void risk_and_liquidity_suite(){
    auto var=kline::kupiec_var_coverage(
        {1,3,2,4},{2,2,2,2},0.75);
    check(var.exceptions==2&&var.observations==4,
          "VaR exception count greater than only");
    check(var.kupiec_lr>0,"Kupiec likelihood ratio positive");
    approx(kline::expected_shortfall_residual(
        {1,3,2,4},{2,2,2,2},{3,3,3,3}),0.5,1e-12,
        "ES breach diagnostic average loss");
    auto beta=kline::rolling_factor_beta(
        {2,4,6,8,10},{1,2,3,4,5},3);
    approx(beta[0],2,1e-12,"Rolling linear factor beta");
    approx(kline::factor_neutral_portfolio_beta({0.5,-0.5},{1,1}),
           0,1e-12,"Portfolio neutral factor exposure");
    auto hedge=kline::hedge_factor_exposure({1,0},{2,1},1);
    approx(hedge[1],-2,1e-12,"Hedging exposure cancellation");
    approx(kline::liquidation_days({1000,2000},{1000,1000},0.2),
           10,1e-12,"Days to liquidate using ADV");
    approx(kline::staged_liquidation_cost(
        {10,20},{0.01,0.02},{0,0},0),
        0.5,1e-12,"Spread-only liquidation cost");
    throws([]{kline::liquidation_days({100},{0},0.1);},
           "Zero ADV cannot liquidate");
}
void strategy_signal_suite(){
    approx(kline::best_quote_order_flow(
        99,99,10,12,101,101,8,7),
        3,1e-12,"Top-of-book order flow changes");
    approx(kline::weighted_order_flow({1,-2},{2,1}),
           0,1e-12,"Time weighted flow");
    approx(kline::fraction_kelly_binary(0.6,1),0.2,1e-12,
           "Binary Kelly fraction");
    approx(kline::normal_kelly_fraction(0.1,0.5,1),0.2,1e-12,
           "Gaussian Kelly exposure");
    approx(kline::volatility_scale_position(0.2,0.1,-1,1),
           -1,1e-12,"Gross-limited volatility scaling");
    approx(kline::merton_jump_drift_correction(0,0,0),
           0,1e-12,"Zero jump compensator");
    auto deterministic=kline::merton_jump_prices(
        100,0.05,0,0,0,0,1,2,42);
    approx(deterministic[2],100*exp(0.1),1e-12,
           "Deterministic jump diffusion limit");
    approx(kline::holdings_entropy({0.5,0.5}),log(2),1e-12,
           "Equal portfolio entropy");
    approx(kline::holdings_effective_number({0.5,0.5}),2,1e-12,
           "Effective holdings");
    approx(kline::herfindahl_exposure({0.5,0.5}),0.5,1e-12,
           "Holdings HHI");
    approx(kline::net_present_value(100,{110},0.1),0,1e-12,
           "NPV zero internal rate");
    approx(kline::weighted_average_cost_of_capital(0.5,0.1,0.06,0.25),
           0.0725,1e-12,"Tax adjusted WACC");
    approx(kline::perpetuity_value(10,0.02,0.07),200,1e-12,
           "Growing perpetuity valuation");
}
int main(){
    try{
        derivatives_suite();bonds_suite();credit_and_margin_suite();
        order_execution_suite();validation_suite();cashflow_data_suite();
        corporate_actions_suite();risk_and_liquidity_suite();
        strategy_signal_suite();
        cout<<"PASS: "<<checks<<" expanded derivatives, trading and portfolio checks\n";
        return 0;
    }catch(const exception& e){
        cerr<<"FAIL: "<<e.what()<<"\n";return 1;
    }
}
