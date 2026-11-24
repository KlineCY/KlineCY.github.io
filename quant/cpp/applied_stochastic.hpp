#pragma once
#include "evidence_common.hpp"
namespace kline { namespace applied_stochastic {
using namespace evidence;
inline Report semivariance(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,0);
    double a=0,b=0;
    for(auto&r:x){double v=r[0]*r[0];
    if(r[0]>0)a+=v;
    else b+=v;
    }return scalar({"PositiveRV","NegativeRV","TotalRV"},{a,b,a+b});
    
}

inline Report bridge(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,3);
    double sigma=positive(p[2]);
    Report o{{"Time","ConditionalMean","ConditionalVariance"},{}};
    for(auto&r:x){double t=r[0];
    require(t>=0&&t<=1,"Time outside unit interval");
    o.rows.push_back({t,p[0]+t*(p[1]-p[0]),sigma*sigma*t*(1-t)});
    }return o;
    
}

inline Report covariation(const Mat& x,const Vec& p){
    validate(x,3,2,2);
    parameters(p,1);
    auto k=count(p[0],2,x.size()-1);
    require((x.size()-1)%k==0,"Stride must divide interval count");
    Report o{{"Stride","Intervals","QVX","QVY","QCov"},{}};
    for(auto step:{std::size_t(1),k}){double a=0,b=0,c=0;
    for(std::size_t i=step;
    i<x.size();
    i+=step){double u=x[i][0]-x[i-step][0],v=x[i][1]-x[i-step][1];
    a+=u*u;
    b+=v*v;
    c+=u*v;
    }o.rows.push_back({double(step),double((x.size()-1)/step),a,b,c});
    }return o;
    
}

inline Report ar_aggregation(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,2);
    double phi=p[0],v=positive(p[1]);
    require(std::abs(phi)<1,"Stationary AR required");
    Report o{{"Horizon","SingleVariance","SumVariance","IndependentVariance"},{}};
    double work=0;
    for(auto&r:x){auto h=count(r[0],1,100000);
    work+=h;
    require(work<=2e7,"Work limit");
    double z=h*v,power=phi;
    for(std::size_t k=1;
    k<h;
    ++k){z+=2*v*(h-k)*power;
    power*=phi;
    }o.rows.push_back({double(h),v,z,h*v});
    }return o;
    
}
} }
