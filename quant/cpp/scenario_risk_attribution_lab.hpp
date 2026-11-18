#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <utility>
#include <vector>
namespace kline {
inline std::vector<double> euler_volatility_contributions(
 const std::vector<double>& weights,const std::vector<std::vector<double>>& covariance){
 check_finite(weights);
 const std::size_t n=weights.size();
 require(covariance.size()==n,"Euler risk matrix dimension mismatch");
 std::vector<double> marginal(n,0),result(n);
 for(std::size_t i=0;i<n;++i)require(covariance[i].size()==n,"Euler covariance not square");
 for(std::size_t i=0;i<n;++i){
  require(covariance[i].size()==n,"Euler covariance not square");
  for(std::size_t j=0;j<n;++j){
   require(std::isfinite(covariance[i][j])&&
           std::abs(covariance[i][j]-covariance[j][i])<1e-9,
           "Euler covariance must be symmetric");
   marginal[i]+=covariance[i][j]*weights[j];
  }
 }
 double var=std::inner_product(weights.begin(),weights.end(),marginal.begin(),0.);
 require(var>0&&std::isfinite(var),"Euler portfolio volatility must be positive");
 for(std::size_t i=0;i<n;++i)result[i]=weights[i]*marginal[i]/std::sqrt(var);
 return result;
}
inline std::vector<double> scenario_expected_shortfall_contributions(
 const std::vector<std::vector<double>>& assetLosses,
 const std::vector<double>& portfolioWeights,double confidence){
 check_finite(portfolioWeights);
 require(!assetLosses.empty()&&assetLosses.size()>=2&&
         std::isfinite(confidence)&&confidence>0&&confidence<1,
         "Expected shortfall requires valid scenarios and confidence");
 const std::size_t n=portfolioWeights.size();
 std::vector<std::pair<double,std::size_t>> rank;
 for(std::size_t t=0;t<assetLosses.size();++t){
  check_finite(assetLosses[t]);
  require(assetLosses[t].size()==n,"Scenario matrix and weight dimensions disagree");
  const double loss=std::inner_product(assetLosses[t].begin(),assetLosses[t].end(),
                                       portfolioWeights.begin(),0.);
  require(std::isfinite(loss),"Portfolio loss not finite");
  rank.push_back({loss,t});
 }
 std::sort(rank.begin(),rank.end(),[](const auto& a,const auto& b){
  return a.first>b.first;
 });
 double mass=(1-confidence)*rank.size(),remaining=mass;
 std::vector<double> contribution(n,0);
 for(const auto& [loss,index]:rank){
  (void)loss;
  const double w=std::min(1.0,remaining);if(w<=0)break;
  for(std::size_t j=0;j<n;++j)contribution[j]+=w*portfolioWeights[j]*assetLosses[index][j];
  remaining-=w;
 }
 for(double& x:contribution)x/=mass;
 return contribution;
}
inline double quadratic_factor_stress_pnl(
 const std::vector<double>& delta,const std::vector<double>& shocks,
 const std::vector<std::vector<double>>& gamma){
 check_finite(delta);check_finite(shocks);
 const std::size_t n=delta.size();
 require(shocks.size()==n&&gamma.size()==n,"Risk-factor dimensions disagree");
 double pnl=std::inner_product(delta.begin(),delta.end(),shocks.begin(),0.);
 for(std::size_t i=0;i<n;++i){
  require(gamma[i].size()==n,"Factor gamma matrix must be square");
  for(std::size_t j=0;j<n;++j){
   require(std::isfinite(gamma[i][j]),"Nonfinite second-order factor sensitivity");
   pnl+=0.5*shocks[i]*gamma[i][j]*shocks[j];
  }
 }
 require(std::isfinite(pnl),"Nonfinite quadratic stress approximation");
 return pnl;
}
inline double probability_weighted_scenario_loss(
 const std::vector<double>& scenarioLoss,const std::vector<double>& probabilities){
 check_finite(scenarioLoss);check_finite(probabilities);
 require(scenarioLoss.size()==probabilities.size(),"Scenario weights not aligned");
 double mass=0,expectation=0;
 for(std::size_t i=0;i<scenarioLoss.size();++i){
  require(probabilities[i]>=0&&probabilities[i]<=1,"Probability outside range");
  mass+=probabilities[i];expectation+=probabilities[i]*scenarioLoss[i];
 }
 require(std::abs(mass-1)<1e-8,"Scenario probabilities must sum to one");
 return expectation;
}
}
