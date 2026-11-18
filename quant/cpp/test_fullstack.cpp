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
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
int total=0;
void ok(bool condition,const string& name){
    total++;
    if(!condition)throw runtime_error("FAILED: "+name);
}
void near(double actual,double expected,double tolerance,const string& name){
    ok(isfinite(actual)&&fabs(actual-expected)<=tolerance,name);
}
void bad(const function<void()>& fn,const string& name){
    bool caught=false;
    try {fn();}catch(const exception&){caught=true;}
    ok(caught,name);
}

void optimization_suite(){
    auto b=kline::project_box({-1,2,9},{0,0,0},{1,1,1});
    near(b[0],0,1e-12,"box lower");
    near(b[1],1,1e-12,"box upper");
    near(b[2],1,1e-12,"box last");
    auto v=kline::project_l1_ball({3,-4},5);
    near(v[0],2,1e-12,"L1 ball first");
    near(v[1],-3,1e-12,"L1 ball second");
    near(kline::project_l1_ball({1,-1},3)[0],1,1e-12,"interior L1 ball");
    near(kline::project_l1_ball({1,-1},0)[1],0,1e-12,"zero L1 ball");
    auto q=kline::solve_box_quadratic({{2,0},{0,2}},{4,2},{0,0},{1,1});
    near(q[0],1,1e-10,"QP box constrained");
    near(q[1],1,1e-10,"QP second variable");
    near(kline::quadratic_objective({{2}},{4},{1}),-3,1e-12,"QP objective");
    near(kline::finite_difference_derivative({0,1,4},1,1),2,1e-12,"central derivative");
    bad([]{kline::project_l1_ball({1,2},-1);},"invalid L1 radius");
    bad([]{kline::project_box({1},{0},{-1});},"invalid box bounds");
}

void ols_learning_suite(){
    kline::Matrix x={{0,1},{1,0},{2,1},{3,0},{4,1},{5,0}};
    vector<double> y;
    for(const auto& row:x)y.push_back(1+2*row[0]-3*row[1]);
    auto ols=kline::ordinary_least_squares(x,y);
    near(ols.coefficients[0],1,1e-9,"QR intercept");
    near(ols.coefficients[1],2,1e-9,"QR slope");
    near(ols.coefficients[2],-3,1e-9,"QR control coefficient");
    near(ols.r_squared,1,1e-12,"QR goodness of fit");
    near(kline::ols_predict(ols,{6,1}),10,1e-9,"QR prediction");
    auto en=kline::fit_elastic_net({{0},{1},{2},{3},{4}},{1,3,5,7,9},0,0);
    near(kline::ridge_predict(en,{5}),11,1e-8,"elastic net zero penalty");
    auto sparse=kline::fit_elastic_net({{0},{1},{2},{3},{4}},{1,3,5,7,9},10,1);
    near(sparse.coefficients[0],0,1e-12,"elastic sparse factor");
    auto huber=kline::huber_weights({0,1,4},2);
    near(huber[2],0.5,1e-12,"robust Huber downweight");
    near(kline::huber_loss({0,1,4},2),(0+0.5+6)/3,1e-12,"Huber loss");
    near(kline::root_mean_squared_error({1,3},{2,4}),1,1e-12,"RMSE");
    near(kline::durbin_watson({1,0,-1}),1,1e-12,"Durbin Watson");
    bad([]{kline::ordinary_least_squares({{1},{1},{1}},{1,2,3});},
        "singular OLS rejected");
    bad([]{kline::fit_elastic_net({{1},{2}},{0,1},-1,0);},
        "negative regularization rejected");
}

