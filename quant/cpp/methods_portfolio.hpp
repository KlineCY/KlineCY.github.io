#pragma once
#include "evidence_common.hpp"
namespace kline { namespace methods_portfolio {
using namespace evidence;
inline Report active_factor_risk(const Mat& x,const Vec& p){
    validate(x,1,3,3);
    parameters(p,1);
    require(p[0]>=0,"Negative factor variance");
    double b=0,d=0;
    for(auto&r:x){require(r[2]>=0,"Negative specific variance");
    b+=r[0]*r[1];
    d+=r[0]*r[0]*r[2];
    }double f=b*b*p[0];
    return scalar({"FactorExposure","FactorVariance","SpecificVariance","TotalVariance"},{b,f,d,f+d});
    
}

inline Report variance_shapley(const Mat& x,const Vec& p){
    validate(x,2,2,2);
    parameters(p,6);
    require(x.size()==2,"Two assets required");
    for(int j:{0,3}){require(p[j]>=0&&p[j+1]>=0&&p[j]*p[j+1]>=p[j+2]*p[j+2],"PSD covariance required");
    }auto V=[&](int w,int k){double a=x[0][w],b=x[1][w];
    return a*a*p[k]+b*b*p[k+1]+2*a*b*p[k+2];
    };
    double a=V(0,0),b=V(1,0),c=V(0,3),d=V(1,3),u=.5*(b-a+d-c),v=.5*(c-a+d-b);
    return scalar({"OldVariance","NewVariance","WeightEffect","CovarianceEffect","Residual"},{a,d,u,v,d-a-u-v});
    
}

inline Report binary_kelly(const Mat& x,const Vec& p){
    validate(x,2,2,2);
    parameters(p,2);
    require(x.size()==2&&x[0][0]>0&&x[1][0]<0,"Positive and negative outcomes required");
    double a=x[0][0],b=-x[1][0],q=x[0][1];
    require(q>0&&q<1&&std::abs(q+x[1][1]-1)<1e-9,"Invalid probabilities");
    require(p[0]<=p[1]&&p[0]>-1/a&&p[1]<1/b,"Infeasible wealth interval");
    double w=(q*a-(1-q)*b)/(a*b),z=std::max(p[0],std::min(p[1],w));
    return scalar({"UnconstrainedWeight","ConstrainedWeight","ExpectedLog"},{w,z,q*std::log1p(a*z)+(1-q)*std::log1p(-b*z)});
    
}

inline Report liability_surplus(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,2);
    double a=positive(p[0]),b=positive(p[1]);
    Report o{{"Scenario","AssetValue","LiabilityValue","Surplus","FundingRatio"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){auto&r=x[i];
    require(r[0]>=-1,"Invalid asset return");
    double u=a*(1+r[0]),v=b*positive(1+r[1]);
    o.rows.push_back({double(i+1),u,v,u-v,u/v});
    }return o;
    
}
} }
