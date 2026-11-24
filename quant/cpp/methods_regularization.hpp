#pragma once
#include "evidence_common.hpp"
namespace kline { namespace methods_regularization {
using namespace evidence;
inline Report orthogonal_lasso(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,1);
    require(p[0]>=0,"Negative penalty");
    Report o{{"Feature","Coefficient","Active"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){auto&r=x[i];
    double v=std::copysign(std::max(0.,std::abs(r[0])-p[0]),r[0])/positive(r[1]);
    o.rows.push_back({double(i+1),v,double(v!=0)});
    }return o;
    
}

inline Report ridge_effective_df(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,1);
    positive(p[0]);
    Report o{{"Mode","Shrinkage","EffectiveDF"},{}};
    double total=0;
    for(std::size_t i=0;
    i<x.size();
    ++i){double s=x[i][0];
    require(s>=0,"Negative squared singular value");
    double v=s/(s+p[0]);
    total+=v;
    o.rows.push_back({double(i+1),v,0});
    }for(auto&r:o.rows)r.back()=total;
    return o;
    
}

inline Report sure_linear(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,2);
    require(p[0]>=0&&p[1]>=0&&p[1]<=double(x.size()),"Invalid variance or effective df");
    double s=0;
    for(auto&r:x)s+=(r[0]-r[1])*(r[0]-r[1]);
    double v=s+2*p[0]*p[1]-x.size()*p[0];
    return scalar({"ResidualSS","SURE","PerObservation"},{s,v,v/x.size()});
    
}

inline Report stability_frequency(const Mat& x,const Vec& p){
    auto d=validate(x,1,1,64);
    parameters(p,1);
    require(p[0]>.5&&p[0]<=1,"Threshold must be above one half");
    Report o{{"Feature","Frequency","Retained"},{}};
    for(std::size_t j=0;
    j<d;
    ++j){double s=0;
    for(auto&r:x){require(r[j]==0||r[j]==1,"Selection indicator must be binary");
    s+=r[j];
    }double v=s/x.size();
    o.rows.push_back({double(j+1),v,double(v>=p[0])});
    }return o;
    
}
} }
