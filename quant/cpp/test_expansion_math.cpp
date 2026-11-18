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
#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace std;
int checks=0;
void check(bool condition,const char* label){
    checks++;if(!condition)throw runtime_error(label);
}
void approx(double actual,double expected,double tolerance,const char* label){
    check(isfinite(actual)&&fabs(actual-expected)<=tolerance,label);
}
void throws(const function<void()>& fn,const char* label){
    bool failed=false;try{fn();}catch(const exception&){failed=true;}
    check(failed,label);
}
void matrix_suite(){
    kline::Matrix a={{2,1},{1,2}};
    auto inv=kline::inverse_matrix(a);
    approx(inv[0][0],2.0/3,1e-11,"Inverse first diagonal");
    approx(inv[0][1],-1.0/3,1e-11,"Inverse first off diagonal");
    approx(kline::determinant(a),3,1e-12,"Determinant 2x2");
    auto identity=kline::matrix_multiply(a,inv);
    approx(identity[0][0],1,1e-11,"Matrix product diagonal");
    approx(identity[0][1],0,1e-11,"Matrix product off diagonal");
    approx(kline::matrix_transpose({{1,2,3},{4,5,6}})[2][1],6,1e-12,
           "Transpose rectangular");
    approx(kline::mahalanobis_squared({2,3},{1,1},{{1,0},{0,4}}),
           2,1e-12,"Mahalanobis quadratic");
    throws([]{kline::inverse_matrix({{1,1},{1,1}});},
           "Singular inverse rejected");
    throws([]{kline::matrix_multiply({{1,2}},{{1,2}});},
           "Invalid matmul shapes");
}
void robust_suite(){
    approx(kline::median({1,2,3,100}),2.5,1e-12,"Median even");
    approx(kline::median_absolute_deviation({1,2,3}),1,1e-12,"MAD");
    approx(kline::trimmed_mean({1,2,3,4,100},0.2),3,1e-12,
           "Symmetric trimming");
    approx(kline::robust_z_score(3,{1,2,3}),0.6744897501960817,1e-12,
           "Robust z");
    approx(kline::huber_location({1,2,3},2),2,1e-9,
           "Huber location");
    check(kline::lag_autocorrelation({1,2,3,4,5},1)>0,
          "Positive autocorrelation of trend");
    throws([]{kline::robust_z_score(5,{1,1,1});},
           "MAD degeneracy rejected");
    throws([]{kline::trimmed_mean({1,2,3},0.5);},
           "Overtrimming rejected");
}
void probability_suite(){
    approx(kline::normal_inverse_cdf(0.5),0,1e-12,
           "Normal median");
    approx(kline::normal_inverse_cdf(0.975),1.959963984540054,1e-10,
           "Normal 97.5th percentile");
    approx(kline::student_t_density(0,1),1/acos(-1.0),1e-12,
           "Cauchy density");
    approx(kline::poisson_probability(0,2),exp(-2),1e-12,
           "Poisson zero count");
    approx(kline::lognormal_density(1,0,1),1/sqrt(2*acos(-1.0)),1e-12,
           "Standard lognormal density");
    approx(kline::normal_interval_probability(-1,1,0,1),
           0.682689492137,1e-9,"Normal central coverage");
    throws([]{kline::normal_inverse_cdf(1);},"Normal p=1 rejected");
    throws([]{kline::poisson_probability(-1,2);},"Negative Poisson count");
}
void spectrum_suite(){
    auto dft=kline::discrete_fourier_transform({1,1,1,1});
    approx(dft[0].real(),4,1e-12,"DFT constant frequency zero");
    approx(abs(dft[1]),0,1e-12,"DFT constant frequency 1");
    auto power=kline::periodogram({1,0,-1,0});
    check(power.size()==3&&power[1]>power[0],"Periodogram dominant signal");
    approx(kline::dominant_frequency({1,0,-1,0}),0.25,1e-12,
           "One cycle per four samples");
    approx(kline::spectral_entropy({1,0,0}),0,1e-12,
           "Degenerate spectral entropy");
    approx(kline::spectral_entropy({1,1}),1,1e-12,
           "Flat spectrum maximal entropy");
    throws([]{kline::spectral_entropy({0,0});},
           "Empty spectral mass");
}
void time_series_suite(){
    vector<double> history={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    auto ar=kline::fit_autoregression(history,1);
    approx(kline::autoregressive_forecast(ar,history),16,1e-8,
           "AR(1) continuation");
    approx(kline::ar_long_run_mean({{1,0.5},0.1,1}),2,1e-12,
           "Stationary AR mean");
    auto diff=kline::difference_series({1,4,9,16},2);
    check(diff.size()==2,"Second differencing length");
    approx(diff[0],2,1e-12,"Second difference value");
    vector<double> residuals;
    for(int i=0;i<60;i++)residuals.push_back(0.01*(1+i%5));
    auto fitted=kline::fit_garch_grid(residuals,10);
    check(fitted.omega>0&&fitted.alpha+fitted.beta<1,
          "GARCH estimated stationary parameters");
    check(fitted.next_variance>0,"GARCH forecast positive");
    approx(kline::garch_unconditional_variance(0.02,0.2,0.3),0.04,1e-12,
           "GARCH unconditional variance");
    approx(kline::garch_forecast_horizon(0.04,0.02,0.2,0.3,3),0.04,1e-12,
           "GARCH mean reversion limit");
    throws([]{kline::fit_garch_grid({1,2,3});},
           "GARCH too little training");
}
void panel_suite(){
    kline::Matrix x={{1},{2},{3},{1},{2},{3}};
    vector<double> y={12,14,16,102,104,106};
    auto model=kline::entity_fixed_effects(x,y,{1,1,1,2,2,2});
    approx(model.coefficients[1],2,1e-10,
           "Fixed effects within slope");
    approx(model.coefficients[0],0,1e-10,
           "Fixed effects residual intercept");
    approx(kline::panel_within_r_squared(x,y,{1,1,1,2,2,2}),
           1,1e-12,"Within R-squared");
    vector<kline::Matrix> cross;
    vector<vector<double>> outcomes;
    for(int t=0;t<4;t++){
        cross.push_back({{0},{1},{2},{3},{4},{5}});
        outcomes.push_back({double(t),double(t+2),double(t+4),
                            double(t+6),double(t+8),double(t+10)});
    }
    auto fit=kline::fama_macbeth(cross,outcomes);
    approx(fit.average_coefficients[1],2,1e-10,
           "Fama-MacBeth slope");
    check(fit.standard_errors.size()==2,
          "Fama MacBeth coefficient uncertainty");
    throws([]{kline::entity_fixed_effects({{1},{2}},{1,2},{1,2});},
           "No repeat panels rejected");
}
void tails_suite(){
    approx(kline::gpd_survival(2,{0,2}),exp(-1),1e-12,
           "Exponential GPD limit");
    approx(kline::gpd_quantile(1-exp(-1),{0,2}),2,1e-12,
           "GPD quantile inverse");
    approx(kline::gpd_survival(1,{0.5,2}),
           pow(1.25,-2),1e-12,"Shape-positive GPD");
    approx(kline::gaussian_expected_shortfall(0,1,0.5),
           sqrt(2/acos(-1.0)),1e-12,"Gaussian ES median");
    check(kline::hill_tail_index({1,2,4,8,16},2)>0,
          "Positive Hill inverse shape");
    throws([]{kline::gpd_quantile(1,{0,1});},
           "GPD p=1 rejected");
}
void inference_suite(){
    auto welch=kline::welch_t_statistic({1,2,3},{2,3,4});
    approx(welch.estimate,-1,1e-12,"Welch difference");
    check(welch.t_value<0,"Welch negative mean shift");
    auto paired=kline::paired_t_statistic({1,4,7},{0,2,3});
    approx(paired.estimate,7.0/3,1e-12,"Paired mean difference");
    approx(kline::population_standard_error({1,2,3}),sqrt(1.0/3),
           1e-12,"Sample standard error");
    approx(kline::shannon_entropy({0.5,0.5}),log(2),1e-12,
           "Binary entropy");
    approx(kline::binary_entropy(0.5),log(2),1e-12,
           "Binary entropy helper");
    approx(kline::kullback_leibler({0.5,0.5},{0.5,0.5}),0,1e-12,
           "KL identical");
    approx(kline::effective_state_count({0.5,0.5}),2,1e-12,
           "Effective equally probable states");
    throws([]{kline::kullback_leibler({1,0},{0,1});},
           "KL singular support");
}
void machine_learning_suite(){
    auto rls=kline::initialize_rls(1,0.01);
    for(int t=0;t<30;t++)
        kline::rls_update(rls,{1.0},2);
    approx(kline::rls_predict(rls,{1.0}),2,1e-3,
           "Recursive least squares learns constant");
    auto near_ids=kline::nearest_neighbors({{0},{1},{10}},{1.2},2);
    check(near_ids[0]==1&&near_ids[1]==0,
          "Nearest neighbor tie-free ordering");
    approx(kline::knn_regression({{0},{1},{10}},{0,2,20},{1.2},1),
           2,1e-12,"KNN regression");
    approx(kline::knn_binary_probability({{0},{1},{10}},{0,1,1},{0.9},1),
           1,1e-12,"KNN classification");
    auto stump=kline::fit_regression_stump(
        {{0},{1},{2},{3},{4}},{0,0,0,10,10});
    approx(kline::stump_predict(stump,{4}),10,1e-12,
           "Decision stump upper leaf");
    approx(kline::stump_training_rmse(stump,
        {{0},{1},{2},{3},{4}},{0,0,0,10,10}),0,1e-12,
           "Decision stump exact training split");
    auto boosted=kline::gradient_boost_stumps(
        {{0},{1},{2},{3},{4}},{0,0,0,10,10},30,0.2);
    check(kline::boosted_predict(boosted,{4})>9,
          "Boosting learns upper leaf");
    auto bayes=kline::fit_gaussian_naive_bayes(
        {{-2},{-1},{1},{2}},{0,0,1,1});
    check(kline::gaussian_nb_probability(bayes,{1.5})>0.9,
          "Gaussian Naive Bayes positive cluster");
    auto iso=kline::fit_isotonic({0.1,0.2,0.3,0.4},{0,1,0,1});
    check(kline::isotonic_predict(iso,0.4)>=
          kline::isotonic_predict(iso,0.1),
          "Monotonic isotonic probabilities");
    throws([]{kline::fit_regression_stump(
        {{1},{1},{1}},{0,1,2});},
        "Constant stump feature rejects split");
    throws([]{kline::fit_gaussian_naive_bayes(
        {{1},{2}},{0,0});},"Bayes missing class");
}
void optimization_suite(){
    auto clipped=kline::eigenvalue_floor_covariance({{1,2},{2,1}},0.01);
    auto eigen=kline::symmetric_eigen(clipped);
    check(eigen.values[1]>0,"Spectral covariance floor");
    auto corr=kline::covariance_correlation({{1,0.5},{0.5,4}});
    approx(corr[0][1],0.25,1e-12,"Covariance correlation mapping");
    auto minvar=kline::unconstrained_min_variance({{1,0},{0,4}});
    approx(minvar[0],0.8,1e-12,"Analytical minimum variance weights");
    auto target=kline::mean_variance_target_return(
        {{1,0},{0,1}},{0.1,0.2},0.15);
    approx(target[0],0.5,1e-12,"Frontier target mean first");
    approx(target[1],0.5,1e-12,"Frontier target mean second");
    auto hrp=kline::hierarchical_risk_parity({{1,0},{0,4}});
    approx(hrp[0],0.8,1e-12,"HRP inverse variance first");
    approx(hrp[1],0.2,1e-12,"HRP inverse variance second");
    throws([]{kline::unconstrained_min_variance({{1,1},{1,1}});},
           "Singular covariance minimum variance");
}
void nonlinear_suite(){
    auto poly=kline::polynomial_features({2,3},3);
    check(poly.size()==7,"Polynomial feature count");
    approx(poly[2],4,1e-12,"Squared feature");
    auto rbf=kline::radial_basis_features({1},{{1},{2}},1);
    approx(rbf[0],1,1e-12,"RBF center peak");
    approx(rbf[1],exp(-0.5),1e-12,"RBF Gaussian tail");
    approx(kline::tanh_activation(0),0,1e-12,"tanh origin");
    auto interaction=kline::interaction_features({2,3,4});
    check(interaction.size()==6,"Pair interactions count");
    approx(interaction[3],6,1e-12,"Pair interaction first pair");
    approx(kline::cross_sectional_cosine_similarity({1,0},{0,1}),0,1e-12,
           "Orthogonal exposures");
    approx(kline::factor_spread_signal({1,-1},{3,2}),1,1e-12,
           "Linear factor spread");
    auto mc=kline::monte_carlo_confidence({1,2,3});
    approx(mc.mean,2,1e-12,"Monte Carlo mean");
    approx(mc.standard_error,sqrt(1.0/3),1e-12,
           "Monte Carlo sample standard error");
    approx(kline::antithetic_call_payoff(100,90,0,0,1,2),10,1e-12,
           "Antithetic zero-vol option");
    auto halton=kline::halton_points(3,{2,3});
    approx(halton[0][0],0.5,1e-12,"Halton first base two");
    approx(halton[1][1],2.0/3,1e-12,"Halton second base three");
    approx(kline::radical_inverse(3,2),0.75,1e-12,
           "Binary radical inverse");
    auto regimes=kline::compare_market_regimes(
        {1,2,3,4,5,6,7,8},4);
    approx(regimes.mean_difference,4,1e-12,"Regime mean change");
    check(kline::cumulative_sum_change_score({1,1,1,4,4,4})>0,
          "CUSUM score detects shift");
}
int main(){
    try{
        matrix_suite();robust_suite();probability_suite();
        spectrum_suite();time_series_suite();panel_suite();
        tails_suite();inference_suite();machine_learning_suite();
        optimization_suite();nonlinear_suite();
        cout<<"PASS: "<<checks<<" expanded quantitative math, econometrics and ML checks\n";
        return 0;
    }catch(const exception& e){
        cerr<<"FAIL: "<<e.what()<<"\n";return 1;
    }
}
