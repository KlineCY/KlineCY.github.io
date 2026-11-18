#pragma once
#include "optimization.hpp"
#include "ols_models.hpp"
#include "elastic_models.hpp"
#include "clustering_models.hpp"
#include "cointegration_models.hpp"
#include "state_models.hpp"
#include "factor_advanced.hpp"
#include "credit_models.hpp"
#include "microstructure_advanced.hpp"
#include "hedging_models.hpp"
#include "stress_models.hpp"
#include "liquidity_models.hpp"
#include "metrics_models.hpp"
#include "resampling_models.hpp"
#include "fixed_income_advanced.hpp"
#include "futures_models.hpp"
#include "fx_models.hpp"
#include "data_quality.hpp"
#include "performance_advanced.hpp"
#include "signal_models.hpp"
#include "market_making_models.hpp"
#include "volatility_surface.hpp"
#include "validation_advanced.hpp"
#include "portfolio_constraints.hpp"
#include "io.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace kline {
inline Matrix fullstack_feature_rows(const CsvTable& table,int first,int argc,
                                      char** argv) {
    require(argc>first,"At least one feature column is required");
    Matrix x(table.rows.size());
    for(int j=first;j<argc;j++){
        auto values=table.numbers(argv[j]);
        for(int t=0;t<int(values.size());t++)x[t].push_back(values[t]);
    }
    return x;
}

inline void fullstack_print_vector(std::ostream& out,
                                   const std::string& label,
                                   const std::vector<double>& values) {
    out<<"Index,"<<label<<"\n";
    for(int i=0;i<int(values.size());i++)
        out<<i<<","<<values[i]<<"\n";
}

