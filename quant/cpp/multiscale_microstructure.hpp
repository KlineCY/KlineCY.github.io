#pragma once
#include "multiscale_common.hpp"
namespace msq {
struct Ticks{Vec t,p;};
inline Ticks ticks(const Vec& x,size_t& at,int n){
    require(at+2*size_t(n)<=x.size());Ticks a;
    for(int i=0;i<n;++i){double t=x[at++],p=x[at++];positive(p);require(i==0||t>a.t.back());a.t.push_back(t);a.p.push_back(std::log(p));}
    return a;
}
inline std::pair<Ticks,Ticks> two_ticks(const Vec& x,size_t at=0){
    require(x.size()>=at+2);int n=integer(x[at++],2,50000),m=integer(x[at++],2,50000);
    require(at+2*size_t(n+m)==x.size());Ticks a=ticks(x,at,n),b=ticks(x,at,m);return {a,b};
}
inline Vec hy(const Ticks& a,const Ticks& b,double shift){
    size_t i=1,j=1;double c=0,overlap=0,n=0,v1=0,v2=0;
    for(size_t k=1;k<a.p.size();++k)v1+=sq(a.p[k]-a.p[k-1]);
    for(size_t k=1;k<b.p.size();++k)v2+=sq(b.p[k]-b.p[k-1]);
    while(i<a.t.size()&&j<b.t.size()){
        double l=std::max(a.t[i-1],b.t[j-1]+shift),r=std::min(a.t[i],b.t[j]+shift);
        if(l<r){c+=(a.p[i]-a.p[i-1])*(b.p[j]-b.p[j-1]);overlap+=r-l;++n;}
        double t=a.t[i],s=b.t[j]+shift;
        if(t<=s)++i;
        if(s<=t)++j;
    }
    return {c,v1,v2,n,overlap};
}
inline Vec hy_covariance(const Vec& x){auto a=two_ticks(x);return hy(a.first,a.second,0);}
inline Vec refresh_covariance(const Vec& x){
    auto ab=two_ticks(x);const Ticks& a=ab.first;const Ticks& b=ab.second;
    size_t i=0,j=0;double now=std::max(a.t[0],b.t[0]);Vec t,u,v;
    while(true){
        while(i+1<a.t.size()&&a.t[i+1]<=now)++i;
        while(j+1<b.t.size()&&b.t[j+1]<=now)++j;
        t.push_back(now);u.push_back(a.p[i]);v.push_back(b.p[j]);
        if(i+1==a.t.size()||j+1==b.t.size())break;
        now=std::max(a.t[i+1],b.t[j+1]);
    }
    require(t.size()>=2);double c=0,p=0,q=0;
    for(size_t k=1;k<t.size();++k){double y=u[k]-u[k-1],z=v[k]-v[k-1];c+=y*z;p+=y*y;q+=z*z;}
    Vec out{c,p,q,double(t.size()),t.back()-t.front()};
    out.insert(out.end(),t.begin(),t.end());return out;
}
inline Vec hy_shift_grid(const Vec& x){
    require(x.size()>=12);int k=integer(x[0],1,201);require(x.size()>size_t(k+1));
    auto a=two_ticks(x,k+1);Vec out{0,0,0};double best=-1,lag=0,c=0;
    for(int j=0;j<k;++j){double d=x[j+1];require(j==0||d>x[j]);Vec z=hy(a.first,a.second,d);
        if(std::abs(z[0])>best){best=std::abs(z[0]);lag=d;c=z[0];}
        out.insert(out.end(),{d,z[0],z[3],z[4]});}
    out[0]=lag;out[1]=c;out[2]=best;return out;
}
inline Vec bipower_variation(const Vec& x){
    require(x.size()>=3&&x.size()<=100000);double pi=std::acos(-1.),rv=0,bv=0,iq=0;
    for(size_t i=0;i<x.size();++i){rv+=sq(x[i]);iq+=sq(sq(x[i]));if(i)bv+=std::abs(x[i]*x[i-1]);}
    bv*=pi/2*x.size()/(x.size()-1.);
    return {rv,bv,rv-bv,std::max(rv-bv,0.),x.size()*iq/3};
}
inline Vec noise_moments(const Vec& x){
    require(x.size()>=3&&x.size()<=100000);double m=avg(x),a=0,b=0;
    for(size_t i=0;i<x.size();++i){a+=sq(x[i]-m);if(i)b+=(x[i]-m)*(x[i-1]-m);}
    // Empirical moments use a common centered-return convention, not an unbiased variance estimate.
    a/=x.size();b/=x.size()-1;
    return {m,a,b,-b,a+2*b,std::max(0.,-b),std::max(0.,a+2*b)};
}
inline Vec lag_cov_noise(const Vec& x){
    require(x.size()>=8&&(x.size()-4)%2==0);int lag=integer(x[0],1,10000);double u=x[1],v=x[2],rho=x[3];
    require(u>=0&&v>=0&&sq(rho)<=u*v+1e-15);
    int n=int((x.size()-4)/2);require(lag<n);
    double a=0,b=0,c=0;
    for(int j=lag;j<n;++j){double r=x[4+2*j]-x[4+2*(j-lag)],s=x[5+2*j]-x[5+2*(j-lag)];
        a+=r*r;b+=s*s;c+=r*s;}
    double w=double(n-lag)*lag,steps=n-1.,factor=steps/w;
    return {a*factor,b*factor,c*factor,(a-2*(n-lag)*u)*factor,(b-2*(n-lag)*v)*factor,(c-2*(n-lag)*rho)*factor,double(n-lag),factor};
}
inline Vec weighted_preaverage(const Vec& x){
    require(x.size()>=7);int k=integer(x[0],2,1000);double noise=x[1];require(noise>=0&&x.size()>size_t(k+2));
    Vec w(x.begin()+2,x.begin()+2+k);int n=int(x.size()-2-k);require(n>=k+1&&n<=100000);
    double A=dot(w,w),B=sq(w.front())+sq(w.back());positive(A);
    for(int j=1;j<k;++j)B+=sq(w[j]-w[j-1]);
    double raw=0;int m=n-k+1;
    for(int i=0;i<m;++i){double z=0;for(int j=0;j<k;++j)z+=w[j]*x[2+k+i+j];raw+=z*z;}
    double factor=double(n)/(m*A);
    return {raw,A,B,m*noise*B,raw*factor,(raw-m*noise*B)*factor,double(m),factor};
}
inline Vec volume_bars(const Vec& x){
    require(x.size()>=5&&(x.size()-1)%4==0);double target=x[0];positive(target);
    Vec out{0,0,0,0,0};double vol=0,signedv=0,notional=0,o=0,h=0,l=0,c=0,start=0,last=0,total=0,nt=0,sv=0;int bars=0;
    auto emit=[&](bool full){out.insert(out.end(),{start,last,o,h,l,c,vol,signedv,notional/vol,full?1.:0.});++bars;require(bars<=50000);vol=0;signedv=0;notional=0;};
    for(size_t i=1;i<x.size();i+=4){
        double t=x[i],price=x[i+1],q=x[i+2],sgn=x[i+3];positive(price);positive(q);require((sgn==1||sgn==-1)&&(i==1||t>=last));require(q/target<=50000);
        total+=q;nt+=q*price;sv+=q*sgn;
        while(q>0){
            if(vol==0){start=t;o=h=l=price;}h=std::max(h,price);l=std::min(l,price);c=price;last=t;
            double room=target-vol;positive(room);bool full=q>=room;double z=full?room:q;
            vol=full?target:vol+z;q=full?q-z:0;signedv+=sgn*z;notional+=price*z;
            if(full||vol>=target)emit(true);
        }
    }
    if(vol>0)emit(false);
    out[0]=bars;out[1]=total;out[2]=nt;out[3]=sv;double conserved=0;for(size_t i=5;i<out.size();i+=10)conserved+=out[i+6];out[4]=total-conserved;return out;
}
}
