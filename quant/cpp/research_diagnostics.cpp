#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>
using Vec=std::vector<double>;
void check(bool ok){if(!ok)throw std::invalid_argument("invalid numerical input");}
void finite(const Vec& v){for(double x:v)check(std::isfinite(x));}
void same(const Vec& a,const Vec& b){check(!a.empty()&&a.size()==b.size());finite(a);finite(b);}
double sum(const Vec& v){return std::accumulate(v.begin(),v.end(),0.0);}
double mean(const Vec& v){return sum(v)/v.size();}
double dot(const Vec& a,const Vec& b){same(a,b);return std::inner_product(a.begin(),a.end(),b.begin(),0.0);}

Vec paired_loss(const Vec& a,const Vec& b){ same(a,b); double d=0,w=0; for(size_t i=0;i<a.size();++i){double z=a[i]*a[i]-b[i]*b[i];d+=z;w+=z>0;}return {d/a.size(),w/a.size()};}

Vec hac_loss(const Vec& x){check(x.size()>=3); finite(x);double m=mean(x),g0=0,g1=0;for(size_t i=0;i<x.size();++i){double z=x[i]-m;g0+=z*z;if(i)g1+=z*(x[i-1]-m);}double v=(g0+g1)/x.size();check(v>0);double se=std::sqrt(v/x.size());return {m,v,se,m/se};}

Vec exact_sign(const Vec& d){check(!d.empty()&&d.size()<=20);finite(d);double obs=std::abs(std::accumulate(d.begin(),d.end(),0.0)),scale=0;for(double z:d)scale+=std::abs(z);size_t count=0,total=size_t(1)<<d.size();for(size_t mask=0;mask<total;++mask){double s=0;for(size_t i=0;i<d.size();++i)s+=((mask>>i)&1?1:-1)*d[i];count+=std::abs(s)+1e-12*scale>=obs;}return {obs,double(count)/total};}

Vec holm_family(const Vec& p){check(!p.empty());finite(p);for(double z:p)check(z>=0&&z<=1);std::vector<size_t> ix(p.size());std::iota(ix.begin(),ix.end(),0);std::stable_sort(ix.begin(),ix.end(),[&](size_t a,size_t b){return p[a]<p[b];});Vec y(p.size());double v=0;for(size_t j=0;j<p.size();++j){v=std::max(v,(p.size()-j)*p[ix[j]]);y[ix[j]]=std::min(1.0,v);}return y;}

Vec cost_break_even(const Vec& w,const Vec& r,const Vec& d,double c,double eta){same(w,r);same(w,d);check(std::isfinite(c)&&std::isfinite(eta)&&c>=0&&eta>=0);double g=dot(w,r),t=0,i=eta*dot(d,d);for(double z:d)t+=std::abs(z);check(t>0);return {g,t,i,g-c*t-i,(g-i)/t};}

Vec drift_turnover(const Vec& w,const Vec& r,const Vec& target,double c){same(w,r);same(w,target);check(std::isfinite(c)&&c>=0);check(std::abs(sum(w)-1)<1e-10&&std::abs(sum(target)-1)<1e-10);double g=1+dot(w,r),t=0;check(g>0);for(size_t i=0;i<w.size();++i){check(w[i]>=0&&target[i]>=0&&r[i]>-1);t+=std::abs(target[i]-w[i]*(1+r[i])/g);}return {g-1,w[0]*(1+r[0])/g,t,c*t};}

Vec shortfall(double side,double q,double p0,double pt,const Vec& qty,const Vec& prices,double fees){same(qty,prices);check((side==1||side==-1)&&q>0&&p0>0&&pt>0&&fees>=0&&std::isfinite(q+p0+pt+fees));double f=sum(qty),ex=0;check(f>0&&f<=q);for(size_t i=0;i<qty.size();++i){check(qty[i]>0&&prices[i]>0);ex+=side*qty[i]*(prices[i]-p0);}double op=side*(q-f)*(pt-p0);return {ex,op,ex+op+fees,dot(qty,prices)/f};}

Vec margin_shock(const Vec& v,const Vec& r,double equity,double m){same(v,r);check(equity>0&&m>=0&&m<=1&&std::isfinite(equity));double e=equity+dot(v,r),g=0;check(e>0);for(size_t i=0;i<v.size();++i){check(r[i]>-1);g+=std::abs(v[i]*(1+r[i]));}return {e,g,g/e,std::max(0.0,m*g-e)};}