inline bool run_fullstack_cli(const std::string& cmd,int argc,char** argv,
                               std::ostream& out) {
    auto number=[&](int i){return parse_number(argv[i]);};
    auto exact=[&](int n){require(argc==n,"Wrong number of arguments");};
    if(cmd=="box-project"){
        exact(6);
        auto table=read_csv(argv[2]);
        auto values=table.numbers(argv[3]),lower=table.numbers(argv[4]);
        fullstack_print_vector(out,"Projected",project_box(values,lower,
                                                           table.numbers(argv[5])));
    }else if(cmd=="l1-project"){
        exact(5);
        fullstack_print_vector(out,"Projected",project_l1_ball(
            read_csv(argv[2]).numbers(argv[3]),number(4)));
    }else if(cmd=="qp2"){
        exact(11);
        Matrix h={{number(2),number(3)},{number(3),number(4)}};
        auto x=solve_box_quadratic(h,{number(5),number(6)},
                                    {number(7),number(9)},
                                    {number(8),number(10)});
        out<<"X "<<x[0]<<"\nY "<<x[1]<<"\n"
           <<"Objective "<<quadratic_objective(h,{number(5),number(6)},x)<<"\n";
    }else if(cmd=="ols"){
        require(argc>=5,"OLS requires label and one or more features");
        auto table=read_csv(argv[2]);
        auto result=ordinary_least_squares(
            fullstack_feature_rows(table,4,argc,argv),table.numbers(argv[3]));
        out<<"Intercept "<<result.coefficients[0]<<"\n";
        for(int i=4;i<argc;i++)out<<argv[i]<<" "<<result.coefficients[i-3]<<"\n";
        out<<"RSquared "<<result.r_squared<<"\nMSE "<<result.mse<<"\n";
    }else if(cmd=="elastic"){
        require(argc>=7,"Elastic-net needs label, two penalties and features");
        auto table=read_csv(argv[2]);
        auto model=fit_elastic_net(fullstack_feature_rows(table,6,argc,argv),
                                   table.numbers(argv[3]),number(4),number(5));
        out<<"StandardizedIntercept "<<model.intercept<<"\n";
        for(int i=6;i<argc;i++)
            out<<argv[i]<<" "<<model.coefficients[i-6]/model.feature_scale[i-6]<<"\n";
    }else if(cmd=="kmeans"){
        require(argc>=5,"K-means needs clusters and feature columns");
        auto model=fit_kmeans(fullstack_feature_rows(read_csv(argv[2]),4,argc,argv),
                              std::stoi(argv[3]));
        out<<"Index,Cluster\n";
        for(int i=0;i<int(model.labels.size());i++)
            out<<i<<","<<model.labels[i]<<"\n";
        out<<"Inertia "<<model.inertia<<"\n";
    }else if(cmd=="cointegration"||cmd=="unitroot"||cmd=="spread-z"){
        if(cmd=="cointegration"){
            exact(5);auto table=read_csv(argv[2]);
            auto result=two_asset_spread_regression(table.numbers(argv[3]),
                                                     table.numbers(argv[4]));
            out<<"HedgeRatio "<<result.hedge_ratio<<"\n"
               <<"Intercept "<<result.intercept<<"\n"
               <<"ResidualAR1 "<<result.residual_ar_slope<<"\n"
               <<"ResidualHalfLife "<<result.residual_half_life<<"\n";
        }else if(cmd=="unitroot"){
            exact(4);
            out<<"DickeyFullerT "<<unit_root_dickey_fuller_t(
                read_csv(argv[2]).numbers(argv[3]))<<"\n";
        }else{
            exact(5);
            fullstack_print_vector(out,"SpreadZScore",spread_zscores(
                read_csv(argv[2]).numbers(argv[3]),std::stoi(argv[4])));
        }
    }else if(cmd=="kalman"){
        exact(8);
        auto filtered=kalman_filter(read_csv(argv[2]).numbers(argv[3]),
                                    {number(4),number(5)},number(6),number(7));
        out<<"Index,State,Variance\n";
        for(int i=0;i<int(filtered.size());i++)
            out<<i<<","<<filtered[i].state<<","<<filtered[i].variance<<"\n";
    }else if(cmd=="regime"){
        exact(10);
        auto posterior=markov_filter_step({number(2),number(3)},
                         {{number(4),number(5)},{number(6),number(7)}},
                         {number(8),number(9)});
        out<<"State0 "<<posterior[0]<<"\nState1 "<<posterior[1]<<"\n";
    }else if(cmd=="factor-neutral"||cmd=="factor-rank"||cmd=="factor-buckets"){
        auto table=read_csv(argv[2]);
        if(cmd=="factor-neutral"){
            require(argc>=5,"Factor neutralization needs control columns");
            auto residuals=multi_factor_residuals(table.numbers(argv[3]),
                      fullstack_feature_rows(table,4,argc,argv));
            fullstack_print_vector(out,"Residual",residuals);
        }else if(cmd=="factor-rank"){
            exact(5);
            fullstack_print_vector(out,"SignedWeight",
                cross_sectional_rank_weights(table.numbers(argv[3]),number(4)));
        }else{
            exact(6);
            fullstack_print_vector(out,"BucketReturn",quantile_bucket_returns(
                table.numbers(argv[3]),table.numbers(argv[4]),std::stoi(argv[5])));
        }
    }else if(cmd=="credit-hazard"){
        exact(4);
        out<<"AnnualHazard "<<implied_hazard_from_cds(number(2),number(3))<<"\n";
    }else if(cmd=="credit-ecl"){
        exact(5);
        out<<"ExpectedLoss "<<expected_credit_loss(number(2),number(3),number(4))<<"\n";
    }else if(cmd=="credit-bond"){
        exact(7);
        out<<"RiskiestZeroPrice "<<risky_zero_coupon_price(
            number(2),number(3),number(4),number(5),number(6))<<"\n";
    }else if(cmd=="roll-spread"){
        exact(4);
        out<<"RollSpread "<<roll_effective_spread(read_csv(argv[2]).numbers(argv[3]))
           <<"\n";
    }else if(cmd=="kyle"){
        exact(5);auto table=read_csv(argv[2]);
        out<<"KyleSlope "<<kyle_lambda(table.numbers(argv[3]),
                                       table.numbers(argv[4]))<<"\n";
    }else if(cmd=="realized-jump"){
        exact(4);
        auto series=read_csv(argv[2]).numbers(argv[3]);
        out<<"RealizedVariance "<<realized_variance(series)<<"\n"
           <<"BipowerVariation "<<bipower_variation(series)<<"\n"
           <<"NonnegativeJumpProxy "<<realized_jump_variation(series)<<"\n";
    }else if(cmd=="option-hedge"){
        exact(7);
        out<<"ApproximatePnL "<<delta_gamma_vega_pnl(
            number(2),number(3),number(4),number(5),number(6))<<"\n";
    }else if(cmd=="futures-hedge"){
        exact(6);
        out<<"Contracts "<<hedge_contract_count(
            number(2),number(3),number(4),number(5))<<"\n";
    }else if(cmd=="stress-worst"){
        require(argc>=7&&(argc-3)%2==0,"Stress needs WEIGHT RETURN pairs");
        auto table=read_csv(argv[2]);std::vector<double> weights;
        Matrix scenarios(table.rows.size());
        for(int j=3;j<argc;j+=2){
            weights.push_back(number(j));
            auto series=table.numbers(argv[j+1]);
            for(int t=0;t<int(series.size());t++)
                scenarios[t].push_back(series[t]);
        }
        out<<"WorstReturn "<<worst_scenario_return(weights,scenarios)<<"\n";
    }else if(cmd=="vol-ramp"){
        exact(7);
        out<<"AllowedGross "<<volatility_target_gross(
            number(2),number(3),number(4),number(5),number(6))<<"\n";
    }else if(cmd=="amihud"){
        exact(5);auto table=read_csv(argv[2]);
        out<<"Amihud "<<amihud_illiquidity(table.numbers(argv[3]),
                                          table.numbers(argv[4]))<<"\n";
    }else if(cmd=="impact-cost"){
        exact(5);
        out<<"ImpactFraction "<<expected_square_root_impact(
            number(2),number(3),number(4))<<"\n";
    }else if(cmd=="alpha-after-cost"){
        exact(7);
        out<<"NetAlpha "<<net_alpha_after_cost(
            number(2),number(3),number(4),number(5),number(6))<<"\n";
    }else if(cmd=="model-metrics"){
        exact(6);auto table=read_csv(argv[2]);
        auto p=table.numbers(argv[3]),y=table.numbers(argv[4]);
        std::vector<int> labels;
        for(double value:y){
            require(value==0||value==1,"Binary labels must be 0 or 1");
            labels.push_back(int(value));
        }
        auto c=binary_confusion(p,labels,number(5));
        out<<"AUC "<<classification_auc(p,labels)<<"\n"
           <<"LogLoss "<<binary_log_loss(p,labels)<<"\n"
           <<"F1 "<<f1_score(c)<<"\n";
    }else if(cmd=="oos-r2"){
        exact(6);auto table=read_csv(argv[2]);
        out<<"OOSR2 "<<out_of_sample_r_squared(
            table.numbers(argv[4]),table.numbers(argv[3]),number(5))<<"\n";
    }else if(cmd=="perm-corr"){
        exact(7);auto table=read_csv(argv[2]);
        out<<"PermutationP "<<permutation_p_value_correlation(
            table.numbers(argv[3]),table.numbers(argv[4]),
            std::stoi(argv[5]),std::stoul(argv[6]))<<"\n";
    }else if(cmd=="jackknife"){
        exact(4);
        auto x=read_csv(argv[2]).numbers(argv[3]);
        out<<"JackknifeSE "<<jackknife_standard_error(
            delete_one_jackknife_means(x))<<"\n";
    }else if(cmd=="zero-bootstrap"){
        exact(5);
        require(number(2)>0&&number(3)>0&&number(4)>0,
                "Zero bootstrap needs maturity, price and redemption");
        out<<"ZeroRate "<<-std::log(number(3)/number(4))/number(2)<<"\n";
    }else if(cmd=="bond-ytm"){
        exact(4);auto table=read_csv(argv[2]);
        out<<"ContinuousYTM "<<yield_to_maturity_continuous(
            number(3),table.numbers("Time"),table.numbers("Cashflow"))<<"\n";
    }else if(cmd=="futures-pnl"){
        exact(6);
        out<<"Pnl "<<held_futures_pnl(number(2),number(3),number(4),number(5))<<"\n";
    }else if(cmd=="futures-basis"){
        exact(5);
        out<<"AnnualLogBasis "<<annualized_futures_basis(
            number(2),number(3),number(4))<<"\n";
    }else if(cmd=="fx-translate"){
        exact(5);
        out<<"DomesticReturn "<<foreign_asset_return_in_base(
            number(2),number(3),number(4))<<"\n";
    }else if(cmd=="fx-hedge"){
        exact(5);
        out<<"SettlementPnL "<<fx_forward_hedge_pnl(
            number(2),number(3),number(4))<<"\n";
    }else if(cmd=="fx-cross"){
        exact(4);
        out<<"CrossRate "<<fx_cross_rate(number(2),number(3))<<"\n";
    }else if(cmd=="series-lag"){
        exact(5);
        fullstack_print_vector(out,"Lagged",lagged_signal(
            read_csv(argv[2]).numbers(argv[3]),std::stoi(argv[4])));
    }else if(cmd=="price-jump"){
        exact(4);
        out<<"MaximumRelativeJump "<<max_relative_jump(
            read_csv(argv[2]).numbers(argv[3]))<<"\n";
    }else if(cmd=="performance-tail"){
        exact(4);auto returns=read_csv(argv[2]).numbers(argv[3]);
        out<<"Omega "<<omega_ratio(returns)<<"\n"
           <<"TailRatio "<<tail_ratio(returns)<<"\n"
           <<"Calmar "<<calmar_ratio(returns)<<"\n";
    }else if(cmd=="performance-trades"){
        exact(4);
        out<<"ProfitFactor "<<profit_factor(read_csv(argv[2]).numbers(argv[3]))<<"\n";
    }else if(cmd=="trend-ema"){
        exact(6);
        fullstack_print_vector(out,"Trend",time_series_trend(
            read_csv(argv[2]).numbers(argv[3]),std::stoi(argv[4]),
                                                 std::stoi(argv[5])));
    }else if(cmd=="hysteresis"){
        exact(6);
        fullstack_print_vector(out,"NextPeriodPosition",
            lagged_position_from_threshold(read_csv(argv[2]).numbers(argv[3]),
                                           number(4),number(5)));
    }else if(cmd=="position-pnl"){
        exact(6);
        auto table=read_csv(argv[2]);
        fullstack_print_vector(out,"NetReturn",strategy_returns_from_lagged_positions(
            table.numbers(argv[3]),table.numbers(argv[4]),number(5)));
    }else if(cmd=="market-maker"){
        exact(8);
        double mid=number(2),inventory=number(3),gamma=number(4),
               variance=number(5),horizon=number(6),arrival=number(7);
        out<<"Bid "<<bid_inventory_quote(mid,inventory,gamma,variance,
                                          horizon,arrival)<<"\n"
           <<"Ask "<<ask_inventory_quote(mid,inventory,gamma,variance,
                                          horizon,arrival)<<"\n";
    }else if(cmd=="variance-forward"){
        exact(6);
        out<<"ForwardVariance "<<variance_forward_rate(
            number(2),number(3),number(4),number(5))<<"\n";
    }else if(cmd=="maturity-count"){
        exact(6);
        auto values=read_csv(argv[2]).numbers(argv[3]);
        std::vector<int> mature;
        for(double value:values){
            require(value>=0&&value==std::floor(value)&&
                    value<=int(values.size())+1000000,
                    "Maturity indices must be integers");
            mature.push_back(int(value));
        }
        out<<"Eligible "<<eligible_training_count(
            mature,std::stoi(argv[4]),std::stoi(argv[5]))<<"\n";
    }else if(cmd=="turnover-cap"){
        exact(6);auto table=read_csv(argv[2]);
        fullstack_print_vector(out,"Target",rebalance_with_turnover_cap(
            table.numbers(argv[3]),table.numbers(argv[4]),number(5)));
    }else if(cmd=="neutral-weights"){
        exact(5);
        fullstack_print_vector(out,"Weight",dollar_neutral_signal(
            read_csv(argv[2]).numbers(argv[3]),number(4)));
    }else if(cmd=="gross-limit"){
        exact(5);
        fullstack_print_vector(out,"Weight",enforce_gross_limit(
            read_csv(argv[2]).numbers(argv[3]),number(4)));
    }else if(cmd=="rank-turnover"){
        exact(5);auto table=read_csv(argv[2]);
        out<<"HalfTurnover "<<rank_turnover(table.numbers(argv[3]),
                                            table.numbers(argv[4]))<<"\n";
    }else if(cmd=="putcall-forward"){
        exact(6);
        out<<"SyntheticForward "<<put_call_synthetic_forward(
            number(2),number(3),number(4),number(5))<<"\n";
    }else if(cmd=="roll-futures"){
        exact(8);
        auto result=roll_futures_position(
            number(2),number(3),number(4),number(5),number(6),number(7));
        out<<"ClosePnL "<<result.close_pnl<<"\n"
           <<"EntryCost "<<result.entry_cost<<"\n"
           <<"NewContracts "<<result.next_contracts<<"\n";
    }else if(cmd=="credit-portfolio"){
        exact(5);auto table=read_csv(argv[2]);
        out<<"PortfolioECL "<<portfolio_expected_credit_loss(
            table.numbers("Exposure"),table.numbers("DefaultProbability"),
            table.numbers("LossGivenDefault"))<<"\n";
    }else if(cmd=="circular-boot"||cmd=="stationary-boot"){
        exact(6);
        auto series=read_csv(argv[2]).numbers(argv[3]);
        std::vector<int> indices=cmd=="circular-boot"?
            circular_block_indices(int(series.size()),std::stoi(argv[4]),
                                   std::stoul(argv[5])):
            stationary_bootstrap_indices(int(series.size()),number(4),
                                         std::stoul(argv[5]));
        out<<"Index,SelectedValue\n";
        for(int i=0;i<int(indices.size());i++)
            out<<i<<","<<series[indices[i]]<<"\n";
    }else if(cmd=="butterfly"){
        exact(5);
        out<<"ButterflyCost "<<butterfly_call_cost(
            number(2),number(3),number(4))<<"\n";
    }else if(cmd=="cost-of-carry"){
        exact(7);
        out<<"FairFutures "<<cash_and_carry_futures(
            number(2),number(3),number(4),number(5),number(6))<<"\n";
    }else return false;
    return true;
}

