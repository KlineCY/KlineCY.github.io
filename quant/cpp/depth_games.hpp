#pragma once
#include "evidence_common.hpp"
namespace kline { namespace depth_games {
using namespace evidence;
inline Report mixed_nash_interior(const Mat& x,const Vec& p){
    validate(x,2,4,4);parameters(p,0);require(x.size()==2,"Two strategies per player required");double a=x[0][0],b=x[0][1],c=x[1][0],d=x[1][1],e=x[0][2],f=x[0][3],g=x[1][2],h=x[1][3];double u=a-b-c+d,v=e-f-g+h;require(std::abs(u)>1e-14&&std::abs(v)>1e-14,"Degenerate indifference");double q=(d-b)/u,z=(h-g)/v;require(z>0&&z<1&&q>0&&q<1,"No strict interior solution");double r1=q*a+(1-q)*b,r2=q*c+(1-q)*d,c1=z*e+(1-z)*g,c2=z*f+(1-z)*h;return scalar({"RowProbFirst","ColProbFirst","RowExpectedPayoff","ColExpectedPayoff","RowIndifferenceGap","ColIndifferenceGap"},{z,q,z*r1+(1-z)*r2,q*c1+(1-q)*c2,r1-r2,c1-c2});
}

inline Report cournot_active_set(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,2);double A=positive(p[0]),B=positive(p[1]),lo=0,hi=A;for(auto&r:x)require(r[0]>=0,"Negative marginal cost");
    for(int k=0;k<80;++k){double m=(lo+hi)/2,s=m;for(auto&r:x)s+=std::max(m-r[0],0.);if(s>A)hi=m;else lo=m;}double P=(lo+hi)/2;Report o{{"Firm","Quantity","MarketPrice","Profit","KKTGap"},{}};
    for(std::size_t i=0;i<x.size();++i){double c=x[i][0],q=std::max(0.,P-c)/B,g=P-c-B*q;o.rows.push_back({double(i+1),q,P,(P-c)*q,q>0?std::abs(g):std::max(0.,g)});}return o;
}

inline Report auction_uniform_bid(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,1);double n=count(p[0],2,1000000);Report o{{"Valuation","EquilibriumBid","WinProbability","ExpectedUtility"},{}};
    for(auto&r:x){double v=r[0];require(v>=0&&v<=1,"Valuation outside support");double q=std::pow(v,n-1),b=v*(n-1)/n;o.rows.push_back({v,b,q,q*(v-b)});}return o;
}

inline Report kyle_linear_equilibrium(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,0);Report o{{"InsiderSlope","PriceImpact","ResidualValueVariance","ExpectedInsiderProfit"},{}};
    for(auto&r:x){double v=std::sqrt(positive(r[0])),u=std::sqrt(positive(r[1]));o.rows.push_back({u/v,v/(2*u),r[0]/2,v*u/2});}return o;
}
} }
