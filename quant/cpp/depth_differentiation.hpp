#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_differentiation {
using namespace evidence;
inline Report logistic_hvp(const Mat& x,const Vec& p){
    auto d=validate(x,1,2,33)-1;parameters(p,2*d);Vec w(p.begin(),p.begin()+d),v(p.begin()+d,p.end()),g(d,0),hv(d,0);
    for(auto&r:x){double y=r.back();require(y==0||y==1,"Binary label required");double z=0,a=0;
        for(std::size_t j=0;j<d;++j){z+=r[j]*w[j];a+=r[j]*v[j];}
        double q=z>=0?1/(1+std::exp(-z)):std::exp(z)/(1+std::exp(z));double b=q*(1-q)*a;
        for(std::size_t j=0;j<d;++j){g[j]+=(q-y)*r[j];hv[j]+=b*r[j];}
    }double cur=dot(v,hv);Report o{{"Feature","Gradient","HessianVector","DirectionalCurvature"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),g[j],hv[j],cur});return o;
}

inline Report softmax_jvp(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,0);double m=x[0][0];for(auto&r:x)m=std::max(m,r[0]);Vec q;double s=0,a=0;
    for(auto&r:x){q.push_back(std::exp(r[0]-m));s+=q.back();}
    for(std::size_t i=0;i<x.size();++i){q[i]/=s;a+=q[i]*x[i][1];}
    Vec v;double z=0;for(std::size_t i=0;i<x.size();++i){v.push_back(q[i]*(x[i][1]-a));z+=v.back();}
    Report o{{"Class","Probability","JacobianVector","SumDerivative"},{}};
    for(std::size_t i=0;i<x.size();++i)o.rows.push_back({double(i),q[i],v[i],z});return o;
}

inline Report gradient_noise(const Mat& x,const Vec& p){
    auto d=validate(x,2,1,64);parameters(p,0);Vec m=means(x);double a=dot(m,m),b=0;
    for(auto&r:x)for(std::size_t j=0;j<d;++j)b+=(r[j]-m[j])*(r[j]-m[j]);b/=x.size()-1;
    return scalar({"MeanNormSquared","CovarianceTrace","NoiseScale"},{a,b,b/positive(a)});
}

inline Report global_gradient_clip(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,1);double c=positive(p[0]),n=0;for(auto&r:x)n=std::hypot(n,r[0]);double s=n>c?c/n:1;
    Report o{{"Coordinate","Original","Clipped","Norm","Scale"},{}};
    for(std::size_t i=0;i<x.size();++i)o.rows.push_back({double(i+1),x[i][0],s*x[i][0],n,s});return o;
}
} }
