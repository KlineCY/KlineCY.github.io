#pragma once
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
#include "io.hpp"
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace kline {

inline Matrix advanced_columns_as_rows(const CsvTable& table,
                                       const std::vector<std::string>& columns) {
    require(!columns.empty(), "At least one numerical feature column required");
    Matrix values(table.rows.size(),std::vector<double>(columns.size()));
    for (int j=0;j<int(columns.size());j++) {
        auto col=table.numbers(columns[j]);
        for (int i=0;i<int(col.size());i++) values[i][j]=col[i];
    }
    return values;
}

inline ZeroCurve advanced_curve(const CsvTable& table) {
    return {table.numbers("Tenor"),table.numbers("ZeroRate")};
}

inline void print_values(std::ostream& out,const std::string& title,
                         const std::vector<double>& values) {
    out<<"Index,"<<title<<"\n";
    for (int i=0;i<int(values.size());i++) out<<i<<","<<values[i]<<"\n";
}

inline bool run_advanced_cli(const std::string& command, int argc,
                              char** argv,std::ostream& out) {
    auto num=[&](int index){return parse_number(argv[index]);};
    auto count=[&](int expected){require(argc==expected,"Incorrect argument count");};
    if (command=="pca" || command=="cov-condition") {
        require(argc>=5,"Use pca/cov-condition CSV FEATURE_A FEATURE_B...");
        auto table=read_csv(argv[2]);
        std::vector<std::string> columns;
        for(int j=3;j<argc;j++)columns.push_back(argv[j]);
        if(command=="pca") {
            auto model=principal_components(advanced_columns_as_rows(table,columns),
                                            int(columns.size()));
            out<<"Component,Eigenvalue\n";
            for(int j=0;j<int(model.eigenvalues.size());j++)
                out<<j+1<<","<<model.eigenvalues[j]<<"\n";
            out<<"Feature";
            for(int j=0;j<int(columns.size());j++)out<<",PC"<<j+1;
            out<<"\n";
            for(int i=0;i<int(columns.size());i++){
                out<<columns[i];
                for(double value:model.loadings[i])out<<","<<value;
                out<<"\n";
            }
        } else {
            auto cov=sample_covariance_matrix(asset_returns_from_columns(table,columns));
            out<<"ConditionNumber "<<condition_number_symmetric(cov)<<"\n";
        }
    } else if(command=="ema") {
        count(5);
        auto values=read_csv(argv[2]).numbers(argv[3]);
        print_values(out,"EMA",exponential_mean(values,num(4)));
    } else if(command=="ewma-var") {
        count(6);
        auto values=read_csv(argv[2]).numbers(argv[3]);
        print_values(out,"PriorVariance",ewma_conditional_variance(values,num(4),num(5)));
    } else if(command=="ar1") {
        count(4);
        auto values=read_csv(argv[2]).numbers(argv[3]);
        auto model=fit_ar1(values);
        out<<"Intercept "<<model.intercept<<"\n"
           <<"Slope "<<model.slope<<"\n"
           <<"ResidualVariance "<<model.residual_variance<<"\n"
           <<"NextPrediction "<<ar1_forecast(model,values.back())<<"\n";
    } else if(command=="garch") {
        count(8);
        print_values(out,"PriorConditionalVariance",
            garch11_forecast_variances(read_csv(argv[2]).numbers(argv[3]),
                                      num(4),num(5),num(6),num(7)));
    } else if(command=="momentum") {
        count(6);
        print_values(out,"Momentum",simple_momentum(read_csv(argv[2]).numbers(argv[3]),
                                                    std::stoi(argv[4]),std::stoi(argv[5])));
    } else if(command=="rsi") {
        count(5);
        print_values(out,"RSI",relative_strength_index(read_csv(argv[2]).numbers(argv[3]),
                                                       std::stoi(argv[4])));
    } else if(command=="atr") {
        count(7);
        auto t=read_csv(argv[2]);
        print_values(out,"ATR",average_true_range(t.numbers(argv[3]),
                    t.numbers(argv[4]),t.numbers(argv[5]),std::stoi(argv[6])));
    } else if(command=="lasso" || command=="logistic") {
        require(argc>=6,"Use lasso/logistic CSV LABEL LAMBDA FEATURE...");
        auto t=read_csv(argv[2]);
        std::vector<std::string> features;
        for(int i=5;i<argc;i++)features.push_back(argv[i]);
        auto rows=advanced_columns_as_rows(t,features);
        if(command=="lasso"){
            auto y=t.numbers(argv[3]);
            auto model=fit_lasso(rows,y,num(4));
            out<<"RawIntercept ";
            double intercept=model.intercept;
            for(int j=0;j<int(features.size());j++)
                intercept-=model.coefficients[j]*model.feature_mean[j]/
                           model.feature_scale[j];
            out<<intercept<<"\nFeature,RawCoefficient\n";
            for(int j=0;j<int(features.size());j++)
                out<<features[j]<<","<<model.coefficients[j]/model.feature_scale[j]<<"\n";
            out<<"TrainingMSE "<<ridge_training_mse(model,rows,y)<<"\n";
        }else{
            auto raw=t.numbers(argv[3]);
            std::vector<int> y;
            for(double v:raw){
                require(v==0 || v==1,"Logistic labels must be binary");
                y.push_back(int(v));
            }
            auto model=fit_logistic(rows,y,num(4));
            out<<"TrainingLogLoss "<<logistic_log_loss(model,rows,y)<<"\n";
            out<<"Index,Probability\n";
            for(int i=0;i<int(rows.size());i++)
                out<<i<<","<<logistic_predict_probability(model,rows[i])<<"\n";
        }
    } else if(command=="risk-budget") {
        require(argc>=7 && (argc-3)%2==0,
                "Use risk-budget CSV ASSET_RETURN BUDGET [MORE_PAIRS...]");
        auto t=read_csv(argv[2]);
        std::vector<std::vector<double>> asset_returns;
        std::vector<std::string> names;
        std::vector<double> budget;
        for(int i=3;i<argc;i+=2){
            names.push_back(argv[i]);
            asset_returns.push_back(t.numbers(argv[i]));
            budget.push_back(num(i+1));
        }
        auto cov=sample_covariance_matrix(asset_returns);
        auto weights=risk_budget_weights(cov,budget);
        auto contributions=volatility_risk_contributions(cov,weights);
        out<<"Asset,Weight,VolatilityContribution\n";
        for(int i=0;i<int(names.size());i++)
            out<<names[i]<<","<<weights[i]<<","<<contributions[i]<<"\n";
    } else if(command=="cap-project") {
        count(5);
        print_values(out,"TargetWeight",
            project_capped_simplex(read_csv(argv[2]).numbers(argv[3]),num(4)));
    } else if(command=="es-contrib") {
        require(argc>=8 && (argc-4)%2==0,
                "Use es-contrib CSV CONFIDENCE WEIGHT RETURN [MORE_PAIRS...]");
        auto t=read_csv(argv[2]);
        std::vector<double> weights;
        std::vector<std::string> names;
        std::vector<std::string> columns;
        for(int i=4;i<argc;i+=2){
            weights.push_back(num(i));
            columns.push_back(argv[i+1]);
            names.push_back(argv[i+1]);
        }
        auto contribution=historical_es_contributions(
            weights,advanced_columns_as_rows(t,columns),num(3));
        out<<"Asset,ESContribution\n";
        for(int i=0;i<int(names.size());i++)
            out<<names[i]<<","<<contribution[i]<<"\n";
    } else if(command=="black76" || command=="bachelier") {
        count(8);
        std::string type=argv[7];
        require(type=="call" || type=="put","Option type must be call or put");
        double value=command=="black76" ?
            black76(num(2),num(3),num(4),num(5),num(6),type=="call") :
            bachelier_price(num(2),num(3),num(4),num(5),num(6),type=="call");
        out<<"OptionPremium "<<value<<"\n";
    } else if(command=="normal-iv") {
        count(8);
        std::string type=argv[7];
        require(type=="call" || type=="put","Option type must be call or put");
        out<<"NormalImpliedVolatility "<<bachelier_implied_volatility(
            num(2),num(3),num(4),num(5),num(6),type=="call")<<"\n";
    } else if(command=="curve-pv" || command=="swap-par" || command=="keyrate") {
        count(4);
        auto curve=advanced_curve(read_csv(argv[2]));
        auto t=read_csv(argv[3]);
        if(command=="swap-par")
            out<<"ParSwapRate "<<par_swap_rate(curve,t.numbers("PaymentTime"))<<"\n";
        else {
            auto times=t.numbers("Time"),cash=t.numbers("Cashflow");
            if(command=="curve-pv")
                out<<"PresentValue "<<present_value_cashflows(curve,times,cash)<<"\n";
            else
                print_values(out,"KeyRateDV01",key_rate_dv01(curve,times,cash));
        }
    } else if(command=="book-walk") {
        count(5);
        auto t=read_csv(argv[2]);
        auto price=t.numbers("Price"),quantity=t.numbers("Quantity");
        std::vector<BookLevel> levels;
        for(int i=0;i<int(price.size());i++)levels.push_back({price[i],quantity[i]});
        std::string side=argv[3];
        require(side=="buy" || side=="sell","Book side must be buy or sell");
        auto result=walk_order_book(levels,num(4),side=="buy");
        out<<"Executed "<<result.executed<<"\n"
           <<"Unfilled "<<result.unfilled<<"\n"
           <<"AveragePrice "<<result.average_price<<"\n";
    } else if(command=="capped-schedule") {
        count(5);
        print_values(out,"ChildOrder",capped_volume_schedule(
            read_csv(argv[2]).numbers(argv[3]),num(4)));
    } else if(command=="cusum") {
        count(7);
        auto values=read_csv(argv[2]).numbers(argv[3]);
        out<<"FirstAlarmIndex "<<first_cusum_alarm(values,num(4),num(5),num(6))
           <<"\n";
    } else if(command=="holm") {
        count(4);
        print_values(out,"AdjustedP",holm_adjusted_p(read_csv(argv[2]).numbers(argv[3])));
    } else if(command=="bootstrap-ci") {
        count(8);
        auto interval=block_bootstrap_interval(
            read_csv(argv[2]).numbers(argv[3]),std::stoi(argv[4]),
            std::stoi(argv[5]),std::stoul(argv[6]),num(7));
        out<<"Mean "<<interval.estimate<<"\n"
           <<"Lower "<<interval.lower<<"\n"
           <<"Upper "<<interval.upper<<"\n";
    } else if(command=="ou-step") {
        count(8);
        out<<"NextOU "<<exact_ou_step(num(2),num(3),num(4),num(5),num(6),num(7))<<"\n";
    } else return false;
    return true;
}

inline void advanced_cli_usage(std::ostream& out) {
    out<<"  pca CSV FEATURE_A FEATURE_B [...]\n"
       <<"  cov-condition CSV ASSET_RETURN_A ASSET_RETURN_B [...]\n"
       <<"  ema CSV COLUMN ALPHA\n"
       <<"  ewma-var CSV RETURN_COLUMN DECAY INITIAL_VARIANCE\n"
       <<"  ar1 CSV COLUMN\n"
       <<"  garch CSV RESIDUAL_COLUMN OMEGA ALPHA BETA INITIAL_VARIANCE\n"
       <<"  momentum CSV PRICE_COLUMN LOOKBACK SKIP\n"
       <<"  rsi CSV CLOSE_COLUMN PERIOD\n"
       <<"  atr CSV HIGH LOW CLOSE PERIOD\n"
       <<"  lasso CSV LABEL LAMBDA FEATURE [...]\n"
       <<"  logistic CSV BINARY_LABEL LAMBDA FEATURE [...]\n"
       <<"  risk-budget CSV RETURN_A BUDGET_A RETURN_B BUDGET_B [...]\n"
       <<"  cap-project CSV SIGNAL_COLUMN MAX_WEIGHT\n"
       <<"  es-contrib CSV CONFIDENCE WEIGHT_A RETURN_A WEIGHT_B RETURN_B [...]\n"
       <<"  black76 FORWARD STRIKE VOL YEARS DISCOUNT call|put\n"
       <<"  bachelier FORWARD STRIKE NORMAL_VOL YEARS DISCOUNT call|put\n"
       <<"  normal-iv PREMIUM FORWARD STRIKE YEARS DISCOUNT call|put\n"
       <<"  curve-pv CURVE_CSV CASHFLOW_CSV\n"
       <<"  swap-par CURVE_CSV PAYMENT_TIMES_CSV\n"
       <<"  keyrate CURVE_CSV CASHFLOW_CSV\n"
       <<"  book-walk BOOK_CSV buy|sell ORDER_QUANTITY\n"
       <<"  capped-schedule CSV CAPACITY_COLUMN ORDER_QUANTITY\n"
       <<"  cusum CSV COLUMN REFERENCE ALLOWANCE ALARM_THRESHOLD\n"
       <<"  holm CSV PVALUE_COLUMN\n"
       <<"  bootstrap-ci CSV COLUMN BLOCK_LENGTH REPLICATES SEED CONFIDENCE\n"
       <<"  ou-step CURRENT LEVEL KAPPA VOLATILITY DT NORMAL_SHOCK\n";
}

} // namespace kline
