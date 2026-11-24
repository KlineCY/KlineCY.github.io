#pragma once
#include "evidence_common.hpp"
namespace kline { namespace model_research {
using namespace evidence;
inline double inf_norm(const Vec& x){
    double v=0;
    for(double a:x) v=std::max(v,std::abs(a));
    return v;
}
inline double soft(double z,double a){
    return std::copysign(std::max(0.,std::abs(z)-a),z);
}
inline double logistic(double z){
    if(z>=0) return 1/(1+std::exp(-z));
    double a=std::exp(z);
    return a/(1+a);
}
inline double softplus(double z){
    return std::max(z,0.)+std::log1p(std::exp(-std::abs(z)));
}
inline void add_outer(Mat& h,const Vec& x,double w){
    for(std::size_t i=0;i<x.size();++i)
        for(std::size_t j=0;j<x.size();++j) h[i][j]+=w*x[i]*x[j];
}
inline double squared(const Vec& x){return dot(x,x);}
inline Vec box_projection(const Vec& z,const Vec& l,const Vec& u,double budget){
    require(z.size()==l.size()&&l.size()==u.size()&&!z.empty(),"Projection dimensions differ");
    double lo=std::numeric_limits<double>::infinity(),hi=-lo;
    for(std::size_t j=0;j<z.size();++j){
        require(l[j]<=u[j],"Invalid box");
        lo=std::min(lo,z[j]-u[j]);
        hi=std::max(hi,z[j]-l[j]);
    }
    require(budget>=sum(l)-1e-12&&budget<=sum(u)+1e-12,"Infeasible budget");
    Vec w(z.size());
    for(int k=0;k<80;++k){
        double m=lo/2+hi/2,s=0;
        for(std::size_t j=0;j<z.size();++j) s+=std::clamp(z[j]-m,l[j],u[j]);
        if(s>budget) lo=m; else hi=m;
    }
    double m=lo/2+hi/2;
    for(std::size_t j=0;j<z.size();++j) w[j]=std::clamp(z[j]-m,l[j],u[j]);
    return w;
}
inline double matrix_lipschitz(const Mat& a){
    double L=0;
    for(const auto& r:a){double s=0;for(double v:r)s+=std::abs(v);L=std::max(L,s);}
    return positive(L);
}
} }
