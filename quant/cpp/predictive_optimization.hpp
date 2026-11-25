#pragma once
#include "predictive_kernel.hpp"
namespace pdq {
inline Vec adamw_trajectory(const Vec& x){
    require(x.size()>=9);int d=integer(x[0],1,100);double eta=x[1],b1=x[2],b2=x[3],eps=x[4],decay=x[5],clip=x[6];
    positive(eta);require(b1>=0&&b1<1&&b2>=0&&b2<1&&eps>0&&decay>=0&&eta*decay<=1&&clip>0&&x.size()>size_t(7+d)&&(x.size()-7-d)%d==0);
    Vec w(x.begin()+7,x.begin()+7+d),m(d),v(d),out;double p1=1,p2=1;
    for(size_t at=7+d;at<x.size();at+=d){
        Vec g(x.begin()+at,x.begin()+at+d);double norm=std::sqrt(dot(g,g)),factor=norm>clip?clip/norm:1;p1*=b1;p2*=b2;
        out.insert(out.end(),{norm,factor});
        for(int j=0;j<d;++j){g[j]*=factor;m[j]=b1*m[j]+(1-b1)*g[j];v[j]=b2*v[j]+(1-b2)*g[j]*g[j];w[j]=(1-eta*decay)*w[j]-eta*(m[j]/(1-p1))/(std::sqrt(v[j]/(1-p2))+eps);}
        for(const auto& a:{w,m,v})out.insert(out.end(),a.begin(),a.end());
    }
    return out;
}
struct RidgeData{int n,d;Mat X;Vec y;};
inline RidgeData ridge_data(const Vec& x,size_t& at,int n,int d){
    require(at+size_t(n)*(d+1)<=x.size());RidgeData a{n,d,zeros(n,d),Vec(n)};
    for(int i=0;i<n;++i){for(double& v:a.X[i])v=x[at++];a.y[i]=x[at++];}return a;
}
inline std::pair<Mat,Vec> ridge_normal(const RidgeData& a,double lambda){
    Mat A=zeros(a.d,a.d);Vec b(a.d);
    for(int j=0;j<a.d;++j){A[j][j]=lambda;for(int i=0;i<a.n;++i){b[j]+=a.X[i][j]*a.y[i];for(int k=0;k<a.d;++k)A[j][k]+=a.X[i][j]*a.X[i][k];}}return {A,b};
}
inline Vec ridge_hypergrad(const Vec& x){
    require(x.size()>=8);int n=integer(x[0],2,200),d=integer(x[1],1,40),m=integer(x[2],1,200);double l=x[3];positive(l);
    size_t at=4;auto train=ridge_data(x,at,n,d),val=ridge_data(x,at,m,d);require(at==x.size());
    auto ab=ridge_normal(train,l);Vec b=solve(ab.first,ab.second),u=solve(ab.first,b),pred=mv(val.X,b),g(d);double loss=0;
    for(int i=0;i<m;++i){double e=pred[i]-val.y[i];loss+=e*e/m;for(int j=0;j<d;++j)g[j]+=2*e*val.X[i][j]/m;}
    for(double& z:u)z=-z;double der=dot(g,u);Vec out{loss,der,l*der};out.insert(out.end(),b.begin(),b.end());out.insert(out.end(),u.begin(),u.end());out.insert(out.end(),pred.begin(),pred.end());return out;
}
inline Vec ridge_influence(const Vec& x){
    require(x.size()>=7);int n=integer(x[0],2,200),d=integer(x[1],1,40);double l=x[2];positive(l);size_t at=3;
    auto a=ridge_data(x,at,n,d);require(at+d==x.size());Vec q(x.begin()+at,x.end());auto ab=ridge_normal(a,l);Vec b=solve(ab.first,ab.second),v=solve(ab.first,q),influence(n),loo(n);double pred=dot(q,b);
    for(int i=0;i<n;++i){double e=a.y[i]-dot(a.X[i],b),k=dot(v,a.X[i]);influence[i]=k*e;double h=dot(a.X[i],solve(ab.first,a.X[i]));require(h<1);loo[i]=pred-influence[i]/(1-h);}
    Vec out{pred};out.insert(out.end(),influence.begin(),influence.end());out.insert(out.end(),loo.begin(),loo.end());return out;
}
inline Vec hutchinson_full(const Vec& x){
    require(x.size()>=5);int d=integer(x[0],2,12);require(x.size()==size_t(1+d*d));Mat A=zeros(d,d);size_t at=1;for(auto& row:A)for(double& v:row)v=x[at++];
    double trace=0,variance=0;for(int i=0;i<d;++i){trace+=A[i][i];for(int j=i+1;j<d;++j){require(std::abs(A[i][j]-A[j][i])<1e-12);variance+=4*sq(A[i][j]);}}
    int N=1<<d;Vec a;double sum0=0,sum2=0;for(int mask=0;mask<N;++mask){Vec z(d);for(int j=0;j<d;++j)z[j]=(mask&(1<<j))?1:-1;double v=dot(z,mv(A,z));a.push_back(v);sum0+=v/N;sum2+=sq(v-trace)/N;}
    Vec out{trace,variance,sum0,sum2,double(N)};out.insert(out.end(),a.begin(),a.end());return out;
}
inline double sigmoid(double a){return a>=0?1/(1+std::exp(-a)):std::exp(a)/(1+std::exp(a));}
inline Vec logistic_hvp(const Vec& x){
    require(x.size()>=8);int n=integer(x[0],2,300),d=integer(x[1],1,80);double l=x[2];require(l>=0&&x.size()==size_t(3+2*d+n*(d+1)));
    Vec b(x.begin()+3,x.begin()+3+d),p(x.begin()+3+d,x.begin()+3+2*d);size_t at=3+2*d;auto data=ridge_data(x,at,n,d);Vec g(d),h(d);double loss=l*dot(b,b)/2;
    for(int j=0;j<d;++j){g[j]=l*b[j];h[j]=l*p[j];}
    for(int i=0;i<n;++i){double y=data.y[i];require(y==0||y==1);const Vec& a=data.X[i];double z=dot(a,b),r=sigmoid(z);loss+=(std::max(z,0.)-z*y+std::log1p(std::exp(-std::abs(z))))/n;
        double f=(r-y)/n,v=r*(1-r)*dot(a,p)/n;for(int j=0;j<d;++j){g[j]+=a[j]*f;h[j]+=a[j]*v;}}
    Vec out{loss,dot(p,h)};out.insert(out.end(),g.begin(),g.end());out.insert(out.end(),h.begin(),h.end());return out;
}
inline Vec cg_ridge(const Vec& x){
    require(x.size()>=7);int n=integer(x[0],2,1000),d=integer(x[1],1,100);double l=x[2];positive(l);size_t at=3;auto data=ridge_data(x,at,n,d);require(at==x.size());
    Vec rhs(d),b(d);for(int i=0;i<n;++i)for(int j=0;j<d;++j)rhs[j]+=data.X[i][j]*data.y[i];
    auto apply=[&](const Vec& z){Vec v(d);for(int j=0;j<d;++j)v[j]=l*z[j];for(const auto& a:data.X){double r=dot(a,z);for(int j=0;j<d;++j)v[j]+=a[j]*r;}return v;};
    Vec r=rhs,p=r;double rr=dot(r,r),tol=1e-12*std::max(1.,std::sqrt(rr));
    for(int it=0;it<4*d&&std::sqrt(rr)>tol;++it){Vec Ap=apply(p);double den=dot(p,Ap);positive(den);double a=rr/den;for(int j=0;j<d;++j){b[j]+=a*p[j];r[j]-=a*Ap[j];}double next=dot(r,r);for(int j=0;j<d;++j)p[j]=r[j]+next/rr*p[j];rr=next;}
    Vec a=apply(b);for(int j=0;j<d;++j)a[j]-=rhs[j];double obj=l*dot(b,b);Vec f=mv(data.X,b);for(int i=0;i<n;++i)obj+=sq(f[i]-data.y[i]);
    Vec out{obj,std::sqrt(dot(a,a))};out.insert(out.end(),b.begin(),b.end());out.insert(out.end(),f.begin(),f.end());return out;
}
}
