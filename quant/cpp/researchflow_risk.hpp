#pragma once
#include "researchflow_ml.hpp"
#include <cstdint>
#include <functional>

namespace kline { namespace researchflow {
inline std::pair<double,double> tail(Vec loss,double alpha){
    check_finite(loss);require(alpha>0&&alpha<1,"Invalid tail confidence");
    std::sort(loss.begin(),loss.end(),std::greater<double>());
    std::size_t q=std::size_t(std::ceil(alpha*loss.size())-1);
    double mass=(1-alpha)*loss.size(),left=mass,v=0;
    for(double a:loss){double w=std::min(1.0,left);v+=w*a;left-=w;if(left<=0)break;}
    return {loss[loss.size()-1-q],v/mass};
}
inline Report rolling_tail(const Mat& x,const Vec& p){
    validate(x,3,1,1);parameters(p,2);auto look=count(p[0],2,x.size()-1);
    double alpha=p[1];require(alpha>0&&alpha<1,"Invalid tail probability");
    Report out{{"Period","Loss","ForecastVaR","ForecastES","VaRBreach","TailScore"},{}};
    for(std::size_t t=look;t<x.size();++t){
        Vec loss;for(std::size_t i=t-look;i<t;++i)loss.push_back(-x[i][0]);
        auto risk=tail(loss,alpha);double l=-x[t][0],e=l-risk.first;
        out.rows.push_back({double(t),l,risk.first,risk.second,l>risk.first?1.0:0.0,e*(alpha-(e<0?1.0:0.0))});
    }
    return out;
}
inline double binary_ll(double zero,double one,double q){
    double v=0;if(zero>0){require(q<1,"Impossible zero event");v+=zero*std::log1p(-q);}
    if(one>0){require(q>0,"Impossible one event");v+=one*std::log(q);}
    return v;
}
inline Report coverage(const Mat& x,const Vec& p){
    validate(x,3,1,1);parameters(p,1);double q=p[0];require(q>0&&q<1,"Breach probability must lie in (0,1)");
    double n=double(x.size()),hits=0,t[2][2]={{0,0},{0,0}};
    for(std::size_t i=0;i<x.size();++i){
        require(x[i][0]==0||x[i][0]==1,"Binary breach series required");hits+=x[i][0];
        if(i)t[int(x[i-1][0])][int(x[i][0])]+=1;
    }
    double uc=2*(binary_ll(n-hits,hits,hits/n)-binary_ll(n-hits,hits,q));
    double total1=t[0][1]+t[1][1],total0=t[0][0]+t[1][0];
    double independent=binary_ll(total0,total1,total1/(n-1)),dependent=0;
    for(int i=0;i<2;++i){double m=t[i][0]+t[i][1];if(m>0)dependent+=binary_ll(t[i][0],t[i][1],t[i][1]/m);}
    double ind=std::max(0.0,2*(dependent-independent));
    uc=std::max(0.0,uc);
    return {{"Observations","Breaches","ExpectedBreaches","BreachRate","N00","N01","N10","N11","LRUnconditional","LRIndependence","LRConditional","AsymptoticPUnconditional","AsymptoticPIndependence","AsymptoticPConditional"},
        {{n,hits,n*q,hits/n,t[0][0],t[0][1],t[1][0],t[1][1],uc,ind,uc+ind,std::erfc(std::sqrt(uc/2)),std::erfc(std::sqrt(ind/2)),std::exp(-(uc+ind)/2)}}};
}
inline Report moving_block_paths(const Mat& x,const Vec& p){
    validate(x,3,1,1);parameters(p,4);
    auto block=count(p[0],1,x.size()),horizon=count(p[1],1,10000),scenarios=count(p[2],1,10000);
    require(horizon*scenarios<=5000000,"Simulation budget exceeded");
    std::uint32_t state=std::uint32_t(count(p[3],0,4294967295ULL));
    for(const auto& r:x)require(r[0]>-1,"Path requires positive wealth");
    Report out{{"Scenario","TerminalNAV","TotalReturn","MaxDrawdown"},{}};
    const auto available=std::uint64_t(x.size()-block+1);
    auto sample=[&](){
        // Deterministic 32-bit generator; rejection avoids modulo bias.
        const std::uint64_t range=std::uint64_t(1)<<32,limit=range-range%available;
        do{state=std::uint32_t(std::uint64_t(state)*1664525ULL+1013904223ULL);}while(std::uint64_t(state)>=limit);
        return std::size_t(std::uint64_t(state)%available);
    };
    for(std::size_t s=0;s<scenarios;++s){
        double nav=1,peak=1,mdd=0;std::size_t t=0;
        while(t<horizon){auto j=sample();for(std::size_t k=0;k<block&&t<horizon;++k,++t){nav=positive(nav*(1+x[j+k][0]));peak=std::max(peak,nav);mdd=std::max(mdd,1-nav/peak);}}
        out.rows.push_back({double(s),nav,nav-1,mdd});
    }
    return out;
}
inline Report factor_attribution(const Mat& x,const Vec& p){
    validate(x,3,3);require(p.size()>=2&&p.size()<=16,"Training, Ridge penalty and factor shock vector required");check_finite(p);
    const auto d=x[0].size()-1,look=count(p[0],2,x.size()-1);
    require(p.size()==d+2,"One scenario shock per factor required");
    double ridge=positive(p[1]);
    Report out{{"Period","Actual","Predicted","Residual","StressFactorPnL","ResidualVol"},{}};
    for(std::size_t j=0;j<d;++j)out.header.push_back("Beta"+std::to_string(j+1));
    for(std::size_t t=look;t<x.size();++t){
        auto fit=ridge_fit(x,t-look,t,ridge);Vec beta(d);
        for(std::size_t j=0;j<d;++j)beta[j]=fit.beta[j]/fit.scale[j];
        double v=predict(fit,x[t]),stress=0,sse=0;
        for(std::size_t j=0;j<d;++j)stress+=beta[j]*p[j+2];
        for(std::size_t i=t-look;i<t;++i){double e=x[i].back()-predict(fit,x[i]);sse+=e*e/look;}
        Vec row={double(t),x[t].back(),v,x[t].back()-v,stress,std::sqrt(sse)};
        row.insert(row.end(),beta.begin(),beta.end());out.rows.push_back(std::move(row));
    }
    return out;
}
inline Report risk_decomposition(const Mat& x,const Vec& p){
    auto d=validate(x,3,2,32);require(p.size()==d+2,"Shrinkage, ridge and fixed weights required");check_finite(p);
    double shrink=p[0],ridge=positive(p[1]);require(shrink>=0&&shrink<=1,"Invalid shrinkage");
    Vec w(p.begin()+2,p.end());require(l1(w)>0,"Zero portfolio risk");
    auto s=covariance_window(x,0,x.size(),shrink,ridge);
    Vec m(d);for(std::size_t j=0;j<d;++j)m[j]=dot(s[j],w);
    double v=positive(dot(w,m)),vol=std::sqrt(v);
    Report out{{"Asset","Weight","MarginalVolatility","ComponentVolatility","RiskShare","PortfolioVolatility"},{}};
    for(std::size_t j=0;j<d;++j)out.rows.push_back({double(j+1),w[j],m[j]/vol,w[j]*m[j]/vol,w[j]*m[j]/v,vol});
    return out;
}
} }
