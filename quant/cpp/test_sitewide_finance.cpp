#include "delta_hedge_cashbook.hpp"
#include "option_delta_gamma_hedge.hpp"
#include "swap_fixed_float_pv.hpp"
#include "credit_survival_curve.hpp"
#include "credit_portfolio_moments.hpp"
#include "fx_hedge_accounting.hpp"
#include "fixed_income_keyrate_hedge.hpp"
#include "tracking_error_attribution.hpp"
#include "volatility_target_rebalance.hpp"
#include "short_margin_stress.hpp"
#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>
using namespace std;
int checks=0;
void check(bool x,const char* message){checks++;if(!x)throw runtime_error(message);}
void near(double x,double value,double tol,const char* message){
    check(isfinite(x)&&fabs(x-value)<=tol,message);
}
void fails(const function<void()>& fn,const char* message){
    bool caught=false;try{fn();}catch(const exception&){caught=true;}
    check(caught,message);
}
int main(){
 try{
    auto hedge=kline::short_option_delta_hedge(
         {100,104},{10,8},{0.5,0.5},0,0);
    near(hedge[0].cash,-40,1e-12,"Initial delta hedge cash");
    near(hedge[0].net_liquidation,0,1e-12,
         "Option delta hedge zero initial liquidation");
    near(hedge[1].net_liquidation,4,1e-12,
         "Hedged option cashbook future PnL");
    near(hedge[1].stock_trade,0,1e-12,
         "No unnecessary hedge trade when delta fixed");
    auto greek=kline::delta_gamma_hedge(10,4,0.5,2);
    near(greek.option,-2,1e-12,"Gamma hedge option units");
    near(greek.underlying,-9,1e-12,"Delta hedge after gamma option");
    near(kline::second_order_greeks_pnl(2,3,4,1,0.1),
         3.9,1e-12,"Delta gamma vega scenario PnL");
    kline::ZeroCurve curve{{1,2},{0.05,0.05}};
    double floating=kline::floating_par_leg_present_value(curve,2,100);
    near(floating,100*(1-exp(-0.1)),1e-12,
         "Par floating swap telescopes discount factors");
    double coupon=floating/
       (100*(exp(-0.05)+exp(-0.1)));
    near(kline::payer_swap_present_value(curve,{1,2},{1,1},
         coupon,100),0,1e-10,"Par payer swap fair value zero");
    auto hazard=kline::HazardCurve{{1,2},{0.1,0.1}};
    near(kline::survival_probability(hazard,3),exp(-0.3),1e-12,
         "Hazard extrapolation constant last rate");
    near(kline::probability_default_by(hazard,1),1-exp(-0.1),1e-12,
         "Default probability from survival");
    double pd=kline::expected_discounted_default_loss(
         hazard,0,0,100,1,1000);
    near(pd,100*(1-exp(-0.1)),1e-9,
         "Integrated expected loss zero rate one year");
    near(kline::independent_credit_expected_loss(
         {100},{0.1},{0.5}),5,1e-12,
         "Expected default loss equals EAD times PD times LGD");
    near(kline::independent_credit_loss_variance(
         {100},{0.1},{0.5}),225,1e-12,
         "Bernoulli credit portfolio loss variance");
    double terminal=kline::hedged_foreign_investment_terminal(
        100,110,6,7,1);
    near(terminal,760,1e-12,"FX short forward hedged terminal domestic");
    near(kline::hedged_foreign_investment_return(700,terminal),
         60.0/700,1e-12,"FX hedged foreign asset total return");
    auto hedge_instruments=kline::solve_keyrate_dv01_hedge(
        {{1,0},{0,2}},{3,4});
    near(hedge_instruments[0],-3,1e-12,"Hedge first key rate node");
    near(hedge_instruments[1],-2,1e-12,"Hedge second key rate node");
    near(kline::ex_ante_tracking_error({{1,0},{0,1}},{1,-1}),
         sqrt(2),1e-12,"Active tracking error risk");
    auto contributions=kline::tracking_error_risk_contributions(
        {{1,0},{0,1}},{1,-1});
    near(contributions[0]+contributions[1],sqrt(2),1e-12,
         "Tracking error risk decomposition sums to TE");
    auto weights=kline::past_only_volatility_target(
        {0.01,-0.01,0.01,-0.01,0.01,-0.01},3,0.1,1.5);
    near(weights[0],0,1e-12,"No prehistory no target weight");
    check(weights[3]>0&&weights[3]<=1.5,
         "Causal target volatility weight constrained");
    auto constant=kline::past_only_volatility_target(
        {0,0,0,0,0},3,0.1,2);
    near(constant[3],2,1e-12,"Zero volatility uses gross exposure cap");
    double equity=kline::short_equity_after_price_move(
         200,10,100,120,0);
    near(equity,0,1e-12,"Short mark-to-market equity");
    near(kline::short_maintenance_margin_ratio(
        200,10,100),0.2,1e-12,"Short maintenance ratio");
    check(kline::short_margin_call_required(
        equity,10,120,0.25),"Short margin call under adverse move");
    fails([]{kline::delta_gamma_hedge(1,2,1,0);},
          "Cannot gamma hedge with zero hedge gamma");
    fails([]{kline::survival_probability(
         {{1,1},{0.1,0.1}},1);},
         "Invalid repeated hazard tenors rejected");
    fails([]{kline::independent_credit_loss_variance(
         {100},{1.2},{0.5});},
         "Default probability over one rejected");
    fails([]{kline::short_option_delta_hedge(
         {100},{10},{0.5},0,0);},
         "Short option path length rejected");
    cout<<"PASS: "<<checks<<" new derivatives, credit, hedge and portfolio assertions\n";
 }catch(const exception& e){cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
