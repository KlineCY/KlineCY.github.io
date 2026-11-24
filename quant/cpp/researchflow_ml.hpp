#pragma once
#include "researchflow_common.hpp"

namespace kline { namespace researchflow {
struct RidgeFit {Vec center,scale,beta;double intercept;};
inline RidgeFit ridge_fit(const Mat& a,std::size_t start,std::size_t end,double lambda){
    require(end<=a.size()&&end>=start+2&&lambda>0,"Invalid Ridge training interval");
    std::size_t d=a[0].size()-1,n=end-start;
    Vec m(d,0),s(d,0),b(d,0);
    double y=0;
    for(std::size_t t=start;t<end;++t){y+=a[t][d]/n;for(std::size_t j=0;j<d;++j)m[j]+=a[t][j]/n;}
    for(std::size_t t=start;t<end;++t)for(std::size_t j=0;j<d;++j){double u=a[t][j]-m[j];s[j]+=u*u/n;}
    for(double& v:s){v=std::sqrt(v);if(v<1e-12)v=1;}
    Mat g(d,Vec(d,0));Vec x(d);
    for(std::size_t t=start;t<end;++t){
        for(std::size_t j=0;j<d;++j)x[j]=(a[t][j]-m[j])/s[j];
        for(std::size_t j=0;j<d;++j){
            b[j]+=x[j]*(a[t][d]-y)/n;
            for(std::size_t k=0;k<=j;++k)g[j][k]+=x[j]*x[k]/n;
        }
    }
    for(std::size_t j=0;j<d;++j){g[j][j]+=lambda;for(std::size_t k=0;k<j;++k)g[k][j]=g[j][k];}
    return {m,s,solve_spd(g,b),y};
}
inline double predict(const RidgeFit& f,const Vec& row){
    double y=f.intercept;
    for(std::size_t j=0;j<f.beta.size();++j)y+=f.beta[j]*(row[j]-f.center[j])/f.scale[j];
    return y;
}
inline Report rolling_ridge(const Mat& x,const Vec& p){
    validate(x,4,2);parameters(p,3);
    auto train=count(p[0],2,x.size()-1),gap=count(p[1],0,x.size()-1);
    double lambda=positive(p[2]);require(train+gap<x.size(),"No out-of-sample period");
    Report out{{"Period","TrainStart","TrainEndExclusive","Prediction","Actual","HistoricalMean","SquaredError","BaselineSquaredError"},{}};
    for(std::size_t t=train+gap;t<x.size();++t){
        std::size_t end=t-gap,start=end-train;
        auto f=ridge_fit(x,start,end,lambda);
        double a=x[t].back(),v=predict(f,x[t]);
        out.rows.push_back({double(t),double(start),double(end),v,a,f.intercept,std::pow(v-a,2),std::pow(f.intercept-a,2)});
    }
    return out;
}
inline Report nested_ridge(const Mat& x,const Vec& p){
    validate(x,6,2);require(p.size()>=5&&p.size()<=24,"Training, gap, validation size and at least two lambdas required");check_finite(p);
    auto train=count(p[0],4,x.size()-1),gap=count(p[1],0,x.size()-1),val=count(p[2],1,train-1);
    require(train+gap<x.size()&&train>val+gap+1,"Insufficient nested training history");
    for(std::size_t j=3;j<p.size();++j)positive(p[j]);
    Report out{{"Period","TrainStart","TrainEndExclusive","SelectedLambda","ValidationMSE","Prediction","Actual","SquaredError"},{}};
    for(std::size_t t=train+gap;t<x.size();++t){
        std::size_t end=t-gap,start=end-train,split=end-val;
        double best=std::numeric_limits<double>::infinity(),chosen=0;
        for(std::size_t j=3;j<p.size();++j){
            auto fit=ridge_fit(x,start,split-gap,p[j]);double mse=0;
            for(std::size_t k=split;k<end;++k){double e=predict(fit,x[k])-x[k].back();mse+=e*e/val;}
            if(mse<best){best=mse;chosen=p[j];}
        }
        auto fit=ridge_fit(x,start,end,chosen);double v=predict(fit,x[t]),a=x[t].back();
        out.rows.push_back({double(t),double(start),double(end),chosen,best,v,a,(v-a)*(v-a)});
    }
    return out;
}
inline Report conformal_rolling(const Mat& x,const Vec& p){
    validate(x,3,2,2);parameters(p,3);
    auto n=count(p[0],2,x.size()-1),gap=count(p[1],0,x.size()-1);
    double alpha=p[2];require(alpha>0&&alpha<1&&n+gap<x.size(),"Invalid calibration parameters");
    std::size_t rank=std::size_t(std::ceil((n+1)*(1-alpha)));
    require(rank>=1&&rank<=n,"Finite conformal quantile unavailable for calibration size");
    Report out{{"Period","CalibrationStart","CalibrationEndExclusive","Prediction","Lower","Upper","Actual","Covered","IntervalWidth"},{}};
    for(std::size_t t=n+gap;t<x.size();++t){
        std::size_t end=t-gap,start=end-n;Vec errors;
        for(std::size_t i=start;i<end;++i)errors.push_back(std::abs(x[i][0]-x[i][1]));
        std::nth_element(errors.begin(),errors.begin()+rank-1,errors.end());
        double q=errors[rank-1],v=x[t][0],a=x[t][1];
        out.rows.push_back({double(t),double(start),double(end),v,v-q,v+q,a,a>=v-q&&a<=v+q?1.0:0.0,2*q});
    }
    return out;
}
inline Report rolling_calibration(const Mat& x,const Vec& p){
    validate(x,3,2,2);parameters(p,5);
    auto n=count(p[0],2,x.size()-1),gap=count(p[1],0,x.size()-1),bins=count(p[2],2,100);
    double a=positive(p[3]),b=positive(p[4]);require(n+gap<x.size(),"No calibration test period");
    for(const auto& row:x)require(row[0]>=0&&row[0]<=1&&(row[1]==0||row[1]==1),"Invalid probability or binary outcome");
    auto bin=[&](double v){return std::min(bins-1,std::size_t(v*bins));};
    Vec mass(bins,0),hits(bins,0);
    auto add=[&](std::size_t i,double s){auto j=bin(x[i][0]);mass[j]+=s;hits[j]+=s*x[i][1];};
    for(std::size_t i=0;i<n;++i)add(i,1);
    Report out{{"Period","RawProbability","CalibratedProbability","Actual","BinCount","RawBrier","CalibratedBrier","CalibratedLogLoss"},{}};
    for(std::size_t t=n+gap;t<x.size();++t){
        if(t>n+gap){add(t-gap-n-1,-1);add(t-gap-1,1);}
        auto j=bin(x[t][0]);double q=(hits[j]+a)/(mass[j]+a+b),y=x[t][1];
        double ll=y==1?-std::log(q):-std::log1p(-q);
        out.rows.push_back({double(t),x[t][0],q,y,mass[j],std::pow(x[t][0]-y,2),std::pow(q-y,2),ll});
    }
    return out;
}
inline Report forecast_comparison(const Mat& x,const Vec& p){
    validate(x,3,3,3);parameters(p,1);auto l=count(p[0],0,x.size()-1);
    Vec differential;double mse1=0,mse2=0,mae1=0,mae2=0,hit1=0,hit2=0,n=double(x.size());
    for(const auto& row:x){
        double e1=row[0]-row[2],e2=row[1]-row[2];
        mse1+=e1*e1/n;mse2+=e2*e2/n;mae1+=std::abs(e1)/n;mae2+=std::abs(e2)/n;
        hit1+=((row[0]>0)==(row[2]>0)?1.0:0.0)/n;hit2+=((row[1]>0)==(row[2]>0)?1.0:0.0)/n;
        differential.push_back(e1*e1-e2*e2);
    }
    double se=positive(hac_mean_standard_error(differential,int(l)));
    return {{"Observations","MSE1","MSE2","MAE1","MAE2","PositiveEventAccuracy1","PositiveEventAccuracy2","LossDifference","HACStandardError","DMStatistic"},
        {{n,mse1,mse2,mae1,mae2,hit1,hit2,mean(differential),se,mean(differential)/se}}};
}
} }
