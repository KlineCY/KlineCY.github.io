#include "linear_svm_sgd.hpp"
#include "quantile_linear_regression.hpp"
#include "expectile_regression.hpp"
#include "ftrl_logistic.hpp"
#include "probability_discrimination.hpp"
#include "data_drift_psi.hpp"
#include "kernel_mmd.hpp"
#include "permutation_importance.hpp"
#include "cross_fitted_residuals.hpp"
#include "rolling_origin_validation.hpp"
#include "isotonic_intervals.hpp"
#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>
using namespace std;
int checks=0;
void check(bool x,const char* message){checks++;if(!x)throw runtime_error(message);}
void near(double value,double expected,double tol,const char* message){
    check(isfinite(value)&&fabs(value-expected)<=tol,message);
}
void fails(const function<void()>& fn,const char* message){
    bool success=false;try{fn();}catch(const exception&){success=true;}
    check(success,message);
}
int main(){
 try{
    kline::Matrix x={{-2},{-1},{0},{1},{2},{3}};
    auto svm=kline::fit_linear_svm_sgd(
        x,{-1,-1,-1,1,1,1},0.01,300,0.15);
    check(kline::svm_decision_score(svm,{2})>0,
          "Linear primal SVM positive class");
    check(kline::svm_decision_score(svm,{-2})<0,
          "Linear primal SVM negative class");
    auto quantile=kline::fit_quantile_linear(
        x,{-3,-1,1,3,5,7},0.5,0.0001,5000,0.3);
    check(quantile.pinball>=0&&quantile.pinball<0.1,
          "Linear median pinball loss small");
    auto expectile=kline::fit_linear_expectile(
        x,{-3,-1,1,3,5,7},0.5,100);
    near(expectile.coefficients[0],1,1e-9,"Median expectile intercept");
    near(expectile.coefficients[1],2,1e-9,"Median expectile slope");
    auto state=kline::new_ftrl_logistic(1,0.15,1,0.01,0.01);
    for(int epoch=0;epoch<150;epoch++){
        for(int i=0;i<int(x.size());i++)
            kline::ftrl_online_update(state,x[i],i>=3?1:0);
    }
    check(kline::ftrl_predict(state,{2})>0.5,
          "FTRL online positive class");
    check(kline::ftrl_predict(state,{-2})<0.5,
          "FTRL online negative class");
    near(kline::binary_auc_mann_whitney(
        {0.1,0.4,0.6,0.9},{0,0,1,1}),1,1e-12,
        "ROC AUC rank-based perfect classification");
    near(kline::binary_auc_mann_whitney(
        {0.5,0.5},{0,1}),0.5,1e-12,"Tied AUC mid-rank");
    near(kline::binary_brier_score({0,1},{0,1}),0,1e-12,
         "Brier score perfect probabilities");
    check(kline::binary_logarithmic_score(
        {0.01,0.99},{0,1})<0.02,"Binary log score near perfect");
    near(kline::population_stability_index(
        {0,1,2,3,4,5,6,7},{0,1,2,3,4,5,6,7},4),0,1e-12,
        "Identical population PSI zero");
    check(kline::population_stability_index(
        {0,1,2,3,4,5,6,7},{10,11,12,13,14,15,16,17},4)>0,
        "Shifted out of domain population PSI positive");
    kline::Matrix a={{-1},{0},{1},{2}};
    kline::Matrix b={{9},{10},{11},{12}};
    check(kline::squared_mmd_rbf(a,b,1)>0.5,
          "RBF MMD detects disjoint distributions");
    auto importance=kline::permutation_importance_mse(
        x,{-3,-1,1,3,5,7},
        [](const vector<double>& row){return 1+2*row[0];},0,88,30);
    check(importance>0.1,"Permutation OOS importance positive");
    auto residuals=kline::contiguous_crossfit_ridge(
        x,{-3,-1,1,3,5,7},3,0.001);
    check(residuals.model_residual.size()==x.size(),
          "Cross-fitting produces all out-of-fold predictions");
    double absolute=0;
    for(double error:residuals.model_residual)absolute+=fabs(error);
    check(absolute<0.2,"Cross-fitted linear model recovers trend");
    auto splits=kline::rolling_origin_splits(30,10,3,4,2,12);
    check(splits.size()==4,"Rolling origin split enumeration");
    for(auto fold:splits){
        check(fold.train_end+2==fold.test_start,
              "Rolling origin has embargo before forecast");
        check(fold.train_end-fold.train_start<=12,
              "Rolling origin caps training history");
    }
    kline::Matrix xx;vector<double> yy;
    for(int i=0;i<30;i++){
        xx.push_back({double(i)/20});
        yy.push_back(1+2.0*i/20);
    }
    auto errors=kline::rolling_origin_ridge_errors(xx,yy,splits,0.001);
    check(errors.size()==12,"Rolling origin validation count");
    auto iso=kline::monotone_quantile_predictions(
        {1,2,3,4},{0,3,1,4},{1,2,3,4});
    check(iso.size()==4,"Isotonic prediction count");
    near(iso[1],2,1e-12,"Isotonic PAV block pooled");
    near(kline::monotonicity_violation({1,2,3},{3,2,1}),
         1,1e-12,"Monotonicity local inversion");
    fails([]{kline::binary_auc_mann_whitney({0.1,0.2},{0,0});},
          "AUC single class rejected");
    fails([]{kline::new_ftrl_logistic(0,0.1,1,0,0);},
          "Invalid FTRL feature count rejected");
    fails([]{kline::rolling_origin_splits(10,9,2,1);},
          "Rolling origin without test space rejected");
    fails([]{kline::squared_mmd_rbf({{1}},{{2}},1);},
          "MMD needs enough samples");
    cout<<"PASS: "<<checks<<" new financial ML and validation assertions\n";
 }catch(const exception& e){cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
