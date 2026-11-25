#pragma once
#include "applied_forecast.hpp"
#include "applied_inference.hpp"
#include "applied_liquidity.hpp"
#include "applied_rates.hpp"
#include "applied_options.hpp"
#include "applied_credit.hpp"
#include "applied_accounts.hpp"
#include "applied_allocation.hpp"
namespace applied_synthesis {
Vec forecast_serial(const Vec& x){pairs(x);Vec b=applied_forecast::brier_groups(x),r{1},s;for(size_t i=0;i<x.size();i+=2){double e=x[i+1]-x[i];check(e!=0);r.push_back(e);s.push_back(e>0?1:-1);}Vec q=applied_inference::ljung_box(r),v=applied_inference::runs_exact(s);return {b[0],b[1],q[0],v[0],v[4]};}

Vec conformal_external(const Vec& x){check(x.size()>=5);size_t n=integer(x[1],1,10000);check(x.size()>=2+n+2&&(x.size()-2-n)%2==0);Vec c{x[0]};c.insert(c.end(),x.begin()+2,x.begin()+2+n);double q=applied_forecast::split_conformal(c)[1];Vec y{x[0]};for(size_t i=2+n;i<x.size();i+=2)y.insert(y.end(),{x[i]-q,x[i]+q,x[i+1]});Vec a=applied_forecast::interval_score(y);return {q,a[1],a[2],a[0]};}

Vec book_markout(const Vec& x){size(x,12);Vec a(x.begin(),x.begin()+4),b(x.begin()+4,x.begin()+8),e(x.begin(),x.begin()+8);Vec m=applied_liquidity::microprice(a),n=applied_liquidity::microprice(b),o=applied_liquidity::ofi_event(e);Vec p=applied_liquidity::passive_markout({x[8],x[9],x[10],m[0],n[0],x[11]});return {n[3]-m[3],o[2],p[0],p[1],p[4]};}

Vec auction_arrival(const Vec& x){check(x.size()>=12&&x[0]>0&&x[1]>0&&x[2]>0&&x[3]>=0&&x[3]<=x[2]&&x[4]>=0);Vec v(x.begin()+5,x.end()),a=applied_liquidity::auction_cross(v);check(x[3]<=a[1]);double e=x[3]*(a[0]-x[0]),o=(x[2]-x[3])*(x[1]-x[0]);return {a[0],a[1],x[3],e,o,e+o+x[3]*x[4]};}

Vec rate_credit_shock(const Vec& x){size(x,7);Vec a(x.begin(),x.begin()+5);double p=applied_credit::risky_zero(a)[0];a[2]+=x[5];double q=applied_credit::risky_zero(a)[0];a[3]+=x[6];double r=applied_credit::risky_zero(a)[0];return {p,q,r,q-p,r-q,r-p};}

Vec immunization_shock(const Vec& x){size(x,6);Vec a(x.begin(),x.begin()+5),w=applied_rates::two_zero_immunize(a);double y=x[4]+x[5],p=w[0]*std::exp(-y*x[0])+w[1]*std::exp(-y*x[1]),l=x[3]*std::exp(-y*x[2]);return {p,l,p-l,.5*w[4]*sq(x[5])};}

Vec replication_path(const Vec& x){size(x,8);check((x[6]==1||x[6]==-1)&&(x[7]==1||x[7]==-1));Vec a(x.begin(),x.begin()+6),v=applied_options::two_step_replication(a);double s1=x[0]*(x[6]==1?x[2]:x[3]),s2=s1*(x[7]==1?x[2]:x[3]);auto pay=[&](double s){return std::max(0.,x[5]*(s-x[1]));};Vec n=applied_options::one_step_claim({s1,s1*x[2],s1*x[3],x[4],pay(s1*x[2]),pay(s1*x[3])});Vec h=applied_options::discrete_hedge({v[0],x[4],pay(s2),x[0],v[1],s1,n[1],s2});return {v[0],n[0],pay(s2),v[1],n[1],h[2]};}

Vec surface_probabilities(const Vec& x){size(x,7);Vec a=applied_options::digital_vertical({x[0],x[1],x[3],x[4],x[6]}),b=applied_options::digital_vertical({x[1],x[2],x[4],x[5],x[6]}),c=applied_options::uneven_butterfly(x);return {a[1],b[1],a[1]-b[1],c[4],c[3]};}

Vec netting_cva(const Vec& x){check(x.size()>=7);Vec a(x.begin()+3,x.end()),e=applied_credit::net_collateral(a);Vec out;for(size_t i=0;i<3;++i)out.push_back(applied_credit::cva_independent({x[0],x[1],e[i],x[2]}).back());return {out[0],out[1],out[2],out[0]-out[1],out[1]-out[2]};}

Vec dependent_cva(const Vec& x){check(x.size()>=5&&x[1]>0);Vec a{x[0]};a.insert(a.end(),x.begin()+2,x.end());Vec w=applied_credit::wrong_way(a);double ind=applied_credit::cva_independent({x[0],x[1],w[0],w[1]}).back(),joint=x[1]*w[2];return {ind,joint,joint-ind,w[4]};}

Vec investor_manager_return(const Vec& x){size(x,4);check(x[2]>=0);Vec t=applied_accounts::twr_exact({x[0],x[1],x[2],x[3],0}),m{2,x[3],0,x[0]};if(x[2]>0)m.insert(m.end(),{1,x[2]});Vec r=applied_accounts::mwr_continuous(m),d=applied_accounts::modified_dietz({x[0],x[3],x[2],.5});return {t[0],r[1],d[0],t[3]};}

Vec futures_borrow_funding(const Vec& x){size(x,13);Vec a(x.begin(),x.begin()+7),b(x.begin()+7,x.end());Vec f=applied_accounts::futures_margin(a),c=applied_accounts::borrow_accrual(b);return {f[1],c.back(),f[1]-c.back(),f[0]-c.back(),f[3]};}

Vec budget_tail(const Vec& x){check(x.size()>=7&&(x.size()-4)%3==0);Vec w=applied_allocation::two_asset_risk_budget({x[0],x[1],x[2]}),a{w[0],w[1],x[3]};a.insert(a.end(),x.begin()+4,x.end());Vec e=applied_allocation::tail_euler(a);return {w[0],w[1],w[2],e[0],e[1],e[2]};}

Vec kl_robust(const Vec& x){size(x,9);Vec a(x.begin(),x.begin()+5),q=applied_allocation::kl_reverse_stress(a),v=applied_allocation::robust_spread({x[0],x[5],x[6],x[7],x[8]});return {q[0],q[1],q[3],v[0],v[1],v[2],v[3]};}

Vec cost_tail(const Vec& x){check(x.size()>=16&&(x.size()-13)%3==0);Vec a(x.begin(),x.begin()+12),w=applied_allocation::l1_box_trade(a),b{w[0],w[1],x[12]};b.insert(b.end(),x.begin()+13,x.end());Vec e=applied_allocation::tail_euler(b);return {w[0],w[1],w[2],e[0],e[1],e[2]};}

Vec execution_wealth(const Vec& x){check(x.size()>=8&&(x.size()-6)%2==0&&x[0]>0);Vec a{x[1],x[2]};a.insert(a.end(),x.begin()+6,x.end());Vec b(x.begin()+1,x.end()),s=applied_liquidity::sweep_asks(a),v=applied_liquidity::sweep_shortfall(b);check(s[1]+v[4]<=x[0]);double pnl=s[0]*x[4]-s[1]-v[4],ideal=x[1]*(x[4]-x[3]);return {pnl,ideal,v[5],x[0]+pnl,ideal-pnl-v[5]};}
}
