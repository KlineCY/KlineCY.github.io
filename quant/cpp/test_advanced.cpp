#include "numerical.hpp"
#include "timeseries.hpp"
#include "indicators.hpp"
#include "regression_models.hpp"
#include "portfolio_risk.hpp"
#include "options_models.hpp"
#include "curve_models.hpp"
#include "execution_models.hpp"
#include "inference_models.hpp"
#include "stochastic_models.hpp"
#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

int assertions=0;

void check(bool ok,const string& label) {
    assertions++;
    if (!ok) throw runtime_error("Failed: "+label);
}

void close_to(double got,double want,double tolerance,const string& label) {
    check(isfinite(got) && fabs(got-want)<=tolerance,label);
}

void must_throw(const function<void()>& run,const string& label) {
    bool failed=false;
    try { run(); } catch(const exception&) {failed=true;}
    check(failed,label);
}

void numerical_tests() {
    auto eigen=kline::symmetric_eigen({{2,1},{1,2}});
    close_to(eigen.values[0],3,1e-10,"largest eigenvalue");
    close_to(eigen.values[1],1,1e-10,"smallest eigenvalue");
    close_to(eigen.vectors[0][0],1/sqrt(2.0),1e-10,"leading eigenvector");
    close_to(eigen.vectors[1][0],1/sqrt(2.0),1e-10,"leading loading");
    close_to(kline::condition_number_symmetric({{2,1},{1,2}}),3,1e-10,
             "symmetric condition number");
    check(isinf(kline::condition_number_symmetric({{1,1},{1,1}})),
          "singular condition number");
    auto pca=kline::principal_components({{1,2},{2,4},{3,6},{4,8}},2);
    close_to(pca.eigenvalues[0],25.0/3,1e-10,"PCA covariance eigenvalue");
    close_to(pca.eigenvalues[1],0,1e-10,"PCA zero residual dimension");
    close_to(pca.loadings[0][0],1/sqrt(5.0),1e-10,"PCA first asset loading");
    check(pca.scores.size()==4 && pca.scores[0].size()==2,"PCA scores shape");
    must_throw([] {kline::principal_components({{1,2},{3}},1);},
               "PCA rejects unaligned features");
    must_throw([] {kline::symmetric_eigen({{1,2},{3,1}});},
               "Eigen decomposition rejects asymmetric matrix");
}

void time_series_tests() {
    auto ema=kline::exponential_mean({1,2,3},0.5);
    close_to(ema[2],2.25,1e-12,"EMA");
    auto ewma=kline::ewma_conditional_variance({2,3},0.5,1);
    close_to(ewma[0],1,1e-12,"EWMA uses previous history");
    close_to(ewma[1],2.5,1e-12,"EWMA next prediction");
    auto ar=kline::fit_ar1({1,3,7,15,31});
    close_to(ar.slope,2,1e-12,"AR slope");
    close_to(ar.intercept,1,1e-12,"AR intercept");
    close_to(kline::ar1_forecast(ar,31),63,1e-12,"AR forecast");
    auto h=kline::garch11_forecast_variances({2,0},1,0.1,0.8,1);
    close_to(h[0],1,1e-12,"GARCH time t variance");
    close_to(h[1],2.2,1e-12,"GARCH one-step forecast");
    auto vols=kline::rolling_annualized_volatility({0.01,-0.01,0.01,-0.01},2,252);
    close_to(vols[0],sqrt(0.0002)*sqrt(252.0),1e-12,"rolling volatility");
    close_to(kline::half_life_ar1(0.5),1,1e-12,"AR half-life");
    must_throw([] {kline::garch11_forecast_variances({1,2},0.1,0.5,0.5,1);},
               "GARCH rejects nonstationary sum");
    must_throw([] {kline::exponential_mean({1,2},0);},"EMA invalid smoothing");
}

void indicators_tests() {
    auto momentum=kline::simple_momentum({100,110,121,133.1},2,1);
    close_to(momentum.front(),0.21,1e-12,"skip-month momentum");
    auto rsi=kline::relative_strength_index({1,2,3,4},2);
    close_to(rsi[0],100,1e-12,"RSI all gains");
    auto flat=kline::relative_strength_index({2,2,2,2},2);
    close_to(flat[0],50,1e-12,"RSI flat market");
    auto atr=kline::average_true_range({11,12,13},{9,10,11},{10,11,12},2);
    close_to(atr[0],2,1e-12,"ATR initial Wilder average");
    close_to(atr[1],2,1e-12,"ATR recursive");
    auto breakout=kline::rolling_breakout_position({10,11,12,13},2);
    close_to(breakout[0],2,1e-12,"out-of-range breakout strength");
    must_throw([] {kline::average_true_range({1,2},{3,1},{2,2},2);},
               "reject invalid price bars");
}

