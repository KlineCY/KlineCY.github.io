#pragma once
#include "range_volatility_lab.hpp"
#include "forecast_verification_lab.hpp"
#include "causal_inference_lab.hpp"
#include "credit_cashflow_lab.hpp"
#include "execution_dynamics_lab.hpp"
#include "portfolio_path_lab.hpp"
#include "io.hpp"
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace kline {
inline int lab_integer(const char* input,int lower,int upper=100000) {
    const double value=parse_number(input);
    require(value>=lower&&value<=upper&&std::floor(value)==value,
            "Research lab needs an integer in range");
    return int(value);
}
inline bool run_empirical_lab_cli(const std::string& c,int argc,char** argv,
                                  std::ostream& out) {
    auto exact=[&](int n){require(argc==n,"Invalid empirical lab command arguments");};
    auto num=[&](int i){return parse_number(argv[i]);};
    if(c=="range-parkinson") {
        exact(5);auto data=read_csv(argv[2]);
        out<<"Variance "<<parkinson_intraday_variance(data.numbers(argv[3]),data.numbers(argv[4]))<<"\n";
    } else if(c=="range-gk"||c=="range-rs") {
        exact(7);auto d=read_csv(argv[2]);
        auto o=d.numbers(argv[3]),h=d.numbers(argv[4]),l=d.numbers(argv[5]),cl=d.numbers(argv[6]);
        out<<"Variance "<<(c=="range-gk"?garman_klass_intraday_variance(o,h,l,cl):
               rogers_satchell_intraday_variance(o,h,l,cl))<<"\n";
    } else if(c=="overnight-gap") {
        exact(5);auto d=read_csv(argv[2]);
        out<<"SquaredGap "<<overnight_gap_second_moment(d.numbers(argv[3]),d.numbers(argv[4]))<<"\n";
    } else if(c=="pinball-loss") {
        exact(6);auto d=read_csv(argv[2]);
        out<<"PinballLoss "<<pinball_mean_loss(d.numbers(argv[3]),d.numbers(argv[4]),num(5))<<"\n";
    } else if(c=="gaussian-crps") {
        exact(6);auto d=read_csv(argv[2]);
        out<<"GaussianCRPS "<<gaussian_crps_mean(d.numbers(argv[3]),d.numbers(argv[4]),
                                               d.numbers(argv[5]))<<"\n";
    } else if(c=="average-precision") {
        exact(5);auto d=read_csv(argv[2]);
        out<<"AveragePrecision "<<average_precision_with_ties(d.numbers(argv[3]),
                                                              d.numbers(argv[4]))<<"\n";
    } else if(c=="ranked-score3") {
        exact(7);auto d=read_csv(argv[2]);
        auto a=d.numbers(argv[3]),b=d.numbers(argv[4]),p=d.numbers(argv[5]),
             y=d.numbers(argv[6]);
        std::vector<std::vector<double>> rows;
        std::vector<int> labels;
        for(std::size_t i=0;i<y.size();++i) {
            require(y[i]>=0&&y[i]<=2&&std::floor(y[i])==y[i],"Invalid ordinal label");
            rows.push_back({a[i],b[i],p[i]});labels.push_back(int(y[i]));
        }
        out<<"RankedProbabilityScore "<<ordinal_ranked_probability_score(rows,labels)<<"\n";
    } else if(c=="rd-local") {
        exact(7);auto d=read_csv(argv[2]);
        auto result=regression_discontinuity_triangular(d.numbers(argv[3]),d.numbers(argv[4]),
                                                          num(5),num(6));
        out<<"TreatmentEffect "<<result.treatmentEffect<<"\nLeftIntercept "<<result.leftIntercept
           <<"\nRightIntercept "<<result.rightIntercept<<"\n";
    } else if(c=="wald-iv") {
        exact(6);auto d=read_csv(argv[2]);
        out<<"IVEstimate "<<wald_instrumental_variable(d.numbers(argv[3]),d.numbers(argv[4]),
                                                       d.numbers(argv[5]))<<"\n";
    } else if(c=="meta-fixed") {
        exact(5);auto d=read_csv(argv[2]);
        auto r=inverse_variance_meta(d.numbers(argv[3]),d.numbers(argv[4]));
        out<<"PooledEffect "<<r.effect<<"\nStdError "<<r.standardError
           <<"\nQ "<<r.heterogeneityQ<<"\nI2 "<<r.iSquared<<"\n";
    } else if(c=="synthetic-simplex") {
        require(argc>=6&&argc<=105,"Synthetic control needs treated and two donor columns");
        auto d=read_csv(argv[2]);auto target=d.numbers(argv[3]);
        std::vector<std::vector<double>> donors(target.size());
        for(int j=4;j<argc;++j) {
            auto column=d.numbers(argv[j]);
            for(std::size_t i=0;i<target.size();++i)donors[i].push_back(column[i]);
        }
        auto fit=synthetic_control_simplex(target,donors);
        out<<"PrePeriodRMSE "<<fit.inSampleRmse<<"\n";
        for(std::size_t i=0;i<fit.weights.size();++i)
            out<<"Donor"<<i+1<<"Weight "<<fit.weights[i]<<"\n";
    } else if(c=="coupon-accrual") {
        exact(7);
        out<<"AccruedInterest "<<bond_accrued_coupon(num(2),num(3),
            lab_integer(argv[4],1,365),lab_integer(argv[5],0),
            lab_integer(argv[6],1))<<"\n";
    } else if(c=="ois-discount") {
        exact(5);auto d=read_csv(argv[2]);
        out<<"DiscountFactor "<<overnight_compounded_discount(d.numbers(argv[3]),
                                                              d.numbers(argv[4]))<<"\n";
    } else if(c=="merton-default") {
        exact(7);
        out<<"DefaultProbability "<<merton_structural_default_probability(
            num(2),num(3),num(4),num(5),num(6))<<"\n";
    } else if(c=="credit-zero") {
        exact(7);
        out<<"RiskyPrice "<<risky_zero_coupon_recovery_of_par(
            num(2),num(3),num(4),num(5),num(6))<<"\n";
    } else if(c=="queue-poisson") {
        exact(6);
        out<<"FullFillProbability "<<poisson_queue_completion_probability(
            num(2),num(3),lab_integer(argv[4],0,10000),
            lab_integer(argv[5],1,10000))<<"\n";
    } else if(c=="transient-impact") {
        exact(6);auto d=read_csv(argv[2]);
        auto path=exponential_transient_impact(d.numbers(argv[3]),num(4),num(5));
        out<<"Index,Impact\n";
        for(std::size_t i=0;i<path.size();++i)out<<i<<","<<path[i]<<"\n";
    } else if(c=="rv-signature") {
        exact(5);auto d=read_csv(argv[2]);
        auto r=realized_variance_signature(d.numbers(argv[3]),
                                          lab_integer(argv[4],1,100000));
        out<<"FineRV "<<r.finest<<"\nCoarseRV "<<r.coarse<<"\nRatio "<<r.ratio<<"\n";
    } else if(c=="volume-bucket") {
        exact(6);auto d=read_csv(argv[2]);
        auto result=fixed_volume_signed_imbalance(d.numbers(argv[3]),d.numbers(argv[4]),num(5));
        out<<"Bucket,AbsoluteImbalance\n";
        for(std::size_t i=0;i<result.size();++i)out<<i<<","<<result[i]<<"\n";
    } else if(c=="leveraged-nav") {
        exact(7);auto d=read_csv(argv[2]);
        auto path=daily_reset_leveraged_nav(d.numbers(argv[3]),num(4),num(5),
                                            lab_integer(argv[6],1,10000));
        out<<"Period,NAV\n";
        for(std::size_t i=0;i<path.size();++i)out<<i<<","<<path[i]<<"\n";
    } else if(c=="drawdown-tail") {
        require(argc>=6&&argc<=104,"Drawdown simulation needs scenario columns and confidence");
        auto d=read_csv(argv[2]);std::vector<std::vector<double>> paths;
        for(int j=3;j<argc-1;++j)paths.push_back(d.numbers(argv[j]));
        auto r=simulated_drawdown_tail(paths,num(argc-1));
        out<<"DrawdownQuantile "<<r.quantile<<"\nDrawdownTailMean "<<r.expectedTail<<"\n";
    } else if(c=="margin-call") {
        exact(7);auto r=position_margin_call(num(2),num(3),num(4),num(5),num(6));
        out<<"Equity "<<r.equity<<"\nMaintenance "<<r.maintenance
           <<"\nDeficiency "<<r.deficiency<<"\n";
    } else if(c=="diversification-2") {
        exact(6);const double w=num(2);
        out<<"DiversificationRatio "<<long_only_diversification_ratio(
            {w,1-w},{{num(3),num(4)},{num(4),num(5)}})<<"\n";
    } else return false;
    return true;
}
inline void empirical_lab_usage(std::ostream& out) {
    out<<"Empirical research CSV and parameter workflows:\n"
       <<"  range-parkinson CSV HIGH LOW | range-gk|range-rs CSV OPEN HIGH LOW CLOSE\n"
       <<"  overnight-gap CSV OPEN PREVIOUS_CLOSE\n"
       <<"  pinball-loss CSV TARGET FORECAST QUANTILE\n"
       <<"  gaussian-crps CSV TARGET MEAN STANDARD_DEVIATION\n"
       <<"  average-precision CSV SCORE BINARY_LABEL\n"
       <<"  ranked-score3 CSV PROB_CLASS0 PROB_CLASS1 PROB_CLASS2 ORDINAL_CLASS\n"
       <<"  rd-local CSV RUNNING OUTCOME CUTOFF BANDWIDTH\n"
       <<"  wald-iv CSV INSTRUMENT TREATMENT OUTCOME\n"
       <<"  meta-fixed CSV EFFECT STDERR\n"
       <<"  synthetic-simplex CSV TREATED DONOR1 DONOR2 [MORE_DONORS]\n"
       <<"  coupon-accrual FACE RATE PAYMENTS DAYS_ELAPSED DAYS_IN_PERIOD\n"
       <<"  ois-discount CSV OVERNIGHT_RATE ACCRUAL_YEARS\n"
       <<"  merton-default ASSET DEBT ASSET_DRIFT ASSET_VOL YEARS\n"
       <<"  credit-zero FACE RATE HAZARD RECOVERY YEARS\n"
       <<"  queue-poisson ARRIVAL_RATE HORIZON QUANTITY_AHEAD OWN_QUANTITY\n"
       <<"  transient-impact CSV SIGNED_ORDERS PER_UNIT PERSISTENCE\n"
       <<"  rv-signature CSV MIDPRICE COARSE_STEP\n"
       <<"  volume-bucket CSV VOLUME SIGN BUCKET_VOLUME\n"
       <<"  leveraged-nav CSV BENCHMARK_RETURN LEVERAGE ANNUAL_FEE PERIODS_PER_YEAR\n"
       <<"  drawdown-tail CSV NAV_SCENARIO1 NAV_SCENARIO2 [MORE] CONFIDENCE\n"
       <<"  margin-call EQUITY SHARES ENTRY_PRICE MARK_PRICE MAINTENANCE\n"
       <<"  diversification-2 WEIGHT_A VAR_A COV_AB VAR_B\n";
}
} // namespace kline
