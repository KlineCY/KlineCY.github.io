#pragma once
#include "system_research_common.hpp"
#include <map>
#include <set>
#include <deque>
#include <tuple>
namespace kline { namespace system_research {
inline Report venue_convex_routing(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,1);double Q=p[0],cap=0,lo=std::numeric_limits<double>::infinity(),hi=-lo;std::set<double> seen;for(auto&r:x){require(seen.insert(r[0]).second&&r[3]>=0,"Invalid venue or capacity");positive(r[2]);cap+=r[3];lo=std::min(lo,r[1]);hi=std::max(hi,r[1]+2*r[2]*r[3]);}require(Q>=0&&Q<=cap,"Infeasible routing quantity");auto q=[&](const Vec&r,double v){return std::clamp((v-r[1])/(2*r[2]),0.,r[3]);};for(int k=0;k<100;++k){double m=lo/2+hi/2,s=0;for(auto&r:x)s+=q(r,m);if(s<Q)lo=m;else hi=m;}double nu=lo/2+hi/2,s=0,c=0,g=0;Report o{{"Venue","Quantity","Cost","MarginalCost","Multiplier","TotalCost","BudgetResidual","KKTResidual"},{}};for(auto&r:x){double a=q(r,nu),v=r[1]*a+r[2]*a*a,m=r[1]+2*r[2]*a,d=m-nu;s+=a;c+=v;if(r[3]>0)g=std::max(g,a==0?std::max(0.,-d):a==r[3]?std::max(0.,d):std::abs(d));o.rows.push_back({r[0],a,v,m,nu,0,0,0});}for(auto&r:o.rows){r[5]=c;r[6]=std::abs(s-Q);r[7]=g;}return o;
}

inline Report fifo_position_ledger(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,1);double initial=positive(p[0]),cash=initial,pos=0,basis=0,realized=0,fees=0;std::deque<std::pair<double,double>> lots;Report o{{"Event","Position","Cash","RealizedPnL","UnrealizedPnL","Fees","Equity","ConservationResidual"},{}};for(std::size_t t=0;t<x.size();++t){auto&r=x[t];double q=r[0],price=positive(r[1]),mark=positive(r[2]),fee=r[3],remain=q;require(fee>=0,"Nonnegative fee required");cash-=q*price+fee;fees+=fee;pos+=q;while(remain!=0&&!lots.empty()&&std::signbit(remain)!=std::signbit(lots.front().first)){auto&lot=lots.front();double old=lot.first,amount=std::min(std::abs(remain),std::abs(old)),closed=std::copysign(amount,old);realized+=closed*(price-lot.second);basis-=closed*lot.second;remain+=closed;lot.first-=closed;if(lot.first==0)lots.pop_front();}if(remain!=0){lots.push_back({remain,price});basis+=remain*price;}double u=pos*mark-basis,E=cash+pos*mark; o.rows.push_back({double(t+1),pos,cash,realized,u,fees,E,std::abs(E-(initial+realized+u-fees))});}return o;
}

inline Report futures_margin_ledger(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,5);double cash=positive(p[0]),prev=positive(p[1]),mult=positive(p[2]),im=p[3],mm=p[4],old=0;require(im>0&&im<=1&&mm>=0&&mm<=im,"Invalid margin ratios");Report o{{"Day","OpeningContracts","Settlement","VariationMargin","Fee","CashAfterSettlement","TargetContracts","InitialMarginRequired","MaintenanceShortfall","ExternalFunding","ClosingCash"},{}};for(std::size_t t=0;t<x.size();++t){auto&r=x[t];double S=positive(r[0]),q=r[1],fee=r[2];require(q==std::floor(q)&&fee>=0,"Invalid futures trade");double vm=mult*old*(S-prev),c=cash+vm-fee,maintenance=std::max(0.,mm*mult*std::abs(old)*S-c),margin=im*mult*std::abs(q)*S,flow=std::max(0.,margin-c);cash=c+flow;o.rows.push_back({double(t+1),old,S,vm,fee,c,q,margin,maintenance,flow,cash});prev=S;old=q;}return o;
}

