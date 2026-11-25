#pragma once
#include "finite_state_common.hpp"
namespace fsq {
struct Tree {
    double s,k,u,d,r,q,cp;
    int n;
    double spot(int t,int j) const {double v=s*std::pow(u,j)*std::pow(d,t-j);require(std::isfinite(v)&&v>0);return v;}
    double payoff(int t,int j) const{return std::max(0.,cp*(spot(t,j)-k));}
};
inline Tree tree_input(const Vec& x){
    require(x.size()>=7);
    Tree z{x[0],x[1],x[2],x[3],x[4],0,x[6],count(x[5],1,300)};
    require(z.s>0&&z.k>0&&z.d>0&&z.d<z.r&&z.r<z.u&&(z.cp==1||z.cp==-1));
    z.q=(z.r-z.d)/(z.u-z.d);
    return z;
}
inline double eu_tree(const Tree& z){
    Vec v(z.n+1);
    for(int j=0;j<=z.n;++j)v[j]=z.payoff(z.n,j);
    for(int t=z.n-1;t>=0;--t)for(int j=0;j<=t;++j)v[j]=(z.q*v[j+1]+(1-z.q)*v[j])/z.r;
    return v[0];
}
// Discretely monitored down-and-out option; rebate is paid when the barrier is touched.
inline Vec barrier_rebate(const Vec& x){
    require(x.size()==9);
    Tree z=tree_input(x);double h=x[7],rebate=x[8];require(h>0&&rebate>=0);
    Vec v(z.n+1),hit(z.n+1),reb(z.n+1);
    for(int j=0;j<=z.n;++j){bool ko=z.spot(z.n,j)<=h;v[j]=ko?rebate:z.payoff(z.n,j);hit[j]=ko;reb[j]=ko?rebate:0;}
    for(int t=z.n-1;t>=0;--t)for(int j=0;j<=t;++j){
        if(z.spot(t,j)<=h){v[j]=rebate;hit[j]=1;reb[j]=rebate;}
        else{v[j]=(z.q*v[j+1]+(1-z.q)*v[j])/z.r;hit[j]=z.q*hit[j+1]+(1-z.q)*hit[j];reb[j]=(z.q*reb[j+1]+(1-z.q)*reb[j])/z.r;}
    }
    double plain=eu_tree(z),ko=v[0]-reb[0],ki=plain-ko;
    return {v[0],ko,reb[0],ki,plain,hit[0],ko+ki-plain};
}
// Multiple exercise rights; one unit per date, mandatory refraction after exercise.
inline Vec swing_rights(const Vec& x){
    require(x.size()==9);Tree z=tree_input(x);
    int rights=count(x[7],1,20),gap=count(x[8],1,301);
    require(z.n<=160);
    // State: remaining rights and dates until the next exercise is permitted.
    int cool=std::min(gap-1,z.n+1);
    std::vector<Mat> nxt(rights+1,Mat(cool+1,Vec(z.n+1))),cur=nxt;
    for(int j=0;j<=z.n;++j)for(int r=1;r<=rights;++r)nxt[r][0][j]=z.payoff(z.n,j);
    int first=0;
    for(int t=z.n-1;t>=0;--t){
        auto cont=[&](int r,int c,int j){return (z.q*nxt[r][c][j+1]+(1-z.q)*nxt[r][c][j])/z.r;};
        for(int r=1;r<=rights;++r)for(int c=0;c<=cool;++c)for(int j=0;j<=t;++j){
            double wait=cont(r,std::max(0,c-1),j);
            double use=c==0?z.payoff(t,j)+cont(r-1,cool,j):-1;
            cur[r][c][j]=std::max(wait,use);
            if(t==0&&r==rights&&c==0&&j==0)first=use>wait+1e-12;
        }
        cur.swap(nxt);
    }
    return {nxt[rights][0][0],nxt[1][0][0],double(first),nxt[rights][0][0]-nxt[1][0][0]};
}
// Discrete fixed-strike lookback call, recombining path states indexed by the maximum node.
inline Vec lookback_max(const Vec& x){
    require(x.size()==7);Tree z=tree_input(x);require(z.cp==1&&z.n<=20);
    Vec levels;
    for(int t=0;t<=z.n;++t)for(int j=0;j<=t;++j)levels.push_back(z.spot(t,j));
    std::sort(levels.begin(),levels.end());levels.erase(std::unique(levels.begin(),levels.end()),levels.end());
    auto idx=[&](double s){return int(std::lower_bound(levels.begin(),levels.end(),s)-levels.begin());};
    std::vector<std::map<int,double>> f(1);f[0][idx(z.s)]=1;
    for(int t=0;t<z.n;++t){
        std::vector<std::map<int,double>> nxt(t+2);
        for(int j=0;j<=t;++j)for(auto p:f[j]){
            nxt[j][std::max(p.first,idx(z.spot(t+1,j)))]+=(1-z.q)*p.second;
            nxt[j+1][std::max(p.first,idx(z.spot(t+1,j+1)))]+=z.q*p.second;
        }
        f.swap(nxt);
    }
    double v=0,em=0,prob=0,mass=0;int states=0;
    for(const auto& a:f)for(auto p:a){double m=levels[p.first];mass+=p.second;em+=m*p.second;v+=std::max(m-z.k,0.)*p.second;prob+=(m>z.k)*p.second;++states;}
    return {v/std::pow(z.r,z.n),em,prob,mass,double(states),eu_tree(z)};
}
// Finite state Arrow-Debreu prices and Markov discount factors (state-dependent).
inline Vec state_price_chain(const Vec& x){
    require(x.size()>=5);int k=count(x[0],2,20),n=count(x[1],1,500);
    require(x.size()==size_t(2+3*k+k*k));
    Vec a(x.begin()+2,x.begin()+2+k);distribution(a);
    Vec m(x.begin()+2+k,x.begin()+2+2*k),h(x.begin()+2+2*k,x.begin()+2+3*k);
    Mat p(k,Vec(k));for(int i=0;i<k;++i){require(m[i]>0);for(int j=0;j<k;++j)p[i][j]=x[2+3*k+i*k+j];distribution(p[i]);}
    Vec price=a,prob=a;
    for(int t=0;t<n;++t){Vec next(k),np(k);for(int i=0;i<k;++i)for(int j=0;j<k;++j){next[j]+=price[i]*m[i]*p[i][j];np[j]+=prob[i]*p[i][j];}price.swap(next);prob.swap(np);}
    double v=std::inner_product(price.begin(),price.end(),h.begin(),0.),e=std::inner_product(prob.begin(),prob.end(),h.begin(),0.);
    Vec out{v,sum(price),e,sum(prob)};out.insert(out.end(),price.begin(),price.end());out.insert(out.end(),prob.begin(),prob.end());return out;
}
}
