#pragma once
#include "applied_common.hpp"
namespace applied_accounts {
Vec twr_exact(const Vec& x){check(x.size()>=3&&x.size()%2==1&&x[0]>0);double v=x[0],g=1,f=0;for(size_t i=1;i<x.size();i+=2){check(x[i]>0&&x[i]+x[i+1]>0);g*=x[i]/v;f+=x[i+1];v=x[i]+x[i+1];}return {g-1,v,f,v-x[0]-f};}

Vec modified_dietz(const Vec& x){check(x.size()>=2&&x.size()%2==0&&x[0]>0&&x[1]>0);double g=x[1]-x[0],c=x[0];for(size_t i=2;i<x.size();i+=2){prob(x[i+1]);g-=x[i];c+=x[i]*x[i+1];}check(c>0);return {g/c,g,c};}

Vec mwr_continuous(const Vec& x){check(x.size()>=4&&x.size()%2==0&&x[0]>0&&x[1]>0);for(size_t i=2;i<x.size();i+=2)check(x[i]>=0&&x[i]<x[0]&&x[i+1]>0);auto f=[&](double r){double s=0;for(size_t i=2;i<x.size();i+=2)s+=x[i+1]*std::exp(r*(x[0]-x[i]));return s;};double a=-2,b=2;check(f(a)<=x[1]&&f(b)>=x[1]);for(int i=0;i<100;++i){double m=(a+b)/2;if(f(m)<x[1])a=m;else b=m;}double r=(a+b)/2;return {r,std::expm1(r),f(r)-x[1]};}

Vec high_water_fee(const Vec& x){check(x.size()>=3&&x[1]>0);prob(x[0]);double nav=x[1],h=nav,f=0,n=0;for(size_t i=2;i<x.size();++i){check(x[i]>0);double gross=nav*x[i],fee=x[0]*std::max(0.,gross-h);nav=gross-fee;f+=fee;n+=fee>0;h=std::max(h,nav);}return {f,nav,h,n};}

Vec futures_margin(const Vec& x){check(x.size()>=6&&x[1]>0&&x[2]>=0&&x[3]>=0&&std::floor(x[0])==x[0]);double c=x[2],lo=c;for(size_t i=5;i<x.size();++i){c+=x[0]*x[1]*(x[i]-x[i-1]);lo=std::min(lo,c);}return {c,c-x[2],lo,std::max(0.,x[3]-lo)};}

Vec borrow_accrual(const Vec& x){check(!x.empty()&&x.size()%3==0);Vec out;double s=0;for(size_t i=0;i<x.size();i+=3){check(x[i]>=0&&x[i+1]>=0&&x[i+2]>0);double f=x[i]*x[i+1]*x[i+2];out.push_back(f);s+=f;}out.push_back(s);return out;}

Vec flow_performance_pipeline(const Vec& x){check(x.size()>=3&&x.size()%2==1&&x[0]>0);size_t n=(x.size()-1)/2;Vec t{x[0]},d{x[0],0};double v=x[0];for(size_t i=0;i<n;++i){double r=x[1+2*i],f=x[2+2*i];check(1+r>0);double pre=v*(1+r);v=pre+f;check(v>0);t.insert(t.end(),{pre,f});d.insert(d.end(),{f,1-double(i+1)/n});}d[1]=v;Vec a=twr_exact(t),b=modified_dietz(d);return {a[0],b[0],a[3],v,a[0]-b[0]};}
}