void regression_tests() {
    kline::Matrix x={{0},{1},{2},{3},{4}};
    vector<double> y={1,3,5,7,9};
    auto lasso=kline::fit_lasso(x,y,0);
    close_to(kline::ridge_predict(lasso,{5}),11,1e-8,"unpenalized Lasso fit");
    auto shrunk=kline::fit_lasso(x,y,0.5);
    check(shrunk.coefficients[0]>0 &&
          shrunk.coefficients[0]<lasso.coefficients[0],
          "Lasso shrinks feature coefficient");
    auto zero=kline::fit_lasso(x,y,10);
    close_to(zero.coefficients[0],0,1e-10,"Lasso sparsity");
    auto logistic=kline::fit_logistic({{-4},{-2},{2},{4}},{0,0,1,1},0.1);
    check(kline::logistic_predict_probability(logistic,{-4})<0.5,
          "logistic negative example");
    check(kline::logistic_predict_probability(logistic,{4})>0.5,
          "logistic positive example");
    check(kline::logistic_log_loss(logistic,{{-4},{-2},{2},{4}},{0,0,1,1})<0.5,
          "logistic fitted log loss");
    close_to(kline::sigmoid(0),0.5,1e-12,"stable sigmoid midpoint");
    check(kline::sigmoid(-1000)>=0 && kline::sigmoid(1000)<=1,
          "stable sigmoid extremes");
    must_throw([] {kline::fit_logistic({{1},{2}},{0,2},0.1);},
               "nonbinary logistic labels rejected");
    must_throw([] {kline::fit_lasso({{1},{2}},{1,2},-1);},
               "negative Lasso penalty rejected");
}

void portfolio_tests() {
    auto cap=kline::project_capped_simplex({0.9,0.1,0},0.6);
    close_to(cap[0],0.6,1e-10,"cap binds");
    close_to(cap[0]+cap[1]+cap[2],1,1e-10,"simplex sum remains one");
    auto risk=kline::risk_budget_weights({{1,0},{0,4}},{1,1});
    close_to(risk[0],2.0/3,1e-9,"risk parity equal risk contributions");
    close_to(risk[1],1.0/3,1e-9,"risk parity smaller high-vol asset");
    auto contributions=kline::volatility_risk_contributions({{1,0},{0,4}},risk);
    close_to(contributions[0],contributions[1],1e-9,"Euler risk budget check");
    auto es=kline::historical_es_contributions({0.5,0.5},
        {{-0.1,0},{0,-0.3},{0.1,0.1},{0.01,0.01}},0.75);
    close_to(es[0],0,1e-12,"scenario ES first asset contribution");
    close_to(es[1],0.15,1e-12,"scenario ES second asset contribution");
    close_to(kline::portfolio_effective_number({0.5,0.5}),2,1e-12,
             "two equally weighted assets");
    must_throw([] {kline::project_capped_simplex({1,2},0.4);},
               "infeasible asset cap rejected");
    must_throw([] {kline::risk_budget_weights({{1,0},{0,0}},{1,1});},
               "zero risk asset rejected");
}

void derivative_tests() {
    double call=kline::black76(100,100,0.2,1,0.95,true);
    double put=kline::black76(100,100,0.2,1,0.95,false);
    close_to(call-put,0,1e-12,"Black76 put-call parity ATM");
    close_to(kline::black76(110,100,0,1,0.95),9.5,1e-12,
             "Black76 intrinsic zero volatility");
    double normal=kline::bachelier_price(100,100,10,1,1);
    close_to(normal,10/sqrt(2*acos(-1.0)),1e-10,"Bachelier ATM");
    close_to(kline::bachelier_implied_volatility(normal,100,100,1,1),
             10,1e-8,"Bachelier volatility inversion");
    double vega=kline::black76_vega(100,100,0.2,1,0.95);
    check(vega>0 && isfinite(vega),"Black76 Vega positive");
    must_throw([] {kline::black76(0,100,0.2,1,1);},
               "invalid Black76 forward");
    must_throw([] {kline::bachelier_implied_volatility(0,100,90,1,1);},
               "below-intrinsic normal option rejected");
}

void curve_tests() {
    kline::ZeroCurve curve{{1,2,3},{0.05,0.05,0.05}};
    close_to(kline::interpolate_zero_rate(curve,1.5),0.05,1e-12,
             "constant continuous rate interpolation");
    close_to(kline::curve_discount(curve,2),exp(-0.1),1e-12,
             "curve discount factor");
    close_to(kline::present_value_cashflows(curve,{2},{100}),
             100*exp(-0.1),1e-12,"cashflow present value");
    double annuity=exp(-0.05)+exp(-0.1);
    close_to(kline::par_swap_rate(curve,{1,2}),
             (1-exp(-0.1))/annuity,1e-12,"par swap coupon rate");
    auto key=kline::key_rate_dv01(curve,{2},{100});
    close_to(key[0],0,1e-10,"no one-year key rate exposure");
    close_to(key[1],100*exp(-0.1)*2*0.0001,1e-9,
             "two-year key rate DV01");
    close_to(key[2],0,1e-10,"no three-year key rate exposure");
    must_throw([] {kline::interpolate_zero_rate({{2,1},{0.03,0.04}},1);},
               "unordered tenor grid rejected");
    must_throw([] {kline::par_swap_rate({{1,2},{0.03,0.04}},{2,1});},
               "unordered payment grid rejected");
}

