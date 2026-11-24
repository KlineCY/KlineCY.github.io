#pragma once
#include "model_research_common.hpp"
namespace kline { namespace model_research {
inline Report var_ridge_forecast(const Mat& x,const Vec& p){
    auto d=validate(x,4,1,16);parameters(p,3);
    auto nt=count(p[0],3,x.size()-1),lag=count(p[1],1,nt-2);
    double lam=positive(p[2]);auto m=d*lag;
    require(m<=64&&double(nt)*m*m<=5e7,"VAR design work limit");
    auto design=[&](std::size_t t){Vec z;z.reserve(m);for(std::size_t k=1;k<=lag;++k)z.insert(z.end(),x[t-k].begin(),x[t-k].end());return z;};
    Mat H(m,Vec(m,0)),b(d,Vec(m,0)),coef(d);
    for(std::size_t t=lag;t<nt;++t){
        Vec z=design(t);add_outer(H,z,1);
        for(std::size_t j=0;j<d;++j)for(std::size_t k=0;k<m;++k)b[j][k]+=z[k]*x[t][j];
    }
    for(std::size_t k=0;k<m;++k)H[k][k]+=lam;
    for(std::size_t j=0;j<d;++j)coef[j]=solve_spd(H,b[j]);
    Vec err(d,0);
    for(std::size_t t=lag;t<nt;++t){Vec z=design(t);for(std::size_t j=0;j<d;++j){double e=x[t][j]-dot(coef[j],z);err[j]+=e*e/(nt-lag);}}
    Report o{{"Time","Asset","Actual","Forecast","Error","TrainingRMSE"},{}};
    for(std::size_t t=nt;t<x.size();++t){Vec z=design(t);for(std::size_t j=0;j<d;++j){double f=dot(coef[j],z);o.rows.push_back({double(t+1),double(j+1),x[t][j],f,x[t][j]-f,std::sqrt(err[j])});}}
    return o;
}

inline Report kalman_grid_fit(const Mat& x,const Vec& p){
    validate(x,4,1,1);parameters(p,8);
    auto nt=count(p[0],2,x.size()-1),g=count(p[5],2,100);
    double ql=positive(p[1]),qh=positive(p[2]),rl=positive(p[3]),rh=positive(p[4]);
    require(qh>=ql&&rh>=rl&&p[7]>=0&&double(g)*g*nt<=5e7,"Invalid grid or work limit");
    auto eval=[&](double Q,double R,std::size_t n,Report* out,double train_nll){
        double m=p[6],P=p[7],ll=0;
        for(std::size_t t=0;t<n;++t){
            double a=P+Q,S=positive(a+R),v=x[t][0]-m,k=a/S,old=m;
            ll+=.5*(std::log(2*std::acos(-1.)*S)+v*v/S);
            m+=k*v;P=a*(R/S);
            if(out)out->rows.push_back({double(t+1),double(t<nt),x[t][0],old,S,v,k,m,P,Q,R,train_nll});
        }
        return ll;
    };
    double best=std::numeric_limits<double>::infinity(),Q=ql,R=rl;
    for(std::size_t i=0;i<g;++i)for(std::size_t j=0;j<g;++j){
        double q=std::exp(std::log(ql)+(std::log(qh)-std::log(ql))*i/(g-1));
        double r=std::exp(std::log(rl)+(std::log(rh)-std::log(rl))*j/(g-1));
        double v=eval(q,r,nt,nullptr,0);
        if(v<best){best=v;Q=q;R=r;}
    }
    require(std::isfinite(best),"No finite grid candidate");
    Report o{{"Time","Train","Observed","Prediction","InnovationVariance","Innovation","Gain","FilteredMean","FilteredVariance","Q","R","TrainingNLL"},{}};
    eval(Q,R,x.size(),&o,best);return o;
}

inline Report hawkes_fixed_decay_fit(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,7);
    double T=positive(p[0]),decay=positive(p[1]),lo=positive(p[2]),hi=positive(p[3]),cap=p[4],tol=positive(p[5]);auto K=count(p[6],1,10000);
    require(hi>=lo&&cap>=0&&cap<1&&double(K)*x.size()*160<=1e8,"Invalid bounds or work limit");
    Vec h;double past=0,C=0,last=0;
    for(std::size_t i=0;i<x.size();++i){
        double t=x[i][0];require(t>last&&t<=T,"Strictly ordered events inside observation window required");
        if(i)past=std::exp(-decay*(t-last))*(past+decay);
        h.push_back(past);C+=-std::expm1(-decay*(T-t));last=t;
    }
    double mu=std::clamp(double(h.size())/T,lo,hi),a=0;
    auto grad=[&](double m,double b){Vec g{T,C};for(double z:h){double v=positive(m+b*z);g[0]-=1/v;g[1]-=z/v;}return g;};
    auto root=[&](double l,double u,auto f){if(f(l)>=0)return l;if(f(u)<=0)return u;for(int k=0;k<80;++k){double m=l/2+u/2;if(f(m)<0)l=m;else u=m;}return l/2+u/2;};
    auto violation=[](double v,double l,double u,double g){if(l==u)return 0.;if(v==l)return std::max(0.,-g);if(v==u)return std::max(0.,g);return std::abs(g);};
    double gap=0;std::size_t it=0;
    for(;it<K;){
        mu=root(lo,hi,[&](double m){return grad(m,a)[0];});
        a=root(0,cap,[&](double b){return grad(mu,b)[1];});
        Vec g=grad(mu,a);gap=std::max(violation(mu,lo,hi,g[0]),violation(a,0,cap,g[1]));++it;
        if(gap<=tol)break;
    }
    require(gap<=tol,"Hawkes coordinate fit did not converge");
    double comp=mu*T+a*C,nll=comp;for(double z:h)nll-=std::log(mu+a*z);
    return scalar({"Baseline","BranchingRatio","NegativeLogLikelihood","KKTResidual","Iterations","Compensator"},{mu,a,nll,gap,double(it),comp});
}

inline Report har_rv_forecast(const Mat& x,const Vec& p){
    validate(x,24,1,1);parameters(p,2);
    auto nt=count(p[0],23,x.size()-1);double lam=positive(p[1]);
    Vec pref(x.size()+1,0);for(std::size_t t=0;t<x.size();++t){require(x[t][0]>=0,"Negative realized variance");pref[t+1]=pref[t]+x[t][0];}
    auto z=[&](std::size_t t){return Vec{1,x[t-1][0],(pref[t]-pref[t-5])/5,(pref[t]-pref[t-22])/22};};
    Mat H(4,Vec(4,0));Vec b(4,0);
    for(std::size_t t=22;t<nt;++t){Vec a=z(t);add_outer(H,a,1);for(std::size_t j=0;j<4;++j)b[j]+=a[j]*x[t][0];}
    for(std::size_t j=1;j<4;++j)H[j][j]+=lam;
    Vec w=solve_spd(H,b);
    Report o{{"Time","Actual","Forecast","Error","Daily","Weekly","Monthly","Intercept","DailyBeta","WeeklyBeta","MonthlyBeta"},{}};
    for(std::size_t t=nt;t<x.size();++t){Vec a=z(t);double v=dot(a,w);o.rows.push_back({double(t+1),x[t][0],v,x[t][0]-v,a[1],a[2],a[3],w[0],w[1],w[2],w[3]});}
    return o;
}

inline Report ou_conditional_fit(const Mat& x,const Vec& p){
    validate(x,5,1,1);parameters(p,2);auto nt=count(p[0],4,x.size()-1);double dt=positive(p[1]),n=nt-1,a=0,b=0;
    for(std::size_t t=1;t<nt;++t){a+=x[t-1][0]/n;b+=x[t][0]/n;}
    double xx=0,xy=0;
    for(std::size_t t=1;t<nt;++t){double z=x[t-1][0]-a;xx+=z*z;xy+=z*(x[t][0]-b);}
    double phi=xy/positive(xx),c=b-phi*a;
    require(phi>0&&phi<1,"Conditional MLE outside stationary OU domain");
    double v=0;for(std::size_t t=1;t<nt;++t){double e=x[t][0]-c-phi*x[t-1][0];v+=e*e/n;}
    positive(v);double k=-std::log(phi)/dt,theta=c/(1-phi),sig=std::sqrt(2*k*v/((1-phi)*(1+phi)));
    Report o{{"Time","Actual","Forecast","Error","Phi","LongRunMean","MeanReversion","DiffusionVol","InnovationVariance"},{}};
    for(std::size_t t=nt;t<x.size();++t){double f=c+phi*x[t-1][0];o.rows.push_back({double(t+1),x[t][0],f,x[t][0]-f,phi,theta,k,sig,v});}
    return o;
}

inline Report dcc_twoasset_grid(const Mat& x,const Vec& p){
    validate(x,5,2,2);parameters(p,4);auto nt=count(p[0],3,x.size()-1),g=count(p[3],2,100);
    double am=p[1],bm=p[2];require(am>=0&&am<1&&bm>=0&&bm<1&&double(g)*g*nt<=5e7,"Invalid DCC grid");
    double u=0,v=0,c=0;
    for(std::size_t t=0;t<nt;++t){u+=x[t][0]*x[t][0]/nt;v+=x[t][1]*x[t][1]/nt;c+=x[t][0]*x[t][1]/nt;}
    positive(u);positive(v);positive(u*v-c*c);
    auto eval=[&](double a,double b,std::size_t n,Report* out,double training){
        double q1=u,q2=v,q12=c,ll=0;
        for(std::size_t t=0;t<n;++t){
            double rho=q12/std::sqrt(positive(q1*q2)),det=positive(1-rho*rho),z=x[t][0],w=x[t][1];
            double loss=.5*(std::log(det)+(z*z-2*rho*z*w+w*w)/det);ll+=loss;
            if(out)out->rows.push_back({double(t+1),double(t<nt),z,w,rho,loss,a,b,training});
            q1=(1-a-b)*u+a*z*z+b*q1;q2=(1-a-b)*v+a*w*w+b*q2;q12=(1-a-b)*c+a*z*w+b*q12;
        }
        return ll;
    };
    double best=std::numeric_limits<double>::infinity(),a=0,b=0;
    for(std::size_t i=0;i<g;++i)for(std::size_t j=0;j<g;++j){
        double z=am*i/(g-1),w=bm*j/(g-1);if(z+w>=1)continue;
        double l=eval(z,w,nt,nullptr,0);if(l<best){best=l;a=z;b=w;}
    }
    require(std::isfinite(best),"No finite DCC candidate");
    Report o{{"Time","Train","Z1","Z2","PredictedCorrelation","ConditionalNLL","A","B","TrainingNLL"},{}};
    eval(a,b,x.size(),&o,best);return o;
}
} }
