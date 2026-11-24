#pragma once
#include "evidence_common.hpp"
namespace kline { namespace methods_counterparty {
using namespace evidence;
inline Report discrete_cva(const Mat& x,const Vec& p){
    validate(x,1,3,3);
    parameters(p,1);
    require(p[0]>=0&&p[0]<=1,"Invalid LGD");
    double prev=1,total=0;
    Report o{{"Period","MarginalPD","Contribution","CumulativeCVA"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){auto&r=x[i];
    positive(r[0]);
    require(r[1]>=0&&r[2]>=0&&r[2]<=prev,"Invalid exposure or survival");
    double dp=prev-r[2],v=p[0]*r[0]*r[1]*dp;
    total+=v;
    prev=r[2];
    o.rows.push_back({double(i+1),dp,v,total});
    }return o;
    
}

inline Report joint_default(const Mat& x,const Vec& p){
    validate(x,1,3,3);
    parameters(p,0);
    Report o{{"Neither","Only1","Only2","Both","Correlation"},{}};
    for(auto&r:x){double a=r[0],b=r[1],c=r[2];
    require(a>0&&a<1&&b>0&&b<1&&c>=std::max(0.,a+b-1)&&c<=std::min(a,b),"Invalid joint probability");
    o.rows.push_back({1-a-b+c,a-c,b-c,c,(c-a*b)/std::sqrt(a*(1-a)*b*(1-b))});
    }return o;
    
}

inline Report tranche_loss(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,2);
    double a=p[0],b=p[1];
    require(a>=0&&b>a&&b<=1,"Invalid attachment/detachment");
    double s=0,m=0,t=0;
    for(auto&r:x){require(r[0]>=0&&r[0]<=1&&r[1]>=0&&r[1]<=1,"Invalid loss or probability");
    s+=r[1];
    m+=r[0]*r[1];
    t+=std::min(std::max(r[0]-a,0.),b-a)*r[1];
    }require(std::abs(s-1)<1e-9,"Probability sum");
    return scalar({"ExpectedPortfolioLoss","ExpectedTrancheLoss","TrancheLossRate"},{m,t,t/(b-a)});
    
}

inline Report constant_hazard_bond(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,4);
    double N=positive(p[0]),r=p[1],lambda=p[2],rec=p[3];
    require(lambda>=0&&rec>=0&&rec<=1,"Invalid hazard or recovery");
    double k=r+lambda;
    Report o{{"Horizon","Survival","SurvivalLeg","RecoveryLeg","Price"},{}};
    for(auto&a:x){double t=a[0];
    require(t>=0,"Negative tenor");
    double s=std::exp(-lambda*t),v=N*std::exp(-k*t),u=N*rec*lambda*(k==0?t:-std::expm1(-k*t)/k);
    o.rows.push_back({t,s,v,u,v+u});
    }return o;
    
}
} }
