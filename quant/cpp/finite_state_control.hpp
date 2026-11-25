#pragma once
#include "finite_state_common.hpp"
namespace fsq {
// Finite-horizon discounted MDP with state/action dependent transition and reward.
struct MDP {int horizon,n,m,initial;double discount;Mat reward;std::vector<Mat> p;};
inline MDP mdp_input(const Vec& x){
    require(x.size()>=7);int t=count(x[0],1,500),n=count(x[1],2,20),m=count(x[2],1,20),initial=count(x[3],0,n-1);
    double d=x[4];require(d>0&&d<=1&&x.size()==size_t(5+n*m*(1+n)));
    MDP a{t,n,m,initial,d,Mat(n,Vec(m)),std::vector<Mat>(n,Mat(m,Vec(n)))};
    size_t at=5;
    for(int i=0;i<n;++i)for(int j=0;j<m;++j){a.reward[i][j]=x[at++];for(int k=0;k<n;++k)a.p[i][j][k]=x[at++];distribution(a.p[i][j]);}
    return a;
}
inline Vec mdp_optimal(const Vec& x){
    MDP a=mdp_input(x);Vec v(a.n);std::vector<std::vector<int>> policy(a.horizon,std::vector<int>(a.n));
    for(int t=a.horizon-1;t>=0;--t){Vec next(a.n,-std::numeric_limits<double>::infinity());
        for(int i=0;i<a.n;++i)for(int j=0;j<a.m;++j){
            double z=a.reward[i][j]+a.discount*std::inner_product(a.p[i][j].begin(),a.p[i][j].end(),v.begin(),0.);
            if(z>next[i]+1e-12){next[i]=z;policy[t][i]=j;}}
        v.swap(next);
    }
    Vec mass(a.n);mass[a.initial]=1;double gain=0,df=1;Vec use(a.m);
    for(int t=0;t<a.horizon;++t){Vec next(a.n);
        for(int i=0;i<a.n;++i){int j=policy[t][i];gain+=df*mass[i]*a.reward[i][j];use[j]+=mass[i];for(int k=0;k<a.n;++k)next[k]+=mass[i]*a.p[i][j][k];}
        mass.swap(next);df*=a.discount;
    }
    Vec out{v[a.initial],gain,v[a.initial]-gain,sum(mass)};for(int j:policy[0])out.push_back(j);out.insert(out.end(),use.begin(),use.end());out.insert(out.end(),mass.begin(),mass.end());return out;
}
// Discounted infinite horizon policy iteration, exact Gaussian elimination for policy evaluation.
inline Vec solve_linear(Mat a,Vec b){
    int n=b.size();require(a.size()==b.size());
    for(int j=0;j<n;++j){int p=j;for(int i=j+1;i<n;++i)if(std::abs(a[i][j])>std::abs(a[p][j]))p=i;
        require(std::abs(a[p][j])>1e-14);std::swap(a[p],a[j]);std::swap(b[p],b[j]);double d=a[j][j];
        for(int k=j;k<n;++k)a[j][k]/=d;
        b[j]/=d;
        for(int i=0;i<n;++i)if(i!=j){double z=a[i][j];for(int k=j;k<n;++k)a[i][k]-=z*a[j][k];b[i]-=z*b[j];}}
    return b;
}
inline Vec policy_iteration(const Vec& x){
    MDP a=mdp_input(x);require(a.discount<1);std::vector<int> policy(a.n);Vec v;int iteration=0;
    for(;iteration<1000;++iteration){
        Mat b(a.n,Vec(a.n));Vec r(a.n);
        for(int i=0;i<a.n;++i){r[i]=a.reward[i][policy[i]];for(int j=0;j<a.n;++j)b[i][j]=(i==j?1.:0.)-a.discount*a.p[i][policy[i]][j];}
        v=solve_linear(b,r);bool changed=false;
        for(int i=0;i<a.n;++i){int best=policy[i];double maxv=v[i];for(int j=0;j<a.m;++j){double z=a.reward[i][j]+a.discount*std::inner_product(a.p[i][j].begin(),a.p[i][j].end(),v.begin(),0.);
                if(z>maxv+1e-11){best=j;maxv=z;}}
            if(best!=policy[i]){policy[i]=best;changed=true;}}
        if(!changed)break;
    }
    require(iteration<1000);double residual=0;
    for(int i=0;i<a.n;++i){double best=-std::numeric_limits<double>::infinity();for(int j=0;j<a.m;++j)best=std::max(best,a.reward[i][j]+a.discount*std::inner_product(a.p[i][j].begin(),a.p[i][j].end(),v.begin(),0.));residual=std::max(residual,std::abs(best-v[i]));}
    Vec out{v[a.initial],residual,residual/(1-a.discount),double(iteration+1)};out.insert(out.end(),v.begin(),v.end());for(int j:policy)out.push_back(j);return out;
}
// Finite-state robust Bellman operator: adversary moves probability mass in a TV ball.
inline Vec worst_tv(const Vec& x){
    require(x.size()>=5&&x.size()%2==1);double eps=x[0];probability(eps);
    Vec p,v;for(size_t i=1;i<x.size();i+=2){p.push_back(x[i]);v.push_back(x[i+1]);}distribution(p);
    int n=p.size();std::vector<int> order(n);std::iota(order.begin(),order.end(),0);
    std::stable_sort(order.begin(),order.end(),[&](int i,int j){return v[i]<v[j];});
    Vec q=p;double remain=eps;int lo=0,hi=n-1;
    while(lo<hi&&remain>1e-15){int a=order[lo],b=order[hi];if(v[a]>=v[b])break;
        double z=std::min({remain,1-q[a],q[b]});q[a]+=z;q[b]-=z;remain-=z;
        if(q[a]>=1-1e-15)++lo;
        if(q[b]<=1e-15)--hi;
        if(z==0&&lo<hi&&q[a]<1-1e-15&&q[b]>1e-15)break;
    }
    double nominal=std::inner_product(p.begin(),p.end(),v.begin(),0.),worst=std::inner_product(q.begin(),q.end(),v.begin(),0.),tv=0;
    for(int i=0;i<n;++i)tv+=std::abs(q[i]-p[i])/2;
    Vec out{nominal,worst,tv,nominal-worst};out.insert(out.end(),q.begin(),q.end());return out;
}
// Optimal liquidation with random finite-state price and an explicit stopping payoff.
inline Vec stopping_policy(const Vec& x){
    require(x.size()>=8);int n=count(x[0],1,500),k=count(x[1],2,30),initial=count(x[2],0,k-1);
    double discount=x[3],carry=x[4];require(discount>0&&discount<=1&&carry>=0&&x.size()==size_t(5+k+k*k));
    Vec payoff(x.begin()+5,x.begin()+5+k);Mat p(k,Vec(k));size_t at=5+k;
    for(int i=0;i<k;++i){for(int j=0;j<k;++j)p[i][j]=x[at++];distribution(p[i]);}
    Vec v=payoff;std::vector<std::vector<int>> stop(n,std::vector<int>(k));
    for(int t=n-1;t>=0;--t){Vec g(k);for(int i=0;i<k;++i){double cont=-carry+discount*std::inner_product(p[i].begin(),p[i].end(),v.begin(),0.);stop[t][i]=payoff[i]>=cont;g[i]=std::max(payoff[i],cont);}v.swap(g);}
    Vec mass(k);mass[initial]=1;double df=1,actual=0,et=0,early=0;
    for(int t=0;t<n;++t){Vec next(k);for(int i=0;i<k;++i){
        if(stop[t][i]){actual+=df*mass[i]*payoff[i];et+=t*mass[i];early+=mass[i];}
        else{actual-=df*mass[i]*carry;for(int j=0;j<k;++j)next[j]+=mass[i]*p[i][j];}}
        df*=discount;mass.swap(next);
    }
    actual+=df*std::inner_product(mass.begin(),mass.end(),payoff.begin(),0.);et+=n*sum(mass);
    return {v[initial],double(stop[0][initial]),early,et,actual,v[initial]-actual,early+sum(mass)};
}
}
