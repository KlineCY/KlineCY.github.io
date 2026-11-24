#pragma once
#include "evidence_common.hpp"
namespace kline { namespace evidence {
inline Report call_auction_grid(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,1);positive(p[0]);std::map<double,Vec> m;
    double B=0,A=0;
    for(const auto& r:x){require(r[0]==1||r[0]==-1,"Invalid order side");positive(r[1]);positive(r[2]);
        auto& a=m[r[1]];if(a.empty())a=Vec(2,0);if(r[0]==1){a[0]+=r[2];B+=r[2];}else a[1]+=r[2];}
    Report o{{"Price","Demand","Supply","Matched","Imbalance","Selected"},{}};
    std::tuple<double,double,double,double> best;std::size_t at=0;
    for(const auto& a:m){A+=a.second[1];double match=std::min(B,A),imb=B-A;
        auto k=std::make_tuple(match,-std::abs(imb),-std::abs(a.first-p[0]),-a.first);
        if(o.rows.empty()||k>best){best=k;at=o.rows.size();}
        o.rows.push_back({a.first,B,A,match,imb,0});B-=a.second[0];}
    o.rows[at][5]=1;return o;
}

inline Report auction_pro_rata(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,1);positive(p[0]);double B=0,A=0;
    std::map<double,std::vector<std::size_t>,std::greater<double>> buy;std::map<double,std::vector<std::size_t>> sell;
    for(std::size_t i=0;i<x.size();++i){const auto& r=x[i];require(r[0]==1||r[0]==-1,"Invalid side");positive(r[1]);positive(r[2]);
        if(r[0]==1&&r[1]>=p[0]){B+=r[2];buy[r[1]].push_back(i);}if(r[0]==-1&&r[1]<=p[0]){A+=r[2];sell[r[1]].push_back(i);}}
    Vec fill(x.size(),0);double Q=std::min(B,A);
    auto allocate=[&](const auto& groups){double left=Q;for(const auto& g:groups){double total=0;for(auto i:g.second)total+=x[i][2];double ratio=std::min(1.0,std::max(0.0,left)/positive(total));for(auto i:g.second)fill[i]=ratio*x[i][2];left-=std::min(left,total);}};
    allocate(buy);allocate(sell);Report o{{"Order","Side","Price","Quantity","Filled","Remaining"},{}};
    for(std::size_t i=0;i<x.size();++i){o.rows.push_back({double(i),x[i][0],x[i][1],x[i][2],fill[i],x[i][2]-fill[i]});}
    return o;
}

inline Report state_event_forecast(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,2);auto L=count(p[0],1,x.size()-1);double a=positive(p[1]);std::map<double,Vec> n;
    Report o{{"Period","State","ActualMove","Down","Flat","Up","ExpectedMove","ActualClassProbability","Brier"},{}};
    for(std::size_t t=0;t<x.size();++t){const auto& r=x[t];require(r[0]>=0&&std::floor(r[0])==r[0],"Invalid state");require(r[1]==-1||r[1]==0||r[1]==1,"Invalid move");
        auto& v=n[r[0]];if(v.empty())v=Vec(3,0);auto j=std::size_t(r[1]+1);double den=sum(v)+3*a;
        if(t>=L){Vec q(3);double loss=0;for(std::size_t k=0;k<3;++k){q[k]=(v[k]+a)/den;double e=q[k]-double(k==j);loss+=e*e;}
            o.rows.push_back({double(t),r[0],r[1],q[0],q[1],q[2],q[2]-q[0],q[j],loss});}
        ++v[j];
    }
    return o;
}

inline Report delayed_entry_survival(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,0);std::map<double,Vec> events;
    for(const auto& r:x){require(r[0]>=0&&r[1]>=r[0]&&(r[2]==0||r[2]==1),"Invalid observation interval");
        auto& a=events[r[0]];if(a.empty())a=Vec(3,0);++a[0];auto& b=events[r[1]];if(b.empty())b=Vec(3,0);++b[r[2]==1?1:2];}
    double risk=0,s=1;Report o{{"Time","AtRisk","Events","Censored","Survival"},{}};
    for(const auto& a:events){const auto& v=a.second;risk+=v[0];require(v[1]+v[2]<=risk,"Risk set underflow");
        if(v[1]+v[2]>0){s*=1-v[1]/positive(risk);o.rows.push_back({a.first,risk,v[1],v[2],s});}risk-=v[1]+v[2];}
    return o;
}
} }
