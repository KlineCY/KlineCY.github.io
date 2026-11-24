#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_kernel {
using namespace evidence;
inline Report nystrom_schur(const Mat& x,const Vec& p){
    auto d=validate(x,1,2,33)-1;parameters(p,1);require(x.size()==d&&p[0]>0,"Invalid landmark dimensions");
    Mat W;Vec k;for(auto&r:x){W.emplace_back(r.begin(),r.end()-1);k.push_back(r.back());}
    Vec a=solve_spd(W,k);double v=dot(k,a),err=p[0]-v;
    require(err>=-1e-10*std::max(1.,p[0]),"Negative Schur residual");
    return scalar({"ApproximatedDiagonal","SchurResidual","RelativeResidual"},{v,err,err/p[0]});
}

inline Report rff_pair(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,2);double a=std::sqrt(2./x.size()),s=0;Report o{{"Feature","Zx","Zy","Product","CumulativeKernel"},{}};
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];double u=a*std::cos(r[0]*p[0]+r[1]),v=a*std::cos(r[0]*p[1]+r[1]);s+=u*v;o.rows.push_back({double(i+1),u,v,u*v,s});}return o;
}

inline Report mahalanobis_residual(const Mat& x,const Vec& p){
    auto d=validate(x,1,2,33)-1;parameters(p,0);require(x.size()==d,"Square covariance required");Mat s;Vec z;
    for(auto&r:x){s.emplace_back(r.begin(),r.end()-1);z.push_back(r.back());}
    Vec v=solve_spd(s,z);double q=dot(z,v);require(q>=0,"Negative squared distance");return scalar({"EuclideanSquared","MahalanobisSquared","Distance"},{dot(z,z),q,std::sqrt(q)});
}

inline Report outofsample_kernel_center(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,1);double a=mean(column(x,0)),b=mean(column(x,1));require(std::abs(b-p[0])<=1e-10*std::max({1.,std::abs(b),std::abs(p[0])}),"Inconsistent training grand mean");Report o{{"TrainingIndex","CenteredCrossKernel"},{}};
    for(std::size_t i=0;i<x.size();++i)o.rows.push_back({double(i+1),x[i][0]-a-x[i][1]+p[0]});return o;
}
} }
