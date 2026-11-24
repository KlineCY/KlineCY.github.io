#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_engineering {
using namespace evidence;
inline Report bitemporal_asof(const Mat& x,const Vec& p){
    validate(x,1,5,5);parameters(p,1);require(p[0]>=0,"Negative staleness limit");std::map<std::tuple<double,double,double>,double> seen;Mat data;std::vector<std::size_t> q;
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];require((r[0]==0||r[0]==1)&&r[1]==std::floor(r[1]),"Invalid record type or asset");if(r[0]==0){require(r[2]<=r[3],"Availability precedes effective time");auto key=std::make_tuple(r[1],r[2],r[3]);auto z=seen.emplace(key,r[4]);require(z.second||z.first->second==r[4],"Conflicting revision");if(z.second)data.push_back(r);}else{require(r[2]==r[3],"Query time fields must agree");q.push_back(i);}}
    require(!q.empty()&&double(q.size())*data.size()<=2e7,"Invalid query count or work limit");Report o{{"QueryRow","Asset","DecisionTime","Found","EffectiveTime","AvailableTime","Value"},{}};
    for(auto i:q){auto&r=x[i];const Vec* best=nullptr;for(auto&a:data)if(a[1]==r[1]&&a[2]<=r[3]&&a[3]<=r[3]&&(!best||std::tie(a[2],a[3])>std::tie((*best)[2],(*best)[3])))best=&a;bool ok=best&&r[3]-(*best)[3]<=p[0];o.rows.push_back({double(i+1),r[1],r[3],double(ok),ok?(*best)[2]:0,ok?(*best)[3]:0,ok?(*best)[4]:0});}return o;
}

inline Report event_watermark(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,1);require(p[0]>=0,"Negative lateness tolerance");double h=x[0][0];Report o{{"Arrival","EventTime","PriorWatermark","Late","UpdatedWatermark"},{}};
    for(std::size_t i=0;i<x.size();++i){double t=x[i][0],old=h-p[0];bool late=i>0&&t<old;h=std::max(h,t);o.rows.push_back({double(i+1),t,old,double(late),h-p[0]});}return o;
}

inline Report event_idempotency(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,0);std::map<double,Vec> seen;double q=0,c=0;Report o{{"Row","EventID","Duplicate","Position","CashChange"},{}};
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];require(r[0]>=0&&r[0]<=9007199254740991.&&r[0]==std::floor(r[0]),"Invalid exact event ID");positive(r[2]);Vec v(r.begin()+1,r.end());auto z=seen.emplace(r[0],v);require(z.second||z.first->second==v,"Conflicting duplicate event");if(z.second){q+=r[1];c-=r[1]*r[2]+r[3];}o.rows.push_back({double(i+1),r[0],double(!z.second),q,c});}return o;
}

inline Report tick_grid(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,2);double d=positive(p[0]),e=p[1];require(e>=0&&e<d/2,"Invalid grid tolerance");Report o{{"Price","NearestTick","GridPrice","AbsoluteError","OnGrid"},{}};
    for(auto&r:x){require(r[0]>=0,"Negative price");double z=r[0]/d;require(z<=9007199254740991.,"Tick integer range exceeded");double k=std::round(z),v=k*d,a=std::abs(r[0]-v);o.rows.push_back({r[0],k,v,a,double(a<=e)});}return o;
}
} }
