#pragma once
#include "applied_common.hpp"
namespace applied_liquidity {
Vec microprice(const Vec& x){size(x,4);double b=x[0],a=x[1],qb=x[2],qa=x[3];check(b>0&&a>b&&qb>0&&qa>0);double m=(a+b)/2,i=(qb-qa)/(qb+qa);return {m,a-b,i,(a*qb+b*qa)/(qb+qa)};}

Vec sweep_asks(const Vec& x){check(x.size()>=4&&x.size()%2==0&&x[0]>0&&x[1]>0);double left=x[0],n=0,last=0;for(size_t i=2;i<x.size();i+=2){double p=x[i],q=x[i+1];check(p>last&&q>0);last=p;if(p<=x[1]){double f=std::min(q,left);left-=f;n+=f*p;}}double f=x[0]-left;return {f,n,f>0?n/f:0,left};}

Vec pro_rata_lots(const Vec& x){check(x.size()>=2);long long q=integer(x[0],1,1000000),s=0;std::vector<long long> a,r,n;for(size_t i=1;i<x.size();++i){long long z=integer(x[i],1,1000000);n.push_back(z);s+=z;}check(q<=s);long long left=q;for(auto z:n){a.push_back(q*z/s);r.push_back(q*z%s);left-=a.back();}std::vector<size_t> ix(n.size());std::iota(ix.begin(),ix.end(),0);std::stable_sort(ix.begin(),ix.end(),[&](size_t i,size_t j){return r[i]>r[j];});for(long long i=0;i<left;++i)++a[ix[i]];return Vec(a.begin(),a.end());}

Vec auction_cross(const Vec& x){check(x.size()>=7&&(x.size()-1)%3==0&&x[0]>0);Vec p;for(size_t i=1;i<x.size();i+=3){check((x[i]==1||x[i]==-1)&&x[i+1]>0);integer(x[i+2],1,1000000);p.push_back(x[i+1]);}std::sort(p.begin(),p.end());std::array<double,4> best{1e100,1e100,1e100,1e100};Vec out;for(double k:p){double b=0,a=0;for(size_t i=1;i<x.size();i+=3){if(x[i]==1&&x[i+1]>=k)b+=x[i+2];if(x[i]==-1&&x[i+1]<=k)a+=x[i+2];}double v=std::min(b,a);std::array<double,4> z{-v,std::abs(b-a),std::abs(k-x[0]),k};if(z<best){best=z;out={k,v,b-a,b,a};}}check(out[1]>0);return out;}

Vec ofi_event(const Vec& x){size(x,8);for(size_t i:{size_t(0),size_t(4)})check(x[i]>0&&x[i+1]>x[i]&&x[i+2]>0&&x[i+3]>0);double b=(x[4]>=x[0]?x[6]:0)-(x[4]<=x[0]?x[2]:0),a=-(x[5]<=x[1]?x[7]:0)+(x[5]>=x[1]?x[3]:0);return {b,a,b+a};}

Vec passive_markout(const Vec& x){size(x,6);double s=x[0],q=x[1],p=x[2],m=x[3],h=x[4],f=x[5];check((s==1||s==-1)&&q>0&&p>0&&m>0&&h>0);double g=s*q*(m-p),v=s*q*(h-p);return {g,v,v-g,q*f,v-q*f};}

Vec sweep_shortfall(const Vec& x){check(x.size()>=7&&(x.size()-5)%2==0&&x[2]>0&&x[3]>0&&x[4]>=0);Vec q{x[0],x[1]};q.insert(q.end(),x.begin()+5,x.end());Vec a=sweep_asks(q);double e=a[1]-a[0]*x[2],o=a[3]*(x[3]-x[2]),f=x[4]*a[0];return {a[0],a[2],e,o,f,e+o+f};}
}
