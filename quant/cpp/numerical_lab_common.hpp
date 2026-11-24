#pragma once
#include "researchflow_common.hpp"
#include <array>
#include <cstdint>
#include <map>
namespace kline { namespace numerical_lab {
using researchflow::Vec;
using researchflow::Mat;
using researchflow::Report;
using researchflow::positive;
using researchflow::validate;
using researchflow::parameters;
using researchflow::count;
using researchflow::dot;
using researchflow::sum;
using researchflow::column;
using researchflow::solve_spd;
inline double norm(const Vec& v){return std::sqrt(dot(v,v));}
inline Vec mul(const Mat& a,const Vec& x){Vec v;for(const auto& row:a)v.push_back(dot(row,x));return v;}
inline Vec tridiagonal(const Vec& lower,const Vec& diagonal,const Vec& upper,const Vec& b){
    const auto n=diagonal.size();
    require(n>0&&b.size()==n&&lower.size()==n&&upper.size()==n,"Tridiagonal dimensions differ");
    Vec d=diagonal,z=b,x(n);
    for(std::size_t i=1;i<n;++i){
        require(std::abs(d[i-1])>1e-15,"Singular tridiagonal pivot");
        double w=lower[i]/d[i-1];d[i]-=w*upper[i-1];z[i]-=w*z[i-1];
    }
    for(std::size_t i=n;i-->0;){
        require(std::abs(d[i])>1e-15,"Singular tridiagonal pivot");
        x[i]=(z[i]-(i+1<n?upper[i]*x[i+1]:0))/d[i];
    }
    check_finite(x);return x;
}
inline double normal(double z){return .5*std::erfc(-z/std::sqrt(2.0));}
inline double bs(double s,double k,double r,double q,double v,double T,bool call){
    positive(s);positive(k);positive(v);positive(T);
    double a=(std::log(s/k)+(r-q+.5*v*v)*T)/(v*std::sqrt(T)),b=a-v*std::sqrt(T);
    if(call)return s*std::exp(-q*T)*normal(a)-k*std::exp(-r*T)*normal(b);
    return k*std::exp(-r*T)*normal(-b)-s*std::exp(-q*T)*normal(-a);
}
inline Report scalar_rows(const std::vector<std::string>& names,const Vec& r){return {names,{r}};}
} }
