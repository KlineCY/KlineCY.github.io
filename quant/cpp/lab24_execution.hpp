#pragma once
#include "lab24_common.hpp"
#include <map>

namespace kline { namespace lab24 {
inline Vec ac_schedule(const Mat& x,const Vec& p){
    inputs(x,p,1,3);require(x.size()==1,"One parent order required");double Q=positive(x[0][0]),eta=positive(p[1]),risk=p[2];require(risk>=0,"Negative risk penalty");auto n=integer(p[0],1,10000);
    if(n==1)return {Q};
    Vec d(n-1,2*eta+risk),rhs(n-1,0),u(n-2,-eta),inv(n+1,0);rhs[0]=eta*Q;inv[0]=Q;
    for(std::size_t i=1;i<d.size();++i){double k=-eta/positive(d[i-1]);d[i]-=k*u[i-1];rhs[i]-=k*rhs[i-1];}
    for(std::size_t i=d.size();i-->0;){double z=rhs[i]-(i+1<d.size()?u[i]*inv[i+2]:0);inv[i+1]=z/positive(d[i]);}
    Vec trades;for(std::size_t i=1;i<=n;++i)trades.push_back(inv[i-1]-inv[i]);return checked(trades);
}

inline Vec participation_schedule(const Mat& x,const Vec& p){
    inputs(x,p,1,2);double left=p[0],rate=p[1];require(left>=0&&rate>=0&&rate<=1,"Invalid order or cap");Vec q;
    for(const auto& r:x){require(r[0]>=0,"Negative market volume");double fill=std::min(left,rate*r[0]);q.push_back(fill);left-=fill;}
    q.push_back(left);return checked(q);
}

inline Vec queue_fifo(const Mat& x,const Vec& p){
    inputs(x,p,2,2);double ahead=p[0],remaining=p[1],total=0;require(ahead>=0&&remaining>=0,"Negative initial queue");
    for(const auto& r:x){require(r[0]>=0&&r[1]>=0&&r[0]<=ahead+1e-10,"Invalid queue event");ahead=std::max(0.0,ahead-r[0]);double before=std::min(ahead,r[1]);ahead-=before;double f=std::min(remaining,r[1]-before);remaining-=f;total+=f;}
    return checked({total,remaining,ahead});
}

inline Vec multi_venue_sweep(const Mat& x,const Vec& p){
    inputs(x,p,3,1);double left=p[0],cost=0;require(left>=0,"Negative order");std::vector<std::size_t> ix(x.size());std::iota(ix.begin(),ix.end(),0);
    for(const auto& r:x){positive(r[0]);require(r[1]>=0&&r[0]+r[2]>0,"Invalid venue quote");}
    std::stable_sort(ix.begin(),ix.end(),[&](std::size_t a,std::size_t b){return x[a][0]+x[a][2]<x[b][0]+x[b][2];});
    Vec q(x.size(),0);for(auto i:ix){q[i]=std::min(left,x[i][1]);left-=q[i];cost+=q[i]*(x[i][0]+x[i][2]);}
    Vec out={cost,p[0]-left,left};out.insert(out.end(),q.begin(),q.end());return checked(out);
}

inline Vec impact_decay_fit(const Mat& x,const Vec& p){
    inputs(x,p,2,0,3);Vec t=col(x,0),y;for(std::size_t i=0;i<x.size();++i){require(t[i]>=0&&(i==0||t[i]>t[i-1]),"Invalid elapsed time");y.push_back(std::log(positive(x[i][1])));}
    double b=covariance(t,y,false)/positive(variance(t,false)),a=mean(y)-b*mean(t),k=positive(-b);
    return checked({std::exp(a),k,std::log(2.0)/k});
}
} }
