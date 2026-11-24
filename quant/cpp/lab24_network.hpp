#pragma once
#include "lab24_common.hpp"
#include <functional>

namespace kline { namespace lab24 {
inline Vec laplacian_energy(const Mat& x,const Vec& p){
    inputs(x,p,5,0);double s=0;
    for(const auto& r:x){integer(r[0],0,100000);integer(r[1],0,100000);require(r[2]>=0,"Negative edge weight");double d=r[3]-r[4];s+=r[2]*d*d;}
    return checked({s});
}

inline Vec pagerank(const Mat& x,const Vec& p){
    inputs(x,p,3,3);auto n=integer(p[0],1,5000),steps=integer(p[2],1,10000);
    double d=p[1];require(d>=0&&d<1,"Invalid damping");
    Vec out(n,0),v(n,1.0/n);
    for(const auto& r:x){auto i=integer(r[0],0,n-1);integer(r[1],0,n-1);require(r[2]>=0,"Negative graph weight");out[i]+=r[2];}
    for(std::size_t k=0;k<steps;++k){
        double dangling=0;for(std::size_t i=0;i<n;++i)if(out[i]==0)dangling+=v[i];
        Vec z(n,(1-d+d*dangling)/n);
        for(const auto& r:x){auto i=std::size_t(r[0]),j=std::size_t(r[1]);if(out[i]>0)z[j]+=d*v[i]*r[2]/out[i];}
        v=std::move(z);
    }
    return checked(v);
}

inline Vec diffusion(const Mat& x,const Vec& p){
    require(p.size()>=3&&p.size()<=502,"Initial state and two parameters required");
    inputs(x,p,3,p.size());std::size_t n=p.size()-2;double h=p[0];auto steps=integer(p[1],1,10000);
    require(h>=0,"Negative diffusion step");Vec v(p.begin()+2,p.end()),degree(n,0);
    for(const auto& r:x){auto i=integer(r[0],0,n-1),j=integer(r[1],0,n-1);require(i!=j&&r[2]>=0,"Invalid edge");degree[i]+=r[2];degree[j]+=r[2];}
    require(h*(*std::max_element(degree.begin(),degree.end()))<=1,"Explicit diffusion violates stability bound");
    for(std::size_t k=0;k<steps;++k){Vec z=v;for(const auto& r:x){auto i=std::size_t(r[0]),j=std::size_t(r[1]);double flow=h*r[2]*(v[j]-v[i]);z[i]+=flow;z[j]-=flow;}v=std::move(z);}
    return checked(v);
}

inline Vec minimax_path(const Mat& x,const Vec& p){
    inputs(x,p,3,3);auto n=integer(p[0],1,1000),s=integer(p[1],0,n-1),t=integer(p[2],0,n-1);
    std::vector<std::vector<std::pair<std::size_t,double>>> adj(n);
    for(const auto& r:x){auto i=integer(r[0],0,n-1),j=integer(r[1],0,n-1);require(r[2]>=0,"Negative distance");adj[i].push_back({j,r[2]});adj[j].push_back({i,r[2]});}
    Vec d(n,std::numeric_limits<double>::infinity());d[s]=0;std::vector<bool> done(n,false);
    for(std::size_t k=0;k<n;++k){std::size_t u=n;for(std::size_t i=0;i<n;++i)if(!done[i]&&(u==n||d[i]<d[u]))u=i;if(u==n||!std::isfinite(d[u]))break;done[u]=true;if(u==t)break;for(auto e:adj[u])d[e.first]=std::min(d[e.first],std::max(d[u],e.second));}
    require(std::isfinite(d[t]),"Target unreachable");
    return {d[t]};
}

inline Vec minimum_spanning_tree(const Mat& x,const Vec& p){
    inputs(x,p,3,1);auto n=integer(p[0],2,10000);Mat edges=x;
    for(const auto& r:edges){integer(r[0],0,n-1);integer(r[1],0,n-1);require(r[2]>=0,"Negative edge distance");}
    std::sort(edges.begin(),edges.end(),[](const Vec& a,const Vec& b){return a[2]<b[2];});
    std::vector<std::size_t> parent(n);std::iota(parent.begin(),parent.end(),0);
    auto root=[&](std::size_t i){while(parent[i]!=i){parent[i]=parent[parent[i]];i=parent[i];}return i;};
    std::size_t used=0;double total=0,last=0;
    for(const auto& r:edges){auto a=root(std::size_t(r[0])),b=root(std::size_t(r[1]));if(a==b)continue;parent[a]=b;total+=r[2];last=r[2];if(++used==n-1)break;}
    require(used==n-1,"Graph disconnected");return checked({total,last});
}
} }
