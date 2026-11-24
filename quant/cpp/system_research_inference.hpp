#pragma once
#include "system_research_common.hpp"
#include <map>
namespace kline { namespace system_research {
inline Report ols_hc3_inference(const Mat& x,const Vec& p){
    auto d=validate(x,3,2,33)-1;parameters(p,0);require(x.size()>d&&double(x.size())*d*d*d<=5e7,"Invalid HC3 design");Mat X,H(d,Vec(d,0));Vec y;for(auto&r:x){X.emplace_back(r.begin()+1,r.end());y.push_back(r[0]);add_outer(H,X.back(),1);}Vec b=fit_ridge(X,y,0,false),var(d,0);double press=0,lev=0;for(std::size_t i=0;i<x.size();++i){Vec v=solve_spd(H,X[i]);double h=dot(X[i],v),e=y[i]-dot(X[i],b);require(h>=-1e-10&&h<1-1e-12,"Unit leverage prevents HC3");double u=e/(1-h);lev+=h/x.size();press+=u*u;for(std::size_t j=0;j<d;++j)var[j]+=u*u*v[j]*v[j];}Report o{{"Feature","Coefficient","HC3Variance","HC3SE","MeanLeverage","PRESS"},{}};for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),b[j],var[j],std::sqrt(var[j]),lev,press});return o;
}

inline Report quantile_crossing_projection(const Mat& x,const Vec& p){
    auto d=validate(x,1,3,33)-1;parameters(p,2*d);for(std::size_t j=0;j<d;++j){require(p[j]>0&&p[j]<1&&(j==0||p[j]>p[j-1]),"Increasing quantile levels required");positive(p[d+j]);}Report o{{"Row","Actual","OriginalPinball","ProjectedPinball","ProjectionLoss","RawCrossings"},{}};for(std::size_t j=0;j<d;++j)o.header.push_back("Q"+std::to_string(j+1));struct Block{std::size_t start,end;double s,w;};auto loss=[](double e,double t){return e*(t-double(e<0));};for(std::size_t i=0;i<x.size();++i){auto&r=x[i];std::vector<Block> b;double raw=0,proj=0,dist=0,cross=0;for(std::size_t j=0;j<d;++j){raw+=loss(r[0]-r[j+1],p[j]);if(j&&r[j]>r[j+1])++cross;b.push_back({j,j+1,p[d+j]*r[j+1],p[d+j]});while(b.size()>1&&b[b.size()-2].s/b[b.size()-2].w>b.back().s/b.back().w){auto z=b.back();b.pop_back();b.back().end=z.end;b.back().s+=z.s;b.back().w+=z.w;}}Vec q(d);for(auto&z:b)for(std::size_t j=z.start;j<z.end;++j)q[j]=z.s/z.w;for(std::size_t j=0;j<d;++j){proj+=loss(r[0]-q[j],p[j]);dist+=p[d+j]*std::pow(q[j]-r[j+1],2);}Vec z{double(i+1),r[0],raw,proj,dist,cross};z.insert(z.end(),q.begin(),q.end());o.rows.push_back(z);}return o;
}

inline Report permutation_randomization(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,0);auto n=x.size();require(n<=40,"Exact randomization dimension limit");std::size_t k=0;double total=0,s=0;for(auto&r:x){require(r[1]==0||r[1]==1,"Binary treatment required");k+=std::size_t(r[1]);total+=r[0];s+=r[0]*r[1];}require(k>0&&k<n,"Both arms required");double obs=s/k-(total-s)/(n-k);double combos=1;for(std::size_t j=1;j<=std::min(k,n-k);++j)combos*=double(n-j+1)/j;require(combos<=2000000,"Too many assignments");double num=0,extreme=0,m=0,v=0;std::function<void(std::size_t,std::size_t,double)> rec=[&](std::size_t i,std::size_t left,double z){if(left==0){double t=z/k-(total-z)/(n-k);++num;extreme+=double(std::abs(t)+1e-12*std::max(1.,std::abs(obs))>=std::abs(obs));double e=t-m;m+=e/num;v+=e*(t-m);return;}for(std::size_t j=i;j+left<=n;++j)rec(j+1,left-1,z+x[j][0]);};rec(0,k,0);return scalar({"ObservedDifference","Assignments","ExtremeAssignments","ExactPValue","NullMean","NullVariance"},{obs,num,extreme,extreme/num,m,v/num});
}

inline Report kaplan_meier_rmst(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,1);double tau=positive(p[0]);std::map<double,Vec> groups;for(auto&r:x){require(r[0]>=0&&(r[1]==0||r[1]==1),"Invalid survival observation");auto&v=groups[r[0]];if(v.empty())v=Vec(2,0);++v[r[1]?0:1];}require(tau<=groups.rbegin()->first,"RMST horizon exceeds observation support");double n=x.size(),S=1,g=0,area=0,last=0;Report o{{"Time","AtRisk","Events","Censored","Survival","GreenwoodVariance","RMST"},{}};for(auto&kv:groups){double t=kv.first,d=kv.second[0],c=kv.second[1];if(t>tau)break;area+=(t-last)*S;S*=1-d/n;if(d>0&&n>d)g+=d/(n*(n-d));o.rows.push_back({t,n,d,c,S,S*S*g,area});n-=d+c;last=t;}if(last<tau)o.rows.push_back({tau,n,0,0,S,S*S*g,area+(tau-last)*S});return o;
}

inline Report bunching_rd_local(const Mat& x,const Vec& p){
    validate(x,4,2,2);parameters(p,3);double c=p[0],h=positive(p[1]),lam=p[2];require(lam>=0,"Nonnegative ridge required");Report o{{"Side","Intercept","Slope","WeightedSSE","EffectiveSample","Jump"},{}};for(int side=0;side<=1;++side){Mat X;Vec y;double n=0,w2=0;for(auto&r:x){double z=r[0]-c,w=std::max(0.,1-std::abs(z)/h);if(int(z>=0)==side&&w>0){double a=std::sqrt(w);X.push_back({a,a*z});y.push_back(a*r[1]);n+=w;w2+=w*w;}}require(X.size()>=2,"Both RD sides require support");Vec b=fit_ridge(X,y,lam,true);double ss=0;for(std::size_t i=0;i<y.size();++i)ss+=std::pow(y[i]-dot(X[i],b),2);o.rows.push_back({double(side),b[0],b[1],ss,n*n/w2,0});}double jump=o.rows[1][1]-o.rows[0][1];for(auto&r:o.rows)r[5]=jump;return o;
}

inline Report newey_west_long_run(const Mat& x,const Vec& p){
    validate(x,2,1,1);parameters(p,1);auto n=x.size(),L=count(p[0],0,n-1);require(double(n)*(L+1)<=5e7,"Long run variance work limit");Vec z=column(x,0);double m=sum(z)/n;for(double&v:z)v-=m;double v=squared(z)/n,s=v;for(std::size_t k=1;k<=L;++k){double c=0;for(std::size_t t=k;t<n;++t)c+=z[t]*z[t-k]/n;s+=2*(1-double(k)/(L+1))*c;}require(s>=-1e-12*std::max(1.,v),"Negative long run variance");s=std::max(0.,s);return scalar({"Mean","SampleCount","LongRunVariance","MeanSE","EffectiveSampleSize","Degenerate"},{m,double(n),s,std::sqrt(s/n),s>0?n*v/s:0,double(s==0)});
}
} }
