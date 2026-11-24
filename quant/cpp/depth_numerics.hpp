#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_numerics {
using namespace evidence;
inline Report backward_error(const Mat& x,const Vec& p){
    auto m=validate(x,1,3,64);
    parameters(p,0);
    auto d=m-2;
    require(x.size()==d,"Square system required");
    Vec z=column(x,d),b=column(x,d+1);
    double an=0,xn=0,bn=0,rn=0;
    for(double v:z)xn=std::max(xn,std::abs(v));
    for(std::size_t i=0;i<d;++i){
        double a=0,v=0;
        for(std::size_t j=0;j<d;++j){a+=std::abs(x[i][j]);v+=x[i][j]*z[j];}
        an=std::max(an,a);bn=std::max(bn,std::abs(b[i]));rn=std::max(rn,std::abs(b[i]-v));
    }
    return scalar({"ResidualInf","RelativeResidual","BackwardError","MatrixInfNorm"},{rn,rn/positive(bn),rn/positive(an*xn+bn),an});
}

inline Report row_equilibration(const Mat& x,const Vec& p){
    auto d=validate(x,1,2,64)-1;
    parameters(p,0);
    Report o{{"Row","Scale"},{}};
    for(std::size_t j=0;j<d;++j)o.header.push_back("A"+std::to_string(j+1));
    o.header.push_back("B");
    for(std::size_t i=0;i<x.size();++i){
        double s=0;
        for(std::size_t j=0;j<d;++j)s=std::max(s,std::abs(x[i][j]));
        positive(s);
        Vec r{double(i+1),s};
        for(double v:x[i])r.push_back(v/s);
        o.rows.push_back(r);
    }
    return o;
}

inline Report two_by_two_spectrum(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,0);
    require(x.size()==2,"Two by two matrix required");
    require(std::abs(x[0][1]-x[1][0])<=1e-12*std::max({1.,std::abs(x[0][1]),std::abs(x[1][0])}),"Symmetric matrix required");
    double a=x[0][0],d=x[1][1],b=x[0][1]/2+x[1][0]/2;
    double m=a/2+d/2,h=std::hypot(a/2-d/2,b);
    return scalar({"EigenLow","EigenHigh","Trace","Determinant"},{m-h,m+h,a+d,a*d-b*b});
}

inline Report precision_conditional(const Mat& x,const Vec& p){
    auto d=validate(x,1,1,64);parameters(p,0);
    require(x.size()==d,"Square covariance required");
    Report o{{"Variable","PrecisionDiagonal","ConditionalVariance","ResidualInf"},{}};
    for(std::size_t j=0;j<d;++j){
        Vec e(d,0);e[j]=1;
        Vec z=solve_spd(x,e),r=mul(x,z);
        double err=0;
        for(std::size_t i=0;i<d;++i)err=std::max(err,std::abs(r[i]-e[i]));
        o.rows.push_back({double(j+1),z[j],1/positive(z[j]),err});
    }
    return o;
}
} }
