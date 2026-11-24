#pragma once
#include "researchflow_ml.hpp"

namespace kline { namespace researchflow {
inline Report rolling_ar(const Mat& x,const Vec& p){
    validate(x,5,1,1);parameters(p,3);
    auto look=count(p[0],4,x.size()-1),order=count(p[1],1,std::min<std::size_t>(look-2,63));
    double lambda=positive(p[2]);require(double(x.size()-look)*look*order*order<=2e8,"AR work limit exceeded");Vec y=column(x,0);
    Mat design;for(std::size_t t=order;t<y.size();++t){
        Vec row;for(std::size_t j=1;j<=order;++j)row.push_back(y[t-j]);row.push_back(y[t]);design.push_back(row);
    }
    Report out{{"Period","Prediction","Actual","Persistence","SquaredError","PersistenceSquaredError"},{}};
    for(std::size_t t=look;t<y.size();++t){
        auto f=ridge_fit(design,t-look,t-order,lambda);
        double v=predict(f,design[t-order]),e=v-y[t],b=y[t-1]-y[t];
        out.rows.push_back({double(t),v,y[t],y[t-1],e*e,b*b});
    }
    return out;
}
inline Report rolling_var(const Mat& x,const Vec& p){
    std::size_t d=validate(x,5,2,16);parameters(p,2);
    auto look=count(p[0],3,x.size()-1);double lambda=positive(p[1]);
    require(double(x.size())*d*(d+1)<=5000000&&double(x.size()-look)*look*d*d*d<=2e8,"VAR work limit exceeded");
    std::vector<Mat> design(d);
    for(std::size_t t=1;t<x.size();++t){
        for(std::size_t j=0;j<d;++j){Vec row=x[t-1];row.push_back(x[t][j]);design[j].push_back(std::move(row));}
    }
    Report out{{"Period","Asset","Prediction","Actual","Persistence","SquaredError","PersistenceSquaredError"},{}};
    for(std::size_t t=look;t<x.size();++t){
        for(std::size_t j=0;j<d;++j){
            auto f=ridge_fit(design[j],t-look,t-1,lambda);
            double v=predict(f,design[j][t-1]),e=v-x[t][j],b=x[t-1][j]-x[t][j];
            out.rows.push_back({double(t),double(j+1),v,x[t][j],x[t-1][j],e*e,b*b});
        }
    }
    return out;
}
inline Report volatility_forecast(const Mat& x,const Vec& p){
    validate(x,2,1,1);parameters(p,5);
    double e=positive(p[0]),g=e,decay=p[1],omega=positive(p[2]),alpha=p[3],beta=p[4];
    require(decay>=0&&decay<1&&alpha>=0&&beta>=0&&alpha+beta<1,"Invalid variance dynamics");
    Report out{{"Period","Return","EWMAForecastVariance","GARCHForecastVariance","EWMAStandardized","GARCHStandardized","EWMAQLIKE","GARCHQLIKE"},{}};
    for(std::size_t t=0;t<x.size();++t){
        double r=x[t][0],q=r*r;
        positive(e);positive(g);
        out.rows.push_back({double(t),r,e,g,r/std::sqrt(e),r/std::sqrt(g),std::log(e)+q/e,std::log(g)+q/g});
        e=decay*e+(1-decay)*q;g=omega+alpha*q+beta*g;
    }
    return out;
}
inline Report regime_forecast(const Mat& x,const Vec& p){
    validate(x,4,2,2);parameters(p,2);
    auto look=count(p[0],2,x.size()-1);double prior=positive(p[1]);
    for(const auto& row:x)require(row[0]==0||row[0]==1,"State must be binary");
    Report out{{"Period","PreviousState","State","State1Probability","Brier","ConditionalReturnPrediction","ActualReturn","SquaredError"},{}};
    for(std::size_t t=look;t<x.size();++t){
        double transitions[2][2]={{prior,prior},{prior,prior}},amount[2]={0,0},n[2]={0,0};
        for(std::size_t i=t-look;i<t;++i){
            int s=int(x[i][0]);amount[s]+=x[i][1];n[s]+=1;
            if(i>t-look)transitions[int(x[i-1][0])][s]+=1;
        }
        int prev=int(x[t-1][0]);double q=transitions[prev][1]/(transitions[prev][0]+transitions[prev][1]);
        // The pooled mean is used when a state has no historical return observations.
        double pooled=(amount[0]+amount[1])/look,m0=n[0]?amount[0]/n[0]:pooled,m1=n[1]?amount[1]/n[1]:pooled;
        double pred=(1-q)*m0+q*m1;
        out.rows.push_back({double(t),double(prev),x[t][0],q,std::pow(q-x[t][0],2),pred,x[t][1],std::pow(pred-x[t][1],2)});
    }
    return out;
}
inline Report filter_forecast(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,3);double state=p[0],v=p[1],q=p[2];
    require(v>=0&&q>=0,"Negative state variance");
    Report out{{"Period","PriorMean","PriorVariance","Observation","Innovation","InnovationVariance","PosteriorMean","PosteriorVariance","LogPredictiveDensity"},{}};
    for(std::size_t t=0;t<x.size();++t){
        double obs=x[t][0],r=positive(x[t][1]),a=v+q,s=positive(a+r),err=obs-state,k=a/s;
        double m=state+k*err,b=(1-k)*(1-k)*a+k*k*r;
        double ll=-.5*(std::log(2*std::acos(-1.0)*s)+err*err/s);
        out.rows.push_back({double(t),state,a,obs,err,s,m,b,ll});state=m;v=b;
    }
    return out;
}
} }
