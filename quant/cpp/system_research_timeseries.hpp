#pragma once
#include "system_research_common.hpp"
namespace kline { namespace system_research {
inline Report haar_energy(const Mat& x,const Vec& p){
    validate(x,2,1,1);parameters(p,1);auto J=count(p[0],1,20),n=x.size();require(n%(std::size_t(1)<<J)==0,"Length must be divisible by 2^J");Vec a=column(x,0),original=a;Mat detail;double root=std::sqrt(2.),total=squared(a);Report o{{"Level","Index","Type","Coefficient","Energy","TotalEnergy","ReconstructionError"},{}};for(std::size_t j=1;j<=J;++j){Vec b(a.size()/2),d=b;for(std::size_t k=0;k<b.size();++k){b[k]=(a[2*k]+a[2*k+1])/root;d[k]=(a[2*k]-a[2*k+1])/root;o.rows.push_back({double(j),double(k+1),1,d[k],d[k]*d[k],total,0});}detail.push_back(d);a=b;}for(std::size_t k=0;k<a.size();++k)o.rows.push_back({double(J),double(k+1),0,a[k],a[k]*a[k],total,0});for(std::size_t j=J;j-->0;){Vec b(2*a.size());for(std::size_t k=0;k<a.size();++k){b[2*k]=(a[k]+detail[j][k])/root;b[2*k+1]=(a[k]-detail[j][k])/root;}a=b;}double error=0;for(std::size_t k=0;k<n;++k)error=std::max(error,std::abs(a[k]-original[k]));for(auto&r:o.rows)r[6]=error;return o;
}

inline Report fractional_filter(const Mat& x,const Vec& p){
    validate(x,2,1,1);parameters(p,2);double d=p[0];auto K=count(p[1],0,x.size()-1);require(d>=0&&d<=1&&double(x.size())*(K+1)<=5e7,"Invalid fractional filter controls");Vec w(K+1,1);for(std::size_t k=1;k<=K;++k)w[k]=w[k-1]*(double(k)-1-d)/k;Report o{{"Time","Filtered","WeightSum","WeightL1"},{}};for(std::size_t t=K;t<x.size();++t){double v=0;for(std::size_t k=0;k<=K;++k)v+=w[k]*x[t-k][0];o.rows.push_back({double(t+1),v,sum(w),l1(w)});}return o;
}

inline Report ou_irregular_profile(const Mat& x,const Vec& p){
    validate(x,4,2,2);parameters(p,3);double lo=positive(p[0]),hi=positive(p[1]);auto G=count(p[2],2,10000),n=x.size()-1;require(hi>=lo&&double(G)*n<=5e7,"Invalid OU grid");Vec dt;for(std::size_t i=0;i<n;++i)dt.push_back(positive(x[i+1][0]-x[i][0]));double best=std::numeric_limits<double>::infinity(),kap=0,mu=0,var=0,score=0;for(std::size_t j=0;j<G;++j){double k=std::exp(std::log(lo)+(std::log(hi)-std::log(lo))*j/(G-1)),num=0,den=0,ld=0;Vec a(n),b(n),z(n);for(std::size_t i=0;i<n;++i){a[i]=-std::expm1(-k*dt[i]);b[i]=positive(-std::expm1(-2*k*dt[i])/(2*k));z[i]=x[i+1][1]-(1-a[i])*x[i][1];num+=a[i]*z[i]/b[i];den+=a[i]*a[i]/b[i];ld+=std::log(b[i]);}double m=num/positive(den),ss=0,g=0;for(std::size_t i=0;i<n;++i){double e=z[i]-a[i]*m;ss+=e*e/b[i];g+=a[i]*e/b[i];}double v=positive(ss/n),f=.5*(n*(1+std::log(2*std::acos(-1.)*v))+ld);if(f<best){best=f;kap=k;mu=m;var=v;score=g;}}return scalar({"Kappa","Mean","DiffusionVariance","NLL","MeanScore"},{kap,mu,var,best,score});
}

inline Report fourier_seasonal_fit(const Mat& x,const Vec& p){
    validate(x,3,3,3);parameters(p,3);double period=positive(p[0]),lam=p[2];auto K=count(p[1],1,12);require(lam>=0,"Nonnegative ridge required");Mat X,Z;Vec y;for(auto&r:x){require(r[2]==0||r[2]==1,"Binary train flag");Vec a{1};for(std::size_t k=1;k<=K;++k){double z=2*std::acos(-1.)*k*std::remainder(r[0],period)/period;a.push_back(std::cos(z));a.push_back(std::sin(z));}Z.push_back(a);if(r[2]){X.push_back(a);y.push_back(r[1]);}}require(!X.empty()&&double(X.size())*Z[0].size()*Z[0].size()<=5e7,"Invalid seasonal design");Vec w=fit_ridge(X,y,lam,true);Report o{{"Time","Train","Actual","Prediction","Residual","Coefficient0"},{}};for(std::size_t k=1;k<=K;++k){o.header.push_back("Cos"+std::to_string(k));o.header.push_back("Sin"+std::to_string(k));}for(std::size_t i=0;i<x.size();++i){double f=dot(Z[i],w);Vec r{x[i][0],x[i][2],x[i][1],f,x[i][1]-f};r.insert(r.end(),w.begin(),w.end());o.rows.push_back(r);}return o;
}

inline Report garch_horizon_risk(const Mat& x,const Vec& p){
    validate(x,1,5,5);parameters(p,1);auto H=count(p[0],1,10000);require(double(H)*x.size()<=1e6,"Risk horizon output limit");Report o{{"Scenario","Horizon","ForecastVariance","CumulativeVariance","MeanVariancePerPeriod"},{}};for(std::size_t i=0;i<x.size();++i){auto&r=x[i];double h=positive(r[0]),w=positive(r[1]),a=r[2],b=r[3],e=r[4],s=0;require(a>=0&&b>=0&&a+b<1,"Stationary GARCH parameters required");h=w+a*e*e+b*h;for(std::size_t k=1;k<=H;++k){s+=h;o.rows.push_back({double(i+1),double(k),h,s,s/k});h=w+(a+b)*h;}}return o;
}

inline Report rolling_forecast_comparison(const Mat& x,const Vec& p){
    validate(x,2,3,3);parameters(p,1);auto n=x.size(),L=count(p[0],0,n-1);require(double(n)*(L+1)<=5e7,"HAC work limit");Vec d;double a=0,b=0;for(auto&r:x){double u=std::pow(r[0]-r[1],2),v=std::pow(r[0]-r[2],2);a+=u/n;b+=v/n;d.push_back(u-v);}double m=sum(d)/n;for(double&v:d)v-=m;double s=squared(d)/n;for(std::size_t h=1;h<=L;++h){double g=0;for(std::size_t t=h;t<n;++t)g+=d[t]*d[t-h]/n;s+=2*(1-double(h)/(L+1))*g;}require(s>=-1e-12*std::max(1.,squared(d)/n),"Negative HAC variance");s=std::max(0.,s);return scalar({"Observations","MSEA","MSEB","MeanLossDiff","HACVariance","DMStatistic","Degenerate"},{double(n),a,b,m,s,s>0?std::sqrt(double(n))*m/std::sqrt(s):0,double(s==0)});
}
} }
