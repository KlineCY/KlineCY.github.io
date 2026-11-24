#pragma once
#include "system_research_common.hpp"
#include <map>
#include <set>
namespace kline { namespace system_research {
inline Report corporate_action_ledger(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,1);double q=positive(p[0]),prev=positive(x[0][0]),V=q*prev;require(x[0][1]==1&&x[0][2]==0,"First row is initial close");Report o{{"Time","Close","Shares","DividendCash","TotalValue","TotalReturn","PriceReturn","ReinvestmentShares"},{}};for(std::size_t t=0;t<x.size();++t){auto&r=x[t];double P=positive(r[0]),a=positive(r[1]),d=r[2];require(d>=0,"Nonnegative dividend required");q*=a;double cash=q*d,value=positive(q*P+cash),ret=value/V-1,pr=a*P/prev-1,reinvest=q+cash/P;o.rows.push_back({double(t+1),P,q,cash,value,ret,pr,reinvest});q=reinvest;V=value;prev=P;}return o;
}

inline Report survivorship_cohort(const Mat& x,const Vec& p){
    validate(x,2,4,4);parameters(p,0);double mass=0,full=0,s=0,sw=0;std::set<double> ids;for(auto&r:x){require(ids.insert(r[0]).second&&r[1]>=0&&r[2]>=-1&&(r[3]==0||r[3]==1),"Invalid cohort row");mass+=r[1];full+=r[1]*r[2];if(!r[3]){sw+=r[1];s+=r[1]*r[2];}}require(std::abs(mass-1)<=1e-10,"Weights must sum to one");s/=positive(sw);Report o{{"Asset","FormationWeight","Return","Contribution","FullReturn","SurvivorReturn","SurvivorWeight"},{}};for(auto&r:x)o.rows.push_back({r[0],r[1],r[2],r[1]*r[2],full,s,sw});return o;
}

inline Report lagged_asof_panel(const Mat& x,const Vec& p){
    validate(x,2,4,4);parameters(p,1);double maxage=p[0];require(maxage>=0,"Nonnegative age limit required");std::map<double,std::map<double,double>> data;Mat query;for(auto&r:x){require(r[0]==0||r[0]==1,"Invalid record type");if(r[0])require(data[r[1]].emplace(r[2],r[3]).second,"Duplicate availability time");else query.push_back(r);}require(!query.empty(),"No decision queries");Report o{{"Asset","DecisionTime","AvailableTime","Age","Value","Missing"},{}};for(auto&r:query){auto&v=data[r[1]];auto it=v.upper_bound(r[2]);if(it==v.begin()){o.rows.push_back({r[1],r[2],0,0,0,1});continue;}--it;double a=r[2]-it->first;bool miss=a>maxage;o.rows.push_back({r[1],r[2],it->first,a,miss?0:it->second,double(miss)});}return o;
}

inline Report session_ohlcv_bars(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,3);double a=p[0],b=p[1],dt=positive(p[2]);require(b>a,"Invalid session");double raw=std::ceil((b-a)/dt);auto n=count(raw,1,250000);Mat v(n,Vec(8,0));double prev=a;for(auto&r:x){double t=r[0],price=positive(r[1]),vol=positive(r[2]);require(t>=prev&&t>=a&&t<b,"Trade outside ordered session");auto k=std::min(n-1,std::size_t(std::floor((t-a)/dt)));auto&z=v[k];if(z[6]==0)z[0]=z[1]=z[2]=price;z[1]=std::max(z[1],price);z[2]=std::min(z[2],price);z[3]=price;z[4]+=vol;z[5]+=price*vol;++z[6];prev=t;}Report o{{"BarStart","BarEnd","Open","High","Low","Close","Volume","VWAP","TradeCount","Empty"},{}};for(std::size_t k=0;k<n;++k){auto&z=v[k];o.rows.push_back({a+k*dt,std::min(b,a+(k+1)*dt),z[0],z[1],z[2],z[3],z[4],z[4]>0?z[5]/z[4]:0,z[6],double(z[6]==0)});}return o;
}

inline Report cross_sectional_weighted_standardize(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,0);struct G{double w=0,m=0,v=0;};std::map<double,G> g;for(auto&r:x){auto&z=g[r[0]];double w=positive(r[2]);z.w+=w;z.m+=w*r[1];}for(auto&kv:g)kv.second.m/=kv.second.w;for(auto&r:x){auto&z=g[r[0]];z.v+=r[2]*std::pow(r[1]-z.m,2)/z.w;}Report o{{"Row","Group","Value","Weight","GroupMean","GroupStd","Z","Degenerate"},{}};for(std::size_t i=0;i<x.size();++i){auto&r=x[i];auto z=g.at(r[0]);double s=std::sqrt(z.v);o.rows.push_back({double(i+1),r[0],r[1],r[2],z.m,s,s>0?(r[1]-z.m)/s:0,double(s==0)});}return o;
}

inline Report label_interval_purge_folds(const Mat& x,const Vec& p){
    validate(x,2,3,3);parameters(p,1);double h=p[0];require(h>=0&&double(x.size())*x.size()<=5e7,"Invalid purge controls");std::map<double,std::vector<std::size_t>> fold;for(std::size_t i=0;i<x.size();++i){auto&r=x[i];require(r[1]>=r[0]&&r[2]==std::floor(r[2]),"Invalid label interval");fold[r[2]].push_back(i);}require(fold.size()>=2&&double(fold.size())*x.size()<=1e6,"Invalid fold count");Report o{{"TestFold","Row","Role","Overlap","Embargo","Start","End"},{}};for(auto&kv:fold)for(std::size_t i=0;i<x.size();++i){auto&r=x[i];bool test=r[2]==kv.first,over=false,emb=false;if(!test)for(auto j:kv.second){auto&s=x[j];over|=r[0]<=s[1]&&s[0]<=r[1];emb|=r[0]>s[1]&&r[0]<=s[1]+h;}emb=emb&&!over;o.rows.push_back({kv.first,double(i+1),test?2.:over||emb?0.:1.,double(over),double(emb),r[0],r[1]});}return o;
}
} }
