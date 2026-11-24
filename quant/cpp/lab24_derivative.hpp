#pragma once
#include "lab24_common.hpp"

namespace kline { namespace lab24 {
inline Vec crr_american(const Mat& x,const Vec& p){
    inputs(x,p,1,8);require(x.size()==1,"Single placeholder row required");double s=positive(p[0]),k=positive(p[1]),r=p[2],div=p[3],sig=positive(p[4]),T=positive(p[5]);auto n=integer(p[6],1,2500);require(p[7]==0||p[7]==1,"Call flag must be binary");
    double dt=T/n,a=sig*std::sqrt(dt),u=std::exp(a),d=1/u,q=(std::exp((r-div)*dt)-d)/(u-d),disc=std::exp(-r*dt);
    require(q>=0&&q<=1&&std::isfinite(disc),"Invalid risk-neutral tree");
    auto payoff=[&](double st){return std::max(0.0,p[7]==1?st-k:k-st);};
    Vec v(n+1),e(n+1);for(std::size_t j=0;j<=n;++j)v[j]=e[j]=payoff(s*std::exp((2.0*j-n)*a));
    for(std::size_t i=n;i-->0;)for(std::size_t j=0;j<=i;++j){v[j]=std::max(payoff(s*std::exp((2.0*j-i)*a)),disc*((1-q)*v[j]+q*v[j+1]));e[j]=disc*((1-q)*e[j]+q*e[j+1]);}
    return checked({v[0],e[0]});
}

inline Vec asian_path(const Mat& x,const Vec& p){
    inputs(x,p,3,4,2);double k=positive(p[0]),T=positive(p[2]);require(p[3]==0||p[3]==1,"Call flag must be binary");Vec payoff;
    double disc=std::exp(-p[1]*T);
    for(const auto& r:x){for(double q:r)positive(q);double a=sum(r)/3;payoff.push_back(disc*std::max(0.0,p[3]==1?a-k:k-a));}
    return checked({mean(payoff),std::sqrt(variance(payoff)/payoff.size())});
}

inline Vec butterfly_density(const Mat& x,const Vec& p){
    inputs(x,p,2,2,3);double T=positive(p[1]),minv=std::numeric_limits<double>::infinity();int bad=0;
    for(std::size_t i=0;i<x.size();++i){positive(x[i][0]);require(x[i][1]>=0,"Negative call price");if(i)require(x[i][0]>x[i-1][0],"Strikes must increase");}
    for(std::size_t i=1;i+1<x.size();++i){double l=(x[i][1]-x[i-1][1])/(x[i][0]-x[i-1][0]),r=(x[i+1][1]-x[i][1])/(x[i+1][0]-x[i][0]);double v=std::exp(p[0]*T)*2*(r-l)/(x[i+1][0]-x[i-1][0]);minv=std::min(minv,v);bad+=v<-1e-12;}
    return checked({minv,double(bad)});
}

inline Vec variance_strip(const Mat& x,const Vec& p){
    inputs(x,p,2,4,2);double F=positive(p[0]),K0=positive(p[1]),T=positive(p[3]),s=0;bool found=false;
    require(K0<=F,"Base strike must not exceed forward");
    for(std::size_t i=0;i<x.size();++i){positive(x[i][0]);require(x[i][1]>=0,"Negative option price");if(i)require(x[i][0]>x[i-1][0],"Strike grid must increase");found=found||x[i][0]==K0;}
    require(found,"Base strike missing");
    for(std::size_t i=0;i<x.size();++i){double dk=i==0?x[1][0]-x[0][0]:(i+1==x.size()?x[i][0]-x[i-1][0]:(x[i+1][0]-x[i-1][0])/2);s+=dk*x[i][1]/(x[i][0]*x[i][0]);}
    double v=(2*std::exp(p[2]*T)*s-std::pow(F/K0-1,2))/T;require(v>=0,"Negative discretized variance");return checked({v,std::sqrt(v)});
}

inline Vec discrete_delta_pnl(const Mat& x,const Vec& p){
    inputs(x,p,3,4,2);require(p[0]>=0&&p[1]>=0&&p[3]>=0&&x.front()[0]==0,"Invalid hedge setup");
    for(std::size_t i=0;i<x.size();++i){positive(x[i][1]);if(i)require(x[i][0]>x[i-1][0],"Hedge times must increase");}
    double h=x[0][2],cost=p[3]*std::abs(h)*x[0][1],cash=p[0]-h*x[0][1]-cost;
    for(std::size_t i=1;i<x.size();++i){cash*=std::exp(p[2]*(x[i][0]-x[i-1][0]));double target=i+1==x.size()?0:x[i][2],change=target-h,fee=p[3]*std::abs(change)*x[i][1];cash-=change*x[i][1]+fee;cost+=fee;h=target;}
    return checked({cash-p[1],cost});
}
} }