void clustering_cointegration_suite(){
    auto result=kline::fit_kmeans({{0},{0.2},{10},{10.2}},2);
    ok(result.labels[0]==result.labels[1],"KMeans first cluster");
    ok(result.labels[2]==result.labels[3],"KMeans second cluster");
    ok(result.labels[0]!=result.labels[2],"KMeans clusters separated");
    near(result.inertia,0.04,1e-12,"KMeans objective");
    near(kline::distance_to_centers({0.1},{{0.1}})[0],0,1e-12,
         "distance to center");
    vector<double> hedge={1,2,3,4,5,6,7,8};
    vector<double> dep;
    for(int i=0;i<8;i++)dep.push_back(2*hedge[i]+
                                            (i%2==0?0.1:-0.1));
    auto spread=kline::two_asset_spread_regression(dep,hedge);
    near(spread.hedge_ratio,2-0.2/42,0.02,"hedge slope tolerance");
    near(kline::mean(spread.spread),0,1e-11,"spread residual centered");
    auto scores=kline::spread_zscores({0,1,0,-1,0,1,0,-1},3);
    ok(scores.size()==5,"spread zscore history");
    bad([]{kline::fit_kmeans({{1},{1}},2);},"identical cluster centers");
    bad([]{kline::unit_root_dickey_fuller_t({1,1,1,1,1,1});},
        "DF constant series rejected");
}

void states_factors_suite(){
    auto posterior=kline::kalman_update({0,1},2,0,1);
    near(posterior.state,1,1e-12,"Kalman posterior mean");
    near(posterior.variance,0.5,1e-12,"Kalman posterior variance");
    auto path=kline::kalman_filter({2,2},{0,1},0,1);
    near(path[1].state,4.0/3,1e-12,"Kalman repeated observation");
    auto state=kline::markov_filter_step({0.5,0.5},
                {{0.8,0.2},{0.2,0.8}},{3,1});
    near(state[0],0.75,1e-12,"Markov predictive Bayes");
    near(state[1],0.25,1e-12,"Markov normalization");
    bad([]{kline::markov_filter_step({0.5,0.5},
         {{0.8,0.1},{0.2,0.8}},{1,1});},"transition normalization");
    auto centered=kline::multi_factor_residuals({1,3,5,7,9},
                                                  {{0},{1},{2},{3},{4}});
    for(double e:centered)near(e,0,1e-10,"factor regression neutralization");
    auto ranks=kline::cross_sectional_rank_weights({1,2,3,4});
    near(accumulate(ranks.begin(),ranks.end(),0.0),0,1e-12,"rank net neutrality");
    double gross=0;for(double v:ranks)gross+=fabs(v);
    near(gross,1,1e-12,"rank gross exposure");
    auto buckets=kline::quantile_bucket_returns({1,2,3,4},{-2,-1,1,2},2);
    near(buckets[0],-1.5,1e-12,"lower quantile return");
    near(buckets[1],1.5,1e-12,"upper quantile return");
    near(kline::factor_monotonicity({1,2,3}),1,1e-12,"factor monotonicity");
    bad([]{kline::cross_sectional_rank_weights({1,1,1});},"constant rank factor");
}

void credit_microstructure_suite(){
    near(kline::survival_probability(0.1,0),1,1e-12,"survival at issue");
    near(kline::default_probability(log(2.0),1),0.5,1e-12,"default odds");
    near(kline::continuous_cds_spread(0.02,0.4),0.012,1e-12,"CDS spread");
    near(kline::implied_hazard_from_cds(0.012,0.4),0.02,1e-12,"CDS hazard");
    near(kline::expected_credit_loss(100,0.05,0.6),3,1e-12,"credit ECL");
    near(kline::risky_zero_coupon_price(100,1,0,log(2.0),0.4),
         70,1e-12,"recovery at maturity simplified price");
    near(kline::portfolio_expected_credit_loss({100,200},{0.01,0.02},
                                               {0.5,0.5}),2.5,1e-12,
         "portfolio ECL");
    near(kline::order_imbalance({10,-5,5}),0.5,1e-12,"trade volume imbalance");
    near(kline::kyle_lambda({2,4,6},{1,2,3}),2,1e-12,"Kyle signed volume slope");
    near(kline::realized_variance({0.1,-0.2}),0.05,1e-12,
         "realized quadratic variation");
    ok(kline::bipower_variation({0.1,-0.2})>0,"bipower variation");
    ok(kline::realized_jump_variation({0.1,-0.2})>=0,"nonnegative jump estimate");
    near(kline::roll_effective_spread({100,101,100,101,100}),
         2*sqrt(8.0/9),1e-12,
         "Roll spread for alternating trades");
    bad([]{kline::kyle_lambda({1,2,3},{1,1,1});},"constant order flow");
    bad([]{kline::implied_hazard_from_cds(0.01,1);},"full recovery hazard unidentifiable");
}

