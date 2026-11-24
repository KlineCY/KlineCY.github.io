#pragma once
#include "numerical_lab_common.hpp"

namespace kline { namespace numerical_lab {
inline Report best_quote_ofi(const Mat& x,const Vec& p){
    validate(x,2,5,5);parameters(p,0);
    Report out{{"Time","Midpoint","Spread","BidContribution","AskContribution","OFI","CumulativeOFI"},{}};
    double acc=0;
    for(std::size_t i=0;i<x.size();++i){
        const auto& r=x[i];positive(r[1]);require(r[2]>r[1]&&r[3]>=0&&r[4]>=0,"Invalid two-sided quote");
        if(i==0)continue;
        const auto& z=x[i-1];require(r[0]>z[0],"Quote timestamps must strictly increase");
        double bid=(r[1]>=z[1]?r[3]:0)-(r[1]<=z[1]?z[3]:0);
        double ask=(r[2]<=z[2]?r[4]:0)-(r[2]>=z[2]?z[4]:0);
        acc+=bid-ask;
        out.rows.push_back({r[0],.5*(r[1]+r[2]),r[2]-r[1],bid,-ask,bid-ask,acc});
    }
    return out;
}
inline Report trade_sign_quote_tick(const Mat& x,const Vec& p){
    validate(x,1,5,5);parameters(p,0);
    Report out{{"Time","TradePrice","Midpoint","InferredSign","SignedQuantity","Method"},{}};
    double prev=0,tick=0;
    for(std::size_t i=0;i<x.size();++i){
        const auto& r=x[i];positive(r[1]);positive(r[2]);positive(r[3]);require(r[3]>r[2]&&r[4]>0,"Invalid trade or quote");
        if(i){require(r[0]>=x[i-1][0],"Trade timestamps must not decrease");if(r[1]!=prev)tick=r[1]>prev?1:-1;}
        double mid=.5*(r[2]+r[3]),sgn=tick,method=tick==0?0:2;
        if(r[1]!=mid){sgn=r[1]>mid?1:-1;method=1;}
        out.rows.push_back({r[0],r[1],mid,sgn,sgn*r[4],method});prev=r[1];
    }
    return out;
}
inline Report preaveraging_variation(const Mat& x,const Vec& p){
    validate(x,8,1,1);parameters(p,1);Vec price=column(x,0),r;
    auto k=count(p[0],2,(x.size()-1)/2);
    for(double v:price)positive(v);
    for(std::size_t i=1;i<price.size();++i)r.push_back(std::log(price[i]/price[i-1]));
    double psi2=0,psi1=0;Vec g(k+1,0);
    for(std::size_t j=0;j<=k;++j){double t=double(j)/k;g[j]=std::min(t,1-t);psi2+=g[j]*g[j]/k;if(j)psi1+=std::pow(g[j]-g[j-1],2)*k;}
    double raw=dot(r,r),pre=0;
    for(std::size_t i=0;i+k<=r.size();++i){double z=0;for(std::size_t j=1;j<k;++j)z+=g[j]*r[i+j-1];pre+=z*z;}
    double correction=psi1*raw/(2*k*k*psi2),estimate=pre/(k*psi2)-correction;
    return scalar_rows({"Returns","Window","RawRV","PreaveragedSum","NoiseCorrection","IntegratedVarianceEstimate"},{double(r.size()),double(k),raw,pre/(k*psi2),correction,estimate});
}
inline Report time_markout(const Mat& x,const Vec& p){
    validate(x,2,5,5);require(!p.empty()&&p.size()<=32,"At least one markout horizon required");check_finite(p);
    Vec time;for(std::size_t i=0;i<x.size();++i){
        const auto& r=x[i];if(i)require(r[0]>x[i-1][0],"Observation timestamps must increase");
        positive(r[1]);positive(r[2]);require(r[3]==1||r[3]==-1||r[3]==0,"Invalid trade direction");
        require(r[4]>=0&&((r[3]==0)==(r[4]==0)),"Inconsistent trade quantity");time.push_back(r[0]);
    }
    Report out{{"Horizon","MatchedTrades","UnmatchedTrades","MatchedQuantity","WeightedMarkoutBps"},{}};
    for(double h:p){
        positive(h);double qty=0,total=0,matched=0,miss=0;
        for(std::size_t i=0;i<x.size();++i){
            if(x[i][3]==0)continue;
            // Last available observed midpoint at or before the target, with no extrapolation beyond the sample.
            double target=time[i]+h;
            if(target>time.back()){++miss;continue;}
            auto j=std::size_t(std::upper_bound(time.begin(),time.end(),target)-time.begin()-1);
            ++matched;double q=x[i][4];qty+=q;total+=q*x[i][3]*(x[j][1]-x[i][2])/x[i][2]*10000;
        }
        require(qty>0,"No matched trades at requested horizon");out.rows.push_back({h,matched,miss,qty,total/qty});
    }
    return out;
}
inline Report depth_liquidity_sweep(const Mat& x,const Vec& p){
    validate(x,1,3,3);require(!p.empty()&&p.size()<=100,"At least one order quantity required");check_finite(p);
    // Ask ladder: strictly increasing prices, available size and explicit per-unit fees.
    for(std::size_t i=0;i<x.size();++i){positive(x[i][0]);require(x[i][1]>=0&&x[i][2]>=0,"Invalid depth or fee");if(i)require(x[i][0]>x[i-1][0],"Ask prices must increase");}
    Report out{{"RequestedQuantity","FilledQuantity","UnfilledQuantity","VWAP","Fee","CostAboveBestAsk","AllInUnitCost"},{}};
    for(double q:p){
        positive(q);double left=q,notional=0,fee=0;
        for(const auto& r:x){double v=std::min(left,r[1]);notional+=v*r[0];fee+=v*r[2];left-=v;if(left==0)break;}
        double fill=positive(q-left);
        out.rows.push_back({q,fill,left,notional/fill,fee,notional+fee-x[0][0]*fill,(notional+fee)/fill});
    }
    return out;
}
} }
