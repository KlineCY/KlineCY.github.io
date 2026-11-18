#pragma once
#include "bayesian_linear_posterior.hpp"
#include "augmented_dickey_fuller.hpp"
#include "kpss_stationarity.hpp"
#include "granger_causality.hpp"
#include "difference_in_differences.hpp"
#include "rolling_pair_spread.hpp"
#include "beta_binomial_posterior.hpp"
#include "weighted_qr_regression.hpp"
#include "hmm_gaussian_two_state.hpp"
#include "gaussian_mixture_em.hpp"
#include "nested_ridge_validation.hpp"
#include "isotonic_real_regression.hpp"
#include "multitarget_ridge.hpp"
#include "exponential_weighted_regression.hpp"
#include "ensemble_forecast_weights.hpp"
#include "conformal_classification.hpp"
#include "ewma_cross_asset_covariance.hpp"
#include "factor_neutral_projection.hpp"
#include "liquidity_adjusted_var.hpp"
#include "hawkes_event_intensity.hpp"
#include "volume_participation_execution.hpp"
#include "longstaff_schwartz_put.hpp"
#include "option_implied_density.hpp"
#include "bond_roll_down.hpp"
#include "io.hpp"
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
namespace kline {
inline std::vector<int> crossdomain_binary_labels(
    const std::vector<double>& values){
    check_finite(values);
    std::vector<int> labels;
    for(double y:values){
        require(y==0||y==1,"Binary classification labels must be 0 or 1");
        labels.push_back(int(y));
    }
    return labels;
}
inline Matrix crossdomain_predictors(const std::vector<double>& values){
    Matrix x;
    for(double value:values)x.push_back({value});
    return x;
}
inline bool run_crossdomain_cli(const std::string& cmd,int argc,
                                 char** argv,std::ostream& out){
    auto exact=[&](int n){require(argc==n,"Incorrect crossdomain CLI arguments");};
    auto num=[&](int i){return parse_number(argv[i]);};
    if(cmd=="bayes-linear"){
        exact(8);auto t=read_csv(argv[2]);
        auto fit=bayesian_linear_posterior(
            crossdomain_predictors(t.numbers(argv[3])),
            t.numbers(argv[4]),num(5),num(6));
        auto prediction=bayesian_linear_prediction(fit,{num(7)});
        out<<"Mean "<<prediction.first<<"\nVariance "<<prediction.second<<"\n";
    }else if(cmd=="adf-augmented"){
        exact(5);
        auto r=augmented_dickey_fuller(
            read_csv(argv[2]).numbers(argv[3]),std::stoi(argv[4]));
        out<<"LevelCoefficient "<<r.level_coefficient<<"\nADF_TStatistic "
           <<r.t_statistic<<"\nObservations "<<r.usable_observations<<"\n";
    }else if(cmd=="kpss-level"){
        exact(5);
        out<<"KPSS "<<kpss_level_statistic(
            read_csv(argv[2]).numbers(argv[3]),std::stoi(argv[4]))<<"\n";
    }else if(cmd=="granger-cause"){
        exact(6);auto t=read_csv(argv[2]);
        auto fit=granger_predictive_test(
            t.numbers(argv[3]),t.numbers(argv[4]),std::stoi(argv[5]));
        out<<"FStatistic "<<fit.f_statistic<<"\n"
           <<"UnrestrictedRSS "<<fit.full_rss<<"\n";
    }else if(cmd=="did-2x2"){
        exact(6);auto t=read_csv(argv[2]);
        auto treated=crossdomain_binary_labels(t.numbers(argv[4]));
        auto post=crossdomain_binary_labels(t.numbers(argv[5]));
        auto fit=difference_in_differences(t.numbers(argv[3]),treated,post);
        out<<"TreatmentEffect "<<fit.treatment_effect<<"\n"
           <<"IndependentCellSE "<<fit.standard_error<<"\n";
    }else if(cmd=="rolling-pair"){
        exact(6);auto t=read_csv(argv[2]);
        auto result=past_only_pair_spreads(
            t.numbers(argv[3]),t.numbers(argv[4]),std::stoi(argv[5]));
        out<<"Index,Intercept,HedgeRatio,Spread\n";
        for(int i=0;i<int(result.size());i++)
            out<<i<<","<<result[i].intercept<<","
               <<result[i].hedge_ratio<<","<<result[i].spread<<"\n";
    }else if(cmd=="beta-posterior"){
        exact(6);
        auto result=beta_binomial_update(std::stoi(argv[2]),
             std::stoi(argv[3]),num(4),num(5));
        out<<"PosteriorMean "<<result.mean<<"\n"
           <<"PosteriorVariance "<<result.variance<<"\n";
    }else if(cmd=="weighted-qr"){
        exact(6);auto t=read_csv(argv[2]);
        auto result=weighted_qr_intercept(
            crossdomain_predictors(t.numbers(argv[3])),
            t.numbers(argv[4]),t.numbers(argv[5]));
        out<<"Intercept "<<result.coefficients[0]<<"\n"
           <<"Slope "<<result.coefficients[1]<<"\n";
    }else if(cmd=="hmm-two"){
        exact(10);
        auto values=read_csv(argv[2]).numbers(argv[3]);
        auto result=gaussian_hmm_two_state(values,{0.5,0.5},
            {{num(8),1-num(8)},{1-num(9),num(9)}},
            {num(4),num(5)},{num(6),num(7)});
        out<<"Index,State0,State1\n";
        for(int i=0;i<int(result.filtered.size());i++)
            out<<i<<","<<result.filtered[i][0]<<","
               <<result.filtered[i][1]<<"\n";
        out<<"LogLikelihood "<<result.log_likelihood<<"\n";
    }else if(cmd=="gmm-two"){
        exact(5);
        auto result=fit_gaussian_mixture_two(
            read_csv(argv[2]).numbers(argv[3]),std::stoi(argv[4]));
        out<<"Weight0 "<<result.weight<<"\nMean0 "<<result.mean0
           <<"\nMean1 "<<result.mean1<<"\nVariance0 "<<result.var0
           <<"\nVariance1 "<<result.var1<<"\n";
    }else if(cmd=="ridge-nested"){
        exact(10);auto t=read_csv(argv[2]);
        auto result=select_ridge_chronologically(
            crossdomain_predictors(t.numbers(argv[3])),
            t.numbers(argv[4]),{num(7),num(8)},
            std::stoi(argv[5]),std::stoi(argv[6]));
        out<<"Penalty "<<result.penalty<<"\nValidationMSE "
           <<result.validation_mse<<"\nNextPrediction "
           <<ridge_predict(result.fitted,{num(9)})<<"\n";
    }else if(cmd=="isotonic-real"){
        exact(6);auto t=read_csv(argv[2]);
        out<<"Prediction "<<isotonic_real_predict(
            fit_isotonic_real(t.numbers(argv[3]),t.numbers(argv[4])),
            num(5))<<"\n";
    }else if(cmd=="multi-ridge"){
        exact(8);auto t=read_csv(argv[2]);
        auto a=t.numbers(argv[4]),b=t.numbers(argv[5]);
        Matrix targets;
        for(int i=0;i<int(a.size());i++)targets.push_back({a[i],b[i]});
        auto fit=fit_multitarget_ridge(
            crossdomain_predictors(t.numbers(argv[3])),targets,num(6));
        auto prediction=multitarget_predict(fit,{num(7)});
        out<<"TargetA "<<prediction[0]<<"\nTargetB "<<prediction[1]<<"\n";
    }else if(cmd=="exp-ols"){
        exact(6);auto t=read_csv(argv[2]);
        auto fit=fit_exponential_decay_ols(
            crossdomain_predictors(t.numbers(argv[3])),
            t.numbers(argv[4]),num(5));
        out<<"Intercept "<<fit.coefficients[0]<<"\n"
           <<"Slope "<<fit.coefficients[1]<<"\n";
    }else if(cmd=="ensemble-weights"){
        exact(6);auto t=read_csv(argv[2]);
        auto a=t.numbers(argv[3]),b=t.numbers(argv[4]);
        Matrix predictors;
        for(int i=0;i<int(a.size());i++)predictors.push_back({a[i],b[i]});
        auto weights=inverse_error_ensemble_weights(
            predictors,t.numbers(argv[5]));
        out<<"Weight0 "<<weights[0]<<"\nWeight1 "<<weights[1]<<"\n";
    }else if(cmd=="conformal-class"){
        exact(7);auto t=read_csv(argv[2]);
        double threshold=binary_conformal_nonconformity_threshold(
            t.numbers(argv[3]),
            crossdomain_binary_labels(t.numbers(argv[4])),num(5));
        auto output=binary_conformal_set(num(6),threshold);
        out<<"Threshold "<<threshold<<"\nCandidateClass\n";
        for(int value:output)out<<value<<"\n";
    }else if(cmd=="ewma-cov2"){
        exact(6);auto t=read_csv(argv[2]);
        auto a=t.numbers(argv[3]),b=t.numbers(argv[4]);
        Matrix observations;
        for(int i=0;i<int(a.size());i++)observations.push_back({a[i],b[i]});
        auto cov=ewma_multivariate_covariance(observations,num(5));
        out<<"Cov11 "<<cov[0][0]<<"\nCov12 "<<cov[0][1]
           <<"\nCov22 "<<cov[1][1]<<"\n";
    }else if(cmd=="factor-neutral2"){
        exact(6);auto t=read_csv(argv[2]);
        auto result=minimum_change_factor_neutral(
            t.numbers(argv[3]),{t.numbers(argv[4])},{num(5)});
        out<<"Index,NeutralWeight\n";
        for(int i=0;i<int(result.size());i++)
            out<<i<<","<<result[i]<<"\n";
    }else if(cmd=="liquidity-var"){
        exact(8);
        out<<"LiquidityAdjustedVaR "<<normal_var_with_liquidation(
            num(2),num(3),num(4),num(5),{num(6)},{num(7)})<<"\n";
    }else if(cmd=="hawkes-intensity"||cmd=="hawkes-likelihood"){
        exact(8);
        auto times=read_csv(argv[2]).numbers(argv[3]);
        out<<(cmd=="hawkes-intensity"?"Intensity ":"LogLikelihood ")
           <<(cmd=="hawkes-intensity"?
              exponential_hawkes_intensity(times,num(4),num(5),num(6),num(7)):
              exponential_hawkes_log_likelihood(times,num(4),num(5),num(6),num(7)))
           <<"\n";
    }else if(cmd=="pov-schedule"){
        exact(6);
        auto values=read_csv(argv[2]).numbers(argv[3]);
        auto result=volume_limited_execution(num(4),values,num(5));
        out<<"Index,Executed\n";
        for(int i=0;i<int(result.fills.size());i++)
            out<<i<<","<<result.fills[i]<<"\n";
        out<<"Unfilled "<<result.unfilled<<"\n";
    }else if(cmd=="lsm-put"){
        require(argc>=9,"LSM needs CSV, strike, rate, dt, and three price columns");
        auto t=read_csv(argv[2]);
        Matrix paths(t.rows.size());
        for(int j=6;j<argc;j++){
            auto values=t.numbers(argv[j]);
            for(int i=0;i<int(paths.size());i++)paths[i].push_back(values[i]);
        }
        out<<"AmericanPut "<<longstaff_schwartz_american_put(
            paths,num(3),num(4),num(5))<<"\n";
    }else if(cmd=="option-density"){
        exact(7);
        out<<"ImpliedDensity "<<call_implied_density_equal_spacing(
            num(2),num(3),num(4),num(5),num(6))<<"\n";
    }else if(cmd=="option-digital"){
        exact(7);
        out<<"DigitalProbability "<<call_implied_probability_above_strike(
            num(2),num(3),num(4),num(5),num(6))<<"\n";
    }else if(cmd=="bond-rolldown"){
        exact(6);
        require(num(5)>0&&num(5)<=num(3),"Bond holding horizon invalid");
        ZeroCurve curve{{1,10},{num(4),num(4)}};
        out<<"RollDownPnL "<<bond_static_curve_rolldown_pnl(
            curve,{num(3)},{num(2)},num(5))<<"\n";
    }else return false;
    return true;
}
inline void crossdomain_cli_usage(std::ostream& out){
    out<<"Crossdomain econometrics, Bayesian finance and market structure:\n"
       <<"  bayes-linear CSV FEATURE TARGET NOISE_VARIANCE PRIOR_VARIANCE QUERY\n"
       <<"  adf-augmented CSV SERIES LAGGED_DIFFERENCES\n"
       <<"  kpss-level CSV SERIES HAC_LAGS\n"
       <<"  granger-cause CSV RESPONSE PREDICTOR LAGS\n"
       <<"  did-2x2 CSV OUTCOME TREATED POST\n"
       <<"  rolling-pair CSV DEPENDENT HEDGE WINDOW\n"
       <<"  beta-posterior SUCCESSES FAILURES PRIOR_A PRIOR_B\n"
       <<"  weighted-qr CSV FEATURE TARGET WEIGHT\n"
       <<"  hmm-two CSV OBS MU0 MU1 STD0 STD1 STAY0 STAY1\n"
       <<"  gmm-two CSV SERIES ITERATIONS\n"
       <<"  ridge-nested CSV FEATURE TARGET TRAIN_END VALID_END LAMBDA1 LAMBDA2 QUERY\n"
       <<"  isotonic-real CSV SCORE OUTCOME QUERY\n"
       <<"  multi-ridge CSV FEATURE TARGET_A TARGET_B LAMBDA QUERY\n"
       <<"  exp-ols CSV FEATURE TARGET HALF_LIFE\n"
       <<"  ensemble-weights CSV FORECAST_A FORECAST_B REALIZED\n"
       <<"  conformal-class CSV PROBABILITY LABEL MISCOVERAGE QUERY\n"
       <<"  ewma-cov2 CSV RETURN_A RETURN_B DECAY\n"
       <<"  factor-neutral2 CSV WEIGHTS EXPOSURES TARGET\n"
       <<"  liquidity-var NOTIONAL MEAN_RETURN VOL CONFIDENCE ORDER_SIZE COST\n"
       <<"  hawkes-intensity|hawkes-likelihood CSV EVENT_TIME HORIZON BASE EXCITE DECAY\n"
       <<"  pov-schedule CSV MARKET_VOLUME ORDER_QTY PARTICIPATION\n"
       <<"  lsm-put CSV STRIKE RATE DT PRICE_T0 PRICE_T1 PRICE_T2 [...]\n"
       <<"  option-density CALL_LOW CALL_MID CALL_HIGH STRIKE_STEP DISCOUNT\n"
       <<"  option-digital CALL_BELOW CALL_ABOVE STRIKE_BELOW STRIKE_ABOVE DISCOUNT\n"
       <<"  bond-rolldown FACE MATURITY FLAT_RATE HOLD_YEARS\n";
}
} // namespace kline
