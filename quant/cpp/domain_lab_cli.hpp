#pragma once
#include "high_frequency_inference_lab.hpp"
#include "causal_robustness_lab.hpp"
#include "market_quality_lab.hpp"
#include "fixed_income_sensitivity_lab.hpp"
#include "classification_audit_lab.hpp"
#include "scenario_risk_attribution_lab.hpp"
#include "vintage_data_audit_lab.hpp"
#include "options_surface_diagnostics_lab.hpp"
#include "io.hpp"
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
namespace kline {
inline int domain_int(const char* value,int lower=2) {
 const double d=parse_number(value);
 require(d>=lower&&d<=100&&std::floor(d)==d,"Invalid integer parameter");
 return int(d);
}
inline bool run_domain_lab_cli(const std::string& c,int argc,char** argv,
                               std::ostream& out){
 auto exact=[&](int n){require(argc==n,"Incorrect extended research command arity");};
 auto num=[&](int i){return parse_number(argv[i]);};
 if(c=="hy-cov"){
  exact(9);auto d=read_csv(argv[2]);
  out<<"HYCovariance "<<hayashi_yoshida_covariance(d.numbers(argv[3]),d.numbers(argv[4]),
     d.numbers(argv[5]),d.numbers(argv[6]),d.numbers(argv[7]),d.numbers(argv[8]))<<"\n";
 }else if(c=="realized-quarticity"||c=="kernel-1"){
  exact(4);auto x=read_csv(argv[2]).numbers(argv[3]);
  out<<(c=="realized-quarticity"?"Quarticity ":"RealizedKernel ")
     <<(c=="realized-quarticity"?realized_quarticity(x):one_lag_bartlett_realized_kernel(x))<<"\n";
 }else if(c=="truncated-rv"){
  exact(5);out<<"TruncatedRV "<<truncated_realized_variance(
   read_csv(argv[2]).numbers(argv[3]),num(4))<<"\n";
 }else if(c=="ht-ate"){
  exact(6);auto d=read_csv(argv[2]);
  out<<"ATE "<<horvitz_thompson_ate(d.numbers(argv[3]),d.numbers(argv[4]),
                                    d.numbers(argv[5]))<<"\n";
 }else if(c=="aipw-ate"){
  exact(8);auto d=read_csv(argv[2]);
  out<<"ATE "<<augmented_ipw_ate(d.numbers(argv[3]),d.numbers(argv[4]),
   d.numbers(argv[5]),d.numbers(argv[6]),d.numbers(argv[7]))<<"\n";
 }else if(c=="smd-covariate"){
  exact(5);auto d=read_csv(argv[2]);
  out<<"SMD "<<standardized_mean_difference(d.numbers(argv[3]),d.numbers(argv[4]))<<"\n";
 }else if(c=="ipw-ess"){
  exact(5);auto d=read_csv(argv[2]);
  out<<"EffectiveSampleSize "<<ipw_effective_sample_size(
   d.numbers(argv[3]),d.numbers(argv[4]))<<"\n";
 }else if(c=="quoted-spread"){
  exact(5);out<<"QuotedSpreadBps "<<quoted_spread_basis_points(
   read_csv(argv[2]).numbers(argv[3]),read_csv(argv[2]).numbers(argv[4]))<<"\n";
 }else if(c=="replenishment"){
  exact(5);auto d=read_csv(argv[2]);
  out<<"Replenishment "<<liquidity_replenishment_ratio(d.numbers(argv[3]),d.numbers(argv[4]))<<"\n";
 }else if(c=="kyle-lambda"){
  exact(5);auto d=read_csv(argv[2]);
  out<<"ImpactLambda "<<kyle_price_impact_slope(d.numbers(argv[3]),d.numbers(argv[4]))<<"\n";
 }else if(c=="sign-autocorr"){
  exact(4);out<<"OrderSignAutocorrelation "<<lag_one_order_sign_correlation(
   read_csv(argv[2]).numbers(argv[3]))<<"\n";
 }else if(c=="forward-zero"){
  exact(6);out<<"ForwardRate "<<continuously_compounded_forward_rate(
   num(2),num(3),num(4),num(5))<<"\n";
 }else if(c=="zero-dv01"){
  exact(5);out<<"DV01 "<<zero_coupon_dv01(num(2),num(3),num(4))<<"\n";
 }else if(c=="cash-duration"||c=="cash-convexity"){
  exact(6);auto d=read_csv(argv[2]);const auto t=d.numbers(argv[3]),flow=d.numbers(argv[4]);
  out<<(c=="cash-duration"?"MacaulayDuration ":"ContinuousConvexity ")
     <<(c=="cash-duration"?cashflow_macaulay_duration(t,flow,num(5)):
       cashflow_continuous_convexity(t,flow,num(5)))<<"\n";
 }else if(c=="mcc"){
  exact(5);auto d=read_csv(argv[2]);
  out<<"MCC "<<matthews_correlation(d.numbers(argv[3]),d.numbers(argv[4]))<<"\n";
 }else if(c=="macro-f1"||c=="balanced-acc"){
  exact(6);auto d=read_csv(argv[2]);
  const auto a=d.numbers(argv[3]),p=d.numbers(argv[4]);int k=domain_int(argv[5]);
  out<<(c=="macro-f1"?"MacroF1 ":"BalancedAccuracy ")
     <<(c=="macro-f1"?macro_f1_score(a,p,k):balanced_multiclass_accuracy(a,p,k))<<"\n";
 }else if(c=="multiclass-brier3"){
  exact(7);auto d=read_csv(argv[2]);
  auto a=d.numbers(argv[3]),b=d.numbers(argv[4]),p=d.numbers(argv[5]),y=d.numbers(argv[6]);
  std::vector<std::vector<double>> matrix;
  for(std::size_t i=0;i<y.size();++i)matrix.push_back({a[i],b[i],p[i]});
  out<<"MulticlassBrier "<<multiclass_brier_score(matrix,y)<<"\n";
 }else if(c=="euler-vol2"){
  exact(7);auto v=euler_volatility_contributions({num(2),num(3)},
                                               {{num(4),num(5)},{num(5),num(6)}});
  out<<"Asset1VolContribution "<<v[0]<<"\nAsset2VolContribution "<<v[1]<<"\n";
 }else if(c=="tail-es2"){
  exact(8);auto d=read_csv(argv[2]);
  auto a=d.numbers(argv[3]),b=d.numbers(argv[4]);
  std::vector<std::vector<double>> losses;
  for(std::size_t i=0;i<a.size();++i)losses.push_back({a[i],b[i]});
  auto v=scenario_expected_shortfall_contributions(losses,{num(5),num(6)},num(7));
  out<<"Asset1ESContribution "<<v[0]<<"\nAsset2ESContribution "<<v[1]<<"\n";
 }else if(c=="quad-factor-pnl2"){
  exact(9);out<<"QuadraticPnL "<<quadratic_factor_stress_pnl(
   {num(2),num(3)},{num(4),num(5)},
   {{num(6),num(7)},{num(7),num(8)}})<<"\n";
 }else if(c=="scenario-expectation"){
  exact(5);auto d=read_csv(argv[2]);
  out<<"ExpectedLoss "<<probability_weighted_scenario_loss(
   d.numbers(argv[3]),d.numbers(argv[4]))<<"\n";
 }else if(c=="vintage-rmse"){
  exact(5);auto d=read_csv(argv[2]);
  out<<"RevisionRMSE "<<vintage_revision_rmse(d.numbers(argv[3]),d.numbers(argv[4]))<<"\n";
 }else if(c=="publish-delay"){
  exact(5);auto d=read_csv(argv[2]);
  out<<"PublicationDelay "<<average_publication_delay(
   d.numbers(argv[3]),d.numbers(argv[4]))<<"\n";
 }else if(c=="quote-stale"){
  exact(5);out<<"StaleFraction "<<stale_quote_fraction(
   read_csv(argv[2]).numbers(argv[3]),num(4))<<"\n";
 }else if(c=="crossed-quotes"){
  exact(5);auto d=read_csv(argv[2]);
  out<<"InvalidQuoteFraction "<<invalid_bid_ask_fraction(
   d.numbers(argv[3]),d.numbers(argv[4]))<<"\n";
 }else if(c=="digital-call"||c=="vanna-call"||c=="volga-call"){
  exact(8);const double s=num(2),k=num(3),r=num(4),q=num(5),vol=num(6),t=num(7);
  out<<(c=="digital-call"?"DigitalCall ":(c=="vanna-call"?"Vanna ":"Volga "))
     <<(c=="digital-call"?black_scholes_cash_digital_call(s,k,r,q,vol,t):
       (c=="vanna-call"?black_scholes_vanna_call(s,k,r,q,vol,t):
                         black_scholes_volga_call(s,k,r,q,vol,t)))<<"\n";
 }else if(c=="forward-variance"){
  exact(6);out<<"ForwardVariance "<<annualized_forward_variance(
   num(2),num(3),num(4),num(5))<<"\n";
 }else return false;
 return true;
}
inline void domain_lab_usage(std::ostream& out){
 out<<"Mathematics and finance extended research laboratories:\n"
    <<"  hy-cov CSV START_A END_A RETURN_A START_B END_B RETURN_B\n"
    <<"  realized-quarticity|kernel-1 CSV LOG_RETURN\n"
    <<"  truncated-rv CSV LOG_RETURN THRESHOLD\n"
    <<"  ht-ate CSV TREATMENT OUTCOME PROPENSITY\n"
    <<"  aipw-ate CSV TREATMENT OUTCOME PROPENSITY MU0 MU1\n"
    <<"  smd-covariate CSV FEATURE TREATMENT\n"
    <<"  ipw-ess CSV TREATMENT PROPENSITY\n"
    <<"  quoted-spread CSV BID ASK\n"
    <<"  replenishment CSV EXECUTED REPLENISHED\n"
    <<"  kyle-lambda CSV SIGNED_VOLUME PRICE_CHANGE\n"
    <<"  sign-autocorr CSV SIGN\n"
    <<"  forward-zero DISCOUNT_START DISCOUNT_END START_YEARS END_YEARS\n"
    <<"  zero-dv01 FACE RATE MATURITY\n"
    <<"  cash-duration|cash-convexity CSV TIME CASHFLOW YIELD\n"
    <<"  mcc CSV ACTUAL PREDICTED\n"
    <<"  macro-f1|balanced-acc CSV ACTUAL PREDICTED CLASSES\n"
    <<"  multiclass-brier3 CSV P0 P1 P2 ACTUAL\n"
    <<"  euler-vol2 W1 W2 VAR1 COV VAR2\n"
    <<"  tail-es2 CSV LOSS1 LOSS2 W1 W2 CONFIDENCE\n"
    <<"  quad-factor-pnl2 DELTA1 DELTA2 SHOCK1 SHOCK2 G11 G12 G22\n"
    <<"  scenario-expectation CSV SCENARIO_LOSS PROBABILITY\n"
    <<"  vintage-rmse CSV INITIAL REVISED\n"
    <<"  publish-delay CSV OBSERVATION_TIMESTAMP RELEASE_TIMESTAMP\n"
    <<"  quote-stale CSV TIMESTAMP MAX_GAP\n"
    <<"  crossed-quotes CSV BID ASK\n"
    <<"  digital-call|vanna-call|volga-call SPOT STRIKE RATE DIVIDEND VOL YEARS\n"
    <<"  forward-variance NEAR_YEARS NEAR_VOL FAR_YEARS FAR_VOL\n";
}
}
