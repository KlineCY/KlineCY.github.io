#pragma once
#include "evidence_common.hpp"
namespace kline { namespace methods_optimization {
using namespace evidence;
inline Report simplex_projection(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,0);
    Vec z=column(x,0),s=z;
    std::sort(s.begin(),s.end(),std::greater<double>());
    double sum=0,theta=0;
    for(std::size_t i=0;
    i<s.size();
    ++i){sum+=s[i];
    double t=(sum-1)/(i+1);
    if(s[i]>t)theta=t;
    }Report o{{"Index","Weight","Threshold"},{}};
    for(std::size_t i=0;
    i<z.size();
    ++i)o.rows.push_back({double(i+1),std::max(0.,z[i]-theta),theta});
    return o;
    
}

inline Report diagonal_kkt(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,1);
    double g=positive(p[0]),a=0,b=0;
    for(auto&r:x){a+=r[0]/positive(r[1]);
    b+=1/r[1];
    }double eta=(a-g)/b;
    Report o{{"Asset","Weight","Multiplier","StationarityResidual"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){auto&r=x[i];
    double w=(r[0]-eta)/(g*r[1]);
    o.rows.push_back({double(i+1),w,eta,g*r[1]*w-r[0]+eta});
    }return o;
    
}

inline Report entropic_prox(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,1);
    double b=positive(p[0]);
    Vec u=column(x,0);
    double m=*std::max_element(u.begin(),u.end()),s=0;
    Vec w;
    for(double z:u){w.push_back(std::exp(b*(z-m)));
    s+=w.back();
    }double v=m+std::log(s)/b;
    Report o{{"Asset","Weight","Objective"},{}};
    for(std::size_t i=0;
    i<u.size();
    ++i)o.rows.push_back({double(i+1),w[i]/s,v});
    return o;
    
}

inline Report quadratic_line_search(const Mat& x,const Vec& p){
    validate(x,1,3,3);
    parameters(p,0);
    double a=0,b=0;
    for(auto&r:x){positive(r[0]);
    a+=r[1]*r[2];
    b+=r[0]*r[2]*r[2];
    }require(a<0,"Descent direction required");
    positive(b);
    return scalar({"Step","DirectionalDerivative","Curvature","ObjectiveDecrease"},{-a/b,a,b,a*a/(2*b)});
    
}
} }