Vec factor_pnl(const Vec& w,const Vec& b,const Vec& e,double f){same(w,b);same(w,e);check(std::isfinite(f));double beta=dot(w,b),fr=beta*f,s=dot(w,e);return {beta,fr,s,fr+s};}

Vec vol_cap(double a,double b,double v1,double cov,double v2,double cap){check(std::isfinite(a+b+v1+cov+v2+cap)&&v1>=0&&v2>=0&&cov*cov<=v1*v2&&cap>0);double v=a*a*v1+2*a*b*cov+b*b*v2;check(v>=0);double s=v==0?1:std::min(1.0,cap/std::sqrt(v));return {v,s,s*s*v,s*(std::abs(a)+std::abs(b))};}

Vec gross_concentration(const Vec& w){check(!w.empty());finite(w);double g=0;for(double z:w)g+=std::abs(z);check(g>0);double h=dot(w,w)/(g*g);return {sum(w),g,h,1/h};}

Vec weighted_tail(const Vec& loss,const Vec& p,double alpha){same(loss,p);check(alpha>0&&alpha<1&&std::abs(sum(p)-1)<1e-10);std::vector<size_t> ix(p.size());std::iota(ix.begin(),ix.end(),0);for(double z:p)check(z>=0);std::sort(ix.begin(),ix.end(),[&](size_t a,size_t b){return loss[a]>loss[b];});double mass=1-alpha,remaining=mass,v=0,bound=0;for(auto i:ix){double take=std::min(remaining,p[i]);if(take>0){v+=take*loss[i];bound=loss[i];remaining-=take;}if(remaining<=1e-14*mass)break;}check(remaining<=1e-10*mass);return {bound,v/mass,mass-remaining};}

Vec regime_moments(const Vec& p,const Vec& m,const Vec& v){same(p,m);same(p,v);check(std::abs(sum(p)-1)<1e-10);double mu=dot(p,m),within=0,between=0;for(size_t i=0;i<p.size();++i){check(p[i]>=0&&v[i]>=0);within+=p[i]*v[i];between+=p[i]*(m[i]-mu)*(m[i]-mu);}return {mu,within,between,within+between};}

Vec rank_perturb(const Vec& x,const Vec& y,size_t k){same(x,y);check(k>0&&k<x.size());auto order=[](const Vec& s){std::vector<size_t> ix(s.size());std::iota(ix.begin(),ix.end(),0);std::stable_sort(ix.begin(),ix.end(),[&](size_t a,size_t b){return s[a]>s[b];});return ix;};auto a=order(x),b=order(y);size_t n=0;for(size_t i=0;i<k;++i)for(size_t j=0;j<k;++j)n+=a[i]==b[j];double d=0;for(size_t i=0;i<x.size();++i)d=std::max(d,std::abs(x[i]-y[i]));double g=x[a[k-1]]-x[a[k]];return {double(n),double(n)/(2*k-n),g,d,g>2*d?1.0:0.0};}

Vec compound_fee(const Vec& r,const Vec& t,double c){same(r,t);check(c>=0&&std::isfinite(c));double g=1,n=1;for(size_t i=0;i<r.size();++i){check(t[i]>=0&&1+r[i]>0&&1+r[i]-c*t[i]>0);g*=1+r[i];n*=1+r[i]-c*t[i];}return {g,n,g-n};}

Vec selection_regret(const Vec& train,const Vec& hold){same(train,hold);size_t j=std::min_element(train.begin(),train.end())-train.begin();double best=*std::min_element(hold.begin(),hold.end());return {double(j+1),hold[j],best,hold[j]-best};}

