#pragma once
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
#include "io.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
namespace kline {
inline int mass_int(const char* token,int lower,int upper=100000){
    double d=parse_number(token);
    require(d>=lower&&d<=upper&&std::floor(d)==d,"Invalid integer command input");
    return int(d);
}
inline void massive_output_vector(std::ostream& out,const std::string& name,
    const std::vector<double>& values){
    out<<"Index,"<<name<<"\n";
    for(int i=0;i<int(values.size());i++)out<<i<<","<<values[i]<<"\n";
}
inline bool run_massive_stats_cli(const std::string& c,int argc,char** argv,
                                 std::ostream& out){
    auto exact=[&](int n){require(argc==n,"Invalid statistics command arity");};
    auto v=[&](int index){return parse_number(argv[index]);};
    if(c=="variance-ratio"){
        exact(5);
        out<<"VarianceRatio "<<overlapping_variance_ratio(
            read_csv(argv[2]).numbers(argv[3]),mass_int(argv[4],2))<<"\n";
    }else if(c=="ljung-box"){
        exact(5);
        out<<"LjungBoxQ "<<ljung_box_q_statistic(
            read_csv(argv[2]).numbers(argv[3]),mass_int(argv[4],1))<<"\n";
    }else if(c=="jarque-bera"){
        exact(4);auto result=normality_moments(read_csv(argv[2]).numbers(argv[3]));
        out<<"Skewness "<<result.skewness<<"\nExcessKurtosis "
           <<result.excess_kurtosis<<"\nJarqueBera "<<result.jarque_bera<<"\n";
    }else if(c=="discrete-entropy"){
        exact(4);out<<"Entropy "<<discrete_entropy(
            read_csv(argv[2]).numbers(argv[3]))<<"\n";
    }else if(c=="kl-divergence"||c=="js-divergence"){
        exact(5);auto table=read_csv(argv[2]);
        auto p=table.numbers(argv[3]),q=table.numbers(argv[4]);
        out<<"Divergence "<<(c=="kl-divergence"?
            kl_divergence(p,q):jensen_shannon_divergence(p,q))<<"\n";
    }else if(c=="tail-coincide"){
        exact(7);auto table=read_csv(argv[2]);
        auto x=table.numbers(argv[3]),y=table.numbers(argv[4]);
        std::string tail=argv[6];
        require(tail=="LOWER"||tail=="UPPER","Expected LOWER or UPPER");
        out<<"ConditionalCoExceedance "<<(tail=="LOWER"?
            lower_tail_conditional_exceedance(x,y,v(5)):
            upper_tail_conditional_exceedance(x,y,v(5)))<<"\n";
    }else if(c=="hac-mean"){
        exact(5);auto result=bartlett_hac_mean(
            read_csv(argv[2]).numbers(argv[3]),mass_int(argv[4],0));
        out<<"Mean "<<result.estimate<<"\nHACVariance "<<result.long_run_variance
           <<"\nStandardError "<<result.standard_error<<"\nT "<<result.t_stat<<"\n";
    }else if(c=="kendall-tau"){
        exact(5);auto table=read_csv(argv[2]);
        out<<"KendallTau "<<kendall_tau_b(
            table.numbers(argv[3]),table.numbers(argv[4]))<<"\n";
    }else if(c=="forecast-loss"){
        exact(7);auto t=read_csv(argv[2]);
        auto r=squared_forecast_loss_comparison(t.numbers(argv[3]),
            t.numbers(argv[4]),t.numbers(argv[5]),mass_int(argv[6],0));
        out<<"MeanLossDifference "<<r.average_difference<<"\nStandardError "
           <<r.standard_error<<"\nStandardized "<<r.standardized<<"\n";
    }else if(c=="drawdown-duration"){
        exact(4);auto r=drawdown_timing(read_csv(argv[2]).numbers(argv[3]));
        out<<"MaximumDrawdown "<<r.largest_fraction<<"\nLongestUnderwater "
           <<r.longest_underwater<<"\nPeakIndex "<<r.peak_index
           <<"\nTroughIndex "<<r.trough_index<<"\n";
    }else if(c=="markov-transitions"){
        exact(6);auto data=read_csv(argv[2]).numbers(argv[3]);
        std::vector<int> state;state.reserve(data.size());
        for(double x:data){
            require(x>=0&&x==std::floor(x),"Markov states must be integers");
            state.push_back(int(x));
        }
        auto p=categorical_transition_probabilities(state,mass_int(argv[4],2),
                                                      v(5));
        out<<"From,To,Probability\n";
        for(int i=0;i<int(p.size());i++)for(int j=0;j<int(p.size());j++)
            out<<i<<","<<j<<","<<p[i][j]<<"\n";
    }else if(c=="kaplan-meier"){
        exact(5);auto t=read_csv(argv[2]);
        auto censor_raw=t.numbers(argv[4]);std::vector<int> censor;
        for(double d:censor_raw){
            require(d==0||d==1,"Censor event must be zero or one");
            censor.push_back(int(d));
        }
        auto s=kaplan_meier(t.numbers(argv[3]),censor);
        out<<"Time,AtRisk,Events,Survival\n";
        for(auto x:s)out<<x.time<<","<<x.at_risk<<","<<x.defaults<<","
                        <<x.survival<<"\n";
    }else if(c=="time-weighted-return"){
        exact(5);auto t=read_csv(argv[2]);
        out<<"TimeWeightedReturn "<<period_time_weighted_return(
            t.numbers(argv[3]),t.numbers(argv[4]))<<"\n";
    }else if(c=="modified-dietz"){
        exact(7);auto t=read_csv(argv[2]);
        out<<"DietzReturn "<<dietz_modified_return(v(5),v(6),
            t.numbers(argv[3]),t.numbers(argv[4]))<<"\n";
    }else if(c=="capacity-net-profit"){
        exact(6);auto result=square_root_capacity_profit(
             v(2),v(3),v(4),v(5));
        out<<"Gross "<<result.gross_profit<<"\nImpact "<<result.impact_cost
           <<"\nNet "<<result.net_profit<<"\n";
    }else if(c=="capacity-optimum"){
        exact(5);
        out<<"OptimalCapital "<<square_root_capacity_optimal_capital(
            v(2),v(3),v(4))<<"\n";
    }else if(c=="cov-shrink2"){
        exact(6);auto r=shrink_covariance_to_diagonal(
            {{v(2),v(3)},{v(3),v(4)}},v(5));
        out<<"C11 "<<r[0][0]<<"\nC12 "<<r[0][1]<<"\nC22 "<<r[1][1]<<"\n";
    }else if(c=="risk-parity2"){
        exact(5);auto r=positive_covariance_risk_parity(
            {{v(2),v(3)},{v(3),v(4)}});
        massive_output_vector(out,"Weight",r);
    }else if(c=="rebalance-band2"){
        exact(7);auto r=no_trade_band_rebalance(
            {v(2),v(3)},{v(4),v(5)},v(6));
        massive_output_vector(out,"Weight",r);
    }else if(c=="weight-drift2"){
        exact(7);auto r=drifted_portfolio_weights(
            {v(2),v(3)},{v(4),v(5)},v(6));
        massive_output_vector(out,"Weight",r);
    }else if(c=="kelly-binary"){
        exact(5);
        out<<"KellyFraction "<<binary_kelly_fraction(v(2),v(3),v(4))<<"\n";
    }else if(c=="fractional-kelly"){
        exact(6);
        out<<"FractionalKelly "<<fractional_kelly_fraction(
            v(2),v(3),v(4),v(5))<<"\n";
    }else if(c=="fund-watermark-fees"){
        exact(7);auto result=charge_fund_fees(v(2),v(3),v(4),v(5),v(6));
        out<<"BeforeFee "<<result.before_fee<<"\nManagement "
           <<result.management_fee<<"\nIncentive "<<result.performance_fee
           <<"\nNet "<<result.after_fee<<"\nHighWatermark "
           <<result.high_watermark_per_share<<"\n";
    }else if(c=="hhi"||c=="effective-holdings"){
        exact(4);auto weights=read_csv(argv[2]).numbers(argv[3]);
        out<<(c=="hhi"?"HHI ":"EffectiveHoldings ")<<
            (c=="hhi"?herfindahl_concentration(weights):
                      effective_holdings_count(weights))<<"\n";
    }else if(c=="loan-amortization"){
        exact(5);auto rows=fixed_rate_amortization(
            v(2),v(3),mass_int(argv[4],1,10000));
        out<<"Period,Interest,Principal,Balance\n";
        for(auto row:rows)out<<row.period<<","<<row.interest<<","
                             <<row.principal<<","<<row.balance<<"\n";
    }else if(c=="information-ratio"){
        exact(5);
        out<<"InformationRatio "<<information_ratio_of_active_returns(
            read_csv(argv[2]).numbers(argv[3]),mass_int(argv[4],1))<<"\n";
    }else if(c=="market-capture"){
        exact(6);auto t=read_csv(argv[2]);
        std::string direction=argv[5];
        require(direction=="UP"||direction=="DOWN",
                "Market capture requires UP or DOWN");
        out<<"CaptureRatio "<<upside_downside_capture(
            t.numbers(argv[3]),t.numbers(argv[4]),direction=="UP")<<"\n";
    }else if(c=="tax-unrealized"){
        exact(6);auto t=read_csv(argv[2]);
        auto quantity=t.numbers(argv[3]),basis=t.numbers(argv[4]);
        std::vector<TaxLot> lots;
        for(int i=0;i<int(quantity.size());i++)
            lots.push_back({0,quantity[i],basis[i]});
        out<<"UnrealizedPnL "<<unrealized_tax_lot_profit(lots,v(5))<<"\n";
    }else if(c=="tax-held"){
        exact(7);auto t=read_csv(argv[2]);
        auto dates=t.numbers(argv[3]),quantity=t.numbers(argv[4]);
        std::vector<TaxLot> lots;
        for(int i=0;i<int(dates.size());i++)
            lots.push_back({int(dates[i]),quantity[i],0});
        out<<"EligibleShares "<<held_more_than_days(
            lots,mass_int(argv[5],0),mass_int(argv[6],0))<<"\n";
    }else return false;
    return true;
}
inline void massive_stats_usage(std::ostream& out){
    out<<"Advanced statistical and asset-management research:\n"
       <<"  variance-ratio CSV INCREMENT HORIZON\n"
       <<"  ljung-box CSV RESIDUAL LAGS | jarque-bera CSV SERIES\n"
       <<"  discrete-entropy CSV PROBABILITY\n"
       <<"  kl-divergence|js-divergence CSV PROBABILITY1 PROBABILITY2\n"
       <<"  tail-coincide CSV X Y FRACTION LOWER|UPPER\n"
       <<"  hac-mean CSV SERIES LAGS | kendall-tau CSV X Y\n"
       <<"  forecast-loss CSV ACTUAL FORECAST1 FORECAST2 HAC_LAGS\n"
       <<"  drawdown-duration CSV EQUITY | markov-transitions CSV STATE CLASSES PRIOR\n"
       <<"  kaplan-meier CSV DURATION EVENT\n"
       <<"  time-weighted-return CSV START END | modified-dietz CSV CASHFLOW FRACTION BEGIN END\n"
       <<"  capacity-net-profit CAPITAL GROSS_ALPHA TURNOVER COEFFICIENT\n"
       <<"  capacity-optimum GROSS_ALPHA TURNOVER COEFFICIENT\n"
       <<"  cov-shrink2 C11 C12 C22 INTENSITY | risk-parity2 VAR1 COV VAR2\n"
       <<"  rebalance-band2 CURRENT1 CURRENT2 TARGET1 TARGET2 BAND\n"
       <<"  weight-drift2 WEIGHT1 WEIGHT2 RETURN1 RETURN2 CASH\n"
       <<"  kelly-binary PROB WIN LOSS | fractional-kelly PROB WIN LOSS FRACTION\n"
       <<"  fund-watermark-fees NAV RETURN HIGH_WATERMARK MGMT_FEE INCENTIVE_FEE\n"
       <<"  hhi|effective-holdings CSV WEIGHT | loan-amortization LOAN RATE PERIODS\n"
       <<"  information-ratio CSV ACTIVE_RETURN PERIODS_PER_YEAR\n"
       <<"  market-capture CSV FUND BENCHMARK UP|DOWN\n"
       <<"  tax-unrealized CSV SHARES COST CURRENT_PRICE\n"
       <<"  tax-held CSV ACQUIRED_DAY SHARES CURRENT_DAY THRESHOLD_DAYS\n";
}
} // namespace kline
