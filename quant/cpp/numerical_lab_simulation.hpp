#pragma once
#include "numerical_lab_common.hpp"

namespace kline { namespace numerical_lab {
inline Report antithetic_gbm(const Mat& x,const Vec& p){
    validate(x,2,1,1);parameters(p,6);
    double S=positive(p[0]),K=positive(p[1]),r=p[2],q=p[3],v=positive(p[4]),T=positive(p[5]),disc=std::exp(-r*T);
    double m=(r-q-.5*v*v)*T,s=v*std::sqrt(T);Vec paired,raw;
    for(const auto& row:x){
        double a=disc*std::max(S*std::exp(m+s*row[0])-K,0.0),b=disc*std::max(S*std::exp(m-s*row[0])-K,0.0);
        raw.push_back(a);paired.push_back((a+b)/2);
    }
    double n=double(x.size());
    return scalar_rows({"Pairs","PlainPrice","PlainSE","AntitheticPrice","PairSE","AnalyticPrice"},{n,mean(raw),std::sqrt(variance(raw)/n),mean(paired),std::sqrt(variance(paired)/n),bs(S,K,r,q,v,T,true)});
}
inline Report geometric_control(const Mat& x,const Vec& p){
    auto d=validate(x,4,2,64);parameters(p,7);
    double S=positive(p[0]),K=positive(p[1]),r=p[2],q=p[3],v=positive(p[4]),T=positive(p[5]);
    auto pilot=count(p[6],2,x.size()-2);
    double dt=T/d,disc=std::exp(-r*T),mu=std::log(S)+(r-q-.5*v*v)*dt*(d+1)/2;
    double var=v*v*dt*(d+1)*(2*d+1)/(6*d),sd=std::sqrt(var);
    double a=(mu-std::log(K)+var)/sd,b=a-sd;
    double known=disc*(std::exp(mu+.5*var)*normal(a)-K*normal(b));
    Vec Y,C;
    for(const auto& row:x){
        double logS=std::log(S),arithmetic=0,geometric=0;
        for(double z:row){logS+=(r-q-.5*v*v)*dt+v*std::sqrt(dt)*z;arithmetic+=std::exp(logS)/d;geometric+=logS/d;}
        Y.push_back(disc*std::max(0.0,arithmetic-K));C.push_back(disc*std::max(0.0,std::exp(geometric)-K));
    }
    Vec y(Y.begin(),Y.begin()+pilot),c(C.begin(),C.begin()+pilot);
    double beta=covariance(y,c)/positive(variance(c));
    Vec adj,raw;
    for(std::size_t i=pilot;i<x.size();++i){raw.push_back(Y[i]);adj.push_back(Y[i]-beta*(C[i]-known));}
    double n=double(raw.size());
    return scalar_rows({"PilotPaths","EvaluationPaths","ControlMean","ControlCoefficient","RawPrice","RawSE","AdjustedPrice","AdjustedSE"},{double(pilot),n,known,beta,mean(raw),std::sqrt(variance(raw)/n),mean(adj),std::sqrt(variance(adj)/n)});
}
inline Report gbm_path_audit(const Mat& x,const Vec& p){
    auto d=validate(x,2,2,64);parameters(p,6);
    double S=positive(p[0]),K=positive(p[1]),r=p[2],q=p[3],v=positive(p[4]),T=positive(p[5]);
    double dt=T/d;
    Report out{{"Path","TerminalSpot","PathMaximum","DiscountedCall","IntegratedVariance"},{}};
    for(std::size_t i=0;i<x.size();++i){
        double logS=std::log(S),peak=S;
        for(double z:x[i]){logS+=(r-q-.5*v*v)*dt+v*std::sqrt(dt)*z;peak=std::max(peak,std::exp(logS));}
        double terminal=std::exp(logS);
        out.rows.push_back({double(i),terminal,peak,std::exp(-r*T)*std::max(terminal-K,0.0),v*v*T});
    }
    return out;
}
inline Report bridge_survival(const Mat& x,const Vec& p){
    auto d=validate(x,2,2,64);parameters(p,5);
    double initial=positive(p[0]),barrier=positive(p[1]),sigma=positive(p[2]),T=positive(p[3]),K=positive(p[4]),dt=T/d;
    require(initial<barrier,"Initial spot reaches upper barrier");
    Report out{{"Path","TerminalSpot","DiscreteSurvival","ConditionalSurvival","DiscretePayoff","BridgePayoff"},{}};
    for(std::size_t i=0;i<x.size();++i){
        double prev=initial,weight=1,discrete=1;
        for(double curr:x[i]){
            positive(curr);
            if(curr>=barrier||prev>=barrier){weight=0;discrete=0;}
            else if(weight>0)weight*=-std::expm1(-2*std::log(barrier/prev)*std::log(barrier/curr)/(sigma*sigma*dt));
            prev=curr;
        }
        double payoff=std::max(prev-K,0.0);
        out.rows.push_back({double(i),prev,discrete,weight,discrete*payoff,weight*payoff});
    }
    return out;
}
inline Report heston_paths(const Mat& x,const Vec& p){
    auto d=validate(x,2,2,64);parameters(p,9);
    require(d%2==0,"Paired independent normal innovations required");
    double S=positive(p[0]),v0=p[1],r=p[2],q=p[3],k=positive(p[4]),theta=p[5],xi=p[6],rho=p[7],T=positive(p[8]);
    require(v0>=0&&theta>=0&&xi>=0&&std::abs(rho)<=1,"Invalid Heston parameters");
    double dt=T/(d/2),root=std::sqrt(dt),a=std::sqrt(std::max(0.0,1-rho*rho));
    Report out{{"Path","TerminalSpot","TerminalVariance","IntegratedVariance","NegativeRawVarianceSteps"},{}};
    for(std::size_t i=0;i<x.size();++i){
        double logS=std::log(S),v=v0,iv=0,neg=0;
        for(std::size_t j=0;j<d;j+=2){
            double vp=std::max(v,0.0),z=x[i][j],w=rho*z+a*x[i][j+1];iv+=vp*dt;
            logS+=(r-q-.5*vp)*dt+std::sqrt(vp)*root*z;
            v+=k*(theta-vp)*dt+xi*std::sqrt(vp)*root*w;if(v<0)++neg;
        }
        out.rows.push_back({double(i),std::exp(logS),std::max(0.0,v),iv,neg});
    }
    return out;
}
} }
