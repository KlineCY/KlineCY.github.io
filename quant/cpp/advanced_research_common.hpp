#pragma once
#include "model_research_common.hpp"
namespace kline { namespace advanced_research {
using namespace model_research;
inline Mat identity(std::size_t d){
    Mat a(d,Vec(d,0));for(std::size_t j=0;j<d;++j)a[j][j]=1;return a;
}
inline Mat invert_spd(const Mat& a){
    auto d=a.size();Mat b(d,Vec(d,0));
    for(std::size_t j=0;j<d;++j){Vec e(d,0);e[j]=1;Vec v=solve_spd(a,e);for(std::size_t i=0;i<d;++i)b[i][j]=v[i];}
    for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<i;++j)b[i][j]=b[j][i]=(b[i][j]+b[j][i])/2;
    return b;
}
inline double matrix_error(const Mat&a,const Mat&b){
    double e=0;for(std::size_t i=0;i<a.size();++i)for(std::size_t j=0;j<a[i].size();++j)e=std::max(e,std::abs(a[i][j]-b[i][j]));return e;
}
inline Vec fit_ridge(const Mat& a,const Vec& y,double penalty,bool intercept){
    std::size_t d=a[0].size();Mat h(d,Vec(d,0));Vec b(d,0);
    for(std::size_t i=0;i<a.size();++i){add_outer(h,a[i],1);for(std::size_t j=0;j<d;++j)b[j]+=a[i][j]*y[i];}
    for(std::size_t j=0;j<d;++j)if(!intercept||j)h[j][j]+=penalty;
    return solve_spd(h,b);
}
} }
