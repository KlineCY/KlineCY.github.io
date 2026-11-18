#pragma once
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
#include "io.hpp"
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
namespace kline {
inline void expansion_finance_print(std::ostream& out,const std::string& name,
                                    const std::vector<double>& values){
    out<<"Index,"<<name<<"\n";
    for(int i=0;i<int(values.size());i++)out<<i<<","<<values[i]<<"\n";
}
inline ZeroCurve expansion_finance_curve(const CsvTable& table){
    return {table.numbers("Tenor"),table.numbers("ZeroRate")};
}
inline std::vector<int> expansion_finance_ints(const std::vector<double>& raw){
    check_finite(raw);std::vector<int> values;
    for(double v:raw){
        require(v==std::floor(v)&&v>=0&&v<=1000000000,
                "Expected nonnegative integer column");
        values.push_back(int(v));
    }
    return values;
}
inline bool run_expansion_finance(const std::string& command,int argc,char** argv,
                                  std::ostream& out){
    auto num=[&](int i){return parse_number(argv[i]);};
    auto exact=[&](int count){require(argc==count,"Incorrect arguments");};
    if(command=="sabr-vol"){
        exact(9);
        out<<"SABRVol "<<sabr_lognormal_volatility(
            num(2),num(3),num(4),num(5),num(6),num(7),num(8))<<"\n";
    }else if(command=="sabr-atm"){
        exact(8);
        out<<"SABRATM "<<sabr_atm_volatility(
            num(2),num(3),num(4),num(5),num(6),num(7))<<"\n";
    }else if(command=="option-reprice"){
        exact(12);
        out<<"ShockPnL "<<reprice_option_shock(
            num(2),num(3),num(4),num(5),num(6),num(7),
            {num(8),num(9),num(10)},std::string(argv[11])=="call")<<"\n";
    }else if(command=="gamma-scalp"){
        exact(7);
        out<<"GammaScalpingApprox "<<gamma_scalping_approx(
            num(2),num(3),num(4),num(5),num(6))<<"\n";
    }else if(command=="bond-risk"){
        exact(4);auto t=read_csv(argv[2]);
        auto r=continuous_bond_risk(t.numbers("Time"),
             t.numbers("Cashflow"),num(3));
        out<<"Price "<<r.price<<"\nDuration "<<r.duration<<"\n"
           <<"Convexity "<<r.convexity<<"\nDV01 "<<r.dv01<<"\n";
    }else if(command=="bond-shock"){
        exact(6);out<<"ApproximatePnL "<<bond_shock_pnl_approx(
            num(2),num(3),num(4),num(5))<<"\n";
    }else if(command=="swap-fixed"||command=="swap-payer"){
        exact(6);
        auto curve=expansion_finance_curve(read_csv(argv[2]));
        auto dates=read_csv(argv[3]).numbers("PaymentTime");
        double r=command=="swap-fixed"?
            fixed_leg_present_value(curve,dates,num(4),num(5)):
            payer_swap_present_value(curve,dates,num(4),num(5));
        out<<"PresentValue "<<r<<"\n";
    }else if(command=="swap-float"){
        exact(5);
        out<<"FloatingPresentValue "<<floating_leg_par_value(
            expansion_finance_curve(read_csv(argv[2])),num(3),num(4))<<"\n";
    }else if(command=="forward-discount"){
        exact(5);
        out<<"ForwardSimple "<<forward_rate_from_discounts(
            expansion_finance_curve(read_csv(argv[2])),num(3),num(4))<<"\n";
    }else if(command=="nelson-siegel"){
        exact(7);
        out<<"ZeroRate "<<nelson_siegel_zero(
            num(2),num(3),num(4),num(5),num(6))<<"\n";
    }else if(command=="curve-slope-ns"){
        exact(8);
        auto curve=nelson_siegel_curve({num(2),num(3)},
                                      num(4),num(5),num(6),num(7));
        out<<"Slope "<<curve_slope(curve,num(2),num(3))<<"\n";
    }else if(command=="credit-vasicek"){
        exact(5);
        out<<"ConditionalDefaultRate "<<vasicek_conditional_default_rate(
            num(2),num(3),num(4))<<"\n";
    }else if(command=="credit-simulate"){
        exact(6);
        auto t=read_csv(argv[2]);
        auto losses=one_factor_credit_losses(
            t.numbers("Exposure"),t.numbers("DefaultProbability"),
            t.numbers("LossGivenDefault"),num(3),std::stoi(argv[4]),
            std::stoul(argv[5]));
        out<<"AverageLoss "<<mean(losses)<<"\n"
           <<"P95Loss "<<quantile(losses,0.95)<<"\n";
    }else if(command=="margin-open"){
        exact(8);
        auto m=open_margin_account(num(2),num(3),num(4),num(5),num(6),num(7));
        out<<"Equity "<<m.equity<<"\nInitial "<<m.initial_margin<<"\n"
           <<"Maintenance "<<m.maintenance_margin<<"\nFreeCash "
           <<m.free_cash<<"\n";
    }else if(command=="margin-mark"){
        exact(9);
        auto initial=open_margin_account(num(2),num(3),num(4),num(6),
                                         num(7),num(8));
        auto updated=mark_margin_account(initial,num(3),num(4),num(5),
                                         num(6),num(7),num(8));
        out<<"Equity "<<updated.equity<<"\n"
           <<"MarginCall "<<(margin_call(updated)?1:0)<<"\n";
    }else if(command=="execution-optimal"){
        exact(7);
        expansion_finance_print(out,"ChildQuantity",
            almgren_chriss_schedule(num(2),std::stoi(argv[3]),
                                    num(4),num(5),num(6)));
    }else if(command=="execution-quadratic"){
        exact(6);
        out<<"ExecutionCost "<<quadratic_execution_cost(
            read_csv(argv[2]).numbers(argv[3]),num(4),num(5))<<"\n";
    }else if(command=="book-top"){
        exact(6);
        LimitBook book;
        update_limit_level(book,true,num(2),num(3));
        update_limit_level(book,false,num(4),num(5));
        out<<"Mid "<<book_mid_price(book)<<"\nMicroPrice "
           <<book_microprice(book)<<"\nImbalance "<<best_level_imbalance(book)<<"\n";
    }else if(command=="attribution-period"){
        exact(7);auto t=read_csv(argv[2]);
        auto a=decompose_single_period(t.numbers(argv[3]),
            t.numbers(argv[4]),t.numbers(argv[5]),num(6));
        out<<"Market "<<a.market<<"\nTrading "<<a.trading<<"\n"
           <<"Fees "<<a.fees<<"\nTotal "<<a.total<<"\n";
    }else if(command=="factor-return-contrib"){
        exact(6);auto t=read_csv(argv[2]);
        out<<"Contribution "<<linear_factor_attribution(
            t.numbers(argv[3]),t.numbers(argv[4]),num(5))<<"\n";
    }else if(command=="triple-barrier"){
        exact(8);
        auto r=label_triple_barrier(read_csv(argv[2]).numbers(argv[3]),
            std::stoi(argv[4]),std::stoi(argv[5]),num(6),num(7));
        out<<"EndIndex "<<r.end<<"\nLabel "<<r.direction<<"\n"
           <<"RealizedReturn "<<r.realized_return<<"\n";
    }else if(command=="barrier-series"){
        exact(7);
        auto labels=label_all_barriers(read_csv(argv[2]).numbers(argv[3]),
                 std::stoi(argv[4]),num(5),num(6));
        out<<"Start,End,Label,RealizedReturn\n";
        for(const auto& v:labels)
            out<<v.start<<","<<v.end<<","<<v.direction<<","
               <<v.realized_return<<"\n";
    }else if(command=="purged-folds"){
        exact(5);auto t=read_csv(argv[2]);
        auto starts=expansion_finance_ints(t.numbers("Start"));
        auto ends=expansion_finance_ints(t.numbers("End"));
        std::vector<LabeledPeriod> periods;
        for(int i=0;i<int(starts.size());i++)
            periods.push_back({starts[i],ends[i]});
        auto folds=purged_kfold(periods,std::stoi(argv[3]),
                                std::stoi(argv[4]));
        out<<"Fold,TrainCount,TestCount\n";
        for(int i=0;i<int(folds.size());i++)
            out<<i<<","<<folds[i].training.size()<<","
               <<folds[i].test.size()<<"\n";
    }else if(command=="expanding-ends"){
        exact(6);
        auto values=expanding_train_endpoints(std::stoi(argv[2]),
            std::stoi(argv[3]),std::stoi(argv[4]),std::stoi(argv[5]));
        out<<"Index,TrainEnd\n";
        for(int i=0;i<int(values.size());i++)
            out<<i<<","<<values[i]<<"\n";
    }else if(command=="calibration-ece"||command=="calibration-brier"){
        require(argc==5||argc==6,"Calibration CSV PROB LABEL [BINS]");
        auto t=read_csv(argv[2]);
        auto p=t.numbers(argv[3]);
        auto labels=expansion_finance_ints(t.numbers(argv[4]));
        if(command=="calibration-ece"){
            exact(6);out<<"ECE "<<expected_calibration_error(
                p,labels,std::stoi(argv[5]))<<"\n";
        }else{
            exact(5);out<<"Brier "<<binary_brier(p,labels)<<"\n";
        }
    }else if(command=="cashflow-account"){
        exact(6);auto t=read_csv(argv[2]);
        auto ledger=time_weighted_equity(num(3),
            t.numbers(argv[4]),t.numbers(argv[5]));
        out<<"EndingEquity "<<ledger.ending_equity<<"\n"
           <<"NetContributions "<<ledger.total_contribution<<"\n";
    }else if(command=="investor-irr"){
        exact(4);
        out<<"IRR "<<money_weighted_irr(read_csv(argv[2]).numbers(argv[3]))
           <<"\n";
    }else if(command=="twr-growth"){
        exact(4);
        out<<"GrowthFactor "<<time_weighted_growth(
            read_csv(argv[2]).numbers(argv[3]))<<"\n";
    }else if(command=="asof-numeric"){
        exact(4);auto records=read_csv(argv[2]),decisions=read_csv(argv[3]);
        auto dates=expansion_finance_ints(records.numbers("Date"));
        auto values=records.numbers("Value");
        std::vector<DatedValue> points;
        for(int i=0;i<int(dates.size());i++)points.push_back({dates[i],values[i]});
        auto query=expansion_finance_ints(decisions.numbers("Date"));
        expansion_finance_print(out,"AsOf",asof_last_known(points,query));
    }else if(command=="align-numeric"){
        exact(4);auto a=read_csv(argv[2]),b=read_csv(argv[3]);
        auto da=expansion_finance_ints(a.numbers("Date"));
        auto db=expansion_finance_ints(b.numbers("Date"));
        auto va=a.numbers("Value"),vb=b.numbers("Value");
        std::vector<DatedValue> aa,bb;
        for(int i=0;i<int(da.size());i++)aa.push_back({da[i],va[i]});
        for(int i=0;i<int(db.size());i++)bb.push_back({db[i],vb[i]});
        std::vector<double> result_b;
        auto result=aligned_intersection(aa,bb,result_b);
        out<<"Date,A,B\n";
        for(int i=0;i<int(result.size());i++)
            out<<result[i].date<<","<<result[i].value<<","
               <<result_b[i]<<"\n";
    }else if(command=="stock-action"){
        exact(6);
        out<<"TotalReturn "<<stock_total_return(num(2),num(3),num(4),
                                                 num(5))<<"\n";
    }else if(command=="stock-action-series"){
        exact(6);auto t=read_csv(argv[2]);
        expansion_finance_print(out,"TotalReturn",stock_total_returns(
            t.numbers(argv[3]),t.numbers(argv[4]),t.numbers(argv[5])));
    }else if(command=="rights-exprice"){
        exact(5);out<<"TheoreticalExPrice "<<
           rights_adjusted_theoretical_ex_price(num(2),num(3),num(4))<<"\n";
    }else if(command=="underwater-episodes"){
        exact(4);
        auto e=drawdown_episodes(read_csv(argv[2]).numbers(argv[3]));
        out<<"Peak,Trough,Recovery,Drawdown\n";
        for(const auto& v:e)
            out<<v.peak<<","<<v.trough<<","<<v.recovery<<","
               <<v.deepest<<"\n";
    }else if(command=="underwater-duration"){
        exact(4);
        out<<"MaximumDuration "<<max_drawdown_duration(
            read_csv(argv[2]).numbers(argv[3]))<<"\n";
    }else if(command=="kupiec-backtest"){
        exact(6);auto t=read_csv(argv[2]);
        auto v=kupiec_var_coverage(t.numbers(argv[3]),
                       t.numbers(argv[4]),num(5));
        out<<"Exceptions "<<v.exceptions<<"\nExceptionRate "
           <<v.realized_exception_rate<<"\nKupiecLR "<<v.kupiec_lr<<"\n";
    }else if(command=="es-breach-residual"){
        exact(6);auto t=read_csv(argv[2]);
        out<<"ESResidual "<<expected_shortfall_residual(
            t.numbers(argv[3]),t.numbers(argv[4]),
            t.numbers(argv[5]))<<"\n";
    }else if(command=="rolling-beta"){
        exact(6);auto t=read_csv(argv[2]);
        expansion_finance_print(out,"Beta",rolling_factor_beta(
            t.numbers(argv[3]),t.numbers(argv[4]),std::stoi(argv[5])));
    }else if(command=="hedge-beta"){
        exact(6);auto t=read_csv(argv[2]);
        expansion_finance_print(out,"HedgedWeight",hedge_factor_exposure(
            t.numbers(argv[3]),t.numbers(argv[4]),
            std::stoi(argv[5])));
    }else if(command=="liquidation-days"){
        exact(6);auto t=read_csv(argv[2]);
        out<<"LiquidationDays "<<liquidation_days(t.numbers(argv[3]),
                   t.numbers(argv[4]),num(5))<<"\n";
    }else if(command=="liquidation-cost"){
        exact(7);auto t=read_csv(argv[2]);
        out<<"TotalCost "<<staged_liquidation_cost(
            t.numbers(argv[3]),t.numbers(argv[4]),
            t.numbers(argv[5]),num(6))<<"\n";
    }else if(command=="quote-ofi"){
        exact(10);out<<"OFI "<<best_quote_order_flow(
            num(2),num(3),num(4),num(5),num(6),num(7),
            num(8),num(9))<<"\n";
    }else if(command=="weighted-ofi"){
        exact(5);auto t=read_csv(argv[2]);
        out<<"WeightedOFI "<<weighted_order_flow(
            t.numbers(argv[3]),t.numbers(argv[4]))<<"\n";
    }else if(command=="kelly-binary"){
        exact(5);out<<"KellyFraction "<<fraction_kelly_binary(
            num(2),num(3),num(4))<<"\n";
    }else if(command=="kelly-normal"){
        exact(5);out<<"KellyWeight "<<normal_kelly_fraction(
            num(2),num(3),num(4))<<"\n";
    }else if(command=="vol-size"){
        exact(6);out<<"Position "<<volatility_scale_position(
            num(2),num(3),num(4),num(5))<<"\n";
    }else if(command=="jump-correction"){
        exact(5);out<<"DriftCorrection "<<merton_jump_drift_correction(
            num(2),num(3),num(4))<<"\n";
    }else if(command=="jump-simulate"){
        exact(11);
        auto prices=merton_jump_prices(num(2),num(3),num(4),num(5),
            num(6),num(7),num(8),std::stoi(argv[9]),std::stoul(argv[10]));
        expansion_finance_print(out,"SimulatedPrice",prices);
    }else if(command=="holdings-entropy"||command=="holdings-hhi"){
        exact(4);
        auto weights=read_csv(argv[2]).numbers(argv[3]);
        if(command=="holdings-entropy")
            out<<"Entropy "<<holdings_entropy(weights)<<"\n"
               <<"EffectiveHoldings "<<holdings_effective_number(weights)<<"\n";
        else out<<"Herfindahl "<<herfindahl_exposure(weights)<<"\n";
    }else if(command=="project-npv"){
        exact(6);
        out<<"NPV "<<net_present_value(num(3),
                 read_csv(argv[2]).numbers(argv[4]),num(5))<<"\n";
    }else if(command=="project-wacc"){
        exact(6);
        out<<"WACC "<<weighted_average_cost_of_capital(
            num(2),num(3),num(4),num(5))<<"\n";
    }else if(command=="perpetuity"){
        exact(5);out<<"Value "<<perpetuity_value(num(2),num(3),num(4))<<"\n";
    }else return false;
    return true;
}
inline void expansion_finance_usage(std::ostream& out){
    out<<"  sabr-vol FORWARD STRIKE YEARS ALPHA BETA RHO NU\n"
       <<"  sabr-atm FORWARD YEARS ALPHA BETA RHO NU\n"
       <<"  option-reprice SPOT STRIKE RATE DIV VOL YEARS D_SPOT D_VOL D_RATE call|put\n"
       <<"  gamma-scalp GAMMA IMPLIED_VOL REALIZED_QUADRATIC_VARIATION YEARS SPOT\n"
       <<"  bond-risk CASHFLOW_CSV YIELD\n"
       <<"  bond-shock PRICE DURATION CONVEXITY RATE_SHOCK\n"
       <<"  swap-fixed|swap-payer CURVE_CSV PAYMENT_CSV RATE NOTIONAL\n"
       <<"  swap-float CURVE_CSV YEARS NOTIONAL\n"
       <<"  forward-discount CURVE_CSV START END\n"
       <<"  nelson-siegel YEARS LEVEL SLOPE CURVATURE DECAY\n"
       <<"  curve-slope-ns SHORT_YEARS LONG_YEARS LEVEL SLOPE CURVATURE DECAY\n"
       <<"  credit-vasicek PD FACTOR_CORRELATION SYSTEMIC_SHOCK\n"
       <<"  credit-simulate CREDIT_CSV CORRELATION SCENARIOS SEED\n"
       <<"  margin-open EQUITY CONTRACTS ENTRY MULTIPLIER INITIAL_RATE MAINTENANCE_RATE\n"
       <<"  margin-mark EQUITY CONTRACTS ENTRY NEW_SETTLE MULTIPLIER INITIAL_RATE MAINTENANCE_RATE\n"
       <<"  execution-optimal SHARES SLICES TEMP_IMPACT DAILY_VOL RISK_AVERSION\n"
       <<"  execution-quadratic CSV QUANTITY TEMP_IMPACT FEE_PER_SHARE\n"
       <<"  book-top BID BID_SIZE ASK ASK_SIZE\n"
       <<"  attribution-period CSV START_WEIGHT END_WEIGHT RETURN COST_RATE\n"
       <<"  factor-return-contrib CSV EXPOSURE FACTOR_RETURN SPECIFIC\n"
       <<"  triple-barrier CSV PRICE START HORIZON PROFIT STOP\n"
       <<"  barrier-series CSV PRICE HORIZON PROFIT STOP\n"
       <<"  purged-folds EVENTS_CSV FOLDS EMBARGO\n"
       <<"  expanding-ends OBSERVATIONS TRAIN_START STEP HORIZON\n"
       <<"  calibration-ece CSV PROB LABEL BINS\n"
       <<"  calibration-brier CSV PROB LABEL\n"
       <<"  cashflow-account CSV INITIAL RETURN CASHFLOW\n"
       <<"  investor-irr CSV CASHFLOW\n"
       <<"  twr-growth CSV RETURN\n"
       <<"  asof-numeric PUBLISHED_CSV DECISION_CSV\n"
       <<"  align-numeric SERIES_A_CSV SERIES_B_CSV\n"
       <<"  stock-action OLD_PRICE NEW_PRICE DIVIDEND SPLIT_FACTOR\n"
       <<"  stock-action-series CSV PRICE DIVIDEND SPLIT_FACTOR\n"
       <<"  rights-exprice PRICE SUBSCRIPTION_RATIO SHARES_PER_OLD\n"
       <<"  underwater-episodes|underwater-duration CSV EQUITY\n"
       <<"  kupiec-backtest CSV REALIZED_LOSS VAR CONFIDENCE\n"
       <<"  es-breach-residual CSV LOSS VAR ES\n"
       <<"  rolling-beta CSV ASSET_RETURN FACTOR_RETURN WINDOW\n"
       <<"  hedge-beta CSV WEIGHTS BETAS HEDGE_INDEX\n"
       <<"  liquidation-days CSV SHARES ADV PARTICIPATION\n"
       <<"  liquidation-cost CSV SHARES SPREAD PARTICIPATION IMPACT_COEFFICIENT\n"
       <<"  quote-ofi BID_OLD BID_NEW BIDSIZE_OLD BIDSIZE_NEW ASK_OLD ASK_NEW ASKSIZE_OLD ASKSIZE_NEW\n"
       <<"  weighted-ofi CSV SIGNED_VOLUME WEIGHT\n"
       <<"  kelly-binary PROBABILITY PAYOFF FRACTION\n"
       <<"  kelly-normal ALPHA VARIANCE CAP\n"
       <<"  vol-size TARGET_VOL EST_VOL SIGNAL GROSS_CAP\n"
       <<"  jump-correction INTENSITY LOGJUMP_MEAN LOGJUMP_STD\n"
       <<"  jump-simulate SPOT DRIFT VOL INTENSITY LOGJUMP_MEAN LOGJUMP_STD DT STEPS SEED\n"
       <<"  holdings-entropy|holdings-hhi CSV WEIGHT\n"
       <<"  project-npv CSV INITIAL_INVESTMENT CASHFLOW_COLUMN DISCOUNT\n"
       <<"  project-wacc EQUITY_WEIGHT COST_EQUITY COST_DEBT TAX\n"
       <<"  perpetuity NEXT_CF GROWTH DISCOUNT\n";
}
} // namespace kline
