#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline void validate_propensities(const std::vector<double>& treatment,
                                  const std::vector<double>& propensity) {
 check_finite(treatment);check_finite(propensity);
 require(treatment.size()==propensity.size()&&treatment.size()>=4,
         "Causal estimator needs aligned samples");
 int observed=0;
 for(std::size_t i=0;i<treatment.size();++i){
   require(treatment[i]==0||treatment[i]==1,
           "Treatment must be exactly binary");
   require(propensity[i]>0&&propensity[i]<1,
           "Propensities must be strictly between zero and one");
   observed+=int(treatment[i]);
 }
 require(observed>0&&observed<int(treatment.size()),
         "Both groups must be observed");
}
inline double horvitz_thompson_ate(
 const std::vector<double>& treatment,const std::vector<double>& outcome,
 const std::vector<double>& propensity) {
 validate_propensities(treatment,propensity);check_finite(outcome);
 require(outcome.size()==treatment.size(),"Outcome and treatment misaligned");
 double sum=0;
 for(std::size_t i=0;i<treatment.size();++i){
   const double d=treatment[i],p=propensity[i];
   sum+=d*outcome[i]/p-(1-d)*outcome[i]/(1-p);
 }
 return sum/treatment.size();
}
inline double augmented_ipw_ate(
 const std::vector<double>& treatment,const std::vector<double>& outcome,
 const std::vector<double>& propensity,const std::vector<double>& mu0,
 const std::vector<double>& mu1) {
 validate_propensities(treatment,propensity);
 check_finite(outcome);check_finite(mu0);check_finite(mu1);
 require(treatment.size()==outcome.size()&&treatment.size()==mu0.size()&&
         treatment.size()==mu1.size(),"AIPW requires aligned nuisance predictions");
 double sum=0;
 for(std::size_t i=0;i<treatment.size();++i){
   const double d=treatment[i],p=propensity[i];
   sum+=mu1[i]-mu0[i]+d*(outcome[i]-mu1[i])/p-
                    (1-d)*(outcome[i]-mu0[i])/(1-p);
 }
 return sum/treatment.size();
}
inline double standardized_mean_difference(
 const std::vector<double>& x,const std::vector<double>& treatment){
 check_finite(x);check_finite(treatment);
 require(x.size()==treatment.size(),"SMD dimensions differ");
 double n0=0,n1=0,m0=0,m1=0;
 for(std::size_t i=0;i<x.size();++i){
   require(treatment[i]==0||treatment[i]==1,"Invalid group label");
   if(treatment[i]==1){++n1;m1+=x[i];}
   else{++n0;m0+=x[i];}
 }
 require(n0>=2&&n1>=2,"SMD requires at least two per group");
 m0/=n0;m1/=n1;double v0=0,v1=0;
 for(std::size_t i=0;i<x.size();++i){
   if(treatment[i]==1)v1+=(x[i]-m1)*(x[i]-m1);
   else v0+=(x[i]-m0)*(x[i]-m0);
 }
 const double scale=std::sqrt((v0/(n0-1)+v1/(n1-1))/2);
 require(scale>0,"SMD requires nonzero pooled variance");
 return (m1-m0)/scale;
}
inline double ipw_effective_sample_size(
 const std::vector<double>& treatment,const std::vector<double>& propensity){
 validate_propensities(treatment,propensity);
 double sum=0,sq=0;
 for(std::size_t i=0;i<treatment.size();++i){
   const double weight=treatment[i]/propensity[i]+
                       (1-treatment[i])/(1-propensity[i]);
   sum+=weight;sq+=weight*weight;
 }
 return sum*sum/sq;
}
}