inline Report fx_cash_ledger(const Mat& x,const Vec& p){
    validate(x,1,5,5);parameters(p,3);double B=p[0],F=p[1],prev=positive(p[2]),E=B+F*prev;Report o{{"Period","BaseCash","ForeignCash","FXRate","FXPnL","InterestPnL","Fee","Equity","ConservationResidual"},{}};for(std::size_t t=0;t<x.size();++t){auto&r=x[t];double S=positive(r[1]),rf=r[2],rb=r[3],fee=r[4];require(rf>-1&&rb>-1&&fee>=0,"Invalid cash return or fee");double fx=F*(S-prev),interest=B*rb+S*F*rf;B=B*(1+rb)-r[0]*S-fee;F=F*(1+rf)+r[0];double e=B+S*F;o.rows.push_back({double(t+1),B,F,S,fx,interest,fee,e,std::abs(e-E-fx-interest+fee)});E=e;prev=S;}return o;
}

inline Report integer_lot_allocation(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,1);auto B=count(p[0],0,100000),n=x.size();std::vector<std::size_t> cost,cap;std::set<double> ids;double work=0;for(auto&r:x){require(ids.insert(r[0]).second&&r[2]>=0,"Invalid allocation row");auto c=count(r[1],1,1000000),k=count(r[3],0,1000000);cost.push_back(c);cap.push_back(std::min(k,B/c));work+=double(B+1)*(cap.back()+1);}require(work<=5e7&&double(n)*(B+1)<=5e6,"Integer allocation work limit");Vec prev(B+1,0),next(B+1);std::vector<std::vector<std::size_t>> choice(n,std::vector<std::size_t>(B+1));for(std::size_t i=0;i<n;++i){for(std::size_t b=0;b<=B;++b){double best=std::numeric_limits<double>::infinity();for(std::size_t k=0;k<=std::min(cap[i],b/cost[i]);++k){double v=prev[b-k*cost[i]]+std::pow(double(k*cost[i])-x[i][2],2);if(v<best){best=v;choice[i][b]=k;}}next[b]=best;}prev.swap(next);}std::size_t b=B;Vec lots(n);for(std::size_t i=n;i-->0;){auto k=choice[i][b];lots[i]=k;b-=k*cost[i];}Report o{{"Asset","Lots","AllocatedValue","TargetValue","SquaredTrackingLoss","TotalCost","UnusedBudget","TotalLoss"},{}};for(std::size_t i=0;i<n;++i){double a=lots[i]*cost[i];o.rows.push_back({x[i][0],lots[i],a,x[i][2],std::pow(a-x[i][2],2),double(B-b),double(b),prev[B]});}return o;
}

inline Report batch_auction_clearing(const Mat& x,const Vec& p){
    validate(x,2,3,3);parameters(p,1);double ref=positive(p[0]);Vec price;bool buy=false,sell=false;for(auto&r:x){require(r[0]==1||r[0]==-1,"Side must be +1 or -1");positive(r[1]);positive(r[2]);buy|=r[0]==1;sell|=r[0]==-1;price.push_back(r[1]);}require(buy&&sell,"Both sides required");std::sort(price.begin(),price.end());price.erase(std::unique(price.begin(),price.end()),price.end());require(double(price.size())*x.size()<=5e7,"Auction work limit");Report o{{"Price","BuyDemand","SellSupply","MatchedVolume","Imbalance","Chosen"},{}};std::tuple<double,double,double,double> best{-1,0,0,0};std::size_t idx=0;for(double p:price){double b=0,s=0;for(auto&r:x){if(r[0]==1&&r[1]>=p)b+=r[2];if(r[0]==-1&&r[1]<=p)s+=r[2];}double v=std::min(b,s);auto key=std::make_tuple(v,-std::abs(b-s),-std::abs(p-ref),-p);if(key>best){best=key;idx=o.rows.size();}o.rows.push_back({p,b,s,v,b-s,0});}o.rows[idx][5]=1;return o;
}
} }
