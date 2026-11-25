#pragma once
#include "finite_state_common.hpp"
namespace fsq {
inline Vec convolution(const Vec& a,const Vec& b){
    require(a.size()+b.size()<=200001);
    Vec c(a.size()+b.size()-1);
    for(size_t i=0;i<a.size();++i)if(a[i]!=0)for(size_t j=0;j<b.size();++j)c[i+j]+=a[i]*b[j];
    return c;
}
inline double log_choose(int n,int k){
    if(k<0||k>n)return -std::numeric_limits<double>::infinity();
    return std::lgamma(n+1.)-std::lgamma(k+1.)-std::lgamma(n-k+1.);
}
inline Vec hypergeometric(int n,int k,int m){
    require(n>=1&&k>=0&&k<=n&&m>=0&&m<=n);
    Vec p(std::min(k,m)+1);double denom=log_choose(n,m);
    for(int i=std::max(0,m-n+k);i<=std::min(k,m);++i)p[i]=std::exp(log_choose(k,i)+log_choose(n-k,m-i)-denom);
    double s=sum(p);require(s>0);for(double& z:p)z/=s;return p;
}
inline Vec tails(const Vec& f,int obs){
    require(obs>=0&&obs<int(f.size())&&f[obs]>0);
    double lo=0,hi=0,mean=0,var=0;
    for(int j=0;j<int(f.size());++j){if(j<=obs)lo+=f[j];if(j>=obs)hi+=f[j];mean+=j*f[j];}
    for(int j=0;j<int(f.size());++j)var+=sq(j-mean)*f[j];
    return {mean,var,lo,hi,std::min(1.,2*std::min(lo,hi)),f[obs]};
}
// Stratified 2x2 conditional randomization: the total treated-success count.
inline Vec stratified_exact(const Vec& x){
    require(!x.empty()&&x.size()%4==0&&x.size()<=160);
    Vec f{1};int observed=0;
    for(size_t i=0;i<x.size();i+=4){
        int n=count(x[i],1,1000),k=count(x[i+1],0,n),m=count(x[i+2],0,n),o=count(x[i+3],std::max(0,m-n+k),std::min(k,m));
        observed+=o;f=convolution(f,hypergeometric(n,k,m));
    }
    Vec a=tails(f,observed),out{double(observed)};out.insert(out.end(),a.begin(),a.end());out.insert(out.end(),f.begin(),f.end());return out;
}
// Exact rank-sum permutation with midranks (scaled by two to avoid fractional indices).
inline Vec rank_sum_exact(const Vec& x){
    require(x.size()>=4);int m=count(x[0],1,40),n=int(x.size())-1;require(n>=3&&n<=40&&m<n);
    Vec z(x.begin()+1,x.end());std::vector<int> order(n),rank(n);std::iota(order.begin(),order.end(),0);
    std::stable_sort(order.begin(),order.end(),[&](int i,int j){return z[i]<z[j];});
    for(int l=0;l<n;){int r=l+1;while(r<n&&z[order[l]]==z[order[r]])++r;for(int i=l;i<r;++i)rank[order[i]]=l+r+1;l=r;}
    int cap=n*(n+1),obs=std::accumulate(rank.begin(),rank.begin()+m,0);
    Mat f(m+1,Vec(cap+1));f[0][0]=1;int seen=0;
    for(int r:rank){++seen;for(int k=std::min(seen,m);k>=1;--k)for(int s=cap;s>=r;--s)f[k][s]+=f[k-1][s-r];}
    double den=sum(f[m]);for(double& v:f[m])v/=den;
    Vec p=tails(f[m],obs);
    return {obs/2.,obs/2.-m*(m+1)/2.,p[0]/2.,p[1]/4.,p[2],p[3],p[4],p[5]};
}
// Wilcoxon signed rank conditional on observed nonzero absolute differences.
inline Vec signed_rank_exact(const Vec& x){
    Vec a;for(double v:x)if(v!=0)a.push_back(v);int n=a.size();require(n>=1&&n<=40);
    std::vector<int> order(n),rank(n);std::iota(order.begin(),order.end(),0);
    std::sort(order.begin(),order.end(),[&](int i,int j){return std::abs(a[i])<std::abs(a[j]);});
    for(int l=0;l<n;){int r=l+1;while(r<n&&std::abs(a[order[l]])==std::abs(a[order[r]]))++r;for(int i=l;i<r;++i)rank[order[i]]=l+r+1;l=r;}
    Vec f{1};int obs=0;
    for(int i=0;i<n;++i){int r=rank[i];if(a[i]>0)obs+=r;Vec g(r+1);g[0]=g[r]=.5;f=convolution(f,g);}
    Vec p=tails(f,obs);
    return {double(n),obs/2.,p[0]/2.,p[1]/4.,p[2],p[3],p[4],p[5]};
}
// Finite-horizon sequential Bernoulli likelihood-ratio test with absorbing decisions.
inline Vec sequential_test(const Vec& x){
    require(x.size()==6);int n=count(x[0],1,2000);double p0=x[1],p1=x[2],low=x[3],high=x[4],truth=x[5];
    require(p0>0&&p0<p1&&p1<1&&low<0&&high>0);probability(truth);
    double up=std::log(p1/p0),down=std::log((1-p1)/(1-p0));
    Vec alive(1,1);double accept0=0,accept1=0,et=0;
    for(int t=1;t<=n;++t){Vec next(t+1);
        for(int s=0;s<t;++s){next[s]+=(1-truth)*alive[s];next[s+1]+=truth*alive[s];}
        for(int s=0;s<=t;++s){double lr=s*up+(t-s)*down;
            if(lr<=low){accept0+=next[s];et+=t*next[s];next[s]=0;}
            else if(lr>=high){accept1+=next[s];et+=t*next[s];next[s]=0;}}
        alive.swap(next);
    }
    double undecided=sum(alive);et+=n*undecided;
    return {accept0,accept1,undecided,et,accept0+accept1+undecided};
}
}
