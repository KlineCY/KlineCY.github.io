#pragma once
#include "lab24_common.hpp"
#include <functional>

namespace kline { namespace lab24 {
inline Vec qr_solve(const Mat& x,const Vec& p){
    inputs(x,p,3,0,2);
    Vec a=col(x,0),b=col(x,1),y=col(x,2);
    double r11=positive(std::sqrt(dot(a,a)));
    for(double& z:a)z/=r11;
    double r12=dot(a,b),b0=std::sqrt(dot(b,b));
    for(std::size_t i=0;i<b.size();++i)b[i]-=r12*a[i];
    double corr=dot(a,b);r12+=corr;
    for(std::size_t i=0;i<b.size();++i)b[i]-=corr*a[i];
    double r22=std::sqrt(dot(b,b));
    require(r22>1e-12*b0&&r22>0,"Rank-deficient design");
    for(double& z:b)z/=r22;
    double v=dot(b,y)/r22,u=(dot(a,y)-r12*v)/r11,sse=0;
    for(const auto& row:x){double e=row[2]-row[0]*u-row[1]*v;sse+=e*e;}
    return checked({u,v,sse});
}

inline Vec spectral2(const Mat& x,const Vec& p){
    inputs(x,p,2,0,2);require(x.size()==2,"A 2 by 2 matrix is required");
    require(std::abs(x[0][1]-x[1][0])<=1e-12*std::max(1.0,std::abs(x[0][1])),"Matrix must be symmetric");
    double a=x[0][0],b=x[0][1],d=x[1][1];
    double mid=0.5*a+0.5*d,gap=std::hypot(0.5*a-0.5*d,b);
    double theta=0.5*std::atan2(b,0.5*a-0.5*d);
    return checked({mid+gap,mid-gap,std::cos(theta),std::sin(theta),2*gap});
}

inline Vec ridge2(const Mat& x,const Vec& p){
    inputs(x,p,3,1,2);double l=positive(p[0]);
    double m1=mean(col(x,0)),m2=mean(col(x,1)),my=mean(col(x,2));
    double a=0,b=0,d=0,u=0,v=0;
    for(const auto& r:x){
        double q=r[0]-m1,s=r[1]-m2,t=r[2]-my;
        a+=q*q;b+=q*s;d+=s*s;u+=q*t;v+=s*t;
    }
    double aa=a+l,dd=d+l,det=positive(aa*dd-b*b);
    double c1=(dd*u-b*v)/det,c2=(aa*v-b*u)/det;
    double df=1+(dd*a+aa*d-2*b*b)/det;
    return checked({my-c1*m1-c2*m2,c1,c2,df});
}

inline Vec woodbury_rank1(const Mat& x,const Vec& p){
    inputs(x,p,2,1);require(p[0]>=0,"Rank-one variance must be nonnegative");
    Vec u=col(x,0),b=col(x,1);
    double scale=p[0]*dot(u,b)/positive(1+p[0]*dot(u,u));
    for(std::size_t i=0;i<b.size();++i)b[i]-=scale*u[i];
    return checked(b);
}

inline Vec power_iteration(const Mat& x,const Vec& p){
    inputs(x,p,3,1,3);require(x.size()==3,"A 3 by 3 matrix is required");
    for(std::size_t i=0;i<3;++i)for(std::size_t j=0;j<3;++j)
        require(std::abs(x[i][j]-x[j][i])<1e-12,"Symmetric matrix required");
    require(x[0][0]>0&&x[0][0]*x[1][1]-x[0][1]*x[0][1]>0,"Positive leading minors required");
    double det=x[0][0]*(x[1][1]*x[2][2]-x[1][2]*x[2][1])-x[0][1]*(x[1][0]*x[2][2]-x[1][2]*x[2][0])+x[0][2]*(x[1][0]*x[2][1]-x[1][1]*x[2][0]);
    positive(det);
    auto multiply=[&](const Vec& v){Vec z;for(const auto& r:x)z.push_back(dot(r,v));return z;};
    Vec v(3,1/std::sqrt(3.0));auto steps=integer(p[0],1,10000);
    for(std::size_t k=0;k<steps;++k){v=multiply(v);double n=positive(std::sqrt(dot(v,v)));for(double& q:v)q/=n;}
    Vec a=multiply(v);double lambda=dot(v,a);for(std::size_t i=0;i<3;++i)a[i]-=lambda*v[i];
    return checked({lambda,std::sqrt(dot(a,a))});
}
} }
