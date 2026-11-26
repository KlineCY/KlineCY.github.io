#pragma once
#include "predictive_kernel.hpp"
namespace pdq {
inline Vec softmax(Vec z){
    double a=*std::max_element(z.begin(),z.end());
    for(double& v:z)v=std::exp(v-a);
    double den=sum(z);positive(den);for(double& v:z)v/=den;return z;
}
inline Vec sparsemax_vjp(const Vec& x){
    require(x.size()>=4&&x.size()%2==0);int n=x.size()/2;require(n<=10000);
    Vec z(x.begin(),x.begin()+n),g(x.begin()+n,x.end()),s=z,p(n),v(n);
    double m=*std::max_element(z.begin(),z.end());for(double& a:z)a-=m;s=z;std::sort(s.begin(),s.end(),std::greater<double>());
    int k=0;double prefix=0,tau=0;
    for(int i=0;i<n;++i){prefix+=s[i];if(s[i]>(prefix-1)/(i+1)){k=i+1;tau=(prefix-1)/k;}}
    require(k>0);double a=0;for(int i=0;i<n;++i){p[i]=std::max(0.,z[i]-tau);if(p[i]>0)a+=g[i]/k;}
    for(int i=0;i<n;++i)if(p[i]>0)v[i]=g[i]-a;
    Vec out{tau+m,double(k),sum(p),sum(v)};out.insert(out.end(),p.begin(),p.end());out.insert(out.end(),v.begin(),v.end());return out;
}
inline Vec layernorm_vjp(const Vec& x){
    require(x.size()>=9&&(x.size()-1)%4==0);double eps=x[0];positive(eps);int n=(x.size()-1)/4;require(n<=10000);
    Vec a(n),scale(n),bias(n),g(n),z(n),h(n),y(n),dx(n),ds(n);
    for(int i=0;i<n;++i){a[i]=x[1+4*i];scale[i]=x[2+4*i];bias[i]=x[3+4*i];g[i]=x[4+4*i];}
    double m=avg(a),v=0;for(double u:a)v+=sq(u-m)/n;double inv=1/std::sqrt(v+eps);
    for(int i=0;i<n;++i){z[i]=(a[i]-m)*inv;h[i]=scale[i]*g[i];y[i]=scale[i]*z[i]+bias[i];ds[i]=z[i]*g[i];}
    double mh=avg(h),hz=dot(h,z)/n;
    for(int i=0;i<n;++i)dx[i]=inv*(h[i]-mh-z[i]*hz);
    Vec out{m,v,dot(y,g),sum(dx)};for(const auto& a0:{y,dx,ds,g})out.insert(out.end(),a0.begin(),a0.end());return out;
}
inline Vec rmsnorm_vjp(const Vec& x){
    require(x.size()>=7&&(x.size()-1)%3==0);double eps=x[0];positive(eps);int n=(x.size()-1)/3;require(n<=10000);
    Vec a(n),s(n),g(n),z(n),h(n),y(n),dx(n),ds(n);
    for(int i=0;i<n;++i){a[i]=x[1+3*i];s[i]=x[2+3*i];g[i]=x[3+3*i];}
    double v=dot(a,a)/n,inv=1/std::sqrt(v+eps);
    for(int i=0;i<n;++i){z[i]=a[i]*inv;h[i]=s[i]*g[i];y[i]=s[i]*z[i];ds[i]=z[i]*g[i];}
    double hz=dot(h,z)/n;for(int i=0;i<n;++i)dx[i]=inv*(h[i]-z[i]*hz);
    Vec out{v,dot(y,g),dot(dx,a)};for(const auto& a0:{y,dx,ds})out.insert(out.end(),a0.begin(),a0.end());return out;
}
inline Vec gelu_vjp(const Vec& x){
    require(x.size()>=2&&x.size()%2==0);Vec y,d,g;double l=0;
    for(size_t i=0;i<x.size();i+=2){double a=x[i],p=cdf(a),v=p+a*pdf(a),b=a*p;y.push_back(b);d.push_back(v);g.push_back(v*x[i+1]);l+=b*x[i+1];}
    Vec out{l};out.insert(out.end(),y.begin(),y.end());out.insert(out.end(),d.begin(),d.end());out.insert(out.end(),g.begin(),g.end());return out;
}
inline Vec causal_attention_vjp(const Vec& x){
    require(x.size()>=6);int n=integer(x[0],2,40),d=integer(x[1],1,30);require(x.size()==size_t(2+4*n*d));
    Mat Q=zeros(n,d),K=Q,V=Q,G=Q,O=Q,dQ=Q,dK=Q,dV=Q,P=zeros(n,n);size_t at=2;
    for(Mat* M:{&Q,&K,&V,&G})for(auto& r:*M)for(double& z:r)z=x[at++];
    double scale=1/std::sqrt(double(d)),loss=0;
    for(int i=0;i<n;++i){
        Vec s(i+1);for(int j=0;j<=i;++j)s[j]=dot(Q[i],K[j])*scale;s=softmax(s);
        for(int j=0;j<=i;++j){P[i][j]=s[j];for(int k=0;k<d;++k)O[i][k]+=s[j]*V[j][k];}
        loss+=dot(G[i],O[i]);double mean=dot(G[i],O[i]);
        for(int j=0;j<=i;++j){double dz=s[j]*(dot(G[i],V[j])-mean);
            for(int k=0;k<d;++k){dV[j][k]+=s[j]*G[i][k];dQ[i][k]+=dz*K[j][k]*scale;dK[j][k]+=dz*Q[i][k]*scale;}}
    }
    Vec out{loss};for(const Mat* M:{&P,&O,&dQ,&dK,&dV})for(const auto& r:*M)out.insert(out.end(),r.begin(),r.end());return out;
}
inline Vec infonce_vjp(const Vec& x){
    require(x.size()>=7);int n=integer(x[0],2,40),d=integer(x[1],1,30);double temp=x[2];positive(temp);require(x.size()==size_t(3+2*n*d));
    Mat Q=zeros(n,d),K=Q,dQ=Q,dK=Q,P=zeros(n,n);size_t at=3;
    for(Mat* M:{&Q,&K})for(auto& r:*M)for(double& v:r)v=x[at++];
    double loss=0,correct=0;
    for(int i=0;i<n;++i){
        Vec z(n);for(int j=0;j<n;++j)z[j]=dot(Q[i],K[j])/temp;
        double m=*std::max_element(z.begin(),z.end());double den=0;for(double a:z)den+=std::exp(a-m);loss+=(m-z[i]+std::log(den))/n;
        correct+=std::max_element(z.begin(),z.end())-z.begin()==i;P[i]=softmax(z);
        for(int j=0;j<n;++j){double u=(P[i][j]-(i==j?1.:0.))/(n*temp);for(int k=0;k<d;++k){dQ[i][k]+=u*K[j][k];dK[j][k]+=u*Q[i][k];}}
    }
    Vec out{loss,correct/n};for(const Mat* M:{&P,&dQ,&dK})for(const auto& r:*M)out.insert(out.end(),r.begin(),r.end());return out;
}
}
