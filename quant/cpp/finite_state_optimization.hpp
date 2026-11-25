#pragma once
#include "finite_state_common.hpp"
namespace fsq {
// Integer transport by successive shortest residual paths with reverse arcs.
inline Vec transport_flow(const Vec& x){
    require(x.size()>=8);
    int m=count(x[0],1,20),n=count(x[1],1,20);
    require(x.size()==size_t(2+m+n+m*n));
    std::vector<int> supply(m),demand(n);int s=0,d=0;
    for(int i=0;i<m;++i){supply[i]=count(x[2+i],0,1000);s+=supply[i];}
    for(int j=0;j<n;++j){demand[j]=count(x[2+m+j],0,1000);d+=demand[j];}
    require(s==d&&s>0&&s<=5000);
    struct Arc{int to,rev,cap;double cost;};
    int source=m+n,sink=source+1,N=sink+1;
    std::vector<std::vector<Arc>> g(N);
    auto arc=[&](int a,int b,int cap,double c){int i=g[a].size(),j=g[b].size();g[a].push_back({b,j,cap,c});g[b].push_back({a,i,0,-c});return i;};
    for(int i=0;i<m;++i)arc(source,i,supply[i],0);
    std::vector<std::vector<int>> idx(m,std::vector<int>(n));
    for(int i=0;i<m;++i)for(int j=0;j<n;++j){double c=x[2+m+n+i*n+j];require(std::abs(c)<=1e8);idx[i][j]=arc(i,m+j,s,c);}
    for(int j=0;j<n;++j)arc(m+j,sink,demand[j],0);
    double cost=0;int done=0;
    while(done<s){
        Vec dist(N,std::numeric_limits<double>::infinity());dist[source]=0;
        std::vector<int> prev(N,-1),edge(N,-1);
        for(int k=0;k<N-1;++k){bool any=false;for(int i=0;i<N;++i)if(std::isfinite(dist[i]))for(int j=0;j<int(g[i].size());++j){
            const auto& e=g[i][j];if(e.cap>0&&dist[i]+e.cost<dist[e.to]-1e-10){dist[e.to]=dist[i]+e.cost;prev[e.to]=i;edge[e.to]=j;any=true;}}
            if(!any)break;
        }
        require(prev[sink]>=0);int f=s-done,steps=0;
        for(int j=sink;j!=source;j=prev[j]){require(prev[j]>=0&&++steps<=N);f=std::min(f,g[prev[j]][edge[j]].cap);}
        for(int j=sink;j!=source;j=prev[j]){auto& e=g[prev[j]][edge[j]];e.cap-=f;g[j][e.rev].cap+=f;}
        done+=f;cost+=f*dist[sink];
    }
    Vec out{cost,double(done),0};double direct=0;
    for(int i=0;i<m;++i)for(int j=0;j<n;++j){double z=s-g[i][idx[i][j]].cap;out.push_back(z);direct+=z*x[2+m+n+i*n+j];}
    out[2]=cost-direct;return out;
}
// Weighted edit-distance style sequence alignment with a symmetric band.
inline Vec band_dtw(const Vec& x){
    require(x.size()>=6);
    int n=count(x[0],1,1000),m=count(x[1],1,1000),b=count(x[2],0,1000);
    double penalty=x[3];require(penalty>=0&&x.size()==size_t(4+n+m)&&std::abs(n-m)<=b);
    Vec a(x.begin()+4,x.begin()+4+n),c(x.begin()+4+n,x.end());
    Mat f(n+1,Vec(m+1,std::numeric_limits<double>::infinity()));
    std::vector<std::vector<int>> p(n+1,std::vector<int>(m+1,-1));f[0][0]=0;
    for(int i=1;i<=n;++i)for(int j=std::max(1,i-b);j<=std::min(m,i+b);++j){
        std::array<double,3> q{f[i-1][j-1],f[i-1][j]+penalty,f[i][j-1]+penalty};
        int k=int(std::min_element(q.begin(),q.end())-q.begin());f[i][j]=q[k]+sq(a[i-1]-c[j-1]);p[i][j]=k;
    }
    require(std::isfinite(f[n][m]));double loss=0,steps=0,warp=0;int i=n,j=m;Vec path;
    while(i>0&&j>0){path.insert(path.end(),{double(i-1),double(j-1)});loss+=sq(a[i-1]-c[j-1]);++steps;int k=p[i][j];require(k>=0);
        if(k==0){--i;--j;}else if(k==1){--i;++warp;}else{--j;++warp;}
    }
    require(i==0&&j==0);Vec out{f[n][m],loss,warp,steps,f[n][m]-loss-penalty*warp};
    for(int k=int(path.size())-2;k>=0;k-=2)out.insert(out.end(),{path[k],path[k+1]});
    return out;
}
// Minimum-cost matching on a small square bipartite graph by subset dynamic programming.
inline Vec assignment(const Vec& x){
    require(!x.empty());int n=count(x[0],1,18);require(x.size()==size_t(1+n*n));
    int N=1<<n;Vec f(N,std::numeric_limits<double>::infinity());std::vector<int> pick(N,-1),pop(N);f[0]=0;
    for(int mask=1;mask<N;++mask)pop[mask]=pop[mask>>1]+(mask&1);
    for(int mask=0;mask<N-1;++mask){int i=pop[mask];for(int j=0;j<n;++j)if(!(mask&(1<<j))){
        int z=mask|(1<<j);double v=f[mask]+x[1+i*n+j];if(v<f[z]-1e-12){f[z]=v;pick[z]=j;}
    }}
    Vec out{f[N-1]},perm(n);double direct=0;
    int mask=N-1;for(int i=n-1;i>=0;--i){int j=pick[mask];require(j>=0);perm[i]=j;direct+=x[1+i*n+j];mask^=1<<j;}
    out.push_back(direct);out.insert(out.end(),perm.begin(),perm.end());return out;
}
// One-dimensional Wasserstein distance by monotone transport with arbitrary positive masses.
inline Vec wasserstein(const Vec& x){
    require(x.size()>=6);int m=count(x[0],1,1000),n=count(x[1],1,1000);
    require(x.size()==size_t(2+2*m+2*n));
    std::vector<std::pair<double,double>> a,b;Vec p,q;
    for(int i=0;i<m;++i){a.push_back({x[2+2*i],x[3+2*i]});p.push_back(a.back().second);}
    for(int j=0;j<n;++j){b.push_back({x[2+2*m+2*j],x[3+2*m+2*j]});q.push_back(b.back().second);}
    distribution(p);distribution(q);std::sort(a.begin(),a.end());std::sort(b.begin(),b.end());
    double w1=0,w2=0,mass=0;size_t i=0,j=0;double l=a[0].second,r=b[0].second;
    while(i<a.size()&&j<b.size()){
        double z=std::min(l,r),d=a[i].first-b[j].first;mass+=z;w1+=z*std::abs(d);w2+=z*d*d;l-=z;r-=z;
        if(l<=1e-15){++i;if(i<a.size())l=a[i].second;}
        if(r<=1e-15){++j;if(j<b.size())r=b[j].second;}
    }
    require(std::abs(mass-1)<1e-10);
    // Independent CDF-area identity for W1.
    std::map<double,double> delta;
    for(auto z:a)delta[z.first]+=z.second;
    for(auto z:b)delta[z.first]-=z.second;
    double area=0,c=0,prev=delta.begin()->first;
    for(auto z:delta){area+=std::abs(c)*(z.first-prev);c+=z.second;prev=z.first;}
    return {w1,std::sqrt(w2),area,mass,w1-area};
}
}