void hedging_stress_suite(){
    near(kline::delta_gamma_vega_pnl(0.5,0.02,2,2,0.01),
         1.06,1e-12,"option second-order pnl");
    near(kline::hedge_contract_count(1000000,1,100,100),-100,1e-12,
         "futures hedge contract size");
    near(kline::futures_variation_margin(5,100,99,101),1000,1e-12,
         "futures daily mark");
    near(kline::option_delta_hedged_pnl(10,12,-0.5,100,102,0.1),
         0.9,1e-12,"delta hedged option pnl");
    near(kline::put_call_synthetic_forward(5,3,100,1),102,1e-12,
         "put-call implied forward");
    near(kline::correlated_stress_sigma({{1,0},{0,4}},{1,1}),
         sqrt(5.0),1e-12,"covariance stress sigma");
    auto outcomes=kline::scenario_portfolio_pnl({1,-1},{{-0.1,0.2},{0.2,0}});
    near(outcomes[0],-0.3,1e-12,"stress portfolio loss");
    near(kline::worst_scenario_return({1,-1},{{-0.1,0.2},{0.2,0}}),
         -0.3,1e-12,"worst scenario");
    near(kline::volatility_target_gross(0.1,0.2,1,0.25,3),
         1.25,1e-12,"volatility targeting ramp limit");
    auto exposure=kline::normalize_gross({-1,1,2},2);
    double g=0;for(double w:exposure)g+=fabs(w);
    near(g,2,1e-12,"normalize gross");
    near(kline::expected_margin_buffer(100,2,0.2),60,1e-12,"margin buffer");
    bad([]{kline::scenario_portfolio_pnl({1,2},{{1}});},
        "stress scenario shape validation");
}

void liquidity_metrics_suite(){
    near(kline::amihud_illiquidity({0.1,-0.2},{100,200}),
         0.001,1e-12,"Amihud mean ratio");
    near(kline::expected_square_root_impact(0.04,0.2,0.5),
         0.02,1e-12,"sqrt liquidity impact");
    near(kline::required_participation(10,100),0.1,1e-12,"participation fraction");
    near(kline::shares_from_notional(1000,20),50,1e-12,"shares from notional");
    near(kline::net_alpha_after_cost(0.01,1,10,5,5),0.008,1e-12,
         "net expected alpha");
    near(kline::classification_auc({0.1,0.4,0.3,0.8},{0,0,1,1}),
         0.75,1e-12,"AUC with ranking");
    near(kline::binary_log_loss({0.5,0.5},{0,1}),log(2.0),1e-12,
         "binary log loss");
    auto c=kline::binary_confusion({0.9,0.1,0.8,0.2},{1,0,0,1});
    ok(c.tp==1&&c.fp==1&&c.tn==1&&c.fn==1,"confusion matrix");
    near(kline::f1_score(c),0.5,1e-12,"F1 harmonic mean");
    near(kline::out_of_sample_r_squared({1,2},{1,2},0),
         1,1e-12,"perfect OOS R2");
    near(kline::precision_at_k({0.1,0.9,0.4},{0,1,0},1),
         1,1e-12,"top ranked classification");
    bad([]{kline::classification_auc({0.1,0.9},{1,1});},
        "AUC single class rejected");
    bad([]{kline::binary_log_loss({-0.1,0.5},{0,1});},
        "negative probability rejected");
}