inline void print_fullstack_usage(std::ostream& out){
    out<<"Full-stack C++17 research commands:\n"
       <<"  box-project CSV VALUES LOWERS UPPERS\n"
       <<"  l1-project CSV VALUES RADIUS\n"
       <<"  qp2 H11 H12 H22 B1 B2 LOW1 HIGH1 LOW2 HIGH2\n"
       <<"  ols CSV LABEL FEATURE...\n"
       <<"  elastic CSV LABEL L1 L2 FEATURE...\n"
       <<"  kmeans CSV CLUSTERS FEATURE...\n"
       <<"  cointegration CSV DEPENDENT HEDGE\n"
       <<"  unitroot CSV LEVEL\n"
       <<"  spread-z CSV SPREAD WINDOW\n"
       <<"  kalman CSV OBS INITIAL_STATE INITIAL_VAR PROCESS_VAR OBS_VAR\n"
       <<"  regime PRIOR0 PRIOR1 P00 P01 P10 P11 LIKE0 LIKE1\n"
       <<"  factor-neutral CSV SIGNAL CONTROL...\n"
       <<"  factor-rank CSV FACTOR GROSS\n"
       <<"  factor-buckets CSV FACTOR FUTURE_RETURN COUNT\n"
       <<"  credit-hazard CDS_SPREAD RECOVERY\n"
       <<"  credit-ecl EXPOSURE DEFAULT_PROB LGD\n"
       <<"  credit-bond FACE YEARS RATE HAZARD RECOVERY\n"
       <<"  roll-spread CSV TRADE_PRICE\n"
       <<"  kyle CSV PRICE_CHANGE SIGNED_VOLUME\n"
       <<"  realized-jump CSV LOG_RETURN\n"
       <<"  option-hedge DELTA GAMMA VEGA SPOT_CHANGE VOL_CHANGE\n"
       <<"  futures-hedge EXPOSURE HEDGE_RATIO FUTURES_PRICE MULTIPLIER\n"
       <<"  stress-worst CSV WEIGHT RETURN [PAIRS...]\n"
       <<"  vol-ramp VOL TARGET PREV_LEVERAGE STEP_LIMIT GROSS_CAP\n"
       <<"  amihud CSV RETURN DOLLAR_VOLUME\n"
       <<"  impact-cost PARTICIPATION DAILY_VOL COEFFICIENT\n"
       <<"  alpha-after-cost GROSS_ALPHA TRADED_NOTIONAL SPREAD_BPS COMMISSION_BPS IMPACT_BPS\n"
       <<"  model-metrics CSV PROBABILITY BINARY_LABEL THRESHOLD\n"
       <<"  oos-r2 CSV REALIZED PREDICTED TRAIN_MEAN\n"
       <<"  perm-corr CSV X Y REPLICATES SEED\n"
       <<"  jackknife CSV DATA\n"
       <<"  zero-bootstrap YEARS PRICE FINAL_REDEMPTION\n"
       <<"  bond-ytm CASHFLOW_CSV BOND_PRICE\n"
       <<"  futures-pnl OLD_SETTLEMENT NEW_SETTLEMENT CONTRACTS MULTIPLIER\n"
       <<"  futures-basis FUTURES_PRICE SPOT_PRICE YEARS\n"
       <<"  fx-translate FOREIGN_ASSET_RETURN START_FX END_FX\n"
       <<"  fx-hedge FOREIGN_NOTIONAL FORWARD_STRIKE SETTLEMENT_FX\n"
       <<"  fx-cross QUOTE_A_PER_B QUOTE_C_PER_A\n"
       <<"  series-lag CSV SIGNAL DELAY\n"
       <<"  price-jump CSV PRICE\n"
       <<"  performance-tail CSV RETURN\n"
       <<"  performance-trades CSV TRADE_PNL\n"
       <<"  trend-ema CSV PRICE FAST_SPAN SLOW_SPAN\n"
       <<"  hysteresis CSV SIGNAL ENTRY EXIT\n"
       <<"  position-pnl CSV POSITION RETURN ONE_WAY_COST_FRACTION\n"
       <<"  market-maker MID INVENTORY GAMMA VARIANCE HORIZON ARRIVAL_PARAMETER\n"
       <<"  variance-forward T1 VOL1 T2 VOL2\n"
       <<"  maturity-count CSV MATURITY_INDEX DECISION GAP\n"
       <<"  turnover-cap CSV OLD_WEIGHT DESIRED_WEIGHT MAX_ONEWAY\n"
       <<"  neutral-weights CSV FACTOR TARGET_GROSS\n"
       <<"  gross-limit CSV WEIGHT TARGET_MAX_GROSS\n"
       <<"  rank-turnover CSV WEIGHT_BEFORE WEIGHT_AFTER\n"
       <<"  putcall-forward CALL PUT STRIKE DISCOUNT\n"
       <<"  roll-futures OLD_ENTRY OLD_EXIT OLD_CONTRACTS NEW_ENTRY MULTIPLIER COMMISSION\n"
       <<"  credit-portfolio CSV [Exposure,DefaultProbability,LossGivenDefault]\n"
       <<"  circular-boot CSV DATA BLOCK_LENGTH SEED\n"
       <<"  stationary-boot CSV DATA EXPECTED_BLOCK SEED\n"
       <<"  butterfly LOWER_CALL MID_CALL UPPER_CALL\n"
       <<"  cost-of-carry SPOT RATE STORAGE INCOME YEARS\n";
}
} // namespace kline
