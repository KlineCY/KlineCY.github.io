#pragma once
#include "evidence_common.hpp"
namespace kline { namespace evidence {
inline Report heterogeneous_cost_rebalance(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,0);double target=0,old=0;
    for(const auto& r:x){require(r[2]>=0&&r[2]<1,"Invalid cost rate");target+=r[2]*std::abs(r[1]);old+=r[2]*std::abs(r[0]);}
    require(target<1&&old<1,"Fee fixed point not economically feasible");
    auto g=[&](double f){double v=0;for(const auto& r:x)v+=r[2]*std::abs((1-f)*r[1]-r[0]);return v;};
    double lo=0,hi=1;for(int k=0;k<80;++k){double m=(lo+hi)/2;if(g(m)>m)lo=m;else hi=m;}
    double f=(lo+hi)/2;Report o{{"Asset","OldWeight","TargetWeight","SignedTrade","Fee","TotalFee","PostTradeEquity"},{}};
    for(std::size_t i=0;i<x.size();++i){const auto& r=x[i];double q=(1-f)*r[1]-r[0];o.rows.push_back({double(i+1),r[0],r[1],q,r[2]*std::abs(q),f,1-f});}
    return o;
}

inline Report corporate_action_cashbook(const Mat& x,const Vec& p){
    validate(x,1,5,5);parameters(p,2);double cash=p[0],shares=p[1];require(cash>=0&&shares>=0,"Nonnegative initial account required");
    Report o{{"Period","Shares","Cash","MarketValue","Equity","DividendCash","Fee"},{}};
    for(std::size_t t=0;t<x.size();++t){const auto& r=x[t];positive(r[0]);positive(r[1]);require(r[2]>=0&&r[4]>=0,"Negative dividend or fee");
        double div=shares*r[2];cash+=div-r[0]*r[3]-r[4];shares=shares*r[1]+r[3];
        require(shares>=0&&cash>=0,"Account cannot finance the proposed trade");
        o.rows.push_back({double(t),shares,cash,r[0]*shares,cash+r[0]*shares,div,r[4]});
    }
    return o;
}

inline Report futures_margin_cashbook(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,2);double cash=positive(p[0]),mult=positive(p[1]),old=0,injected=0,prev=x[0][0];
    Report o{{"Period","Contracts","VariationMargin","Fee","Cash","RequiredMargin","FreeCash","ExternalTopup","EquityNetOfTopups"},{}};
    for(std::size_t t=0;t<x.size();++t){const auto& r=x[t];positive(r[0]);require(std::floor(r[1])==r[1]&&r[2]>0&&r[2]<=1&&r[3]>=0,"Invalid futures settlement row");
        double pnl=old*mult*(r[0]-prev),need=std::abs(r[1])*mult*r[0]*r[2];cash+=pnl-r[3];
        double top=std::max(0.0,need-cash);cash+=top;injected+=top;
        o.rows.push_back({double(t),r[1],pnl,r[3],cash,need,cash-need,top,cash-injected});old=r[1];prev=r[0];
    }
    return o;
}

inline Report currency_return_attribution(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,0);Report o{{"Period","Local","FX","Interaction","BaseReturn","LocalContribution","FXContribution","InteractionContribution","TotalContribution"},{}};
    for(std::size_t t=0;t<x.size();++t){const auto& r=x[t];require(r[0]>=-1&&r[1]>-1,"Invalid simple return");
        double q=r[0]*r[1],z=r[0]+r[1]+q,w=r[2];
        o.rows.push_back({double(t),r[0],r[1],q,z,w*r[0],w*r[1],w*q,w*z});}
    return o;
}
} }