void execution_tests() {
    auto market=kline::walk_order_book({{100,10},{101,5}},12,true);
    close_to(market.executed,12,1e-12,"book walk full fill");
    close_to(market.average_price,1202.0/12,1e-12,"book walk volume price");
    auto partial=kline::walk_order_book({{100,10},{101,5}},20,true);
    close_to(partial.unfilled,5,1e-12,"book walk insufficient displayed volume");
    auto twap=kline::twap_schedule(12,3);
    close_to(twap[0],4,1e-12,"TWAP per slice");
    auto volume=kline::capped_volume_schedule({3,8,10},12);
    close_to(accumulate(volume.begin(),volume.end(),0.0),12,1e-10,
             "capacity schedule conserves order size");
    close_to(volume[0],3,1e-10,"small slice capacity respected");
    close_to(kline::passive_fill_quantity(10,7,12),5,1e-12,"FIFO displayed queue");
    close_to(kline::signed_markout(100,99.9,1),-0.1,1e-12,
             "buy-side adverse markout");
    must_throw([] {kline::walk_order_book({{101,2},{100,5}},3,true);},
               "unsorted sell levels rejected");
    must_throw([] {kline::capped_volume_schedule({1,2},4);},
               "impossible execution schedule rejected");
}

void inference_tests() {
    auto interval=kline::block_bootstrap_interval({1,1,1,1,1},2,50,42);
    close_to(interval.estimate,1,1e-12,"bootstrap sample mean");
    close_to(interval.lower,1,1e-12,"bootstrap lower");
    close_to(interval.upper,1,1e-12,"bootstrap upper");
    auto cusum=kline::cusum_positive({1,1,5,5},1,0.5);
    close_to(cusum[2],3.5,1e-12,"CUSUM surprise");
    check(kline::first_cusum_alarm({1,1,5,5},1,0.5,4)==3,
          "CUSUM alarm at fourth observation");
    auto holm=kline::holm_adjusted_p({0.01,0.02,0.5});
    close_to(holm[0],0.03,1e-12,"Holm first familywise adjustment");
    close_to(holm[1],0.04,1e-12,"Holm second adjustment");
    close_to(holm[2],0.5,1e-12,"Holm largest adjustment");
    check(kline::hac_mean_tstat({1,2,3,4},1)>0,
          "positive HAC mean statistic");
    must_throw([] {kline::first_cusum_alarm({1},1,0,0);},
               "CUSUM zero alarm threshold rejected");
}

void stochastic_tests() {
    auto lower=kline::cholesky_psd({{1,0.5},{0.5,1}});
    close_to(lower[0][0],1,1e-12,"Cholesky first pivot");
    close_to(lower[1][0],0.5,1e-12,"Cholesky covariance reconstruction");
    close_to(lower[1][0]*lower[1][0]+lower[1][1]*lower[1][1],
             1,1e-12,"Cholesky second variance");
    auto deterministic=kline::gaussian_correlated_paths(
        {100,50},{0.05,0.1},{{0,0},{0,0}},2,1,12);
    close_to(deterministic[1][0],100*exp(0.05),1e-10,"zero-vol first GBM");
    close_to(deterministic[2][1],50*exp(0.2),1e-10,"zero-vol second GBM");
    auto random=kline::gaussian_correlated_paths(
        {100,50},{0,0},{{0.04,0.01},{0.01,0.09}},4,0.2,12);
    auto repeat=kline::gaussian_correlated_paths(
        {100,50},{0,0},{{0.04,0.01},{0.01,0.09}},4,0.2,12);
    close_to(random[4][1],repeat[4][1],1e-12,"seed reproducibility");
    close_to(kline::exact_ou_step(3,1,log(2.0),0,1,0),2,1e-12,
             "exact OU conditional mean");
    must_throw([] {kline::cholesky_psd({{1,2},{2,1}});},
               "indefinite covariance rejected");
    must_throw([] {kline::gaussian_correlated_paths(
        {100},{0},{{0.1}},0,1,1);},"zero simulation length rejected");
}

int main() {
    try {
        numerical_tests();
        time_series_tests();
        indicators_tests();
        regression_tests();
        portfolio_tests();
        derivative_tests();
        curve_tests();
        execution_tests();
        inference_tests();
        stochastic_tests();
        cout<<"PASS: "<<assertions<<" advanced quantitative C++17 assertions\n";
        return 0;
    } catch(const exception& e) {
        cerr<<e.what()<<"\n";
        return 1;
    }
}
