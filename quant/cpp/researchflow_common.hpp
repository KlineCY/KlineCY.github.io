#pragma once
#include "io.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <functional>
#include <limits>
#include <numeric>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace kline { namespace researchflow {
using Vec=std::vector<double>;
using Mat=std::vector<Vec>;
struct Report {
    std::vector<std::string> header;
    Mat rows;
};
inline double sum(const Vec& a){return std::accumulate(a.begin(),a.end(),0.0);}
inline double dot(const Vec& a,const Vec& b){
    require(a.size()==b.size(),"Incompatible vector dimensions");
    return std::inner_product(a.begin(),a.end(),b.begin(),0.0);
}
inline double l1(const Vec& a){double v=0;for(double x:a)v+=std::abs(x);return v;}
inline double positive(double v){require(std::isfinite(v)&&v>0,"Expected a positive finite number");return v;}
inline std::size_t count(double x,std::size_t low,std::size_t high){
    require(std::isfinite(x)&&x>=double(low)&&x<=double(high)&&x==std::floor(x),"Invalid integer parameter");
    return std::size_t(x);
}
inline std::size_t validate(const Mat& a,std::size_t minrow,std::size_t mincol,std::size_t maxcol=64){
    require(a.size()>=minrow&&a.size()<=250000,"Invalid observation count");
    std::size_t d=a[0].size();
    require(d>=mincol&&d<=maxcol,"Invalid matrix column count");
    for(const auto& r:a){require(r.size()==d,"Ragged matrix");check_finite(r);}
    return d;
}
inline void parameters(const Vec& p,std::size_t n){
    require(p.size()==n,"Incorrect parameter count");
    for(double v:p)require(std::isfinite(v),"Nonfinite parameter");
}
inline Mat read_matrix(const std::string& path){
    auto t=read_csv(path);Mat out;out.reserve(t.rows.size());
    for(const auto& row:t.rows){Vec r;for(const auto& field:row)r.push_back(parse_number(field));out.push_back(std::move(r));}
    return out;
}
inline void write(std::ostream& out,const Report& a){
    require(!a.header.empty()&&!a.rows.empty(),"Empty research output");
    for(const auto& r:a.rows){require(r.size()==a.header.size(),"Malformed research output");check_finite(r);}
    for(std::size_t i=0;i<a.header.size();++i){if(i)out<<',';out<<a.header[i];}out<<'\n'<<std::setprecision(16);
    for(const auto& r:a.rows){for(std::size_t i=0;i<r.size();++i){if(i)out<<',';out<<r[i];}out<<'\n';}
}
inline Vec column(const Mat& a,std::size_t j){
    Vec out;out.reserve(a.size());for(const auto& r:a)out.push_back(r.at(j));return out;
}
inline Vec solve_spd(const Mat& a,const Vec& b){
    const std::size_t n=validate(a,1,1);
    require(n==a.size()&&b.size()==n,"Square system required");check_finite(b);
    Mat L(n,Vec(n,0));Vec z(n),x(n);
    for(std::size_t i=0;i<n;++i){
        for(std::size_t j=0;j<=i;++j){
            require(std::abs(a[i][j]-a[j][i])<=1e-10*std::max({1.0,std::abs(a[i][j]),std::abs(a[j][i])}),"Symmetric system required");
            double v=a[i][j];for(std::size_t k=0;k<j;++k)v-=L[i][k]*L[j][k];
            L[i][j]=i==j?std::sqrt(positive(v)):v/L[j][j];
        }
        double v=b[i];for(std::size_t k=0;k<i;++k)v-=L[i][k]*z[k];z[i]=v/L[i][i];
    }
    for(std::size_t i=n;i-->0;){double v=z[i];for(std::size_t k=i+1;k<n;++k)v-=L[k][i]*x[k];x[i]=v/L[i][i];}
    check_finite(x);return x;
}
inline Mat covariance_window(const Mat& r,std::size_t start,std::size_t end,double shrink,double ridge){
    require(end<=r.size()&&end>=start+2&&shrink>=0&&shrink<=1&&ridge>0,"Invalid covariance window");
    const std::size_t d=r[0].size(),n=end-start;Vec m(d,0);
    for(std::size_t t=start;t<end;++t)for(std::size_t j=0;j<d;++j)m[j]+=r[t][j]/n;
    Mat s(d,Vec(d,0));
    for(std::size_t t=start;t<end;++t)for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<=i;++j)
        s[i][j]+=(r[t][i]-m[i])*(r[t][j]-m[j])/(n-1);
    double mu=0;for(std::size_t i=0;i<d;++i)mu+=s[i][i]/d;
    for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<=i;++j){
        double v=(1-shrink)*s[i][j]+(i==j?shrink*mu+ridge:0);
        s[i][j]=s[j][i]=v;
    }
    return s;
}
struct PortfolioStep {double net, gross, fee, turnover;Vec drift;};
inline PortfolioStep trade_step(const Vec& old,const Vec& target,const Vec& ret,double cost){
    require(old.size()==target.size()&&target.size()==ret.size(),"Portfolio dimensions differ");
    check_finite(old);check_finite(target);check_finite(ret);
    require(cost>=0&&cost<1&&cost*l1(target)<1,"Invalid transaction cost or leverage");
    for(double r:ret)require(r>=-1,"Return below -100 percent");
    // Fee equation is expressed as a fraction of pre-trade equity.
    auto turnover=[&](double fee){double q=0;for(std::size_t j=0;j<old.size();++j)q+=std::abs((1-fee)*target[j]-old[j]);return q;};
    require(cost*l1(old)<1,"Liquidation cost exhausts equity");
    double lo=0,hi=1;
    for(int k=0;k<65;++k){double m=(lo+hi)/2;if(m<cost*turnover(m))lo=m;else hi=m;}
    double fee=cost==0?0:(lo+hi)/2,turn=turnover(fee);
    double gross=dot(target,ret),g=positive(1+gross);
    Vec drift(target.size());
    for(std::size_t j=0;j<drift.size();++j)drift[j]=target[j]*(1+ret[j])/g;
    double net=positive((1-fee)*g)-1;
    return {net,gross,fee,turn,drift};
}
template<class Rule>inline Report backtest(const Mat& r,std::size_t look,double cost,Rule rule){
    const auto d=r[0].size();
    require(look>=2&&look<r.size()&&cost>=0&&cost<1,"Invalid lookback or cost");
    require(double(r.size()-look)*look*d*d<=2e8,"Portfolio research work limit exceeded");
    for(const auto& row:r)for(double v:row)require(v>=-1,"Return below -100 percent");
    Report out{{"Period","GrossReturn","NetReturn","Turnover","FeeFraction","NAV","Drawdown","GrossExposure","NetExposure"},{}};
    for(std::size_t j=0;j<d;++j)out.header.push_back("Weight"+std::to_string(j+1));
    Vec old(d,0);double nav=1,peak=1;
    for(std::size_t t=look;t<r.size();++t){
        Vec w=rule(t);
        auto step=trade_step(old,w,r[t],cost);nav=positive(nav*(1+step.net));peak=std::max(peak,nav);
        Vec row={double(t),step.gross,step.net,step.turnover,step.fee,nav,1-nav/peak,l1(w),sum(w)};
        row.insert(row.end(),w.begin(),w.end());out.rows.push_back(std::move(row));old=std::move(step.drift);
    }
    return out;
}
} }
