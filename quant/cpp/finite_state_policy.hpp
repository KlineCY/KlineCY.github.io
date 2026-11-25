#pragma once
#include "finite_state_common.hpp"
namespace fsq {
// Inventory schedule with terminal completion, capacity, convex cost and time-dependent risk.
inline Vec inventory_schedule(const Vec& x){
    require(x.size()>=6&&(x.size()-2)%4==0);int q=count(x[0],1,300),n=int((x.size()-2)/4);
    require(n<=100);double risk=x[1];require(risk>=0);
    Vec c(n),eta(n),vol(n);std::vector<int> cap(n);
    int capacity=0;
    for(int t=0;t<n;++t){c[t]=x[2+4*t];eta[t]=x[3+4*t];vol[t]=x[4+4*t];cap[t]=count(x[5+4*t],0,q);require(c[t]>=0&&eta[t]>=0&&vol[t]>=0);capacity+=cap[t];}
    require(capacity>=q);
    Vec f(q+1,std::numeric_limits<double>::infinity());f[0]=0;
    std::vector<std::vector<int>> policy(n,std::vector<int>(q+1,-1));
    for(int t=n-1;t>=0;--t){Vec cur(q+1,std::numeric_limits<double>::infinity());
        for(int r=0;r<=q;++r)for(int a=0;a<=std::min(r,cap[t]);++a){
            int left=r-a;double v=c[t]*a+eta[t]*a*a+risk*vol[t]*left*left+f[left];
            if(v<cur[r]-1e-12){cur[r]=v;policy[t][r]=a;}
        }
        f.swap(cur);
    }
    int r=q;Vec trades;double costs=0,penalty=0;
    for(int t=0;t<n;++t){int a=policy[t][r];require(a>=0);r-=a;trades.push_back(a);costs+=c[t]*a+eta[t]*a*a;penalty+=risk*vol[t]*r*r;}
    Vec out{f[q],costs,penalty,double(r),f[q]-costs-penalty};out.insert(out.end(),trades.begin(),trades.end());return out;
}
// Two-asset integer lot rebalance with actual cash budget and per-lot fees.
inline Vec lot_rebalance(const Vec& x){
    require(x.size()==11);
    int cash=count(x[0],0,100000),a=count(x[1],0,300),b=count(x[2],0,300),p=count(x[3],1,10000),q=count(x[4],1,10000),f=count(x[5],0,10000);
    double w1=x[6],w2=x[7],gamma=x[8];int ma=count(x[9],0,300),mb=count(x[10],0,300);
    probability(w1);probability(w2);require(w1+w2<=1+1e-12&&gamma>=0);
    double v=cash+a*p+b*q;require(v>0);
    double best=std::numeric_limits<double>::infinity();Vec out;
    for(int i=0;i<=ma;++i)for(int j=0;j<=mb;++j){
        double fees=double(f)*(std::abs(i-a)+std::abs(j-b));
        double c=v-i*p-j*q-fees,wealth=v-fees;
        if(c<0||wealth<=0)continue;
        double e=sq(i*p/wealth-w1)+sq(j*q/wealth-w2)+sq(c/wealth-(1-w1-w2));
        double obj=e+gamma*fees/v;
        if(obj<best-1e-14){best=obj;out={double(i),double(j),c,fees,wealth,e,obj,wealth-c-i*p-j*q};}
    }
    require(!out.empty());return out;
}
// Finite horizon consumption-free allocation with regimes, discrete exposure and switching fee.
// At each decision the next realized multiplier depends on next regime, then regime is observed.
inline Vec regime_policy(const Vec& x){
    require(x.size()>=8);
    int n=count(x[0],1,100),k=count(x[1],2,10),m=count(x[2],2,30),initial=count(x[3],0,k-1),old=count(x[4],0,m-1);
    double fee=x[5];require(fee>=0);
    require(x.size()==size_t(6+m+k+k*k));
    Vec weights(x.begin()+6,x.begin()+6+m),ret(x.begin()+6+m,x.begin()+6+m+k);
    for(int j=0;j<m;++j){probability(weights[j]);if(j)require(weights[j]>weights[j-1]);}
    Mat p(k,Vec(k));size_t at=6+m+k;
    for(int i=0;i<k;++i){for(int j=0;j<k;++j)p[i][j]=x[at++];distribution(p[i]);}
    for(double w:weights)for(double r:ret)require(1+w*r>0);
    require(fee*(weights.back()-weights.front())<1);
    Mat v(k,Vec(m));std::vector<std::vector<std::vector<int>>> pol(n,std::vector<std::vector<int>>(k,std::vector<int>(m)));
    for(int t=n-1;t>=0;--t){
        Mat next(k,Vec(m,-std::numeric_limits<double>::infinity()));
        for(int i=0;i<k;++i)for(int a=0;a<m;++a)for(int b=0;b<m;++b){
            double z=std::log1p(-fee*std::abs(weights[b]-weights[a]));
            for(int j=0;j<k;++j)z+=p[i][j]*(std::log1p(weights[b]*ret[j])+v[j][b]);
            if(z>next[i][a]+1e-14){next[i][a]=z;pol[t][i][a]=b;}
        }
        v.swap(next);
    }
    // Independent forward probability flow under the selected policy.
    Mat mass(k,Vec(m));mass[initial][old]=1;double logwealth=0,turn=0;
    for(int t=0;t<n;++t){Mat next(k,Vec(m));for(int i=0;i<k;++i)for(int a=0;a<m;++a){
        int b=pol[t][i][a];double z=mass[i][a],dw=std::abs(weights[b]-weights[a]);
        turn+=z*dw;logwealth+=z*std::log1p(-fee*dw);
        for(int j=0;j<k;++j){double q=z*p[i][j];next[j][b]+=q;logwealth+=q*std::log1p(weights[b]*ret[j]);}
    }
        mass.swap(next);}
    double total=0;for(const auto& z:mass)total+=sum(z);
    int first=pol[0][initial][old];
    return {v[initial][old],weights[first],logwealth,turn,total,v[initial][old]-logwealth};
}
// Exact path-dependent drawdown probabilities on a log-wealth lattice.
inline Vec drawdown_chain(const Vec& x){
    require(x.size()>=7);int n=count(x[0],1,150),k=count(x[1],2,8),init=count(x[2],0,k-1),limit=count(x[3],1,1000);
    double step=x[4];require(step>0&&step<=1);
    require(x.size()==size_t(5+k+k*k));
    std::vector<int> move(k);for(int i=0;i<k;++i)move[i]=count(x[5+i],-10,10);
    require(n*step*10<=600);
    Mat p(k,Vec(k));size_t a=5+k;
    for(int i=0;i<k;++i){for(int j=0;j<k;++j)p[i][j]=x[a++];distribution(p[i]);}
    // State (current regime, log-wealth index, drawdown from running peak, max drawdown).
    // The maximum drawdown is capped at the monitoring threshold.
    using Key=std::array<int,4>;std::map<Key,double> f;
    f[{init,0,0,0}]=1;
    require(n<=60&&limit<=100);
    for(int t=0;t<n;++t){std::map<Key,double> next;for(const auto& z:f)for(int j=0;j<k;++j){
        if(p[z.first[0]][j]==0)continue;
        int d=std::max(0,z.first[2]-move[j]);
        int m=std::min(limit,std::max(z.first[3],d));
        next[{j,z.first[1]+move[j],d,m}]+=z.second*p[z.first[0]][j];
        require(next.size()<=1000000);
    }
        f.swap(next);}
    double prob=0,wealth=0,elog=0,mass=0,terminaldd=0;
    for(const auto& z:f){double p=z.second;prob+=(z.first[3]>=limit)*p;wealth+=std::exp(step*z.first[1])*p;elog+=step*z.first[1]*p;terminaldd+=(1-std::exp(-step*z.first[2]))*p;mass+=p;}
    return {prob,wealth,elog,terminaldd,mass,double(f.size())};
}
}
