#include "mean_cvar_allocation.hpp"
#include "risk_budget_allocation.hpp"
#include "black_litterman_complete.hpp"
#include "turnover_penalized_allocation.hpp"
#include "factor_risk_covariance.hpp"
#include "online_covariance.hpp"
#include "tax_lot_fifo.hpp"
#include "integer_portfolio.hpp"
#include "portfolio_drawdown_control.hpp"
#include "stress_factor_mapping.hpp"
#include "par_curve_bootstrap.hpp"
#include "credit_migration.hpp"
#include "credit_spread_risk.hpp"
#include "american_option_sensitivities.hpp"
#include "barrier_option_mc.hpp"
#include "variance_swap_accounting.hpp"
#include "continuous_futures_series.hpp"
#include "fx_forward_parity.hpp"
#include "market_order_fills.hpp"
#include "vwap_execution_schedule.hpp"
#include "short_borrow_ledger.hpp"
#include "liquidity_capacity.hpp"
#include "trade_fill_reconciliation.hpp"
#include "implementation_shortfall.hpp"
#include "lagged_strategy_backtest.hpp"
#include "execution_intensity.hpp"
#include <cmath>
#include <functional>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <vector>
using namespace std;
int checks=0;
void ensure(bool good,const char* label){checks++;if(!good)throw runtime_error(label);}
void close(double got,double expected,double tolerance,const char* label){
    ensure(isfinite(got)&&fabs(got-expected)<=tolerance,label);
}
void rejects(const function<void()>& job,const char* label){
    bool rejected=false;try{job();}catch(const exception&){rejected=true;}
    ensure(rejected,label);
}
void portfolio_suite(){
    kline::Matrix scenarios={{0.01,0.05},{-0.03,0.02},{-0.02,0.01},
                             {-0.04,0.02},{0.01,0.01}};
    close(kline::empirical_cvar_loss(scenarios,{1,0},0.8),0.04,1e-12,
          "Worst individual CVaR loss");
    auto cv=kline::minimum_cvar_long_only(scenarios,0.8,1200,0.4);
    ensure(cv[1]>cv[0],"Minimum CVaR selects low-tail-loss asset");
    close(cv[0]+cv[1],1,1e-12,"CVaR weights fully invested");
    auto budget=kline::risk_budget_weights({{1,0},{0,4}},{0.5,0.5});
    close(budget[0],2.0/3,1e-4,"Equal risk contribution first weight");
    close(budget[1],1.0/3,1e-4,"Equal risk contribution second weight");
    auto bl=kline::black_litterman_full({0},{{1}},{{1}},{0.1},{1},1);
    close(bl.mean[0],0.05,1e-12,"Black Litterman posterior view mean");
    close(bl.covariance[0][0],1.5,1e-12,
          "Black Litterman full posterior covariance");
    auto turn=kline::projected_turnover_allocation(
        {0,0},{{1,0},{0,1}},{0.5,0.5},1,0.1);
    close(turn[0],0.5,1e-7,"Symmetric turnover allocation");
    close(turn[1],0.5,1e-7,"Symmetric turnover second weight");
    close(kline::turnover_penalized_objective(
        {0.5,0.5},{0,0},{{1,0},{0,1}},{0.5,0.5},1,0.1),
        0.25,1e-12,"Turnover objective risk value");
    auto factor=kline::factor_structured_covariance(
        {{1},{2}},{{0.04}},{0.01,0.02});
    close(factor[0][0],0.05,1e-12,"Factor covariance idiosyncratic first");
    close(factor[0][1],0.08,1e-12,"Factor covariance cross exposure");
    close(factor[1][1],0.18,1e-12,"Factor covariance idiosyncratic second");
    auto risk=kline::portfolio_factor_exposure({{1},{2}},{0.3,0.7});
    close(risk[0],1.7,1e-12,"Portfolio factor exposure");
    auto state=kline::new_online_covariance(2);
    for(int i=1;i<=3;i++)kline::update_online_covariance(state,{double(i),double(2*i)});
    auto cov=kline::finalized_online_covariance(state);
    close(cov[0][0],1,1e-12,"Online Welford sample covariance");
    close(cov[0][1],2,1e-12,"Online cross covariance");
    close(cov[1][1],4,1e-12,"Online second variance");
    rejects([]{kline::risk_budget_weights({{1,2},{2,1}},{0.5,0.5});},
            "Indefinite covariance rejects risk budgeting");
}
void cashflow_suite(){
    kline::FifoLedger ledger;
    kline::fifo_buy(ledger,10,10,2);
    kline::fifo_buy(ledger,5,12);
    kline::fifo_sell(ledger,12,15,3);
    close(ledger.realized_pnl,51,1e-12,"FIFO realized inventory PnL");
    close(ledger.remaining_shares,3,1e-12,"FIFO remaining shares");
    close(ledger.remaining_basis,36,1e-12,"FIFO remaining basis");
    rejects([&]{kline::fifo_sell(ledger,4,10);},"FIFO rejects uncovered sale");
    auto orders=kline::budgeted_whole_share_allocation(
        100,{0.5,0.5},{10,20});
    ensure(orders.shares[0]==5&&orders.shares[1]==2,
           "Whole share allocation counts");
    close(orders.cash,10,1e-12,"Cash preserved after integer shares");
    close(orders.invested+orders.commission+orders.cash,100,
          1e-12,"Whole share capital conservation");
    auto drawdown=kline::causal_drawdown_leverage(
        {100,95,90,100},0.2,1);
    close(drawdown[0],1,1e-12,"Initial full risk");
    close(drawdown[1],0.75,1e-12,"Risk reduction after drawdown");
    close(drawdown[2],0.5,1e-12,"Risk reduction with larger drawdown");
    auto scenarios=kline::factor_scenario_asset_returns(
        {{1,0},{0.5,1}},{-0.1,0.02},{0.01,0});
    close(scenarios[0],-0.09,1e-12,"Factor stress first asset");
    close(scenarios[1],-0.03,1e-12,"Factor stress second asset");
    close(kline::portfolio_factor_scenario_loss(
        {0.5,0.5},{{1,0},{0.5,1}},{-0.1,0.02},{0.01,0}),
        0.06,1e-12,"Portfolio stress reconciles");
}
void derivative_suite(){
    auto curve=kline::bootstrap_annual_par_bonds({0.05,0.05,0.05});
    for(int i=0;i<3;i++)
        close(curve.discounts[i],std::pow(1.05,-(i+1)),1e-10,
              "Bootstrap par bond continuous discounts");
    close(kline::par_coupon_rate_from_discounts(curve.discounts),
          0.05,1e-12,"Rebuilt flat coupon par rate");
    auto migration=kline::credit_rating_distribution(
        {1,0},{{0.8,0.2},{0,1}},2);
    close(migration[0],0.64,1e-12,"Survival migration year two");
    close(migration[1],0.36,1e-12,"Default absorption year two");
    close(kline::credit_migration_expected_loss(migration,{0,100}),
          36,1e-12,"Credit migration expected loss");
    close(kline::credit_spread_dv01({1},{100},0,0),0.01,2e-8,
          "Single cashflow credit spread DV01");
    close(kline::credit_spread_duration({1},{100},0,0),1,1e-8,
          "Unit duration credit spread");
    auto greeks=kline::american_binomial_greeks(
        100,100,0.02,0,0.2,1,80,true);
    ensure(greeks.value>0&&greeks.delta>0&&greeks.delta<1,
           "American call option price and delta");
    ensure(greeks.vega>0,"American option vega positive");
    auto barrier=kline::down_and_out_call_monte_carlo(
        100,90,80,0,0,1,12,200,42);
    close(barrier.mean,10,1e-12,"Zero volatility barrier call price");
    close(barrier.standard_error,0,1e-12,"Deterministic barrier MC SE");
    close(kline::annualized_realized_variance({0.01,-0.01}),
          0.0252,1e-12,"Variance swap realized annual variance");
    close(kline::variance_swap_settlement(0.05,0.04,100),1,
          1e-12,"Variance swap settlement");
    close(kline::vega_notional_to_variance_notional(20,0.2),50,
          1e-12,"Volatility to variance notional");
    auto adjusted=kline::back_adjust_additive_futures(
        {100,101,110,111},{102},{110},{2});
    ensure(adjusted==vector<double>({108,109,110,111}),
           "Causal additive continuous future historically backadjusted");
    auto held=kline::held_contract_futures_returns(
        {100,110},{101,112},{100,110});
    close(held[0],0.01,1e-12,"Held futures return first leg");
    close(kline::covered_fx_forward(7,0.05,0.02,1),
          7*exp(0.03),1e-12,"FX covered forward rate");
    close(kline::fx_covered_basis(7*exp(0.03),7,0.05,0.02,1),
          0,1e-12,"Covered interest parity zero basis");
    rejects([]{kline::bootstrap_annual_par_bonds({0.1,0.99});},
            "Impossible par discount factors rejected");
}
void execution_suite(){
    auto fill=kline::sweep_market_order({{101,2},{102,5}},4,true);
    close(fill.filled,4,1e-12,"Market order filled size");
    close(fill.remaining,0,1e-12,"Market order residual size");
    close(fill.average_price,101.5,1e-12,"Liquidity-weighted trade VWAP");
    close(kline::market_order_slippage(fill,100,true),0.015,1e-12,
          "Buy slippage against arrival midpoint");
    auto shallow=kline::sweep_market_order({{99,2}},4,false);
    close(shallow.remaining,2,1e-12,"Partial fill preserves unfilled quantity");
    auto schedule=kline::volume_profile_schedule(100,{1,3});
    close(schedule[0],25,1e-12,"VWAP first child order");
    close(schedule[1],75,1e-12,"VWAP second child order");
    close(kline::executed_vwap({100,102},{1,3}),101.5,1e-12,
          "Executed trade VWAP");
    close(kline::daily_stock_borrow_fee(100,10,0.36,360),1,
          1e-12,"Daily stock lending fee");
    close(kline::short_dividend_payment(100,0.5),50,1e-12,
          "Dividend compensation for borrowed stock");
    close(kline::restricted_short_proceeds(100,10,0.2),1200,1e-12,
          "Restricted borrow proceeds collateral");
    auto capped=kline::capacity_constrained_shares(
        {200,-100},{1000,1000},0.1);
    close(capped[0],100,1e-12,"ADV participation clips buy side");
    close(capped[1],-100,1e-12,"ADV participation sell side");
    close(kline::participation_weighted_execution_time(
        {200,-100},{1000,1000},0.1),2,1e-12,"ADV liquidation two days");
    auto accounting=kline::reconcile_fills({{10,100,1},{-5,110,1}});
    close(accounting.net_shares,5,1e-12,"Fill net inventory");
    close(accounting.gross_shares,15,1e-12,"Fill gross traded inventory");
    close(accounting.cash_change,-452,1e-12,"Fill cash reconciliation");
    close(accounting.commissions,2,1e-12,"Fill commission total");
    auto shortfall=kline::implementation_shortfall(10,100,103,{{6,101,0}});
    close(shortfall.execution_cost,6,1e-12,"Executed implementation shortfall");
    close(shortfall.opportunity_cost,12,1e-12,"Missed buy fill opportunity");
    close(shortfall.total,18,1e-12,"Implementation shortfall total");
    auto strategy=kline::lagged_signal_strategy(
        {1,1,-1},{0,0.1,0.1},0.01);
    close(strategy[0].active_weight,0,1e-12,
          "Day zero cannot act on current signal");
    close(strategy[1].net_return,0.09,1e-12,"Causal signal and fees");
    close(strategy[2].equity,1.199,1e-12,"Causal compounded net equity");
    close(kline::exponential_fill_intensity(2,1,0),2,1e-12,
          "Zero quote distance maximum intensity");
    close(kline::poisson_arrival_fill_probability(2,1),
          1-exp(-2),1e-12,"Poisson arrival quote fill probability");
    ensure(kline::queue_depletion_probability(3,10)>0,
           "Simplified queue depletion probability");
    rejects([]{kline::sweep_market_order({{102,1},{101,1}},2,true);},
            "Out-of-order orderbook levels rejected");
    rejects([]{kline::lagged_signal_strategy({2,0},{0,0},0);},
            "Over-levered signal rejected when traded");
}
int main(){
 try{portfolio_suite();cashflow_suite();derivative_suite();execution_suite();
     cout<<"PASS: "<<checks<<" additional quantitative portfolio, derivative and execution checks\n";
 }catch(const exception& e){cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
