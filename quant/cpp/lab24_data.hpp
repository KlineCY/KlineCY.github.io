#pragma once
#include "lab24_common.hpp"
#include <map>

namespace kline { namespace lab24 {
inline Vec asof_age(const Mat& x,const Vec& p){
    inputs(x,p,3,1);require(p[0]>=0,"Negative max age");Vec a=col(x,0),out;
    for(std::size_t i=1;i<a.size();++i)require(a[i]>a[i-1],"Available times must increase");
    for(const auto& r:x){auto it=std::upper_bound(a.begin(),a.end(),r[2]);if(it==a.begin()){out.insert(out.end(),{0,0,0});continue;}auto j=std::size_t(it-a.begin()-1);double age=r[2]-a[j];if(age>p[0])out.insert(out.end(),{0,0,0});else out.insert(out.end(),{1,x[j][1],age});}
    return checked(out);
}

inline Vec purged_mask(const Mat& x,const Vec& p){
    inputs(x,p,2,3);require(p[0]<=p[1]&&p[2]>=0,"Invalid test interval");Vec out;
    for(const auto& r:x){require(r[1]>=r[0],"Reversed label interval");bool overlap=r[0]<=p[1]&&r[1]>=p[0],embargo=r[0]>p[1]&&r[0]<=p[1]+p[2];out.push_back(overlap||embargo?0:1);}
    return out;
}

inline Vec label_uniqueness(const Mat& x,const Vec& p){
    inputs(x,p,2,0);std::size_t T=0;
    for(const auto& r:x){auto a=integer(r[0],0,1000000),b=integer(r[1],0,1000000);require(a<=b,"Reversed event interval");T=std::max(T,b);}
    Vec delta(T+2,0),prefix(T+2,0);
    for(const auto& r:x){++delta[std::size_t(r[0])];--delta[std::size_t(r[1])+1];}
    double c=0;for(std::size_t t=0;t<=T;++t){c+=delta[t];prefix[t+1]=prefix[t]+(c>0?1/c:0);}
    Vec out;for(const auto& r:x){auto a=std::size_t(r[0]),b=std::size_t(r[1]);out.push_back((prefix[b+1]-prefix[a])/double(b-a+1));}return checked(out);
}

inline Vec revision_latest(const Mat& x,const Vec& p){
    inputs(x,p,3,1);std::map<double,std::pair<double,double>> latest;
    for(const auto& r:x){require(r[1]>=r[0],"Publication precedes reference");if(r[1]>p[0])continue;auto it=latest.find(r[0]);if(it==latest.end()||r[1]>it->second.first)latest[r[0]]={r[1],r[2]};else if(r[1]==it->second.first)require(r[2]==it->second.second,"Conflicting vintage");}
    require(!latest.empty(),"No available vintage");Vec out;for(const auto& q:latest)out.insert(out.end(),{q.first,q.second.first,q.second.second});return checked(out);
}

inline Vec event_time_bars(const Mat& x,const Vec& p){
    inputs(x,p,2,1);double cap=positive(p[0]),v=0,turn=0,o=0,h=0,l=0,c=0;Vec out;
    auto emit=[&](){require(out.size()<600000,"Too many volume bars");out.insert(out.end(),{o,h,l,c,v,turn/v});v=0;turn=0;};
    for(const auto& r:x){positive(r[0]);require(r[1]>=0,"Negative trade quantity");double remain=r[1];
        while(remain>0){if(v==0){o=h=l=r[0];}h=std::max(h,r[0]);l=std::min(l,r[0]);c=r[0];double q=std::min(remain,cap-v);require(q>0&&remain-q<remain,"Volume resolution too small");v+=q;turn+=q*r[0];remain-=q;if(v>=cap*(1-1e-12))emit();}
    }
    if(v>0)emit();require(!out.empty(),"No positive trade volume");return checked(out);
}
} }