void emit(const std::string& id,int scenario,const std::vector<std::string>& fields,const Vec& values){check(fields.size()==values.size());finite(values);for(size_t i=0;i<fields.size();++i)std::cout<<id<<','<<scenario<<','<<fields[i]<<','<<values[i]<<'\n';}
int main(){try{std::cout<<std::setprecision(17)<<"id,scenario,field,value\n";
    emit("paired_loss",1,{"mean_delta","win_rate"},paired_loss(Vec{2,1,-1},Vec{1,1,0}));
    emit("paired_loss",2,{"mean_delta","win_rate"},paired_loss(Vec{1,1,1},Vec{0,0,3}));
    emit("hac_loss",1,{"mean","long_run_variance","standard_error","z"},hac_loss(Vec{1,2,3,4}));
    emit("hac_loss",2,{"mean","long_run_variance","standard_error","z"},hac_loss(Vec{-1,1,-1,1}));
    emit("exact_sign",1,{"statistic","p_value"},exact_sign(Vec{1,2,3}));
    emit("exact_sign",2,{"statistic","p_value"},exact_sign(Vec{1,-1}));
    emit("holm_family",1,{"p1","p2","p3","p4"},holm_family(Vec{.01,.04,.03,.2}));
    emit("holm_family",2,{"p1","p2","p3","p4"},holm_family(Vec{.4,.4,.01,.9}));
    emit("cost_break_even",1,{"gross","turnover","impact","net","break_even_fee"},cost_break_even(Vec{.6,-.4},Vec{.03,.01},Vec{.2,-.1},.001,.02));
    emit("cost_break_even",2,{"gross","turnover","impact","net","break_even_fee"},cost_break_even(Vec{.6,-.4},Vec{0,.01},Vec{.2,-.1},.001,.02));
    emit("drift_turnover",1,{"gross_return","first_drift_weight","turnover","cost"},drift_turnover(Vec{.6,.4},Vec{.1,-.05},Vec{.5,.5},.002));
    emit("drift_turnover",2,{"gross_return","first_drift_weight","turnover","cost"},drift_turnover(Vec{.6,.4},Vec{.1,.1},Vec{.6,.4},.002));
    emit("shortfall",1,{"execution","opportunity","total","average_fill"},shortfall(1,60,100,103,Vec{30,20},Vec{101,102},4));
    emit("shortfall",2,{"execution","opportunity","total","average_fill"},shortfall(-1,60,100,97,Vec{30,20},Vec{99,98},4));
    emit("margin_shock",1,{"equity","gross","leverage","shortfall"},margin_shock(Vec{100,-60},Vec{-.15,.1},80,.4));
    emit("margin_shock",2,{"equity","gross","leverage","shortfall"},margin_shock(Vec{100,-60},Vec{.1,-.1},80,.4));
    emit("factor_pnl",1,{"beta","factor_return","specific_return","total"},factor_pnl(Vec{.5,-.5},Vec{1.2,.8},Vec{.01,-.02},.03));
    emit("factor_pnl",2,{"beta","factor_return","specific_return","total"},factor_pnl(Vec{.5,-.5},Vec{1,1},Vec{.01,-.02},.03));
    emit("vol_cap",1,{"variance","scale","scaled_variance","scaled_gross"},vol_cap(.6,-.4,.04,.01,.09,.1));
    emit("vol_cap",2,{"variance","scale","scaled_variance","scaled_gross"},vol_cap(.6,-.4,.04,.01,.09,.2));
    emit("gross_concentration",1,{"net","gross","hhi","effective_count"},gross_concentration(Vec{.5,-.3,.2}));
    emit("gross_concentration",2,{"net","gross","hhi","effective_count"},gross_concentration(Vec{.5,-.5}));
    emit("weighted_tail",1,{"tail_boundary","expected_shortfall","tail_mass"},weighted_tail(Vec{0,.1,.3},Vec{.6,.3,.1},.8));
    emit("weighted_tail",2,{"tail_boundary","expected_shortfall","tail_mass"},weighted_tail(Vec{0,.1,.3},Vec{.6,.3,.1},.95));
    emit("regime_moments",1,{"mean","within","between","variance"},regime_moments(Vec{.75,.25},Vec{.01,-.03},Vec{.0001,.0009}));
    emit("regime_moments",2,{"mean","within","between","variance"},regime_moments(Vec{.75,.25},Vec{.01,.01},Vec{.0001,.0009}));
    emit("rank_perturb",1,{"overlap","jaccard","gap","perturbation","certificate"},rank_perturb(Vec{.3,.2,.1},Vec{.29,.09,.12},2));
    emit("rank_perturb",2,{"overlap","jaccard","gap","perturbation","certificate"},rank_perturb(Vec{.3,.2,.1},Vec{.29,.19,.11},2));
    emit("compound_fee",1,{"gross_wealth","net_wealth","wealth_gap"},compound_fee(Vec{.1,-.1},Vec{.5,.5},.01));
    emit("compound_fee",2,{"gross_wealth","net_wealth","wealth_gap"},compound_fee(Vec{.1,-.1},Vec{1,0},.01));
    emit("selection_regret",1,{"selected_index","holdout_loss","oracle_loss","regret"},selection_regret(Vec{.1,.2,.3},Vec{.4,.2,.1}));
    emit("selection_regret",2,{"selected_index","holdout_loss","oracle_loss","regret"},selection_regret(Vec{.3,.1,.2},Vec{.4,.1,.2}));
return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
