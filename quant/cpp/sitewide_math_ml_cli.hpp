#pragma once
#include "partial_correlation.hpp"
#include "trend_smoothing.hpp"
#include "huber_robust_regression.hpp"
#include "empirical_distribution.hpp"
#include "permutation_inference.hpp"
#include "stationary_bootstrap.hpp"
#include "partial_autocorrelation.hpp"
#include "page_hinkley.hpp"
#include "blocked_sample_moments.hpp"
#include "distance_covariance.hpp"
#include "weighted_quantiles.hpp"
#include "linear_svm_sgd.hpp"
#include "quantile_linear_regression.hpp"
#include "expectile_regression.hpp"
#include "ftrl_logistic.hpp"
#include "probability_discrimination.hpp"
#include "data_drift_psi.hpp"
#include "kernel_mmd.hpp"
#include "permutation_importance.hpp"
#include "cross_fitted_residuals.hpp"
#include "rolling_origin_validation.hpp"
#include "isotonic_intervals.hpp"
#include "io.hpp"
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
namespace kline {
inline void sitewide_series_output(std::ostream& out,const std::string& name,
                         const std::vector<double>& x){
    out<<"Index,"<<name<<"\n";
    for(int i=0;i<int(x.size());i++)out<<i<<","<<x[i]<<"\n";
}
inline std::vector<int> sitewide_binary(const std::vector<double>& x){
    check_finite(x);std::vector<int> result;
    for(double v:x){
        require(v==0||v==1,"Required binary label column");
        result.push_back(int(v));
    }
    return result;
}
inline std::vector<double> sitewide_signed(const std::vector<double>& x){
    check_finite(x);
    for(double v:x)require(v==-1||v==1,"SVM label must be -1 or +1");
    return x;
}
inline Matrix sitewide_one_column(const std::vector<double>& x){
    Matrix result;for(double v:x)result.push_back({v});return result;
}
inline bool run_sitewide_math_ml_cli(const std::string& cmd,int argc,char** argv,
                                    std::ostream& out){
    auto exact=[&](int arity){require(argc==arity,
        "Incorrect mathematics or ML research arguments");};
    auto number=[&](int i){return parse_number(argv[i]);};
    if(cmd=="partial-corr2"){
        exact(6);
        auto cov=partial_correlation_matrix(
            {{number(2),number(3)},{number(3),number(4)}},number(5));
        out<<"PartialCorrelation "<<cov[0][1]<<"\n";
    }else if(cmd=="trend-smooth"){
        exact(5);
        auto series=read_csv(argv[2]).numbers(argv[3]);
        sitewide_series_output(out,"SmoothedTrend",
            second_difference_trend(series,number(4)));
    }else if(cmd=="huber-fit"){
        exact(6);auto t=read_csv(argv[2]);
        auto fit=huber_regression(sitewide_one_column(t.numbers(argv[3])),
                t.numbers(argv[4]),number(5));
        sitewide_series_output(out,"Coefficient",fit.coefficients);
    }else if(cmd=="empirical-cdf"){
        exact(5);
        out<<"CDF "<<empirical_cdf(
            read_csv(argv[2]).numbers(argv[3]),number(4))<<"\n";
    }else if(cmd=="ks-two"){
        exact(5);auto t=read_csv(argv[2]);
        out<<"KS "<<two_sample_ks_statistic(
            t.numbers(argv[3]),t.numbers(argv[4]))<<"\n";
    }else if(cmd=="permutation-means"){
        exact(7);auto t=read_csv(argv[2]);
        auto group=sitewide_binary(t.numbers(argv[3]));
        auto val=t.numbers(argv[4]);
        std::vector<double> treatment,control;
        for(int i=0;i<int(val.size());i++)
            (group[i]?treatment:control).push_back(val[i]);
        auto test=permutation_mean_difference(treatment,control,
                         std::stoi(argv[5]),std::stoul(argv[6]));
        out<<"Difference "<<test.observed<<"\nPValue "<<test.p_value<<"\n";
    }else if(cmd=="stationary-bootstrap"){
        exact(6);
        sitewide_series_output(out,"BootstrapSample",
            stationary_bootstrap_sample(
                read_csv(argv[2]).numbers(argv[3]),
                number(4),std::stoul(argv[5])));
    }else if(cmd=="pacf"){
        exact(5);
        sitewide_series_output(out,"PACF",partial_autocorrelations(
            read_csv(argv[2]).numbers(argv[3]),std::stoi(argv[4])));
    }else if(cmd=="page-hinkley"){
        exact(6);
        auto observations=page_hinkley_upper(
            read_csv(argv[2]).numbers(argv[3]),number(4),number(5));
        out<<"Index,Statistic,Alarm\n";
        for(const auto& point:observations)
            out<<point.index<<","<<point.statistic<<","<<point.alarm<<"\n";
    }else if(cmd=="aggregate-return"){
        exact(5);
        sitewide_series_output(out,"CompoundedReturn",
            time_aggregated_nonoverlap_returns(
                read_csv(argv[2]).numbers(argv[3]),std::stoi(argv[4])));
    }else if(cmd=="bipower"){
        exact(5);
        sitewide_series_output(out,"BipowerVariation",
            rolling_bipower_variation(
                read_csv(argv[2]).numbers(argv[3]),std::stoi(argv[4])));
    }else if(cmd=="distance-cov"){
        exact(5);auto t=read_csv(argv[2]);
        out<<"DistanceCovarianceSquared "<<
            biased_distance_covariance_squared(
            t.numbers(argv[3]),t.numbers(argv[4]))<<"\n";
    }else if(cmd=="weighted-quantile"||cmd=="weighted-cvar"){
        exact(6);auto t=read_csv(argv[2]);
        double result=cmd=="weighted-quantile"?
            weighted_empirical_quantile(t.numbers(argv[3]),
                t.numbers(argv[4]),number(5)):
            weighted_empirical_tail_mean(t.numbers(argv[3]),
                t.numbers(argv[4]),number(5));
        out<<(cmd=="weighted-quantile"?"WeightedQuantile ":"WeightedCVaR ")
           <<result<<"\n";
    }else if(cmd=="linear-svm"){
        exact(8);auto t=read_csv(argv[2]);
        auto model=fit_linear_svm_sgd(
            sitewide_one_column(t.numbers(argv[3])),
            sitewide_signed(t.numbers(argv[4])),number(7),
            std::stoi(argv[6]),0.1);
        out<<"DecisionScore "<<svm_decision_score(model,{number(5)})<<"\n";
    }else if(cmd=="quantile-linear"){
        exact(9);auto t=read_csv(argv[2]);
        auto fit=fit_quantile_linear(
            sitewide_one_column(t.numbers(argv[3])),
            t.numbers(argv[4]),number(5),0.001,
            std::stoi(argv[7]),number(8));
        out<<"Prediction "<<fit.coefficients[0]+fit.coefficients[1]*number(6)
           <<"\nPinball "<<fit.pinball<<"\n";
    }else if(cmd=="expectile-linear"){
        exact(7);auto t=read_csv(argv[2]);
        auto fit=fit_linear_expectile(
            sitewide_one_column(t.numbers(argv[3])),
            t.numbers(argv[4]),number(5));
        out<<"Prediction "<<fit.coefficients[0]+fit.coefficients[1]*number(6)
           <<"\n";
    }else if(cmd=="ftrl-logit"){
        exact(7);auto t=read_csv(argv[2]);
        auto x=t.numbers(argv[3]);
        auto y=sitewide_binary(t.numbers(argv[4]));
        auto model=new_ftrl_logistic(1,0.15,1,0.01,0.01);
        int epochs=std::stoi(argv[5]);
        require(epochs>0&&epochs<=5000,"Invalid online FTRL epochs");
        for(int repeat=0;repeat<epochs;repeat++)
            for(int i=0;i<int(y.size());i++)
                ftrl_online_update(model,{x[i]},y[i]);
        out<<"Probability "<<ftrl_predict(model,{number(6)})<<"\n";
    }else if(cmd=="binary-auc"||cmd=="brier"||cmd=="binary-logloss"){
        exact(5);auto t=read_csv(argv[2]);
        auto score=t.numbers(argv[3]),label=sitewide_binary(t.numbers(argv[4]));
        double result=cmd=="binary-auc"?binary_auc_mann_whitney(score,label):
            cmd=="brier"?binary_brier_score(score,label):
                         binary_logarithmic_score(score,label);
        out<<"Statistic "<<result<<"\n";
    }else if(cmd=="psi"){
        exact(6);auto t=read_csv(argv[2]);
        out<<"PSI "<<population_stability_index(
            t.numbers(argv[3]),t.numbers(argv[4]),std::stoi(argv[5]))<<"\n";
    }else if(cmd=="kernel-mmd"){
        exact(6);auto t=read_csv(argv[2]);
        auto grouping=sitewide_binary(t.numbers(argv[3]));
        auto x=t.numbers(argv[4]);
        Matrix a,b;
        for(int i=0;i<int(grouping.size());i++)
            (grouping[i]?a:b).push_back({x[i]});
        out<<"MMD2 "<<squared_mmd_rbf(a,b,number(5))<<"\n";
    }else if(cmd=="permutation-importance"){
        exact(8);auto train=read_csv(argv[2]),test=read_csv(argv[3]);
        auto model=fit_ridge(sitewide_one_column(train.numbers(argv[4])),
              train.numbers(argv[5]),0.01);
        out<<"MSEIncrease "<<permutation_importance_mse(
            sitewide_one_column(test.numbers(argv[4])),
            test.numbers(argv[5]),
            [model](const std::vector<double>& row){
                return ridge_predict(model,row);
            },0,std::stoul(argv[6]),std::stoi(argv[7]))<<"\n";
    }else if(cmd=="crossfit-ridge"){
        exact(7);auto t=read_csv(argv[2]);
        auto residual=contiguous_crossfit_ridge(
            sitewide_one_column(t.numbers(argv[3])),
            t.numbers(argv[4]),std::stoi(argv[5]),number(6));
        sitewide_series_output(out,"CrossFittedResidual",residual.model_residual);
    }else if(cmd=="rolling-origin"){
        exact(10);auto t=read_csv(argv[2]);
        int n=int(t.rows.size());
        auto folds=rolling_origin_splits(n,std::stoi(argv[5]),
                    std::stoi(argv[6]),std::stoi(argv[7]),
                    std::stoi(argv[8]));
        sitewide_series_output(out,"OutOfSampleError",
            rolling_origin_ridge_errors(
                sitewide_one_column(t.numbers(argv[3])),
                t.numbers(argv[4]),folds,number(9)));
    }else if(cmd=="isotonic-monotone"){
        exact(6);auto t=read_csv(argv[2]);
        auto predicted=monotone_quantile_predictions(
            t.numbers(argv[3]),t.numbers(argv[4]),{number(5)});
        out<<"Prediction "<<predicted[0]<<"\n";
    }else if(cmd=="monotone-violation"){
        exact(5);auto t=read_csv(argv[2]);
        out<<"LargestViolation "<<monotonicity_violation(
            t.numbers(argv[3]),t.numbers(argv[4]))<<"\n";
    }else return false;
    return true;
}
inline void sitewide_math_ml_usage(std::ostream& out){
    out<<"Mathematics and machine learning research algorithms:\n"
       <<"  partial-corr2 C11 C12 C22 RIDGE\n"
       <<"  trend-smooth CSV SERIES PENALTY\n"
       <<"  huber-fit CSV FEATURE TARGET THRESHOLD\n"
       <<"  empirical-cdf CSV SERIES QUERY\n"
       <<"  ks-two CSV SERIES1 SERIES2\n"
       <<"  permutation-means CSV GROUP VALUE REPLICATES SEED\n"
       <<"  stationary-bootstrap CSV SERIES EXPECTED_BLOCK SEED\n"
       <<"  pacf CSV SERIES LAGS | page-hinkley CSV SERIES ALLOWANCE LIMIT\n"
       <<"  aggregate-return CSV RETURN PERIODS | bipower CSV LOG_RETURN WINDOW\n"
       <<"  distance-cov CSV SERIES1 SERIES2\n"
       <<"  weighted-quantile|weighted-cvar CSV VALUES WEIGHTS LEVEL\n"
       <<"  linear-svm CSV FEATURE SIGNED_CLASS QUERY EPOCHS L2\n"
       <<"  quantile-linear CSV FEATURE TARGET QUANTILE QUERY EPOCHS STEP\n"
       <<"  expectile-linear CSV FEATURE TARGET EXPECTILE QUERY\n"
       <<"  ftrl-logit CSV FEATURE BINARY_LABEL EPOCHS QUERY\n"
       <<"  binary-auc|brier|binary-logloss CSV SCORE LABEL\n"
       <<"  psi CSV REFERENCE CURRENT BINS\n"
       <<"  kernel-mmd CSV GROUP FEATURE BANDWIDTH\n"
       <<"  permutation-importance TRAIN_CSV TEST_CSV FEATURE TARGET SEED REPS\n"
       <<"  crossfit-ridge CSV FEATURE TARGET FOLDS L2\n"
       <<"  rolling-origin CSV FEATURE TARGET INITIAL TEST STEP GAP L2\n"
       <<"  isotonic-monotone CSV SCORE TARGET QUERY\n"
       <<"  monotone-violation CSV SCORE TARGET\n";
}
} // namespace kline
