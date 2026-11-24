#pragma once
#include "researchflow_common.hpp"

namespace kline { namespace researchflow {

inline Report cta(const Mat& r,const Vec& p){
    std::size_t d=validate(r,3,2);parameters(p,5);
    auto look=count(p[0],2,r.size()-1);
    double floor=positive(p[2]),budget=positive(p[3]),cap=positive(p[4]);
    require(budget<=4&&cap<=budget,"Invalid CTA risk allocation");
    return backtest(r,look,p[1],[&](std::size_t t){
        Vec w(d,0);
        for(std::size_t j=0;j<d;++j){
            Vec x;double wealth=1;
            for(std::size_t k=t-look;k<t;++k){x.push_back(r[k][j]);wealth*=1+r[k][j];}
            double trend=wealth-1,vol=std::max(floor,std::sqrt(variance(x)));
            if(std::abs(trend)>1e-12)w[j]=(trend>0?1:-1)/vol;
        }
        double scale=l1(w);
        if(scale>0)for(double& v:w)v=std::clamp(budget*v/scale,-cap,cap);
        return w;
    });
}
inline Report cross_section(const Mat& r,const Vec& p){
    std::size_t d=validate(r,3,2);parameters(p,4);
    auto look=count(p[0],2,r.size()-1),k=count(p[2],1,d/2);
    double gross=positive(p[3]);require(gross<=4,"Gross limit exceeds four");
    return backtest(r,look,p[1],[&](std::size_t t){
        Vec score(d,1),w(d,0);std::vector<std::size_t> ix(d);
        std::iota(ix.begin(),ix.end(),0);
        for(std::size_t s=t-look;s<t;++s)for(std::size_t j=0;j<d;++j)score[j]*=1+r[s][j];
        std::stable_sort(ix.begin(),ix.end(),[&](std::size_t a,std::size_t b){return score[a]<score[b];});
        // A tied boundary is not split into arbitrary long and short positions.
        if(score[ix[k-1]]==score[ix[k]]||score[ix[d-k-1]]==score[ix[d-k]])return w;
        for(std::size_t j=0;j<k;++j){w[ix[j]]=-gross/(2*k);w[ix[d-1-j]]=gross/(2*k);}
        return w;
    });
}
inline Report minimum_variance(const Mat& r,const Vec& p){
    std::size_t d=validate(r,3,2);parameters(p,5);
    auto look=count(p[0],2,r.size()-1);double shrink=p[2],ridge=positive(p[3]),maxgross=positive(p[4]);
    require(shrink>=0&&shrink<=1&&maxgross>=1&&maxgross<=4,"Invalid covariance or gross constraint");
    return backtest(r,look,p[1],[&](std::size_t t){
        auto s=covariance_window(r,t-look,t,shrink,ridge);
        Vec w=solve_spd(s,Vec(d,1));double z=positive(sum(w));
        for(double& q:w)q/=z;
        // Infeasible unconstrained solution is rejected, not advertised as a constrained optimum.
        require(l1(w)<=maxgross+1e-10,"Unconstrained minimum variance exceeds gross limit");
        return w;
    });
}
inline Report equal_risk(const Mat& r,const Vec& p){
    std::size_t d=validate(r,3,2);parameters(p,5);
    auto look=count(p[0],2,r.size()-1);double shrink=p[2],ridge=positive(p[3]);
    auto sweeps=count(p[4],1,5000);require(shrink>=0&&shrink<=1,"Invalid shrinkage");
    return backtest(r,look,p[1],[&](std::size_t t){
        const auto s=covariance_window(r,t-look,t,shrink,ridge);
        Vec w(d);double b=1.0/d;
        for(std::size_t j=0;j<d;++j)w[j]=std::sqrt(b/s[j][j]);
        double residual=1;
        for(std::size_t k=0;k<sweeps;++k){
            for(std::size_t j=0;j<d;++j){
                double c=0;for(std::size_t l=0;l<d;++l)if(l!=j)c+=s[j][l]*w[l];
                double root=std::hypot(c,2*std::sqrt(s[j][j]*b));
                w[j]=c>=0?2*b/(c+root):(root-c)/(2*s[j][j]);
            }
            residual=0;
            for(std::size_t j=0;j<d;++j)residual=std::max(residual,std::abs(w[j]*dot(s[j],w)-b));
            if(residual<1e-10)break;
        }
        require(residual<1e-7,"Risk parity did not converge");
        double z=positive(sum(w));for(double& v:w)v/=z;
        return w;
    });
}
inline Report performance(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,3);
    double annual=positive(p[0]),rf=p[1],alpha=p[2];
    require(rf>-1&&alpha>0&&alpha<1,"Invalid rate or tail probability");
    Vec r=column(x,0),benchmark=column(x,1),active,loss;
    double nav=1,bnav=1,peak=1,mdd=0,er=std::expm1(std::log1p(rf)/annual);
    for(std::size_t i=0;i<x.size();++i){
        require(r[i]>-1&&benchmark[i]>-1,"NAV metrics require positive wealth");
        nav=positive(nav*(1+r[i]));bnav=positive(bnav*(1+benchmark[i]));
        peak=std::max(peak,nav);mdd=std::max(mdd,1-nav/peak);
        active.push_back(r[i]-benchmark[i]);loss.push_back(-r[i]);
    }
    double sd=std::sqrt(positive(variance(r))),te=std::sqrt(positive(variance(active)));
    double beta=covariance(r,benchmark)/positive(variance(benchmark));
    std::sort(loss.begin(),loss.end(),std::greater<double>());
    double mass=(1-alpha)*loss.size(),left=mass,es=0;
    for(double v:loss){double a=std::min(1.0,left);es+=a*v;left-=a;if(left<=0)break;}
    auto q=std::size_t(std::ceil(alpha*loss.size())-1);
    return {{"Observations","TotalReturn","BenchmarkReturn","CAGR","AnnualVol","SharpeIID","MaxDrawdown","TrackingError","InformationRatioIID","Beta","VaR","ExpectedShortfall"},
        {{double(r.size()),nav-1,bnav-1,std::pow(nav,annual/r.size())-1,sd*std::sqrt(annual),(mean(r)-er)/sd*std::sqrt(annual),mdd,te*std::sqrt(annual),mean(active)/te*std::sqrt(annual),beta,loss[loss.size()-1-q],es/mass}}};
}
} }
