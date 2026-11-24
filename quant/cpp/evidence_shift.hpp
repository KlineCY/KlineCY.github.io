#pragma once
#include "evidence_common.hpp"
namespace kline { namespace evidence {
inline Report discrete_shift_weight(const Mat& x,const Vec& p){
    validate(x,2,3,3);parameters(p,0);std::map<double,Vec> m;double ns=0,nt=0;
    for(const auto& r:x){require(r[0]>=0&&r[0]==std::floor(r[0])&&(r[1]==0||r[1]==1),"Invalid category or domain");
        auto& a=m[r[0]];if(a.empty())a=Vec(3,0);if(r[1]==0){++a[0];a[2]+=r[2];++ns;}else{++a[1];++nt;}}
    positive(ns);positive(nt);double total=0,sq=0,loss=0;Report o{{"Category","SourceCount","TargetCount","Weight","SourceMeanLossOrTotalWeightedLoss","EffectiveSampleSize"},{}};
    for(const auto& v:m){const auto& a=v.second;positive(a[0]);double w=(a[1]/nt)/(a[0]/ns);total+=w*a[0];sq+=w*w*a[0];loss+=w*a[2];
        o.rows.push_back({v.first,a[0],a[1],w,a[2]/a[0],w>0?a[0]:0});}
    o.rows.push_back({-1,ns,nt,total/ns,loss/positive(total),total*total/positive(sq)});return o;
}

inline Report empirical_wasserstein(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,0);std::map<double,Vec> m;double n=0,k=0,ms=0,mt=0;
    for(const auto& r:x){require(r[1]==0||r[1]==1,"Binary domain required");auto& a=m[r[0]];if(a.empty())a=Vec(2,0);++a[std::size_t(r[1])];if(r[1]==0){++n;ms+=r[0];}else{++k;mt+=r[0];}}
    positive(n);positive(k);double F=0,G=0,W=0,D=0,prev=m.begin()->first;
    for(const auto& a:m){W+=(a.first-prev)*std::abs(F-G);F+=a.second[0]/n;G+=a.second[1]/k;D=std::max(D,std::abs(F-G));prev=a.first;}
    return scalar({"Wasserstein1","KSDistance","SourceN","TargetN","PooledRange","MeanShift"},{W,D,n,k,m.rbegin()->first-m.begin()->first,mt/k-ms/n});
}

inline Report adaptive_prediction_set(const Mat& x,const Vec& p){
    auto d=validate(x,2,4,34)-2;parameters(p,1);require(p[0]>0&&p[0]<1,"Invalid alpha");
    auto score=[&](const Vec& r,std::size_t j){double s=0;for(std::size_t k=0;k<d;++k)if(r[k+2]>=r[j+2])s+=r[k+2];return std::min(1.0,s);};
    Vec cal;bool testing=false;
    for(const auto& r:x){require(r[0]==0||r[0]==1,"Invalid split flag");auto y=count(r[1],0,d-1);double z=0;
        for(std::size_t j=0;j<d;++j){require(r[j+2]>=0&&r[j+2]<=1,"Invalid probability");z+=r[j+2];}require(std::abs(z-1)<1e-10,"Probability row must sum to one");
        if(r[0]==1)testing=true;else{require(!testing,"Calibration must precede test");cal.push_back(score(r,y));}}
    require(!cal.empty()&&testing,"Calibration and test rows required");auto rank=std::size_t(std::ceil((cal.size()+1)*(1-p[0])));std::sort(cal.begin(),cal.end());double q=rank>cal.size()?1:cal[rank-1];
    Report o{{"Period","Label","Threshold","SetSize","Covered"},{}};
    for(std::size_t i=0;i<x.size();++i)if(x[i][0]==1){auto y=std::size_t(x[i][1]);double n=0;for(std::size_t j=0;j<d;++j)if(score(x[i],j)<=q+1e-12)++n;o.rows.push_back({double(i),double(y),q,n,double(score(x[i],y)<=q+1e-12)});}
    return o;
}

inline Report risk_coverage_curve(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,0);std::map<double,Vec,std::greater<double>> groups;
    for(const auto& r:x){require(r[0]>=0&&r[0]<=1&&(r[1]==0||r[1]==1),"Invalid confidence or error");auto& a=groups[r[0]];if(a.empty())a=Vec(2,0);++a[0];a[1]+=r[1];}
    double n=0,e=0;Report o{{"Threshold","Accepted","Coverage","Errors","SelectiveRisk"},{}};
    for(const auto& a:groups){n+=a.second[0];e+=a.second[1];o.rows.push_back({a.first,n,n/x.size(),e,e/n});}return o;
}
} }
