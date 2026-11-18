#pragma once
#include "stats.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline double matthews_correlation(
 const std::vector<double>& actual,const std::vector<double>& predicted){
 check_finite(actual);check_finite(predicted);
 require(actual.size()==predicted.size(),"Classification labels must align");
 double tp=0,tn=0,fp=0,fn=0;
 for(std::size_t i=0;i<actual.size();++i){
  require((actual[i]==0||actual[i]==1)&&(predicted[i]==0||predicted[i]==1),
          "MCC requires binary classes");
  if(actual[i]==1&&predicted[i]==1)++tp;
  else if(actual[i]==0&&predicted[i]==0)++tn;
  else if(actual[i]==0)++fp;
  else ++fn;
 }
 const double scale=(tp+fp)*(tp+fn)*(tn+fp)*(tn+fn);
 require(scale>0,"MCC is undefined for degenerate class marginals");
 return (tp*tn-fp*fn)/std::sqrt(scale);
}
inline void audit_multiclass_inputs(
 const std::vector<double>& actual,const std::vector<double>& predicted,int classes){
 check_finite(actual);check_finite(predicted);
 require(actual.size()==predicted.size()&&classes>=2&&classes<=100,
         "Multiclass sample dimensions invalid");
 for(std::size_t i=0;i<actual.size();++i)
  require(actual[i]>=0&&actual[i]<classes&&std::floor(actual[i])==actual[i]&&
          predicted[i]>=0&&predicted[i]<classes&&std::floor(predicted[i])==predicted[i],
          "Class labels must be integral and in range");
}
inline double macro_f1_score(const std::vector<double>& actual,
                             const std::vector<double>& predicted,int classes){
 audit_multiclass_inputs(actual,predicted,classes);
 double score=0;
 for(int k=0;k<classes;++k){
  double tp=0,fp=0,fn=0;
  for(std::size_t i=0;i<actual.size();++i){
   if(actual[i]==k&&predicted[i]==k)++tp;
   else if(actual[i]!=k&&predicted[i]==k)++fp;
   else if(actual[i]==k)++fn;
  }
  const double denominator=2*tp+fp+fn;
  score+=denominator>0?2*tp/denominator:0;
 }
 return score/classes;
}
inline double balanced_multiclass_accuracy(
 const std::vector<double>& actual,const std::vector<double>& predicted,int classes){
 audit_multiclass_inputs(actual,predicted,classes);
 double score=0;
 for(int k=0;k<classes;++k){
  double correct=0,positive=0;
  for(std::size_t i=0;i<actual.size();++i)if(actual[i]==k){
   ++positive;if(predicted[i]==k)++correct;
  }
  require(positive>0,"Balanced accuracy requires observed support for each class");
  score+=correct/positive;
 }
 return score/classes;
}
inline double multiclass_brier_score(
 const std::vector<std::vector<double>>& probabilities,
 const std::vector<double>& actual){
 check_finite(actual);
 require(probabilities.size()==actual.size(),"Brier probabilities and labels differ");
 const std::size_t classes=probabilities.front().size();
 require(classes>=2&&classes<=100,"Invalid class dimension");
 double total=0;
 for(std::size_t i=0;i<actual.size();++i){
  require(std::floor(actual[i])==actual[i]&&actual[i]>=0&&actual[i]<classes&&
          probabilities[i].size()==classes,"Invalid Brier category");
  double psum=0;
  for(std::size_t k=0;k<classes;++k){
   const double p=probabilities[i][k];
   require(std::isfinite(p)&&p>=0&&p<=1,"Invalid forecast probability");
   psum+=p;
   total+=(p-(k==std::size_t(actual[i])?1.0:0.0))*
          (p-(k==std::size_t(actual[i])?1.0:0.0));
  }
  require(std::abs(psum-1)<1e-8,"Probability rows must sum to one");
 }
 return total/actual.size();
}
}
