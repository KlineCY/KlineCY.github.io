#include "cholesky_numerics.hpp"
#include "householder_qr.hpp"
#include "conjugate_gradient.hpp"
#include "natural_splines.hpp"
#include "root_solvers.hpp"
#include "adaptive_quadrature.hpp"
#include "numerical_derivatives.hpp"
#include "bfgs_optimizer.hpp"
#include "logistic_irls.hpp"
#include "multiclass_softmax.hpp"
#include "ols_hac_inference.hpp"
#include "time_series_tests.hpp"
#include "bootstrap_intervals.hpp"
#include "kalman_smoother.hpp"
#include "arima_forecasts.hpp"
#include "change_point_detection.hpp"
#include "kernel_ridge.hpp"
#include "gaussian_process.hpp"
#include "conformal_regression.hpp"
#include "platt_calibration.hpp"
#include "rolling_forecast_tests.hpp"
#include "feature_filtering.hpp"
#include "adam_linear.hpp"
#include "mlp_regression.hpp"
#include "bagged_stumps.hpp"
#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace std;
int checks=0;
void ensure(bool good,const char* label){checks++;if(!good)throw runtime_error(label);}
void close(double got,double expected,double tolerance,const char* label){
    ensure(isfinite(got)&&fabs(got-expected)<=tolerance,label);
}
void rejects(const function<void()>& job,const char* label){
    bool rejected=false;try{job();}catch(const exception&){rejected=true;}
    ensure(rejected,label);
}
void linear_algebra(){
    kline::Matrix a={{4,2},{2,3}};
    auto L=kline::cholesky_factor(a);
    close(L[0][0],2,1e-12,"Cholesky diagonal");
    close(L[1][0],1,1e-12,"Cholesky lower offdiagonal");
    close(L[1][1],sqrt(2),1e-12,"Cholesky lower diagonal");
    auto sol=kline::solve_cholesky(a,{6,5});
    close(sol[0],1,1e-12,"Cholesky system first");
    close(sol[1],1,1e-12,"Cholesky system second");
    close(kline::log_determinant_positive_definite(a),log(8),1e-12,
          "SPD log determinant");
    close(kline::multivariate_gaussian_log_density({0},{0},{{1}}),
          -0.5*log(2*acos(-1.0)),1e-12,"Gaussian zero log density");
    auto qr=kline::solve_householder_least_squares(
        {{1,0},{1,1},{1,2},{1,3}},{1,3,5,7});
    close(qr.coefficients[0],1,1e-10,"Householder intercept");
    close(qr.coefficients[1],2,1e-10,"Householder slope");
    close(qr.residual_sum_squares,0,1e-10,"Householder residual SSE");
    auto cg=kline::conjugate_gradient(a,{6,5});
    close(cg.x[0],1,1e-10,"Conjugate gradient first");
    close(cg.x[1],1,1e-10,"Conjugate gradient second");
    ensure(cg.iterations<=2,"CG exact finite dimension convergence");
    rejects([]{kline::cholesky_factor({{1,2},{2,1}});},
            "Indefinite Cholesky rejection");
    rejects([]{kline::solve_householder_least_squares(
        {{1,1},{1,1},{1,1}},{1,2,3});},
        "Rank deficient QR rejection");
    rejects([]{kline::conjugate_gradient({{1,2},{2,1}},{1,1});},
            "CG rejects non-SPD input");
}
void numerics(){
    auto spl=kline::fit_natural_spline({0,1,2},{1,3,5});
    close(kline::spline_value(spl,0.5),2,1e-12,"Linear spline first");
    close(kline::spline_value(spl,1.5),4,1e-12,"Linear spline second");
    rejects([&]{kline::spline_value(spl,3);},"Spline extrapolation forbidden");
    close(kline::bisect_root([](double x){return x*x-2;},0,2),
          sqrt(2),1e-10,"Bisection analytic root");
    close(kline::safeguarded_newton(
        [](double x){return x*x-2;},[](double x){return 2*x;},0,2),
        sqrt(2),1e-10,"Safeguarded Newton root");
    close(kline::adaptive_simpson([](double x){return x*x;},0,1),
          1.0/3,1e-10,"Simpson polynomial integral");
    auto objective=[](const vector<double>& v){
        return pow(v[0]-2,2)+3*pow(v[1]+1,2);
    };
    auto grad=kline::finite_difference_gradient(objective,{0,0});
    close(grad[0],-4,1e-7,"Numerical gradient first");
    close(grad[1],6,1e-7,"Numerical gradient second");
    auto h=kline::finite_difference_hessian(objective,{0,0});
    close(h[0][0],2,1e-5,"Hessian first");
    close(h[1][1],6,1e-5,"Hessian second");
    auto fit=kline::bfgs_minimize(objective,
        [](const vector<double>& x){
            return vector<double>{2*(x[0]-2),6*(x[1]+1)};
        },{0,0});
    close(fit.x[0],2,1e-5,"BFGS optimum x");
    close(fit.x[1],-1,1e-5,"BFGS optimum y");
    rejects([]{kline::bisect_root([](double x){return x*x+1;},0,2);},
            "Root bracket must change sign");
}
void statistics(){
    kline::Matrix samples={{-2},{-1},{0},{1},{2},{3}};
    auto lr=kline::fit_logistic_irls(samples,{0,0,0,1,1,1},0.1);
    ensure(kline::logistic_probability(lr,{2})>0.7,
           "Logistic IRLS positive probability");
    ensure(kline::logistic_probability(lr,{-2})<0.3,
           "Logistic IRLS negative probability");
    ensure(lr.log_loss>0&&isfinite(lr.log_loss),"IRLS finite penalized loss");
    auto soft=kline::fit_softmax({{-2},{-1},{0},{1},{2},{3}},
                         {0,0,1,1,2,2},3,0.08,600,0.01);
    auto probs=kline::softmax_probabilities(soft,{2.5});
    close(probs[0]+probs[1]+probs[2],1,1e-12,"Softmax probability simplex");
    ensure(probs[2]>probs[0],"Softmax ordered class information");
    auto ols=kline::householder_ols_intercept(
        {{0},{1},{2},{3},{4},{5}},{1,3,5,7,9,11});
    close(ols.coefficients[1],2,1e-10,"QR OLS slope");
    auto hac=kline::ols_hac_standard_errors(
        {{0},{1},{2},{3},{4},{5}},
        {0.1,-0.2,0.1,-0.2,0.1,-0.2},2);
    ensure(hac.size()==2&&hac[0]>0&&hac[1]>0,
           "Newey West standard errors finite");
    ensure(kline::ljung_box_q({1,2,3,4,5,6,7,8,9},2)>0,
           "Ljung Box positive serial correlation");
    ensure(kline::jarque_bera_statistic({1,2,3,4,5,6})>=0,
           "Jarque Bera nonnegative statistic");
    ensure(kline::engle_arch_lm({1,2,1,3,1,4,1,2,1,4,1,3,1,2,1,3,1},1)>=0,
           "ARCH LM nonnegative");
    auto interval=kline::block_bootstrap_mean_interval(
        {1,2,3,4,5,6,7,8,9,10},3,100,0.9,2026);
    close(interval.estimate,5.5,1e-12,"Bootstrap sample center");
    ensure(interval.lower<interval.upper,"Block bootstrap confidence width");
    auto filtered=kline::kalman_filter({1,2,3,4},{0,1},0.1,0.5);
    auto smoothed=kline::rts_random_walk_smoother({1,2,3,4},
                                                {0,1},0.1,0.5);
    close(smoothed.back().state,filtered.back().state,1e-12,
          "Kalman filtered final equals smoothed final");
    ensure(smoothed[0].state>=filtered[0].state,
           "RTS smoothing uses future measurements");
    vector<double> levels;
    for(int i=0;i<25;i++)levels.push_back(i+1);
    auto arima=kline::fit_arima_p1_0(levels,1);
    close(kline::forecast_arima_one_step(arima,levels),26,1e-7,
          "ARIMA integrated linear trend");
    auto break_detect=kline::best_single_mean_break(
        {0,0,0,0,0,3,3,3,3,3},3);
    ensure(break_detect.index==5,"Known single change point");
    auto break_vec=kline::binary_segmentation_mean(
        {0,0,0,0,0,3,3,3,3,3},1,3,0.1);
    ensure(break_vec==vector<int>({5}),"Binary segmentation break");
}
void ml(){
    kline::Matrix x={{-2},{-1},{0},{1},{2},{3}};
    vector<double> y={4,1,0,1,4,9};
    auto kr=kline::fit_kernel_ridge(x,y,0.01,1);
    close(kline::kernel_ridge_prediction(kr,{1}),1,0.2,
          "Kernel ridge prediction near training value");
    auto gp=kline::gaussian_process_predict(x,y,{1},1,0.1);
    ensure(gp.latent_variance>=0&&gp.latent_variance<1,
           "GP posterior uncertainty contracts");
    auto interval=kline::split_conformal_interval(
        {1,2,3,4,5,6,7,8,9,10},
        {2,3,4,5,6,7,8,9,10,11},5,0.1);
    close(interval.lower,4,1e-12,"Conformal lower limit");
    close(interval.upper,6,1e-12,"Conformal upper limit");
    close(kline::empirical_conformal_coverage({2,2},{1,1},{2,4}),
          0.5,1e-12,"Conformal realized coverage");
    auto platt=kline::fit_platt_scaling({-2,-1,0,1,2,3},
                                       {0,0,0,1,1,1},0.1);
    ensure(kline::platt_probability(platt,2)>
           kline::platt_probability(platt,-2),
           "Platt probability monotone");
    auto dm=kline::diebold_mariano_squared_error(
        {0,0,0,0,0,0},{1,2,3,4,5,6},{1,2,3,4,5,6},1);
    ensure(dm.mean_loss_difference>0&&dm.hac_se>0&&dm.t>0,
           "DM improves second prediction");
    rejects([]{kline::diebold_mariano_squared_error(
        {1,2,3,4},{1,2,3,4},{1,2,3,4},1);},
        "Identical forecast losses must reject zero HAC variance");
    auto best=kline::select_top_abs_correlation(
        {{1,3},{2,1},{3,3},{4,1},{5,3},{6,1}},
        {1,2,3,4,5,6},1);
    ensure(best==vector<int>({0}),"Strongest correlation selected");
    auto sub=kline::subset_feature_columns({{1,2},{3,4}},{1});
    close(sub[0][0],2,1e-12,"Selected feature column");
    auto adam=kline::fit_adam_linear(x,{-3,-1,1,3,5,7},700,0.03);
    close(kline::adam_linear_prediction(adam,{2}),5,0.05,
          "Adam linear fit");
    auto mlp=kline::fit_mlp_regression(
        {{-1},{0},{1},{2},{3}}, {2,2,2,2,2},4,300,0.02);
    close(kline::mlp_prediction(mlp,{1}),2,0.05,"MLP constant target");
    auto bagged=kline::fit_bagged_stumps(
        {{0},{1},{2},{3},{4},{5}},
        {0,0,0,10,10,10},25,42);
    ensure(kline::bagged_stumps_prediction(bagged,{5})>6,
           "Bagged stumps separate high values");
    rejects([]{kline::split_conformal_interval(
        {1,2,3},{1,2,3},0,0.01);},
        "Impossible finite sample conformal coverage");
}
int main(){
 try{linear_algebra();numerics();statistics();ml();
     cout<<"PASS: "<<checks<<" additional mathematical, statistical and ML checks\n";
 }catch(const exception& e){cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
