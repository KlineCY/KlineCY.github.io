#pragma once
#include "researchflow_common.hpp"
#include <map>

namespace kline { namespace researchflow {
inline Report order_tca(const Mat& x,const Vec& p){
    validate(x,1,8,8);parameters(p,0);
    struct Order {double side=0,arrival=0,target=0,mark=0,quantity=0,notional=0,fees=0;};
    std::map<std::size_t,Order> orders;
    for(const auto& r:x){
        auto id=count(r[0],0,1000000000);
        require(r[1]==1||r[1]==-1,"Side must be +1 for buy or -1 for sell");
        positive(r[2]);positive(r[3]);positive(r[4]);positive(r[7]);
        require(r[5]>=0&&r[6]>=0,"Negative fill or fee");
        auto& o=orders[id];
        if(o.side==0){o.side=r[1];o.arrival=r[2];o.target=r[3];o.mark=r[7];}
        require(o.side==r[1]&&o.arrival==r[2]&&o.target==r[3]&&o.mark==r[7],"Inconsistent order metadata");
        o.quantity+=r[5];o.notional+=r[4]*r[5];o.fees+=r[6];
        require(o.quantity<=o.target+1e-10*o.target,"Order overfilled");
    }
    Report out{{"Order","Side","TargetQuantity","FilledQuantity","UnfilledQuantity","FillRate","ExecutionCost","OpportunityCost","Fees","TotalShortfall","ShortfallBps"},{}};
    for(const auto& q:orders){
        const auto& o=q.second;double remaining=std::max(0.0,o.target-o.quantity);
        double execution=o.side*(o.notional-o.arrival*o.quantity);
        double opportunity=o.side*(o.mark-o.arrival)*remaining;
        double total=execution+opportunity+o.fees;
        out.rows.push_back({double(q.first),o.side,o.target,o.quantity,remaining,o.quantity/o.target,execution,opportunity,o.fees,total,total/(o.target*o.arrival)*10000});
    }
    return out;
}
inline Report participation_audit(const Mat& x,const Vec& p){
    validate(x,1,5,5);parameters(p,4);double side=p[0],target=positive(p[1]),arrival=positive(p[2]),cap=p[3];
    require((side==1||side==-1)&&cap>0&&cap<=1,"Invalid side or participation cap");
    Report out{{"Period","FillQuantity","CumulativeFilled","Remaining","Participation","CapBreach","MarketVWAPToDate","ExecutionVWAPToDate","ExecutionShortfallToDate","FeeToDate"},{}};
    double market=0,marketvalue=0,fill=0,notional=0,fees=0;
    for(std::size_t t=0;t<x.size();++t){
        const auto& r=x[t];positive(r[0]);positive(r[1]);positive(r[3]);require(r[2]>=0&&r[2]<=r[0]&&r[4]>=0,"Invalid market or fill data");
        market+=r[0];marketvalue+=r[0]*r[1];fill+=r[2];notional+=r[2]*r[3];fees+=r[4];
        require(fill<=target+1e-10*target,"Target quantity exceeded");
        double share=r[2]/r[0];
        out.rows.push_back({double(t),r[2],fill,std::max(0.0,target-fill),share,share>cap+1e-12?1.0:0.0,marketvalue/market,fill>0?notional/fill:0,side*(notional-arrival*fill)+fees,fees});
    }
    return out;
}
inline Report spread_decomposition(const Mat& x,const Vec& p){
    validate(x,1,5,5);parameters(p,0);
    Report out{{"Trade","EffectiveSpreadBps","RealizedSpreadBps","PriceImpactBps","CumulativeEffectiveBps","CumulativeRealizedBps","CumulativeImpactBps"},{}};
    double volume=0,es=0,rs=0,impact=0;
    for(std::size_t i=0;i<x.size();++i){
        const auto& r=x[i];require(r[0]==1||r[0]==-1,"Trade sign must be binary");positive(r[1]);positive(r[2]);positive(r[3]);positive(r[4]);
        double e=2*r[0]*(r[2]-r[1])/r[1]*10000;
        double s=2*r[0]*(r[2]-r[3])/r[1]*10000;
        double z=2*r[0]*(r[3]-r[1])/r[1]*10000;
        volume+=r[4];es+=e*r[4];rs+=s*r[4];impact+=z*r[4];
        out.rows.push_back({double(i),e,s,z,es/volume,rs/volume,impact/volume});
    }
    return out;
}
inline Report optimal_participation(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,2);double target=positive(p[0]),rho=p[1];
    require(rho>0&&rho<=1,"Participation cap must be in (0,1]");
    double cap=0,lo=std::numeric_limits<double>::infinity(),hi=-lo;
    for(const auto& r:x){
        positive(r[0]);require(r[1]>=0,"Negative linear execution cost");positive(r[2]);
        cap+=rho*r[0];lo=std::min(lo,r[1]);hi=std::max(hi,r[1]+2*r[2]*rho);
    }
    require(target<=cap,"Parent order exceeds forecast capacity");
    auto quantity=[&](const Vec& r,double nu){return std::clamp((nu-r[1])*r[0]/(2*r[2]),0.0,rho*r[0]);};
    for(int i=0;i<100;++i){
        double m=lo+(hi-lo)/2,s=0;for(const auto& r:x)s+=quantity(r,m);
        if(s<target)lo=m;else hi=m;
    }
    double nu=lo+(hi-lo)/2,total=0;
    Report out{{"Period","ForecastVolume","AllocatedQuantity","Participation","ExpectedCost","CumulativeQuantity","DualPrice"},{}};
    for(std::size_t i=0;i<x.size();++i){
        const auto& r=x[i];double q=quantity(r,nu);total+=q;
        out.rows.push_back({double(i),r[0],q,q/r[0],r[1]*q+r[2]*q*q/r[0],total,nu});
    }
    require(std::abs(total-target)<1e-8*std::max(1.0,target),"Quantity constraint not solved");
    return out;
}
inline Report competing_fill(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,0);
    std::map<double,Vec> events;
    for(const auto& r:x){
        require(r[0]>=0,"Negative elapsed time");auto e=count(r[1],0,2);
        auto& n=events[r[0]];if(n.empty())n=Vec(3,0);++n[e];
    }
    double risk=double(x.size()),survival=1,fill=0,cancel=0;
    Report out{{"Elapsed","AtRisk","Fills","Cancellations","Censored","StillOpenProbability","FillIncidence","CancelIncidence"},{}};
    for(const auto& entry:events){
        const auto& n=entry.second;double f=n[1],c=n[2];
        fill+=survival*f/risk;cancel+=survival*c/risk;
        survival*=1-(f+c)/risk;
        out.rows.push_back({entry.first,risk,f,c,n[0],survival,fill,cancel});risk-=sum(n);
    }
    return out;
}
} }
