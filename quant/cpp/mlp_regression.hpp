#pragma once
#include "learning.hpp"
#include <cmath>
#include <vector>
namespace kline {
struct MlpRegression{
    Matrix input_weights;
    std::vector<double> hidden_bias,output_weights;
    double output_bias=0,train_mse=0;
};
inline double mlp_prediction(const MlpRegression& m,const std::vector<double>& x){
    check_finite(x);
    require(!m.input_weights.empty()&&
            m.input_weights.size()==m.output_weights.size(),
            "Invalid MLP hidden width");
    double y=m.output_bias;
    for(int h=0;h<int(m.output_weights.size());h++){
        require(m.input_weights[h].size()==x.size(),"MLP feature width");
        double z=m.hidden_bias[h];
        for(int j=0;j<int(x.size());j++)z+=m.input_weights[h][j]*x[j];
        y+=m.output_weights[h]*std::tanh(z);
    }
    return y;
}
inline MlpRegression fit_mlp_regression(
    const Matrix& x,const std::vector<double>& y,int hidden=8,
    int epochs=500,double learning_rate=0.01){
    check_feature_table(x,y);
    require(hidden>0&&hidden<=256&&epochs>0&&epochs<=100000&&
            std::isfinite(learning_rate)&&learning_rate>0,
            "Invalid MLP training hyperparameters");
    int n=int(x.size()),p=int(x[0].size());
    MlpRegression m;
    m.input_weights.assign(hidden,std::vector<double>(p));
    m.hidden_bias.assign(hidden,0);
    m.output_weights.assign(hidden,0.05);
    m.output_bias=mean(y);
    for(int h=0;h<hidden;h++)
        for(int j=0;j<p;j++)
            m.input_weights[h][j]=0.07*std::sin((h+1.0)*(j+1));
    for(int epoch=0;epoch<epochs;epoch++){
        Matrix dw(hidden,std::vector<double>(p,0));
        std::vector<double> db(hidden,0),dv(hidden,0);
        double dy=0;
        for(int i=0;i<n;i++){
            std::vector<double> activations(hidden);
            double prediction=m.output_bias;
            for(int h=0;h<hidden;h++){
                double z=m.hidden_bias[h];
                for(int j=0;j<p;j++)z+=m.input_weights[h][j]*x[i][j];
                activations[h]=std::tanh(z);
                prediction+=m.output_weights[h]*activations[h];
            }
            double error=2*(prediction-y[i])/n;
            dy+=error;
            for(int h=0;h<hidden;h++){
                dv[h]+=error*activations[h];
                double backward=error*m.output_weights[h]*
                                  (1-activations[h]*activations[h]);
                db[h]+=backward;
                for(int j=0;j<p;j++)dw[h][j]+=backward*x[i][j];
            }
        }
        m.output_bias-=learning_rate*dy;
        for(int h=0;h<hidden;h++){
            m.output_weights[h]-=learning_rate*dv[h];
            m.hidden_bias[h]-=learning_rate*db[h];
            for(int j=0;j<p;j++)
                m.input_weights[h][j]-=learning_rate*dw[h][j];
        }
    }
    for(int i=0;i<n;i++)
        m.train_mse+=std::pow(mlp_prediction(m,x[i])-y[i],2)/n;
    return m;
}
} // namespace kline
