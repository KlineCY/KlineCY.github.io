#pragma once
#include "evidence_common.hpp"
namespace kline { namespace methods_geometry {
using namespace evidence;
inline Report gram_schmidt(const Mat& x,const Vec& p){
    auto d=validate(x,2,1,64);
    parameters(p,0);
    require(x.size()>=d,"Tall matrix required");
    Mat Q;
    Report out{{"Column","RDiagonal","OrthogonalityError"},{}};
    for(std::size_t j=0;
    j<d;
    ++j){Vec v=column(x,j);
    double scale=std::sqrt(dot(v,v));
    for(const auto&q:Q){double r=dot(q,v);
    for(std::size_t i=0;
    i<v.size();
    ++i)v[i]-=r*q[i];
    }double n=std::sqrt(dot(v,v));
    require(n>1e-12*positive(scale),"Rank deficient column");
    for(auto&z:v)z/=n;
    double e=std::abs(dot(v,v)-1);
    for(const auto&q:Q)e=std::max(e,std::abs(dot(v,q)));
    Q.push_back(v);
    out.rows.push_back({double(j+1),n,e});
    }return out;
    
}

inline Report rank_one_inverse(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,0);
    double den=1;
    for(auto&r:x)den+=r[1]*r[1]/positive(r[0]);
    Report o{{"Index","InverseDiagonal","DetRatio"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){auto&r=x[i];
    double a=1/r[0],b=a*r[1];
    o.rows.push_back({double(i+1),a-b*b/den,den});
    }return o;
    
}

inline Report gershgorin(const Mat& x,const Vec& p){
    auto d=validate(x,1,1,64);
    parameters(p,0);
    require(x.size()==d,"Square matrix required");
    Report o{{"Row","Lower","Upper","GlobalLower"},{}};
    double low=std::numeric_limits<double>::infinity();
    for(std::size_t i=0;
    i<d;
    ++i){double r=0;
    for(std::size_t j=0;
    j<d;
    ++j){require(std::abs(x[i][j]-x[j][i])<=1e-10*std::max({1.,std::abs(x[i][j]),std::abs(x[j][i])}),"Symmetric matrix required");
    if(i!=j)r+=std::abs(x[i][j]);
    }low=std::min(low,x[i][i]-r);
    o.rows.push_back({double(i+1),x[i][i]-r,x[i][i]+r,0});
    }for(auto&r:o.rows)r.back()=low;
    return o;
    
}

inline Report spectral_participation(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,0);
    double s=0;
    for(auto&r:x){require(r[0]>=0,"Negative eigenvalue");
    s+=r[0];
    }positive(s);
    double q=0,h=0;
    for(auto&r:x){double v=r[0]/s;
    q+=v*v;
    if(v>0)h-=v*std::log(v);
    }return scalar({"Trace","ParticipationRank","EntropyRank"},{s,1/q,std::exp(h)});
    
}
} }
