#pragma once
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
#include "massive_stats_cli.hpp"
#include "io.hpp"
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
namespace kline {
inline bool massive_option_type(const char* argument){
    std::string option=argument;
    require(option=="CALL"||option=="PUT","Option type must be CALL or PUT");
    return option=="CALL";
}
inline bool massive_market_side(const char* argument){
    std::string side=argument;
    require(side=="BUY"||side=="SELL","Side must be BUY or SELL");
    return side=="BUY";
}
inline bool run_massive_finance_cli(const std::string& c,int argc,char** argv,
                                   std::ostream& out){
    auto exact=[&](int n){require(argc==n,"Invalid derivatives or microstructure command arity");};
    auto v=[&](int index){return parse_number(argv[index]);};
    if(c=="parity-forward"){
        exact(7);out<<"ForwardComponent "<<parity_forward_component(
            v(2),v(3),v(4),v(5),v(6))<<"\n";
    }else if(c=="parity-error"){
        exact(9);out<<"ParityError "<<call_put_parity_deviation(
            v(2),v(3),v(4),v(5),v(6),v(7),v(8))<<"\n";
    }else if(c=="american-crr"){
        exact(10);out<<"AmericanPrice "<<american_crr_option(
            v(2),v(3),v(4),v(5),v(6),v(7),
            mass_int(argv[8],1,20000),massive_option_type(argv[9]))<<"\n";
    }else if(c=="black76-price"||c=="black76-vega"){
        exact(8);auto result=black76_futures_option(
            v(2),v(3),v(4),v(5),v(6),massive_option_type(argv[7]));
        out<<(c=="black76-price"?"Black76Price ":"Black76Vega ")
           <<(c=="black76-price"?result.value:result.vega)<<"\n";
    }else if(c=="bachelier-price"||c=="bachelier-delta"){
        exact(8);auto result=bachelier_option(
            v(2),v(3),v(4),v(5),v(6),massive_option_type(argv[7]));
        out<<(c=="bachelier-price"?"NormalOptionPrice ":"NormalOptionDelta ")
           <<(c=="bachelier-price"?result.value:result.delta_forward)<<"\n";
    }else if(c=="barrier-down-bridge"||c=="barrier-up-bridge"){
        exact(7);auto result=c=="barrier-down-bridge"?
            log_brownian_bridge_down_survival(v(2),v(3),v(4),v(5),v(6)):
            log_brownian_bridge_up_survival(v(2),v(3),v(4),v(5),v(6));
        out<<"BridgeSurvival "<<result<<"\n";
    }else if(c=="implied-black-vol"){
        exact(9);out<<"ImpliedVol "<<implied_black_scholes_volatility(
            v(2),v(3),v(4),v(5),v(6),v(7),
            massive_option_type(argv[8]))<<"\n";
    }else if(c=="call-monotonicity"||c=="call-convexity"){
        exact(5);auto t=read_csv(argv[2]);
        auto strikes=t.numbers(argv[3]),prices=t.numbers(argv[4]);
        out<<"Violation "<<(c=="call-monotonicity"?
            maximum_call_vertical_violation(strikes,prices):
            maximum_call_convexity_violation(strikes,prices))<<"\n";
    }else if(c=="dupire-local-variance"){
        exact(9);
        out<<"LocalVariance "<<dupire_local_volatility_squared(
            v(2),v(3),v(4),v(5),v(6),v(7),v(8))<<"\n";
    }else if(c=="cds-protection"){
        exact(9);auto t=read_csv(argv[2]);
        auto p=cds_discrete_legs(
            {t.numbers(argv[3]),t.numbers(argv[4])},
             v(5),v(6),v(7),mass_int(argv[8],1,365));
        out<<"PremiumAnnuity "<<p.premium_annuity<<"\nProtection "
           <<p.default_protection<<"\nFairSpread "<<p.fair_spread<<"\n";
    }else if(c=="fx-cross-rate"){
        exact(4);out<<"CrossRate "<<cross_rate_multiplication(v(2),v(3))<<"\n";
    }else if(c=="fx-triangular-error"){
        exact(5);out<<"LogMispricing "<<triangular_log_mispricing(
            v(2),v(3),v(4))<<"\n";
    }else if(c=="futures-variation"){
        exact(7);auto steps=futures_daily_settlement(
            read_csv(argv[2]).numbers(argv[3]),v(4),v(5),v(6),0);
        out<<"Index,Variation,Equity,Shortfall\n";
        for(int i=0;i<int(steps.size());i++)
            out<<i<<","<<steps[i].variation_margin<<","
               <<steps[i].equity<<","<<steps[i].maintenance_shortfall<<"\n";
    }else if(c=="futures-margin-shortfall"){
        exact(8);auto steps=futures_daily_settlement(
            read_csv(argv[2]).numbers(argv[3]),v(4),v(5),v(6),v(7));
        out<<"Index,Variation,Equity,Shortfall\n";
        for(int i=0;i<int(steps.size());i++)
            out<<i<<","<<steps[i].variation_margin<<","
               <<steps[i].equity<<","<<steps[i].maintenance_shortfall<<"\n";
    }else if(c=="poisson-glm"){
        exact(9);auto t=read_csv(argv[2]);auto x=t.numbers(argv[3]);
        auto y=t.numbers(argv[4]);
        Matrix features;for(double z:x)features.push_back({z});
        auto model=fit_poisson_glm_sgd(features,y,v(6),
                  mass_int(argv[7],1,100000),v(8));
        out<<"ConditionalCount "<<predict_poisson_glm(model,{v(5)})<<"\n";
    }else if(c=="multiclass-loss"){
        exact(6);auto t=read_csv(argv[2]);
        auto a=t.numbers(argv[3]),b=t.numbers(argv[4]),
             labels=t.numbers(argv[5]);
        Matrix p;std::vector<int> y;
        for(int i=0;i<int(labels.size());i++){
            require(labels[i]==0||labels[i]==1,"Binary class target expected");
            p.push_back({a[i],b[i]});y.push_back(int(labels[i]));
        }
        out<<"MultiClassLogLoss "<<multiclass_negative_log_likelihood(p,y)
           <<"\n";
    }else if(c=="reliability-bins"){
        exact(6);auto t=read_csv(argv[2]);
        auto probability=t.numbers(argv[3]),raw=t.numbers(argv[4]);
        std::vector<int> y;
        for(double value:raw){
            require(value==0||value==1,"Calibration labels must be binary");
            y.push_back(int(value));
        }
        auto bins=binary_reliability_bins(probability,y,mass_int(argv[5],2,1000));
        out<<"Bin,Count,MeanProbability,EventRate\n";
        for(int i=0;i<int(bins.size());i++)out<<i<<","<<bins[i].count
                <<","<<bins[i].mean_forecast<<","<<bins[i].event_rate<<"\n";
    }else if(c=="interval-coverage"){
        exact(6);auto t=read_csv(argv[2]);
        auto r=interval_coverage_report(t.numbers(argv[3]),
                                       t.numbers(argv[4]),t.numbers(argv[5]));
        out<<"Coverage "<<r.empirical_coverage<<"\nAverageWidth "
           <<r.average_width<<"\nObservations "<<r.observations<<"\n";
    }else if(c=="online-moments"){
        exact(4);auto values=read_csv(argv[2]).numbers(argv[3]);
        OnlineMomentsState state;
        for(double value:values)state=online_moment_update(state,value);
        out<<"Count "<<state.observations<<"\nMean "<<state.mean_value
           <<"\nVariance "<<online_moment_sample_variance(state)<<"\n";
    }else if(c=="regression-metrics"){
        exact(5);auto t=read_csv(argv[2]);
        auto r=out_of_sample_regression_metrics(
            t.numbers(argv[3]),t.numbers(argv[4]));
        out<<"MAE "<<r.mae<<"\nRMSE "<<r.rmse<<"\nBias "<<r.mean_bias<<"\n";
    }else if(c=="economic-threshold"){
        exact(5);out<<"OptimalProbabilityThreshold "<<optimal_probability_threshold(
            v(2),v(3),v(4))<<"\n";
    }else if(c=="economic-utility"){
        exact(6);out<<"ExpectedUtility "<<expected_trade_utility(
            v(2),v(3),v(4),v(5))<<"\n";
    }else if(c=="book-market-sweep"){
        exact(8);auto t=read_csv(argv[2]);
        auto side=t.numbers(argv[3]),price=t.numbers(argv[4]),
             size=t.numbers(argv[5]);
        BookReplay book;
        for(int i=0;i<int(side.size());i++){
            require(side[i]==0||side[i]==1,"Book side must be bid 1 or ask 0");
            apply_orderbook_level(book,side[i]==1,price[i],size[i]);
        }
        auto result=sweep_visible_orderbook(book,massive_market_side(argv[6]),v(7));
        out<<"Filled "<<result.filled<<"\nRemaining "<<result.remaining
           <<"\nVWAP "<<result.vwap<<"\nNotional "<<result.notional<<"\n";
    }else if(c=="market-vwap"){
        exact(5);auto t=read_csv(argv[2]);
        out<<"MarketVWAP "<<market_volume_weighted_price(
            t.numbers(argv[3]),t.numbers(argv[4]))<<"\n";
    }else if(c=="vwap-slippage"){
        exact(5);out<<"Slippage "<<vwap_slippage_fraction(
            v(2),v(3),massive_market_side(argv[4]))<<"\n";
    }else if(c=="trade-durations"){
        exact(4);auto r=trade_arrival_duration_stats(
            read_csv(argv[2]).numbers(argv[3]));
        out<<"MeanSeconds "<<r.average_seconds<<"\nDurationCV "
           <<r.coefficient_of_variation<<"\n";
    }else if(c=="volume-bars"){
        exact(6);auto t=read_csv(argv[2]);
        auto bars=fixed_volume_bars(t.numbers(argv[3]),
                                  t.numbers(argv[4]),v(5));
        out<<"Bar,Volume,VWAP\n";
        for(int i=0;i<int(bars.size());i++)
            out<<i<<","<<bars[i].volume<<","<<bars[i].vwap<<"\n";
    }else if(c=="inventory-penalty"){
        exact(7);out<<"RiskPenalty "<<inventory_variance_penalty(
            v(2),v(3),v(4),v(5),v(6))<<"\n";
    }else if(c=="inventory-mark"){
        exact(5);out<<"InventoryPnL "<<inventory_mark_to_market_pnl(
            v(2),v(3),v(4))<<"\n";
    }else return false;
    return true;
}
inline void massive_finance_usage(std::ostream& out){
    out<<"Advanced derivatives, credit, financial ML and execution:\n"
       <<"  parity-forward SPOT K RATE DIVIDEND T\n"
       <<"  parity-error CALL PUT SPOT K RATE DIVIDEND T\n"
       <<"  american-crr SPOT K RATE DIVIDEND VOL T STEPS CALL|PUT\n"
       <<"  black76-price|black76-vega FORWARD K RATE VOL T CALL|PUT\n"
       <<"  bachelier-price|bachelier-delta FORWARD K RATE NORMAL_VOL T CALL|PUT\n"
       <<"  barrier-down-bridge|barrier-up-bridge S0 S1 BARRIER VOL YEARS\n"
       <<"  implied-black-vol OPTION_PRICE SPOT K RATE DIVIDEND T CALL|PUT\n"
       <<"  call-monotonicity|call-convexity CSV STRIKE CALL_PRICE\n"
       <<"  dupire-local-variance K CALL_VALUE DCDT DCDK D2CDK2 RATE DIVIDEND\n"
       <<"  cds-protection CSV TENOR HAZARD MATURITY DISCOUNT_RATE RECOVERY FREQUENCY\n"
       <<"  fx-cross-rate A B | fx-triangular-error A B DIRECT_C\n"
       <<"  futures-variation CSV SETTLE CONTRACTS MULTIPLIER CASH\n"
       <<"  futures-margin-shortfall CSV SETTLE CONTRACTS MULTIPLIER CASH MAINTENANCE\n"
       <<"  poisson-glm CSV FEATURE COUNT QUERY LEARNING_RATE EPOCHS L2\n"
       <<"  multiclass-loss CSV PROB0 PROB1 LABEL\n"
       <<"  reliability-bins CSV PROB LABEL BINS\n"
       <<"  interval-coverage CSV REALIZED LOWER UPPER\n"
       <<"  online-moments CSV VALUE | regression-metrics CSV ACTUAL PREDICTION\n"
       <<"  economic-threshold GAIN LOSS COST | economic-utility PROB GAIN LOSS COST\n"
       <<"  book-market-sweep CSV BID_FLAG PRICE SIZE BUY|SELL SHARES\n"
       <<"  market-vwap CSV PRICE VOLUME | vwap-slippage EXEC BENCH BUY|SELL\n"
       <<"  trade-durations CSV SECONDS | volume-bars CSV PRICE SIZE TARGET\n"
       <<"  inventory-penalty SHARES VOL PRICE YEARS GAMMA\n"
       <<"  inventory-mark SHARES PRIOR_MID CURRENT_MID\n";
}
} // namespace kline
