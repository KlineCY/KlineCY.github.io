#pragma once
#include "evidence_common.hpp"
namespace kline { namespace monitoring {
using namespace evidence;
inline Report bernoulli_mixture(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,2);
    double q=p[0],alpha=p[1];
    require(q>0&&q<1&&alpha>0&&alpha<1,"Invalid null or significance");
    double successes=0,peak=0,log_e=0;
    Report out{{"Time","Successes","LogE","MaxLogE","Crossed"},{}};
    for(std::size_t i=0;i<x.size();++i){
        double y=x[i][0],t=double(i+1);
        require(y==0||y==1,"Binary observations required");
        // Uniform beta mixture predictive probability based only on past outcomes.
        double pred=(successes+1)/(t+1);
        log_e+=y?std::log(pred/q):std::log((1-pred)/(1-q));
        successes+=y;peak=std::max(peak,log_e);
        out.rows.push_back({t,successes,log_e,peak,double(peak>=-std::log(alpha))});
    }
    return out;
}
inline Report bounded_betting(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,3);
    double mu=p[0],lambda=p[1],alpha=p[2];
    require(mu>0&&mu<1&&lambda>0&&lambda<1/mu&&alpha>0&&alpha<1,"Invalid betting parameters");
    double log_e=0,peak=0;
    Report out{{"Time","Observation","LogE","MaxLogE","Crossed"},{}};
    for(std::size_t i=0;i<x.size();++i){
        double y=x[i][0];require(y>=0&&y<=1,"Bounded observations required");
        log_e+=std::log1p(lambda*(y-mu));peak=std::max(peak,log_e);
        out.rows.push_back({double(i+1),y,log_e,peak,double(peak>=-std::log(alpha))});
    }
    return out;
}
inline Report alpha_spending(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,1);
    require(p[0]>0&&p[0]<1,"Invalid family error level");
    double spent=0,reject=0;
    Report out{{"Time","PValue","LocalAlpha","Spent","Reject","AnyReject"},{}};
    for(std::size_t i=0;i<x.size();++i){
        double t=double(i+1),v=x[i][0],a=p[0]/(t*(t+1));
        require(v>=0&&v<=1,"Invalid p-value");
        spent+=a;double r=double(v<=a);reject=std::max(reject,r);
        out.rows.push_back({t,v,a,spent,r,reject});
    }
    return out;
}
inline Report likelihood_stopping(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,4);
    double q=p[0],r=p[1],alpha=p[2],beta=p[3];
    require(q>0&&q<r&&r<1&&alpha>0&&alpha<1&&beta>0&&beta<1,"Invalid hypotheses or thresholds");
    double log_lr=0,decision=0,stop=0;
    Report out{{"Time","LogLR","Decision","StopTime"},{}};
    for(std::size_t i=0;i<x.size();++i){
        double y=x[i][0];require(y==0||y==1,"Binary observations required");
        if(decision==0){
            log_lr+=y?std::log(r/q):std::log((1-r)/(1-q));
            if(log_lr>=-std::log(alpha)){decision=1;stop=double(i+1);}
            else if(log_lr<=std::log(beta)){decision=-1;stop=double(i+1);}
        }
        out.rows.push_back({double(i+1),log_lr,decision,stop});
    }
    return out;
}
} }
