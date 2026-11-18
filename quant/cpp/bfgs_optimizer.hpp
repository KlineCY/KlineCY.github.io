#pragma once
#include "numerical_derivatives.hpp"
#include <cmath>
#include <functional>
#include <numeric>
#include <vector>
namespace kline {
struct BfgsFit{std::vector<double> x;double objective=0;int iterations=0;};
inline BfgsFit bfgs_minimize(
    const std::function<double(const std::vector<double>&)>& f,
    const std::function<std::vector<double>(const std::vector<double>&)>& gradient,
    std::vector<double> x,int max_iterations=300,double tolerance=1e-7){
    check_finite(x);
    require(max_iterations>0&&std::isfinite(tolerance)&&tolerance>0,
            "Invalid BFGS parameters");
    int n=int(x.size());Matrix inverse(n,std::vector<double>(n,0));
    for(int i=0;i<n;i++)inverse[i][i]=1;
    double objective=f(x);require(std::isfinite(objective),"Invalid initial objective");
    for(int iteration=0;iteration<max_iterations;iteration++){
        auto g=gradient(x);
        check_finite(g);require(int(g.size())==n,"Gradient shape mismatch");
        double norm=std::sqrt(std::inner_product(g.begin(),g.end(),g.begin(),0.0));
        if(norm<tolerance)return {x,objective,iteration};
        std::vector<double> direction(n,0);
        for(int i=0;i<n;i++)for(int j=0;j<n;j++)
            direction[i]-=inverse[i][j]*g[j];
        double slope=std::inner_product(g.begin(),g.end(),direction.begin(),0.0);
        if(slope>=0){
            for(int i=0;i<n;i++){direction[i]=-g[i];
                std::fill(inverse[i].begin(),inverse[i].end(),0);
                inverse[i][i]=1;
            }
            slope=-norm*norm;
        }
        double alpha=1.0;
        std::vector<double> next(n);
        double next_objective=0;
        bool found=false;
        for(int step=0;step<45;step++){
            for(int i=0;i<n;i++)next[i]=x[i]+alpha*direction[i];
            next_objective=f(next);
            if(std::isfinite(next_objective)&&
               next_objective<=objective+1e-4*alpha*slope){
                found=true;break;
            }
            alpha*=0.5;
        }
        require(found,"BFGS Armijo line search failed");
        auto ng=gradient(next);check_finite(ng);
        require(int(ng.size())==n,"Updated gradient shape mismatch");
        std::vector<double> s(n),y(n);
        for(int i=0;i<n;i++){s[i]=next[i]-x[i];y[i]=ng[i]-g[i];}
        double sy=std::inner_product(s.begin(),s.end(),y.begin(),0.0);
        if(sy>1e-12){
            double rho=1/sy;
            std::vector<double> Hy(n,0);
            for(int i=0;i<n;i++)for(int j=0;j<n;j++)Hy[i]+=inverse[i][j]*y[j];
            double yHy=std::inner_product(y.begin(),y.end(),Hy.begin(),0.0);
            for(int i=0;i<n;i++)for(int j=0;j<n;j++)
                inverse[i][j]+=(1+rho*yHy)*rho*s[i]*s[j]-
                                rho*(s[i]*Hy[j]+Hy[i]*s[j]);
        }
        x=next;objective=next_objective;
    }
    throw std::runtime_error("BFGS reached iteration limit");
}
} // namespace kline
