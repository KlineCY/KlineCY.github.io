#pragma once
#include "learning.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>
namespace kline {
struct SoftmaxFit{Matrix coefficients;int classes=0;double loss=0;};
inline std::vector<double> softmax_probabilities(const SoftmaxFit& m,
                                  const std::vector<double>& row){
    check_finite(row);
    require(m.classes>=2&&int(m.coefficients.size())==m.classes,
            "Invalid softmax model");
    std::vector<double> logits(m.classes);
    for(int c=0;c<m.classes;c++){
        require(m.coefficients[c].size()==row.size()+1,
                "Softmax feature mismatch");
        logits[c]=m.coefficients[c][0];
        for(int j=0;j<int(row.size());j++)
            logits[c]+=m.coefficients[c][j+1]*row[j];
    }
    double largest=*std::max_element(logits.begin(),logits.end()),sum=0;
    for(double& z:logits){z=std::exp(z-largest);sum+=z;}
    for(double& z:logits)z/=sum;
    return logits;
}
inline SoftmaxFit fit_softmax(const Matrix& x,const std::vector<int>& labels,
                    int classes,double learning_rate=0.05,int epochs=500,
                    double l2=0.001){
    require(!x.empty()&&x.size()==labels.size()&&classes>=2&&classes<=50&&
            std::isfinite(learning_rate)&&learning_rate>0&&
            epochs>0&&epochs<=10000&&std::isfinite(l2)&&l2>=0,
            "Invalid softmax training inputs");
    int n=int(x.size()),p=int(x[0].size());
    require(p>0,"Softmax requires features");
    for(const auto& row:x){require(int(row.size())==p,"Softmax shape mismatch");check_finite(row);}
    for(int y:labels)require(y>=0&&y<classes,"Softmax class outside range");
    SoftmaxFit m;m.classes=classes;m.coefficients.assign(classes,std::vector<double>(p+1,0));
    for(int epoch=0;epoch<epochs;epoch++){
        Matrix grad(classes,std::vector<double>(p+1,0));
        double loss=0;
        for(int i=0;i<n;i++){
            auto prob=softmax_probabilities(m,x[i]);
            loss-=std::log(std::max(1e-300,prob[labels[i]]))/n;
            for(int c=0;c<classes;c++){
                double error=prob[c]-(c==labels[i]?1.0:0.0);
                grad[c][0]+=error/n;
                for(int j=0;j<p;j++)grad[c][j+1]+=error*x[i][j]/n;
            }
        }
        for(int c=0;c<classes;c++)for(int j=0;j<=p;j++){
            if(j>0){grad[c][j]+=l2*m.coefficients[c][j];
                    loss+=0.5*l2*m.coefficients[c][j]*m.coefficients[c][j];}
            m.coefficients[c][j]-=learning_rate*grad[c][j];
        }
        m.loss=loss;
    }
    return m;
}
} // namespace kline
