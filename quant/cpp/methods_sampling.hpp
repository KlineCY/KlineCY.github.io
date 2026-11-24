#pragma once
#include "evidence_common.hpp"
namespace kline { namespace methods_sampling {
using namespace evidence;
inline Report weighted_effective_n(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,0);
    double s=0,m=0;
    for(auto&r:x){require(r[0]>=0,"Negative weight");
    s+=r[0];
    m=std::max(m,r[0]);
    }positive(s);
    double q=0;
    for(auto&r:x){double v=r[0]/s;
    q+=v*v;
    }return scalar({"SumWeight","EffectiveN","MaxNormalized"},{s,1/q,m/s});
    
}

inline Report stratified_mean(const Mat& x,const Vec& p){
    validate(x,1,4,4);
    parameters(p,0);
    double N=0;
    for(auto&r:x){count(r[0],2,1000000000);
    count(r[1],2,std::size_t(r[0]));
    require(r[3]>=0,"Negative sample variance");
    N+=r[0];
    }double a=0,v=0;
    for(auto&r:x){double w=r[0]/N;
    a+=w*r[2];
    v+=w*w*(1-r[1]/r[0])*r[3]/r[1];
    }return scalar({"Estimate","Variance","SE"},{a,v,std::sqrt(v)});
    
}

inline Report median_of_means(const Mat& x,const Vec& p){
    validate(x,2,1,1);
    parameters(p,1);
    auto B=count(p[0],1,x.size());
    require(x.size()%B==0,"Equal block lengths required");
    auto n=x.size()/B;
    Vec v(B,0);
    double s=0;
    for(std::size_t i=0;
    i<x.size();
    ++i){v[i/n]+=x[i][0]/n;
    s+=x[i][0];
    }std::sort(v.begin(),v.end());
    double med=(v[(B-1)/2]+v[B/2])/2;
    return scalar({"Blocks","RobustMean","OrdinaryMean"},{double(B),med,s/x.size()});
    
}

inline Report self_normalized_is(const Mat& x,const Vec& p){
    validate(x,2,2,2);
    parameters(p,0);
    double s=0,a=0;
    for(auto&r:x){require(r[0]>=0,"Negative weight");
    s+=r[0];
    a+=r[0]*r[1];
    }double m=a/positive(s),q=0,v=0;
    for(auto&r:x){double w=r[0]/s;
    q+=w*w;
    v+=w*w*(r[1]-m)*(r[1]-m);
    }return scalar({"Estimate","EffectiveN","DeltaSE"},{m,1/q,std::sqrt(v*x.size()/(x.size()-1))});
    
}
} }