void resampling_fixedincome_suite(){
    auto circular=kline::circular_block_indices(10,4,42);
    ok(circular.size()==10,"circular resample length");
    for(int value:circular)ok(value>=0&&value<10,"circular index domain");
    auto stationary=kline::stationary_bootstrap_indices(10,3,42);
    ok(stationary.size()==10,"stationary resampling length");
    auto again=kline::stationary_bootstrap_indices(10,3,42);
    ok(stationary==again,"seed reproducibility");
    auto p=kline::permutation_p_value_correlation({1,2,3,4},{1,2,3,4},100,19);
    ok(p>0&&p<=1,"permutation p finite range");
    auto jack=kline::delete_one_jackknife_means({1,2,3});
    near(jack[0],2.5,1e-12,"jackknife first");
    near(kline::jackknife_standard_error(jack),sqrt(1.0/3),1e-12,
         "jackknife mean SE");
    near(kline::bootstrap_continuous_zero_rate(
       2,100*exp(-0.1),{}, {},100,{{1},{0.05}}),0.05,1e-12,
       "zero coupon yield bootstrap");
    near(kline::bond_price_from_yield({1,2},{5,105},0.05),
         5*exp(-0.05)+105*exp(-0.1),1e-12,"continuous bond price");
    auto bond_price=kline::bond_price_from_yield({1,2},{5,105},0.04);
    near(kline::yield_to_maturity_continuous(bond_price,{1,2},{5,105}),
         0.04,1e-10,"YTM inversion");
    auto hedges=kline::key_rate_hedge_contracts(
        {{1,0},{0,2}},{-2,-4});
    near(hedges[0],2,1e-12,"key rate hedge first");
    near(hedges[1],2,1e-12,"key rate hedge second");
    bad([]{kline::yield_to_maturity_continuous(100,{1},{-100});},
        "negative coupon flow disallowed");
}

void futures_fx_suite(){
    near(kline::held_futures_pnl(100,102,3,10),60,1e-12,"contract pnl");
    auto roll=kline::roll_futures_position(100,102,3,105,10,2);
    near(roll.close_pnl,60,1e-12,"realized old contract pnl");
    near(roll.entry_cost,12,1e-12,"commission for close and reopen");
    near(roll.next_contracts,3,1e-12,"positions preserved on roll");
    near(kline::futures_basis(105,100),0.05,1e-12,"future spot basis");
    near(kline::cash_and_carry_futures(100,0.05,0.01,0.02,1),
         100*exp(0.04),1e-12,"cost of carry");
    near(kline::fx_cross_rate(7,0.9),6.3,1e-12,"cross FX");
    near(kline::fx_inverse(2),0.5,1e-12,"FX inversion");
    near(kline::foreign_asset_return_in_base(0.1,7,8),
         1.1*8/7-1,1e-12,"foreign asset returns");
    near(kline::fx_forward_hedge_pnl(100,7.1,7),10,1e-12,"FX hedge forward");
    near(kline::forward_premium_annualized(7,7*exp(0.02),1),
         0.02,1e-12,"annualized FX basis");
    bad([]{kline::annualized_futures_basis(-1,100,1);},
        "negative futures basis log disallowed");
}

void quality_performance_suite(){
    double nan=numeric_limits<double>::quiet_NaN();
    auto quality=kline::inspect_series({nan,2,2,nan,3});
    ok(quality.missing==2&&quality.repeated_values==1,
       "quality missing and repeats");
    near(quality.coverage,0.6,1e-12,"valid coverage");
    auto filled=kline::forward_fill({nan,2,nan,nan,3},1);
    ok(isnan(filled[0])&&filled[2]==2&&isnan(filled[3]),
       "bounded forward fill");
    auto delayed=kline::lagged_signal({1,2,3,4},1);
    ok(delayed==vector<double>({1,2,3}),"lagged historical signal");
    near(kline::max_relative_jump({100,150,120}),0.5,1e-12,
         "relative gap screen");
    near(kline::omega_ratio({0.2,-0.1,0.1,-0.2}),1,1e-12,"Omega ratio");
    near(kline::ulcer_index({100,90,100}),sqrt(0.01/3),1e-12,
         "ulcer underwater rms");
    near(kline::profit_factor({3,-1,2,-1}),2.5,1e-12,"profit factor");
    near(kline::calmar_ratio({0.1,-0.1},2),-0.1,1e-12,
         "Calmar with annual returns");
    bad([]{kline::profit_factor({1,2});},"no losing trades");
}

