#pragma once
#include "matrix_algorithms.hpp"
#include "robust_statistics.hpp"
#include "probability_models.hpp"
#include "spectral_analysis.hpp"
#include "autoregressive_models.hpp"
#include "volatility_estimation.hpp"
#include "panel_econometrics.hpp"
#include "fama_macbeth.hpp"
#include "tail_distributions.hpp"
#include "hypothesis_testing.hpp"
#include "information_theory.hpp"
#include "online_learning.hpp"
#include "neighbors_models.hpp"
#include "tree_models.hpp"
#include "boosting_models.hpp"
#include "naive_bayes_models.hpp"
#include "isotonic_calibration.hpp"
#include "covariance_regularization.hpp"
#include "efficient_frontier.hpp"
#include "hierarchical_portfolio.hpp"
#include "nonlinear_features.hpp"
#include "factor_interactions.hpp"
#include "monte_carlo_precision.hpp"
#include "quasi_monte_carlo.hpp"
#include "regime_diagnostics.hpp"
#include "io.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
namespace kline {
inline void expansion_print_series(std::ostream& out,const std::string& label,
                                   const std::vector<double>& values){
    out<<"Index,"<<label<<"\n";
    for(int t=0;t<int(values.size());t++)
        out<<t<<","<<values[t]<<"\n";
}
inline Matrix expansion_numeric_rows(const CsvTable& t,
                                      const std::vector<std::string>& columns){
    require(!columns.empty(),"No feature columns");
    Matrix rows(t.rows.size());
    for(const auto& col:columns){
        auto values=t.numbers(col);
        for(int i=0;i<int(values.size());i++)rows[i].push_back(values[i]);
    }
    return rows;
}
inline bool run_expansion_math(const std::string& command,int argc,char** argv,
                               std::ostream& out){
    auto number=[&](int i){return parse_number(argv[i]);};
    auto exact=[&](int count){require(argc==count,"Incorrect arguments");};
    if(command=="matrix-det2"){
        exact(6);
        out<<"Determinant "<<determinant({{number(2),number(3)},
                                         {number(4),number(5)}})<<"\n";
    }else if(command=="matrix-inverse2"){
        exact(6);
        auto a=inverse_matrix({{number(2),number(3)},{number(4),number(5)}});
        out<<"Row,Col0,Col1\n0,"<<a[0][0]<<","<<a[0][1]<<"\n1,"
           <<a[1][0]<<","<<a[1][1]<<"\n";
    }else if(command=="matrix-mahal2"){
        exact(9);
        out<<"MahalanobisSquared "<<mahalanobis_squared(
            {number(2),number(3)},{number(4),number(5)},
            {{number(6),number(7)},{number(7),number(8)}})<<"\n";
    }else if(command=="robust-median" || command=="robust-mad" ||
             command=="robust-autocorr" || command=="robust-trim" ||
             command=="robust-huber" || command=="robust-z"){
        require(argc>=4&&argc<=6,"Robust statistics CSV and column");
        auto values=read_csv(argv[2]).numbers(argv[3]);
        if(command=="robust-median"){exact(4);out<<"Median "<<median(values)<<"\n";}
        else if(command=="robust-mad"){exact(4);out<<"MAD "<<median_absolute_deviation(values)<<"\n";}
        else if(command=="robust-autocorr"){exact(5);out<<"ACF "<<lag_autocorrelation(values,std::stoi(argv[4]))<<"\n";}
        else if(command=="robust-trim"){exact(5);out<<"TrimmedMean "<<trimmed_mean(values,number(4))<<"\n";}
        else if(command=="robust-huber"){exact(5);out<<"HuberLocation "<<huber_location(values,number(4))<<"\n";}
        else{exact(5);out<<"RobustZ "<<robust_z_score(number(4),values)<<"\n";}
    }else if(command=="normal-quantile"){
        exact(3);out<<"NormalQuantile "<<normal_inverse_cdf(number(2))<<"\n";
    }else if(command=="student-density"){
        exact(4);out<<"Density "<<student_t_density(number(2),number(3))<<"\n";
    }else if(command=="poisson-probability"){
        exact(4);out<<"Probability "<<poisson_probability(std::stoi(argv[2]),number(3))<<"\n";
    }else if(command=="lognormal-density"){
        exact(5);out<<"Density "<<lognormal_density(number(2),number(3),number(4))<<"\n";
    }else if(command=="normal-interval"){
        exact(6);out<<"Probability "<<normal_interval_probability(
            number(2),number(3),number(4),number(5))<<"\n";
    }else if(command=="periodogram"){
        exact(4);
        expansion_print_series(out,"Power",periodogram(read_csv(argv[2]).numbers(argv[3])));
    }else if(command=="dominant-frequency"){
        exact(4);
        out<<"Frequency "<<dominant_frequency(read_csv(argv[2]).numbers(argv[3]))<<"\n";
    }else if(command=="spectral-entropy"){
        exact(4);
        out<<"SpectralEntropy "<<spectral_entropy(
            periodogram(read_csv(argv[2]).numbers(argv[3])))<<"\n";
    }else if(command=="arp-fit"){
        exact(5);
        auto x=read_csv(argv[2]).numbers(argv[3]);
        auto fit=fit_autoregression(x,std::stoi(argv[4]));
        out<<"Forecast "<<autoregressive_forecast(fit,x)<<"\n"
           <<"ResidualVariance "<<fit.residual_variance<<"\n";
        expansion_print_series(out,"Coefficients",fit.coefficients);
    }else if(command=="series-difference"){
        exact(5);
        expansion_print_series(out,"Difference",difference_series(
            read_csv(argv[2]).numbers(argv[3]),std::stoi(argv[4])));
    }else if(command=="garch-estimate"){
        exact(5);auto fit=fit_garch_grid(read_csv(argv[2]).numbers(argv[3]),
                                          std::stoi(argv[4]));
        out<<"Omega "<<fit.omega<<"\nAlpha "<<fit.alpha<<"\nBeta "
           <<fit.beta<<"\nNextVariance "<<fit.next_variance<<"\n"
           <<"NegativeLogLikelihood "<<fit.negative_log_likelihood<<"\n";
    }else if(command=="garch-steady"){
        exact(5);
        out<<"UnconditionalVariance "<<garch_unconditional_variance(
            number(2),number(3),number(4))<<"\n";
    }else if(command=="garch-horizon"){
        exact(8);
        out<<"HorizonVariance "<<garch_forecast_horizon(
            number(2),number(3),number(4),number(5),std::stoi(argv[6]))<<"\n";
    }else if(command=="panel-fe"){
        exact(6);
        auto t=read_csv(argv[2]);
        auto ids=t.numbers(argv[3]),label=t.numbers(argv[4]);
        std::vector<int> entity;
        for(double v:ids){
            require(v==std::floor(v)&&v>=0&&v<=100000000,
                    "Entity IDs must be nonnegative integers");
            entity.push_back(int(v));
        }
        Matrix x=expansion_numeric_rows(t,{argv[5]});
        auto fit=entity_fixed_effects(x,label,entity);
        out<<"WithinSlope "<<fit.coefficients[1]<<"\n"
           <<"WithinRSquared "<<fit.r_squared<<"\n";
    }else if(command=="gpd-survival"){
        exact(5);
        out<<"Survival "<<gpd_survival(number(2),{number(3),number(4)})<<"\n";
    }else if(command=="gpd-quantile"){
        exact(5);
        out<<"Quantile "<<gpd_quantile(number(2),{number(3),number(4)})<<"\n";
    }else if(command=="hill-index"){
        exact(5);
        out<<"HillTailIndex "<<hill_tail_index(
            read_csv(argv[2]).numbers(argv[3]),std::stoi(argv[4]))<<"\n";
    }else if(command=="gaussian-es"){
        exact(5);
        out<<"ExpectedShortfall "<<gaussian_expected_shortfall(
            number(2),number(3),number(4))<<"\n";
    }else if(command=="welch-t" || command=="paired-t"){
        exact(5);auto table=read_csv(argv[2]);
        auto a=table.numbers(argv[3]),b=table.numbers(argv[4]);
        auto result=command=="welch-t"?welch_t_statistic(a,b):
                                          paired_t_statistic(a,b);
        out<<"Difference "<<result.estimate<<"\n"
           <<"StandardError "<<result.standard_error<<"\n"
           <<"TStatistic "<<result.t_value<<"\n";
    }else if(command=="entropy"){
        exact(4);
        out<<"ShannonEntropy "<<shannon_entropy(
            read_csv(argv[2]).numbers(argv[3]))<<"\n";
    }else if(command=="kl-divergence"){
        exact(5);auto table=read_csv(argv[2]);
        out<<"KL "<<kullback_leibler(table.numbers(argv[3]),
                                     table.numbers(argv[4]))<<"\n";
    }else if(command=="binary-entropy"){
        exact(3);
        out<<"Entropy "<<binary_entropy(number(2))<<"\n";
    }else if(command=="rls-train"){
        exact(6);auto t=read_csv(argv[2]);
        auto x=t.numbers(argv[3]),y=t.numbers(argv[4]);
        auto state=initialize_rls(2,number(5));
        for(int i=0;i<int(x.size());i++)
            rls_update(state,{1,x[i]},y[i]);
        out<<"Intercept "<<state.coefficients[0]<<"\n"
           <<"Slope "<<state.coefficients[1]<<"\n";
    }else if(command=="knn-regress" || command=="knn-probability"){
        exact(7);auto t=read_csv(argv[2]);
        Matrix x=expansion_numeric_rows(t,{argv[3]});
        int k=std::stoi(argv[6]);
        if(command=="knn-regress")
            out<<"Prediction "<<knn_regression(x,t.numbers(argv[4]),
                                                {number(5)},k)<<"\n";
        else{
            auto raw=t.numbers(argv[4]);std::vector<int> labels;
            for(double v:raw){
                require(v==0||v==1,"Binary labels expected");
                labels.push_back(int(v));
            }
            out<<"Probability "<<knn_binary_probability(x,labels,
                                                         {number(5)},k)<<"\n";
        }
    }else if(command=="stump-regress" || command=="boost-regress"){
        exact(7);auto t=read_csv(argv[2]);
        auto y=t.numbers(argv[4]);
        Matrix x=expansion_numeric_rows(t,{argv[3]});
        if(command=="stump-regress"){
            auto fit=fit_regression_stump(x,y);
            out<<"Prediction "<<stump_predict(fit,{number(5)})<<"\n"
               <<"TrainingRMSE "<<stump_training_rmse(fit,x,y)<<"\n";
        }else{
            auto fit=gradient_boost_stumps(x,y,std::stoi(argv[6]));
            out<<"Prediction "<<boosted_predict(fit,{number(5)})<<"\n";
        }
    }else if(command=="gaussian-nb"){
        exact(6);auto t=read_csv(argv[2]);
        Matrix x=expansion_numeric_rows(t,{argv[3]});
        auto raw=t.numbers(argv[4]);
        std::vector<int> labels;
        for(double value:raw){
            require(value==0||value==1,"Naive Bayes binary label");
            labels.push_back(int(value));
        }
        out<<"Probability "<<gaussian_nb_probability(
            fit_gaussian_naive_bayes(x,labels),{number(5)})<<"\n";
    }else if(command=="isotonic-fit"){
        exact(6);auto t=read_csv(argv[2]);
        auto scores=t.numbers(argv[3]),raw=t.numbers(argv[4]);
        std::vector<int> labels;
        for(double value:raw){
            require(value==0||value==1,"Isotonic binary label");
            labels.push_back(int(value));
        }
        out<<"CalibratedProbability "<<isotonic_predict(
            fit_isotonic(scores,labels),number(5))<<"\n";
    }else if(command=="cov-eigen-floor"){
        exact(6);auto t=read_csv(argv[2]);
        auto a=t.numbers(argv[3]),b=t.numbers(argv[4]);
        auto cov=sample_covariance_matrix({a,b});
        auto adjusted=eigenvalue_floor_covariance(cov,number(5));
        out<<"Cov00 "<<adjusted[0][0]<<"\nCov01 "<<adjusted[0][1]
           <<"\nCov11 "<<adjusted[1][1]<<"\n";
    }else if(command=="gmv2"||command=="hrp2"){
        exact(5);auto t=read_csv(argv[2]);
        auto cov=sample_covariance_matrix(
            {t.numbers(argv[3]),t.numbers(argv[4])});
        auto w=command=="gmv2"?unconstrained_min_variance(cov):
                                hierarchical_risk_parity(cov);
        expansion_print_series(out,"Weight",w);
    }else if(command=="mean-frontier2"){
        exact(8);auto t=read_csv(argv[2]);
        auto cov=sample_covariance_matrix(
            {t.numbers(argv[3]),t.numbers(argv[4])});
        auto w=mean_variance_target_return(cov,
                      {number(6),number(7)},number(5));
        expansion_print_series(out,"TargetWeight",w);
    }else if(command=="polynomial-features"){
        exact(5);
        expansion_print_series(out,"Polynomial",
            polynomial_features(read_csv(argv[2]).numbers(argv[3]),
                                std::stoi(argv[4])));
    }else if(command=="rbf-value"){
        exact(5);
        out<<"RBF "<<radial_basis_features({number(2)},{{number(3)}},
                                           number(4))[0]<<"\n";
    }else if(command=="interaction-features"){
        exact(4);
        expansion_print_series(out,"Interaction",interaction_features(
            read_csv(argv[2]).numbers(argv[3])));
    }else if(command=="cosine-similarity"){
        exact(5);auto t=read_csv(argv[2]);
        out<<"Cosine "<<cross_sectional_cosine_similarity(
            t.numbers(argv[3]),t.numbers(argv[4]))<<"\n";
    }else if(command=="mc-confidence"){
        exact(4);auto fit=monte_carlo_confidence(
            read_csv(argv[2]).numbers(argv[3]));
        out<<"Mean "<<fit.mean<<"\nSE "<<fit.standard_error<<"\n"
           <<"Lower95 "<<fit.lower95<<"\nUpper95 "<<fit.upper95<<"\n";
    }else if(command=="antithetic-call"){
        exact(8);
        out<<"AntitheticPremium "<<antithetic_call_payoff(
            number(2),number(3),number(4),number(5),number(6),
            number(7))<<"\n";
    }else if(command=="halton2"){
        exact(3);
        auto points=halton_points(std::stoi(argv[2]),{2,3});
        out<<"Index,Base2,Base3\n";
        for(int i=0;i<int(points.size());i++)
            out<<i<<","<<points[i][0]<<","<<points[i][1]<<"\n";
    }else if(command=="regime-compare"){
        exact(5);auto fit=compare_market_regimes(
            read_csv(argv[2]).numbers(argv[3]),std::stoi(argv[4]));
        out<<"FirstMean "<<fit.mean_first<<"\nSecondMean "
           <<fit.mean_second<<"\nDifference "<<fit.mean_difference<<"\n";
    }else if(command=="cusum-change"){
        exact(4);
        out<<"ChangeScore "<<cumulative_sum_change_score(
            read_csv(argv[2]).numbers(argv[3]))<<"\n";
    }else return false;
    return true;
}
inline void expansion_math_usage(std::ostream& out){
    out<<"  matrix-det2 A11 A12 A21 A22\n"
       <<"  matrix-inverse2 A11 A12 A21 A22\n"
       <<"  matrix-mahal2 X1 X2 CENTER1 CENTER2 C11 C12 C22\n"
       <<"  robust-median|robust-mad CSV SERIES\n"
       <<"  robust-autocorr CSV SERIES LAG\n"
       <<"  robust-trim CSV SERIES FRACTION\n"
       <<"  robust-huber CSV SERIES THRESHOLD\n"
       <<"  robust-z CSV SERIES VALUE\n"
       <<"  normal-quantile P | student-density X DF\n"
       <<"  poisson-probability COUNT INTENSITY\n"
       <<"  lognormal-density VALUE LOGMEAN LOGSTD\n"
       <<"  normal-interval LOW HIGH MEAN STD\n"
       <<"  periodogram|dominant-frequency|spectral-entropy CSV SERIES\n"
       <<"  arp-fit CSV SERIES LAGS | series-difference CSV SERIES ORDER\n"
       <<"  garch-estimate CSV RETURN GRID_RESOLUTION\n"
       <<"  garch-steady OMEGA ALPHA BETA\n"
       <<"  garch-horizon VAR OMEGA ALPHA BETA HORIZON\n"
       <<"  panel-fe CSV ENTITY_ID OUTCOME CONTROL\n"
       <<"  gpd-survival|gpd-quantile VALUE SHAPE SCALE\n"
       <<"  hill-index CSV POSITIVE_LOSS TOP_COUNT\n"
       <<"  gaussian-es MEAN_LOSS SIGMA CONFIDENCE\n"
       <<"  welch-t|paired-t CSV SERIES_A SERIES_B\n"
       <<"  entropy CSV PROBABILITIES | kl-divergence CSV P Q\n"
       <<"  binary-entropy P\n"
       <<"  rls-train CSV FEATURE TARGET RIDGE\n"
       <<"  knn-regress|knn-probability CSV FEATURE LABEL QUERY K\n"
       <<"  stump-regress|boost-regress CSV FEATURE TARGET QUERY ROUNDS\n"
       <<"  gaussian-nb|isotonic-fit CSV FEATURE LABEL QUERY\n"
       <<"  cov-eigen-floor CSV RETURN_A RETURN_B FLOOR_FRACTION\n"
       <<"  gmv2|hrp2 CSV RETURN_A RETURN_B\n"
       <<"  mean-frontier2 CSV RETURN_A RETURN_B TARGET MEAN_A MEAN_B\n"
       <<"  polynomial-features CSV SERIES DEGREE\n"
       <<"  rbf-value X CENTER BANDWIDTH\n"
       <<"  interaction-features CSV SERIES\n"
       <<"  cosine-similarity CSV SERIES_A SERIES_B\n"
       <<"  mc-confidence CSV SAMPLE\n"
       <<"  antithetic-call SPOT STRIKE RATE VOL MATURITY NORMAL\n"
       <<"  halton2 COUNT\n"
       <<"  regime-compare CSV SERIES BREAKPOINT\n"
       <<"  cusum-change CSV SERIES\n";
}
} // namespace kline
