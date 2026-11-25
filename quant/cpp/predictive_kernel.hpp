#pragma once
#include "multiscale_common.hpp"
namespace pdq {
using namespace msq;
inline Vec mv(const Mat& A,const Vec& x){Vec y;for(const auto& r:A)y.push_back(dot(r,x));return y;}
inline Mat zeros(int n,int m){return Mat(n,Vec(m));}
inline double rbf(double x,double y,double g){return std::exp(-g*sq(x-y));}
inline Vec kernel_center(const Vec& x){
    require(x.size()>=5);int n=integer(x[0],2,100);double g=x[1],q=x[2];positive(g);require(x.size()==size_t(n+3));
    Mat K=zeros(n,n);Vec m(n),k(n);double grand=0,km=0;
    for(int i=0;i<n;++i){k[i]=rbf(x[i+3],q,g);km+=k[i]/n;for(int j=0;j<n;++j){K[i][j]=rbf(x[i+3],x[j+3],g);m[i]+=K[i][j]/n;}grand+=m[i]/n;}
    Vec out{grand,1-2*km+grand};for(int i=0;i<n;++i)out.push_back(k[i]-m[i]-km+grand);
    for(int i=0;i<n;++i)for(int j=0;j<n;++j)out.push_back(K[i][j]-m[i]-m[j]+grand);
    return out;
}
inline Vec fourier_gram(const Vec& x){
    require(x.size()>=6);int n=integer(x[0],2,100),m=integer(x[1],1,1000);double gamma=x[2];positive(gamma);require(x.size()==size_t(3+n+m));
    Mat Z=zeros(n,2*m);
    for(int i=0;i<n;++i)for(int j=0;j<m;++j){double a=x[3+i]*x[3+n+j];Z[i][2*j]=std::cos(a)/std::sqrt(double(m));Z[i][2*j+1]=std::sin(a)/std::sqrt(double(m));}
    double s=0,worst=0;Vec out{0,0};for(int i=0;i<n;++i)for(int j=0;j<n;++j){double k=dot(Z[i],Z[j]),e=k-rbf(x[3+i],x[3+j],gamma);s+=sq(e);worst=std::max(worst,std::abs(e));out.push_back(k);}
    out[0]=std::sqrt(s/(n*n));out[1]=worst;return out;
}
inline Vec nystrom_ridge(const Vec& x){
    require(x.size()>=9);int n=integer(x[0],2,100),m=integer(x[1],1,n);double g=x[2],eta=x[3],lambda=x[4],q=x[5];positive(g);positive(eta);positive(lambda);require(x.size()==size_t(6+2*n+m));
    Vec p(n),y(n),z(m);for(int i=0;i<n;++i){p[i]=x[6+2*i];y[i]=x[7+2*i];}
    std::set<int> seen;for(int j=0;j<m;++j){int i=integer(x[6+2*n+j],0,n-1);require(seen.insert(i).second);z[j]=p[i];}
    Mat W=zeros(m,m),C=zeros(n,m),A=zeros(m,m);Vec rhs(m),k(m);
    for(int j=0;j<m;++j){k[j]=rbf(q,z[j],g);for(int l=0;l<m;++l)W[j][l]=rbf(z[j],z[l],g)+(j==l?eta:0);}
    for(int i=0;i<n;++i)for(int j=0;j<m;++j)C[i][j]=rbf(p[i],z[j],g);
    for(int j=0;j<m;++j){for(int i=0;i<n;++i)rhs[j]+=C[i][j]*y[i];for(int l=0;l<m;++l){A[j][l]=lambda*W[j][l];for(int i=0;i<n;++i)A[j][l]+=C[i][j]*C[i][l];}}
    Vec b=solve(A,rhs),f=mv(C,b);double rss=0,pen=lambda*dot(b,mv(W,b));for(int i=0;i<n;++i)rss+=sq(f[i]-y[i]);
    Vec out{dot(k,b),rss,pen,rss+pen};out.insert(out.end(),f.begin(),f.end());return out;
}
inline Vec ridge_loo(const Vec& x){
    require(x.size()>=7);int n=integer(x[0],3,200);double g=x[1],lambda=x[2];positive(g);positive(lambda);require(x.size()==size_t(3+2*n));
    Mat A=zeros(n,n);Vec y(n);for(int i=0;i<n;++i){y[i]=x[4+2*i];for(int j=0;j<n;++j)A[i][j]=rbf(x[3+2*i],x[3+2*j],g)+(i==j?lambda:0);}
    Vec a=solve(A,y),loo(n),fit(n),h(n);double mse=0;
    for(int i=0;i<n;++i){Vec e(n);e[i]=1;Vec u=solve(A,e);positive(u[i]);loo[i]=y[i]-a[i]/u[i];fit[i]=y[i]-lambda*a[i];h[i]=1-lambda*u[i];mse+=sq(y[i]-loo[i])/n;}
    Vec out{mse,sum(h)};out.insert(out.end(),loo.begin(),loo.end());out.insert(out.end(),fit.begin(),fit.end());out.insert(out.end(),h.begin(),h.end());return out;
}
inline Vec gp_derivative(const Vec& x){
    require(x.size()>=6);int n=integer(x[0],1,100);double g=x[1],noise=x[2],q=x[3];positive(g);positive(noise);require(x.size()==size_t(4+2*n));
    Mat A=zeros(n,n);Vec y(n),k(n),d(n);
    for(int i=0;i<n;++i){double p=x[4+2*i];y[i]=x[5+2*i];k[i]=rbf(q,p,g);d[i]=-2*g*(q-p)*k[i];for(int j=0;j<n;++j)A[i][j]=rbf(p,x[4+2*j],g)+(i==j?noise:0);}
    Vec a=solve(A,y),u=solve(A,k),v=solve(A,d);
    return {dot(k,a),dot(d,a),1-dot(k,u),2*g-dot(d,v),-dot(k,v)};
}
inline Vec harmonic_graph(const Vec& x){
    require(x.size()>=8);int n=integer(x[0],2,100),m=integer(x[1],1,n-1);double ridge=x[2];positive(ridge);require(x.size()==size_t(3+n*(n-1)/2+2*m));
    Mat W=zeros(n,n);size_t at=3;Vec degree(n);
    for(int i=0;i<n;++i)for(int j=i+1;j<n;++j){double w=x[at++];require(w>=0);W[i][j]=W[j][i]=w;degree[i]+=w;degree[j]+=w;}
    Vec f(n);std::vector<bool> labeled(n);for(int j=0;j<m;++j){int i=integer(x[at++],0,n-1);require(!labeled[i]);labeled[i]=true;f[i]=x[at++];}
    std::vector<int> U;for(int i=0;i<n;++i)if(!labeled[i])U.push_back(i);
    Mat A=zeros(n-m,n-m);Vec b(n-m);for(int i=0;i<n-m;++i){int u=U[i];for(int j=0;j<n-m;++j)A[i][j]=(i==j?degree[u]+ridge:0)-W[u][U[j]];for(int j=0;j<n;++j)if(labeled[j])b[i]+=W[u][j]*f[j];}
    Vec z=solve(A,b);for(int i=0;i<n-m;++i)f[U[i]]=z[i];
    double energy=0,res=0;for(int i=0;i<n;++i){for(int j=i+1;j<n;++j)energy+=W[i][j]*sq(f[i]-f[j]);if(!labeled[i]){energy+=ridge*sq(f[i]);res=std::max(res,std::abs((degree[i]+ridge)*f[i]-dot(W[i],f)));}}
    Vec out{energy,res};out.insert(out.end(),f.begin(),f.end());return out;
}
}
