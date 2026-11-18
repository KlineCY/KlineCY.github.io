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
#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>
using namespace std;
int checks=0;
void check(bool ok,const char* reason){checks++;if(!ok)throw runtime_error(reason);}
void approx(double x,double y,double tolerance,const char* reason){
    check(isfinite(x)&&fabs(x-y)<=tolerance,reason);
}
void rejected(const function<void()>& f,const char* reason){
    bool threw=false;try{f();}catch(const exception&){threw=true;}
    check(threw,reason);
}
int main(){
  try{
    auto correlation=kline::partial_correlation_matrix({{1,0.5},{0.5,1}},0);
    approx(correlation[0][0],1,1e-12,"Partial correlation diagonal");
    approx(correlation[0][1],0.5,1e-12,"Partial correlation off-diagonal");
    auto precision=kline::precision_from_covariance({{4,0},{0,9}},0);
    approx(precision[0][0],0.25,1e-12,"Precision inverse first");
    approx(precision[1][1],1.0/9,1e-12,"Precision inverse second");
    rejected([]{kline::partial_correlation_matrix({{1,2},{2,1}},0);},
             "Indefinite inverse covariance rejected");
    auto smoothed=kline::second_difference_trend({1,2,3,4,5},300);
    for(int i=0;i<5;i++)
        approx(smoothed[i],i+1,1e-8,"Linear trend invariant to smoothing");
    approx(kline::trend_roughness(smoothed),0,1e-9,
           "Linear trend roughness vanishes");
    auto raw=kline::second_difference_trend({1,2,8,4,5},0);
    approx(raw[2],8,1e-12,"Zero penalty leaves data unchanged");
    auto huber=kline::huber_regression(
       {{-2},{-1},{0},{1},{2}}, {-2,-1,0,1,20},1,100);
    auto ols=kline::householder_ols_intercept(
       {{-2},{-1},{0},{1},{2}}, {-2,-1,0,1,20});
    check(huber.coefficients[1]<ols.coefficients[1],
          "Huber outlier influence below OLS");
    approx(kline::empirical_cdf({1,2,2,3},2),0.75,1e-12,
           "Empirical CDF inclusive ties");
    approx(kline::two_sample_ks_statistic({1,2,3},{1,2,3}),0,1e-12,
           "Two sample KS identical samples");
    approx(kline::two_sample_ks_statistic({1,2,3},{8,9,10}),1,1e-12,
           "Two sample KS separated samples");
    approx(kline::one_sample_ks_statistic({1,2,3},{1.0/3,2.0/3,1}),
           1.0/3,1e-12,"Single sample KS CDF boundaries");
    auto perm=kline::permutation_mean_difference(
       {10,11,12,13},{0,1,2,3},400,17);
    approx(perm.observed,10,1e-12,"Permutation treatment mean contrast");
    check(perm.p_value<0.05,"Permutation assignment separates groups");
    check(perm.p_value>0,"Permutation plus-one probability positive");
    auto indices=kline::stationary_bootstrap_indices(25,3,44);
    check(indices.size()==25,"Stationary bootstrap sample length");
    for(int id:indices)check(id>=0&&id<25,"Stationary bootstrap valid index");
    auto series=kline::stationary_bootstrap_sample({1,2,3,4,5},2,13);
    check(series.size()==5,"Stationary bootstrap values size");
    auto acf=kline::autocorrelations({1,2,3,4,5,6,7,8,9,10},2);
    check(acf[1]>0&&acf[1]<1,"ACF positive bounded lag");
    auto pacf=kline::partial_autocorrelations(
        {1,2,3,4,5,6,7,8,9,10},2);
    approx(pacf[0],1,1e-12,"PACF zero lag one");
    approx(pacf[1],acf[1],1e-12,"PACF first lag equals ACF");
    std::vector<double> change={0,0,0,0,0,0,0,0,0,0,
       3,3,3,3,3,3,3,3,3,3};
    auto signal=kline::page_hinkley_upper(change,0.01,1);
    check(signal.size()==change.size(),"Page-Hinkley outputs every observation");
    check(!signal[0].alarm,"No immediate detection when no change");
    bool alerted=false;
    for(int i=10;i<int(signal.size());i++)alerted|=signal[i].alarm;
    check(alerted,"Page Hinkley detects positive shift");
    auto compounded=kline::time_aggregated_nonoverlap_returns(
         {0.1,0.1,-0.1,0.1},2);
    approx(compounded[0],0.21,1e-12,"Compounded first window");
    approx(compounded[1],-0.01,1e-12,"Compounded second window");
    auto bpv=kline::rolling_bipower_variation(
         {0.1,-0.1,0.1,-0.1,0.1},3);
    check(bpv.size()==3&&bpv[0]>0,"Rolling bipower positive");
    double dc=kline::biased_distance_covariance_squared(
        {1,2,3,4},{2,4,6,8});
    check(dc>0,"Distance covariance detects dependence");
    approx(dc,kline::biased_distance_covariance_squared(
        {2,4,6,8},{1,2,3,4}),1e-12,
        "Distance covariance symmetric in samples");
    approx(kline::weighted_empirical_quantile(
        {1,2,10},{1,1,8},0.5),10,1e-12,"Weighted quantile median");
    approx(kline::weighted_empirical_tail_mean(
        {1,2,10},{1,1,8},0.5),10,1e-12,"Weighted CVaR tail");
    rejected([]{kline::weighted_empirical_quantile(
        {1,2},{0,0},0.5);},"Weighted distribution zero mass rejected");
    rejected([]{kline::stationary_bootstrap_indices(10,0,1);},
             "Bootstrap negative expected length rejected");
    rejected([]{kline::second_difference_trend({1,2},1);},
             "Trend too short rejected");
    rejected([]{kline::time_aggregated_nonoverlap_returns(
        {-1.2,0.1},2);},"Compounding below total loss rejected");
    cout<<"PASS: "<<checks<<" new statistical mathematics numerical and safety checks\n";
  }catch(const exception& e){cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