void signal_making_suite(){
    auto trend=kline::time_series_trend({100,110,120,130},2,4);
    ok(trend.size()==4&&trend[3]>0,"positive EMA trend");
    auto pos=kline::lagged_position_from_threshold({2,2,0,0,-2,-2},1,0.5);
    ok(pos==vector<double>({0,1,1,0,0,-1}),"next-period causal position");
    auto pnl=kline::strategy_returns_from_lagged_positions({0,1,1},
                                                            {0.1,0.1,-0.1},0.01);
    near(pnl[1],0.09,1e-12,"net signal return");
    near(pnl[2],-0.1,1e-12,"unchanged position no extra costs");
    auto rolling=kline::rolling_zscore_past_only({0,1,2,3},2);
    near(rolling[0],3,1e-12,"past only z-score");
    near(kline::reservation_price(100,2,0.1,4,1),99.2,1e-12,
         "inventory penalized reservation");
    double spread=kline::optimal_market_making_spread(0.1,4,1,2);
    ok(spread>0,"market making positive optimal spread");
    double bid=kline::bid_inventory_quote(100,2,0.1,4,1,2);
    double ask=kline::ask_inventory_quote(100,2,0.1,4,1,2);
    near((bid+ask)/2,99.2,1e-12,"symmetric quote around reservation");
    near(kline::quoted_inventory_skew(bid,ask,100),-0.8,1e-12,
         "inventory quote skew");
    near(kline::total_implied_variance(0.2,2),0.08,1e-12,
         "term variance");
    near(kline::variance_forward_rate(1,0.2,2,0.2),0.04,1e-12,
         "flat forward variance");
    near(kline::call_vertical_spread(9,6),3,1e-12,"vertical spread");
    near(kline::butterfly_call_cost(12,9,7),1,1e-12,
         "nonnegative butterfly premium");
    ok(kline::nonnegative_equal_strike_butterfly(12,9,7),
       "valid butterfly");
    bad([]{kline::variance_forward_rate(1,0.4,2,0.2);},
        "variance calendar inversion");
}

void controls_validation_suite(){
    auto kept=kline::purge_label_intervals(
        {{0,3},{2,5},{5,7},{9,10},{12,14}}, {4,8},2);
    ok(kept==vector<int>({0,4}),"purged overlap and after-test embargo");
    ok(kline::eligible_training_count({2,3,5,7,8},6,1)==2,
       "matured label cut-off");
    ok(kline::chronological_holdout_indices(10,3,1)==vector<int>({7,8,9}),
       "holdout indices");
    auto gross=kline::enforce_gross_limit({2,-2},1);
    near(gross[0],0.5,1e-12,"gross constraint");
    auto neutral=kline::dollar_neutral_signal({1,2,3});
    near(accumulate(neutral.begin(),neutral.end(),0.0),0,1e-12,
         "dollar neutral");
    auto turn=kline::rebalance_with_turnover_cap({0.5,0.5},{1,0},0.1);
    near(turn[0],0.6,1e-12,"one-way turnover cap");
    ok(kline::satisfies_net_gross({0.5,-0.5},0,1),"net gross feasible");
    ok(!kline::satisfies_net_gross({1,0},0.1,2),"net exposure breach");
    bad([]{kline::purge_label_intervals({{1,1}},{2,3});},
        "invalid label interval");
}

int main(){
    try {
        optimization_suite();
        ols_learning_suite();
        clustering_cointegration_suite();
        states_factors_suite();
        credit_microstructure_suite();
        hedging_stress_suite();
        liquidity_metrics_suite();
        resampling_fixedincome_suite();
        futures_fx_suite();
        quality_performance_suite();
        signal_making_suite();
        controls_validation_suite();
        cout<<"PASS: "<<total<<" full-stack quantitative C++17 checks\n";
        return 0;
    } catch(const exception& e){
        cerr<<e.what()<<"\n";
        return 1;
    }
}
