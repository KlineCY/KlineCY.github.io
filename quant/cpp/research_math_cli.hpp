#pragma once
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
#include "io.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
namespace kline {
inline void research_csv_series(std::ostream& out,const std::string& name,
                                const std::vector<double>& values){
    out<<"Index,"<<name<<"\n";
    for(int i=0;i<int(values.size());i++)
        out<<i<<","<<values[i]<<"\n";
}
inline Matrix research_single_predictor(const std::vector<double>& values){
    Matrix x;for(double v:values)x.push_back({v});return x;
}
inline bool run_research_math_cli(const std::string& cmd,int argc,char** argv,
                                   std::ostream& out){
    auto exact=[&](int n){require(argc==n,"Incorrect research math arguments");};
    auto num=[&](int i){return parse_number(argv[i]);};
    auto matrix2=[&](int i){
        return Matrix{{num(i),num(i+1)},{num(i+1),num(i+2)}};
    };
    if(cmd=="spd-solve"||cmd=="cg-solve"){
        exact(7);
        auto a=matrix2(2);
        std::vector<double> b={num(5),num(6)};
        auto x=cmd=="spd-solve"?solve_cholesky(a,b):
                                  conjugate_gradient(a,b).x;
        research_csv_series(out,"Solution",x);
    }else if(cmd=="spd-logdet"){
        exact(5);
        out<<"LogDet "<<log_determinant_positive_definite(matrix2(2))<<"\n";
    }else if(cmd=="normal-logpdf2"){
        exact(10);
        out<<"LogDensity "<<multivariate_gaussian_log_density(
            {num(2),num(3)},{num(4),num(5)},
            {{num(6),num(7)},{num(7),num(8)}})<<"\n";
    }else if(cmd=="qr-ols"){
        require(argc>=5,"QR OLS needs CSV, response and features");
        auto table=read_csv(argv[2]);Matrix x(table.rows.size());
        for(int j=4;j<argc;j++){
            auto column=table.numbers(argv[j]);
            for(int t=0;t<int(column.size());t++)
                x[t].push_back(column[t]);
        }
        auto fit=householder_ols_intercept(x,table.numbers(argv[3]));
        research_csv_series(out,"Coefficient",fit.coefficients);
        out<<"ResidualSSE "<<fit.residual_sum_squares<<"\n";
    }else if(cmd=="natural-spline"){
        exact(6);auto t=read_csv(argv[2]);
        out<<"Interpolated "<<spline_value(
            fit_natural_spline(t.numbers(argv[3]),t.numbers(argv[4])),num(5))
           <<"\n";
    }else if(cmd=="root-bisect"||cmd=="root-newton"){
        exact(5);
        double target=num(2),left=num(3),right=num(4);
        auto f=[&](double x){return x*x-target;};
        auto df=[](double x){return 2*x;};
        out<<"Root "<<(cmd=="root-bisect"?
            bisect_root(f,left,right):safeguarded_newton(f,df,left,right))<<"\n";
    }else if(cmd=="integrate-square"){
        exact(4);
        out<<"Integral "<<adaptive_simpson([](double v){return v*v;},
                                            num(2),num(3))<<"\n";
    }else if(cmd=="integrate-normal-density"){
        exact(4);
        out<<"Probability "<<adaptive_simpson([](double v){
            return std::exp(-0.5*v*v)/std::sqrt(2*std::acos(-1.0));
        },num(2),num(3))<<"\n";
    }else if(cmd=="quad-gradient"){
        exact(6);
        auto f=[&](const std::vector<double>& v){
            return num(4)*v[0]*v[0]+num(5)*v[1]*v[1];
        };
        research_csv_series(out,"Gradient",finite_difference_gradient(f,
                           {num(2),num(3)}));
    }else if(cmd=="quad-hessian"){
        exact(4);
        auto f=[&](const std::vector<double>& v){
            return num(2)*v[0]*v[0]+num(3)*v[1]*v[1];
        };
        Matrix h=finite_difference_hessian(f,{0,0});
        out<<"H11 "<<h[0][0]<<"\nH22 "<<h[1][1]<<"\n";
    }else if(cmd=="bfgs-quadratic"){
        exact(6);
        auto f=[&](const std::vector<double>& v){
            return std::pow(v[0]-num(4),2)+
                   std::pow(v[1]-num(5),2);
        };
        auto g=[&](const std::vector<double>& v){
            return std::vector<double>{2*(v[0]-num(4)),2*(v[1]-num(5))};
        };
        auto fit=bfgs_minimize(f,g,{num(2),num(3)});
        out<<"X "<<fit.x[0]<<"\nY "<<fit.x[1]<<"\nObjective "<<fit.objective<<"\n";
    }else if(cmd=="logit-irls"||cmd=="platt-calibrate"){
        exact(7);auto table=read_csv(argv[2]);
        auto feature=table.numbers(argv[3]),raw=table.numbers(argv[4]);
        std::vector<int> labels;
        for(double y:raw){
            require(y==0||y==1,"Binary outcome required");
            labels.push_back(int(y));
        }
        double result=cmd=="logit-irls"?
            logistic_probability(fit_logistic_irls(research_single_predictor(
                                 feature),labels,num(6)),{num(5)}):
            platt_probability(fit_platt_scaling(feature,labels,num(6)),num(5));
        out<<"Probability "<<result<<"\n";
    }else if(cmd=="softmax-train"){
        exact(8);
        auto table=read_csv(argv[2]);
        auto feature=table.numbers(argv[3]),raw=table.numbers(argv[4]);
        std::vector<int> labels;
        for(double y:raw){
            require(y>=0&&y==std::floor(y)&&y<=100,
                    "Multiclass outcome must be nonnegative integer");
            labels.push_back(int(y));
        }
        auto model=fit_softmax(research_single_predictor(feature),labels,
                  std::stoi(argv[5]),0.05,std::stoi(argv[7]));
        research_csv_series(out,"Probability",softmax_probabilities(model,{num(6)}));
    }else if(cmd=="ols-hac"){
        exact(6);auto t=read_csv(argv[2]);
        auto x=research_single_predictor(t.numbers(argv[4]));
        auto fit=householder_ols_intercept(x,t.numbers(argv[3]));
        std::vector<double> residual;
        auto response=t.numbers(argv[3]);
        for(int i=0;i<int(x.size());i++)
            residual.push_back(response[i]-(fit.coefficients[0]+
                                             fit.coefficients[1]*x[i][0]));
        research_csv_series(out,"HACStandardError",
                           ols_hac_standard_errors(x,residual,std::stoi(argv[5])));
    }else if(cmd=="ljung-box"||cmd=="arch-lm"){
        exact(5);
        auto x=read_csv(argv[2]).numbers(argv[3]);
        out<<(cmd=="ljung-box"?"LjungBox ":"ARCH_LM ")
           <<(cmd=="ljung-box"?ljung_box_q(x,std::stoi(argv[4])):
                               engle_arch_lm(x,std::stoi(argv[4])))<<"\n";
    }else if(cmd=="jarque-bera"){
        exact(4);
        out<<"JarqueBera "<<jarque_bera_statistic(
            read_csv(argv[2]).numbers(argv[3]))<<"\n";
    }else if(cmd=="block-bootstrap-ci"){
        exact(8);
        auto fit=block_bootstrap_mean_interval(
            read_csv(argv[2]).numbers(argv[3]),
            std::stoi(argv[4]),std::stoi(argv[5]),num(6),
            std::stoul(argv[7]));
        out<<"Mean "<<fit.estimate<<"\nLower "<<fit.lower<<"\nUpper "<<fit.upper<<"\n";
    }else if(cmd=="rts-smooth"){
        exact(8);
        auto result=rts_random_walk_smoother(
            read_csv(argv[2]).numbers(argv[3]),
            {num(4),num(5)},num(6),num(7));
        out<<"Index,State,Variance\n";
        for(int i=0;i<int(result.size());i++)
            out<<i<<","<<result[i].state<<","<<result[i].variance<<"\n";
    }else if(cmd=="arima-one"){
        exact(5);
        auto values=read_csv(argv[2]).numbers(argv[3]);
        out<<"Forecast "<<forecast_arima_one_step(
            fit_arima_p1_0(values,std::stoi(argv[4])),values)<<"\n";
    }else if(cmd=="mean-break"){
        exact(5);
        auto result=best_single_mean_break(
            read_csv(argv[2]).numbers(argv[3]),std::stoi(argv[4]));
        out<<"Index "<<result.index<<"\nSSEImprovement "<<result.improvement<<"\n";
    }else if(cmd=="multi-break"){
        exact(7);
        auto breaks=binary_segmentation_mean(
            read_csv(argv[2]).numbers(argv[3]),std::stoi(argv[4]),
            std::stoi(argv[5]),num(6));
        out<<"Index,BreakIndex\n";
        for(int i=0;i<int(breaks.size());i++)
            out<<i<<","<<breaks[i]<<"\n";
    }else if(cmd=="kernel-predict"||cmd=="gp-predict"){
        exact(8);auto t=read_csv(argv[2]);
        auto x=research_single_predictor(t.numbers(argv[3]));
        auto y=t.numbers(argv[4]);
        if(cmd=="kernel-predict")
            out<<"Prediction "<<kernel_ridge_prediction(
                fit_kernel_ridge(x,y,num(6),num(7)),{num(5)})<<"\n";
        else{
            auto gp=gaussian_process_predict(x,y,{num(5)},num(6),num(7));
            out<<"Mean "<<gp.mean<<"\nVariance "<<gp.latent_variance<<"\n";
        }
    }else if(cmd=="conformal-interval"){
        exact(7);auto t=read_csv(argv[2]);
        auto bound=split_conformal_interval(t.numbers(argv[3]),
               t.numbers(argv[4]),num(5),num(6));
        out<<"Lower "<<bound.lower<<"\nUpper "<<bound.upper<<"\n"
           <<"Radius "<<bound.radius<<"\n";
    }else if(cmd=="conformal-coverage"){
        exact(6);auto t=read_csv(argv[2]);
        out<<"Coverage "<<empirical_conformal_coverage(
            t.numbers(argv[3]),t.numbers(argv[4]),
            t.numbers(argv[5]))<<"\n";
    }else if(cmd=="forecast-dm"){
        exact(7);auto t=read_csv(argv[2]);
        auto compare=diebold_mariano_squared_error(
            t.numbers(argv[3]),t.numbers(argv[4]),
            t.numbers(argv[5]),std::stoi(argv[6]));
        out<<"LossDifference "<<compare.mean_loss_difference<<"\n"
           <<"HAC_SE "<<compare.hac_se<<"\nT "<<compare.t<<"\n";
    }else if(cmd=="feature-filter"){
        require(argc>=7,"Feature selection needs CSV, target, K and features");
        auto t=read_csv(argv[2]);Matrix x(t.rows.size());
        for(int j=5;j<argc;j++){
            auto column=t.numbers(argv[j]);
            for(int i=0;i<int(column.size());i++)x[i].push_back(column[i]);
        }
        auto selected=select_top_abs_correlation(x,t.numbers(argv[3]),
                                                std::stoi(argv[4]));
        out<<"Rank,Feature\n";
        for(int i=0;i<int(selected.size());i++)
            out<<i<<","<<argv[5+selected[i]]<<"\n";
    }else if(cmd=="adam-predict"){
        exact(8);auto t=read_csv(argv[2]);
        auto fit=fit_adam_linear(
            research_single_predictor(t.numbers(argv[3])),
            t.numbers(argv[4]),std::stoi(argv[6]),num(7));
        out<<"Prediction "<<adam_linear_prediction(fit,{num(5)})<<"\n"
           <<"TrainingMSE "<<fit.training_mse<<"\n";
    }else if(cmd=="mlp-predict"){
        exact(9);auto t=read_csv(argv[2]);
        auto fit=fit_mlp_regression(
            research_single_predictor(t.numbers(argv[3])),
            t.numbers(argv[4]),std::stoi(argv[6]),
            std::stoi(argv[7]),num(8));
        out<<"Prediction "<<mlp_prediction(fit,{num(5)})<<"\n"
           <<"TrainingMSE "<<fit.train_mse<<"\n";
    }else if(cmd=="bagged-predict"){
        exact(8);auto t=read_csv(argv[2]);
        auto fit=fit_bagged_stumps(
            research_single_predictor(t.numbers(argv[3])),
            t.numbers(argv[4]),std::stoi(argv[6]),std::stoul(argv[7]));
        out<<"Prediction "<<bagged_stumps_prediction(fit,{num(5)})<<"\n";
    }else return false;
    return true;
}
inline void research_math_cli_usage(std::ostream& out){
    out<<"Advanced mathematical and ML research:\n"
       <<"  spd-solve|cg-solve A11 A12 A22 B1 B2\n"
       <<"  spd-logdet A11 A12 A22\n"
       <<"  normal-logpdf2 X1 X2 MU1 MU2 C11 C12 C22\n"
       <<"  qr-ols CSV TARGET FEATURE...\n"
       <<"  natural-spline CSV KNOT VALUE QUERY\n"
       <<"  root-bisect|root-newton TARGET LEFT RIGHT\n"
       <<"  integrate-square|integrate-normal-density LEFT RIGHT\n"
       <<"  quad-gradient X Y A B | quad-hessian A B\n"
       <<"  bfgs-quadratic START_X START_Y TARGET_X TARGET_Y\n"
       <<"  logit-irls|platt-calibrate CSV FEATURE LABEL QUERY RIDGE\n"
       <<"  softmax-train CSV FEATURE LABEL CLASSES QUERY EPOCHS\n"
       <<"  ols-hac CSV TARGET FEATURE LAGS\n"
       <<"  ljung-box|arch-lm CSV SERIES LAGS\n"
       <<"  jarque-bera CSV SERIES\n"
       <<"  block-bootstrap-ci CSV SERIES BLOCK REPLICATES CONFIDENCE SEED\n"
       <<"  rts-smooth CSV OBS INITIAL_MEAN INITIAL_VAR PROCESS_VAR OBS_VAR\n"
       <<"  arima-one CSV SERIES LAGS\n"
       <<"  mean-break CSV SERIES MIN_LENGTH\n"
       <<"  multi-break CSV SERIES MAXIMUM MIN_LENGTH PENALTY\n"
       <<"  kernel-predict|gp-predict CSV FEATURE TARGET QUERY REG LENGTH\n"
       <<"  conformal-interval CSV CALIB_PRED CALIB_TARGET QUERY ALPHA\n"
       <<"  conformal-coverage CSV CENTER RADIUS ACTUAL\n"
       <<"  forecast-dm CSV PRED_A PRED_B ACTUAL HAC_LAGS\n"
       <<"  feature-filter CSV TARGET K FEATURE...\n"
       <<"  adam-predict CSV FEATURE TARGET QUERY EPOCHS STEP\n"
       <<"  mlp-predict CSV FEATURE TARGET QUERY HIDDEN EPOCHS STEP\n"
       <<"  bagged-predict CSV FEATURE TARGET QUERY TREES SEED\n";
}
} // namespace kline
