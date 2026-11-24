#pragma once
#include "lab24_common.hpp"
#include <map>

namespace kline { namespace lab24 {
inline Vec scalar_kalman(const Mat& x,const Vec& p){
    inputs(x,p,2,3);double m=p[0],v=p[1],q=p[2],ll=0;
    require(v>=0&&q>=0,"Invalid state variance");
    for(const auto& r:x){
        positive(r[1]);double prior=v+q,s=positive(prior+r[1]),e=r[0]-m,k=prior/s;
        m+=k*e;v=(1-k)*(1-k)*prior+k*k*r[1];
        ll-=0.5*(std::log(2*std::acos(-1.0)*s)+e*e/s);
    }
    return checked({m,v,ll});
}

inline Vec hmm_forward(const Mat& x,const Vec& p){
    inputs(x,p,2,3);for(double q:p)require(q>=0&&q<=1,"Invalid probability");
    double a=p[0],b=1-a,ll=0;
    for(std::size_t t=0;t<x.size();++t){
        if(t){double z=a*p[1]+b*(1-p[2]);b=a*(1-p[1])+b*p[2];a=z;}
        require(x[t][0]>=0&&x[t][1]>=0,"Negative observation likelihood");
        a*=x[t][0];b*=x[t][1];double z=positive(a+b);ll+=std::log(z);a/=z;b/=z;
    }
    return checked({a,b,ll});
}

inline Vec viterbi(const Mat& x,const Vec& p){
    inputs(x,p,2,3);for(double q:p)require(q>0&&q<1,"Transition and prior must lie in (0,1)");
    for(const auto& r:x){positive(r[0]);positive(r[1]);}
    double a=std::log(p[0])+std::log(x[0][0]),b=std::log(1-p[0])+std::log(x[0][1]);
    std::vector<std::pair<int,int>> back(x.size());
    for(std::size_t t=1;t<x.size();++t){
        double aa=a+std::log(p[1]),ba=b+std::log(1-p[2]),ab=a+std::log(1-p[1]),bb=b+std::log(p[2]);
        back[t]={aa>=ba?0:1,ab>=bb?0:1};a=std::max(aa,ba)+std::log(x[t][0]);b=std::max(ab,bb)+std::log(x[t][1]);
    }
    Vec states(x.size());int z=a>=b?0:1;
    for(std::size_t t=x.size();t-->0;){states[t]=z;if(t)z=z==0?back[t].first:back[t].second;}
    return states;
}

inline Vec ou_transition_fit(const Mat& x,const Vec& p){
    inputs(x,p,1,1,4);double dt=positive(p[0]);Vec a,b;
    for(std::size_t i=1;i<x.size();++i){a.push_back(x[i-1][0]);b.push_back(x[i][0]);}
    double phi=covariance(a,b,false)/positive(variance(a,false));
    require(phi>0&&phi<1,"OU autoregressive coefficient must lie in (0,1)");
    double alpha=mean(b)-phi*mean(a),k=-std::log(phi)/dt,sse=0;
    for(std::size_t i=0;i<a.size();++i){double e=b[i]-alpha-phi*a[i];sse+=e*e;}
    return checked({k,alpha/(1-phi),std::sqrt(2*k*(sse/a.size())/(1-phi*phi))});
}

inline Vec markov_hitting(const Mat& x,const Vec& p){
    inputs(x,p,1,1);require(x.size()==1,"Single normal-state probability required");
    auto h=integer(p[0],1,1000000);double s=x[0][0];require(s>=0&&s<1,"Stay probability must be in [0,1)");
    double hit=s==0?1:-std::expm1(double(h)*std::log(s));
    return checked({hit,1/(1-s)});
}
} }
