#pragma once
#include "applied_common.hpp"
namespace applied_forecast {
Vec brier_groups(const Vec& x){pairs(x);std::map<double,Vec> g;double bs=0,avg=0;for(size_t i=0;i<x.size();i+=2){prob(x[i]);binary(x[i+1]);g[x[i]].push_back(x[i+1]);bs+=sq(x[i]-x[i+1]);avg+=x[i+1];}double n=x.size()/2.,rel=0,res=0;avg/=n;for(const auto& a:g){double y=mean(a.second),w=a.second.size()/n;rel+=w*sq(a.first-y);res+=w*sq(y-avg);}double u=avg*(1-avg);return {bs/n,rel,res,u,bs/n-rel+res-u};}

Vec pinball(const Vec& x){check(x.size()>=3&&x.size()%2==1);double t=x[0],loss=0,u=0,o=0;check(t>0&&t<1);for(size_t i=1;i<x.size();i+=2){double e=x[i]-x[i+1];loss+=e>=0?t*e:(t-1)*e;u+=e>0;o+=e<0;}return {loss/((x.size()-1)/2),u,o};}

Vec ensemble_crps(const Vec& x){check(x.size()>=3);Vec s(x.begin()+1,x.end());std::sort(s.begin(),s.end());double a=0,b=0,m=s.size();for(size_t i=0;i<s.size();++i){a+=std::abs(s[i]-x[0]);b+=(2.*i-m+1)*s[i];}a/=m;b/=m*m;return {a-b,a,b};}

Vec split_conformal(const Vec& x){check(x.size()>=2&&x[0]>0&&x[0]<1);Vec s(x.begin()+1,x.end());for(double z:s)check(z>=0);std::sort(s.begin(),s.end());size_t k=static_cast<size_t>(std::ceil((s.size()+1)*(1-x[0])-1e-12));check(k>0&&k<=s.size());return {double(k),s[k-1],1-x[0]};}

Vec isotonic_fit(const Vec& x){pairs(x);struct B{double sw,sy;size_t n;};std::vector<B> b;for(size_t i=0;i<x.size();i+=2){check(x[i+1]>0);b.push_back({x[i+1],x[i]*x[i+1],1});while(b.size()>1){auto r=b.back(),l=b[b.size()-2];if(l.sy/l.sw<=r.sy/r.sw)break;b.pop_back();b.back()={l.sw+r.sw,l.sy+r.sy,l.n+r.n};}}Vec out;for(auto z:b)out.insert(out.end(),z.n,z.sy/z.sw);return out;}

Vec interval_score(const Vec& x){check(x.size()>=4&&(x.size()-1)%3==0&&x[0]>0&&x[0]<1);double a=x[0],s=0,c=0,w=0,n=(x.size()-1)/3.;for(size_t i=1;i<x.size();i+=3){double l=x[i],u=x[i+1],y=x[i+2];check(l<=u);w+=u-l;s+=u-l+2/a*(std::max(0.,l-y)+std::max(0.,y-u));c+=l<=y&&y<=u;}return {s/n,c/n,w/n};}

Vec frozen_calibration(const Vec& x){check(x.size()>=7);size_t n=integer(x[0],2,10000);check(x.size()>=1+2*n+2&&(x.size()-1-2*n)%2==0);Vec p,y;for(size_t i=0;i<n;++i){double a=x[1+2*i],b=x[2+2*i];prob(a);binary(b);if(i)check(a>p.back());p.push_back(a);y.insert(y.end(),{b,1});}Vec fit=isotonic_fit(y);double raw=0,cal=0,m=(x.size()-1-2*n)/2.;size_t blocks=1;for(size_t i=1;i<fit.size();++i)blocks+=fit[i]!=fit[i-1];for(size_t i=1+2*n;i<x.size();i+=2){prob(x[i]);binary(x[i+1]);auto it=std::upper_bound(p.begin(),p.end(),x[i]);size_t j=it==p.begin()?0:size_t(it-p.begin()-1);raw+=sq(x[i]-x[i+1]);cal+=sq(fit[j]-x[i+1]);}return {raw/m,cal/m,(raw-cal)/m,double(blocks)};}
}
