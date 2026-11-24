#pragma once
#include "evidence_common.hpp"
namespace kline { namespace evidence {
inline Report twfe_balanced(const Mat& x,const Vec& p){
    validate(x,4,4,4);parameters(p,0);std::map<double,Vec> unit,time;std::set<std::pair<double,double>> seen;Vec grand(2,0);
    for(const auto& r:x){require(seen.emplace(r[0],r[1]).second,"Duplicate panel cell");
        auto& a=unit[r[0]];auto& b=time[r[1]];if(a.empty())a=Vec(2,0);if(b.empty())b=Vec(2,0);
        for(std::size_t j=0;j<2;++j){a[j]+=r[j+2];b[j]+=r[j+2];grand[j]+=r[j+2];}}
    require(unit.size()>=2&&time.size()>=2&&unit.size()*time.size()==x.size(),"Balanced panel required");
    Mat z;double xx=0,xy=0;
    for(const auto& r:x){Vec v(2);for(std::size_t j=0;j<2;++j)v[j]=r[j+2]-unit[r[0]][j]/time.size()-time[r[1]][j]/unit.size()+grand[j]/x.size();xx+=v[0]*v[0];xy+=v[0]*v[1];z.push_back(v);}
    double b=xy/positive(xx),sse=0;for(const auto& v:z)sse+=(v[1]-b*v[0])*(v[1]-b*v[0]);
    return scalar({"Observations","Units","Periods","Coefficient","ResidualSSE","WithinTreatmentSS"},{double(x.size()),double(unit.size()),double(time.size()),b,sse,xx});
}

inline Report cohort_att(const Mat& x,const Vec& p){
    validate(x,4,4,4);parameters(p,0);std::map<double,std::map<double,double>> y;std::map<double,double> g;std::set<double> times,cohorts;
    for(const auto& r:x){require(r[1]>=0&&(r[2]==-1||r[2]>=0),"Invalid date or treatment date");
        auto a=g.emplace(r[0],r[2]);require(a.first->second==r[2],"Treatment cohort changed");require(y[r[0]].emplace(r[1],r[3]).second,"Duplicate panel cell");times.insert(r[1]);if(r[2]>=0)cohorts.insert(r[2]);}
    require(y.size()*times.size()==x.size(),"Balanced panel required");require(double(cohorts.size())*times.size()*y.size()<=5e7,"Cohort work limit exceeded");
    Report o{{"Cohort","Period","Baseline","TreatedN","ControlN","TreatedChange","ControlChange","ATT"},{}};
    for(double c:cohorts){auto it=times.find(c);require(it!=times.end()&&it!=times.begin(),"Treatment date and pre-period required");double base=*std::prev(it);
        for(;it!=times.end();++it){double t=*it,na=0,nb=0,a=0,b=0;
            for(const auto& v:y){double h=g.at(v.first),change=v.second.at(t)-v.second.at(base);
                if(h==c){++na;a+=change;}else if(h==-1||h>t){++nb;b+=change;}}
            positive(na);positive(nb);o.rows.push_back({c,t,base,na,nb,a/na,b/nb,a/na-b/nb});}}
    require(!o.rows.empty(),"No treated cohorts");return o;
}

inline Report local_linear_rd(const Mat& x,const Vec& p){
    validate(x,4,2,2);parameters(p,2);double c=p[0],h=positive(p[1]);Vec a(2),b(2),n(2,0);
    for(int side=0;side<2;++side){Mat A(2,Vec(2,0));Vec z(2,0);
        for(const auto& r:x){double u=r[0]-c,w=1-std::abs(u)/h;if(w<=0||int(u>=0)!=side)continue;++n[side];Vec v={1,u};
            for(std::size_t i=0;i<2;++i){z[i]+=w*v[i]*r[1];for(std::size_t j=0;j<2;++j)A[i][j]+=w*v[i]*v[j];}}
        require(n[side]>=2,"Insufficient local observations");auto beta=solve_spd(A,z);a[side]=beta[0];b[side]=beta[1];}
    return scalar({"LeftIntercept","RightIntercept","LeftSlope","RightSlope","Discontinuity","LeftN","RightN"},{a[0],a[1],b[0],b[1],a[1]-a[0],n[0],n[1]});
}

inline Report oaxaca_reference(const Mat& x,const Vec& p){
    auto d=validate(x,4,3,20)-1;parameters(p,0);Mat A[2]={Mat(d,Vec(d,0)),Mat(d,Vec(d,0))};Vec b[2]={Vec(d,0),Vec(d,0)},m[2]={Vec(d,0),Vec(d,0)};double n[2]={0,0},y[2]={0,0};
    for(const auto& r:x){require(r[0]==0||r[0]==1,"Binary group required");auto g=std::size_t(r[0]);Vec z={1};z.insert(z.end(),r.begin()+1,r.end()-1);++n[g];y[g]+=r.back();
        for(std::size_t i=0;i<d;++i){m[g][i]+=z[i];b[g][i]+=z[i]*r.back();for(std::size_t j=0;j<d;++j)A[g][i][j]+=z[i]*z[j];}}
    for(int g=0;g<2;++g){require(n[g]>=d,"Insufficient group observations");for(double& v:m[g])v/=n[g];y[g]/=n[g];}
    Vec beta0=solve_spd(A[0],b[0]),beta1=solve_spd(A[1],b[1]),dx=m[1],db=beta1;
    for(std::size_t j=0;j<d;++j){dx[j]-=m[0][j];db[j]-=beta0[j];}
    double composition=dot(dx,beta0),coefficient=dot(m[1],db),gap=y[1]-y[0];
    return scalar({"Mean0","Mean1","Gap","Composition","Coefficient","IdentityResidual","N0","N1"},{y[0],y[1],gap,composition,coefficient,gap-composition-coefficient,n[0],n[1]});
}
} }
