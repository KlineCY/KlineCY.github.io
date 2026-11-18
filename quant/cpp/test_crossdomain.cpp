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
#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace std;
int assertions=0;
void okay(bool condition,const char* title){
    assertions++;if(!condition)throw runtime_error(title);
}
void near(double actual,double value,double tolerance,const char* title){
    okay(isfinite(actual)&&fabs(actual-value)<=tolerance,title);
}
void fails(const function<void()>& action,const char* title){
    bool caught=false;try{action();}catch(const exception&){caught=true;}
    okay(caught,title);
}
void econometrics(){
    kline::Matrix x;
    vector<double> y,levels;
    for(int i=0;i<40;i++){
        double f=(i-20)/10.0;
        x.push_back({f});y.push_back(1+2*f);
        levels.push_back(sin(i*0.7)+0.3*cos(i*0.29));
    }
    auto post=kline::bayesian_linear_posterior(x,y,0.01,1000);
    auto estimated=kline::bayesian_linear_prediction(post,{1});
    near(estimated.first,3,0.001,"Bayesian posterior regression mean");
    okay(estimated.second>=0.01,"Bayesian predictive uncertainty");
    okay(post.posterior_covariance[0][0]>0,
         "Positive Bayesian posterior variance");
    auto adf=kline::augmented_dickey_fuller(levels,2);
    okay(isfinite(adf.t_statistic)&&adf.usable_observations==37,
         "ADF sufficient sample and finite t");
    okay(kline::kpss_level_statistic(levels,3)>=0,
         "KPSS nonnegative statistic");
    vector<double> predictor,dependent;
    for(int i=0;i<70;i++){
        predictor.push_back(sin(i*.3)+cos(i*.59));
        dependent.push_back((i>0?0.5*predictor[i-1]:0)+
                            0.3*sin(i*.7)+0.1*cos(i*.17));
    }
    auto granger=kline::granger_predictive_test(dependent,predictor,2);
    okay(granger.f_statistic>=0&&granger.full_rss<=granger.restricted_rss+1e-9,
         "Nested Granger regression sum of squares");
    auto did=kline::difference_in_differences(
        {10,10,11,11,12,12,15,15},
        {0,0,0,0,1,1,1,1},{0,0,1,1,0,0,1,1});
    near(did.treatment_effect,2,1e-12,"Difference in differences 2x2");
    near(did.standard_error,0,1e-12,"Within-cell fixed observations");
    vector<double> dependent_pair,hedge;
    for(int i=0;i<30;i++){
        hedge.push_back(i*0.2+0.1*sin(i));
        dependent_pair.push_back(1+2*hedge.back());
    }
    auto spread=kline::past_only_pair_spreads(dependent_pair,hedge,8);
    okay(spread.size()==22,"Rolling pair history must lag decision");
    near(spread.back().hedge_ratio,2,1e-8,"Rolling past-only hedge ratio");
    near(spread.back().spread,0,1e-8,"Rolling pair residual");
    auto beta=kline::beta_binomial_update(3,1);
    near(beta.mean,2.0/3,1e-12,"Bayesian Bernoulli posterior");
    near(beta.variance,8.0/252,1e-12,"Beta posterior variance");
    auto weighted=kline::weighted_qr_intercept(
        {{0},{1},{2},{3}},{1,3,5,7},{1,2,2,1});
    near(weighted.coefficients[0],1,1e-10,"Weighted QR intercept");
    near(weighted.coefficients[1],2,1e-10,"Weighted QR slope");
    fails([]{kline::kpss_level_statistic({1,1,1,1,1,1},2);},
         "Constant KPSS sample rejected");
    fails([]{kline::difference_in_differences(
        {1,2},{0,1},{0,1});},"Incomplete DID cells rejected");
}
void predictive(){
    auto hmm=kline::gaussian_hmm_two_state(
        {0,0.1,4.8,5.1},{0.5,0.5},
        {{0.9,0.1},{0.1,0.9}},{0,5},{1,1});
    okay(hmm.filtered.size()==4&&isfinite(hmm.log_likelihood),
         "Gaussian HMM observations and log likelihood");
    near(hmm.filtered.back()[0]+hmm.filtered.back()[1],1,1e-12,
         "HMM posterior normalization");
    okay(hmm.filtered.front()[0]>0.9&&hmm.filtered.back()[1]>0.9,
         "Gaussian HMM state change filtering");
    auto gmm=kline::fit_gaussian_mixture_two(
        {-6,-5.5,-5,-4.5,-4,4,4.5,5,5.5,6},75);
    okay(gmm.mean0<0&&gmm.mean1>0&&gmm.var0>0&&gmm.var1>0,
         "Two-component GMM EM clusters");
    kline::Matrix x;vector<double> y;
    for(int i=0;i<24;i++){
        x.push_back({i*0.1});y.push_back(1+2*i*0.1+0.01*sin(i));
    }
    auto selection=kline::select_ridge_chronologically(
        x,y,{0.0001,0.1},12,18);
    okay(selection.validation_mse>=0&&selection.penalty>0,
         "Chronologically selected ridge penalty");
    near(kline::ridge_predict(selection.fitted,{2}),5,0.12,
         "Chronological ridge refit after validation");
    auto iso=kline::fit_isotonic_real(
        {1,2,3,4},{0,3,1,4});
    near(kline::isotonic_real_predict(iso,2.5),2,1e-12,
         "Isotonic real pooled violation");
    auto multi=kline::fit_multitarget_ridge(x,
        [&](){kline::Matrix targets;
              for(auto r:x)targets.push_back({1+2*r[0],-1+r[0]});
              return targets;}(),0.0001);
    auto m=kline::multitarget_predict(multi,{2});
    near(m[0],5,0.01,"Multitarget Ridge first task");
    near(m[1],1,0.01,"Multitarget Ridge second task");
    auto decay=kline::fit_exponential_decay_ols(
        {{0},{1},{2},{3},{4}},{1,3,5,7,9},2);
    near(decay.coefficients[1],2,1e-10,"Exponential weighted OLS slope");
    auto ensemble=kline::inverse_error_ensemble_weights(
        {{1,10},{2,10},{3,10}}, {1,2,3});
    okay(ensemble[0]>ensemble[1],"Lower-error ensemble favored");
    near(ensemble[0]+ensemble[1],1,1e-12,"Ensemble normalized weights");
    near(kline::weighted_model_forecast({0.25,0.75},{1,3}),
         2.5,1e-12,"Ensemble weighted forecast");
    double q=kline::binary_conformal_nonconformity_threshold(
        {0.1,0.2,0.8,0.9,0.85,0.15},{0,0,1,1,1,0},0.2);
    okay(q>=0&&q<=1,"Classification conformal quantile");
    auto predictions=kline::binary_conformal_set(0.95,0.2);
    okay(predictions.size()==1&&predictions[0]==1,
         "Classification conformal positive set");
    fails([]{kline::gaussian_hmm_two_state({1},{0.5,0.5},
        {{1,0}}, {0,1},{1,1});},"HMM transition shape rejected");
    fails([]{kline::select_ridge_chronologically(
        {{1},{2},{3},{4}}, {1,2,3,4},{0.1,1},3,4);},
        "Short nested train rejected");
}
void risk_microstructure(){
    auto ewma=kline::ewma_multivariate_covariance(
        {{1,2},{2,3}},0);
    near(ewma[0][0],4,1e-12,"EWMA current return variance");
    near(ewma[0][1],6,1e-12,"EWMA current covariance");
    auto neutral=kline::minimum_change_factor_neutral(
        {0.6,0.4},{{1,1}},{0});
    near(neutral[0]+neutral[1],0,1e-12,"Factor neutral equality");
    near(neutral[0]-neutral[1],0.2,1e-12,"Minimum-change preservation");
    double var=kline::normal_var_with_liquidation(
        100,0,0.1,0.975,{10,20},{0.01,0.01});
    near(var,100*1.959963984540054*0.1+0.3,1e-7,
         "Liquidity-adjusted parametric VaR");
    near(kline::exponential_hawkes_intensity({},2,1,2,3),1,
         1e-12,"Hawkes baseline intensity");
    near(kline::exponential_hawkes_log_likelihood(
        {1,2},3,1,0,1),-3,1e-12,
        "Poisson nested within Hawkes log likelihood");
    auto schedule=kline::volume_limited_execution(
        100,{100,200},0.2);
    near(schedule.fills[0],20,1e-12,"POV first interval");
    near(schedule.fills[1],40,1e-12,"POV second interval");
    near(schedule.unfilled,40,1e-12,"POV unfilled quantity carried");
    near(kline::realized_participation_ratio(schedule.fills,{100,200}),
         0.2,1e-12,"POV realized participation");
    kline::Matrix deterministic(6,{90,90,90});
    near(kline::longstaff_schwartz_american_put(
        deterministic,100,0,1),10,1e-12,
        "LSM deterministic immediate exercise put");
    near(kline::call_implied_probability_above_strike(
        12,10,100,110,1),0.2,1e-12,"Call spread digital probability");
    near(kline::call_implied_density_equal_spacing(
        12,10,9,10,1),0.01,1e-12,"Call convexity risk-neutral density");
    kline::ZeroCurve curve{{1,2,3},{0.05,0.05,0.05}};
    near(kline::bond_static_curve_rolldown_pnl(
        curve,{2},{100},1),100*(exp(-0.05)-exp(-0.1)),1e-12,
        "Flat curve bond rolling carry");
    fails([]{kline::ewma_multivariate_covariance(
        {{1,2},{1}},0.9);},"Ragged EWMA covariance rejected");
    fails([]{kline::call_implied_density_equal_spacing(
        1,4,1,1,1);},"No arbitrage convexity rejection");
}
int main(){
 try{econometrics();predictive();risk_microstructure();
     cout<<"PASS: "<<assertions<<" crossdomain C++17 econometrics, ML and trading checks\n";
 }catch(const exception& e){cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
