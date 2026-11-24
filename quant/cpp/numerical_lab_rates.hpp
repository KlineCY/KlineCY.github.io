#pragma once
#include "numerical_lab_common.hpp"

namespace kline { namespace numerical_lab {
inline double ns_loading(double z){return std::abs(z)<1e-6?1-z/2+z*z/6:-std::expm1(-z)/z;}
inline Report nelson_siegel(const Mat& x,const Vec& p){
    validate(x,4,2,2);parameters(p,2);double lo=positive(p[0]),hi=positive(p[1]);
    require(lo<hi,"Invalid decay search interval");
    for(const auto& r:x)positive(r[0]);
    // Profile least squares over a logarithmically spaced decay grid.
    double best=std::numeric_limits<double>::infinity(),decay=lo;Vec beta;
    for(int k=0;k<=300;++k){
        double tau=std::exp(std::log(lo)+(std::log(hi)-std::log(lo))*k/300);
        Mat A(3,Vec(3,0));Vec b(3,0);
        for(const auto& r:x){double z=r[0]/tau,f=ns_loading(z);Vec v={1,f,f-std::exp(-z)};
            for(std::size_t i=0;i<3;++i){b[i]+=v[i]*r[1];for(std::size_t j=0;j<3;++j)A[i][j]+=v[i]*v[j];}}
        Vec v;try{v=solve_spd(A,b);}catch(const std::invalid_argument&){continue;}
        double rss=0;
        for(const auto& r:x){double z=r[0]/tau,f=ns_loading(z),e=v[0]+v[1]*f+v[2]*(f-std::exp(-z))-r[1];rss+=e*e;}
        if(rss<best){best=rss;decay=tau;beta=v;}
    }
    require(beta.size()==3,"No nonsingular Nelson-Siegel profile");
    Report out{{"Maturity","ObservedYield","FittedYield","Residual","Discount","InstantaneousForward","Beta0","Beta1","Beta2","Tau","RSS"},{}};
    for(const auto& r:x){
        double t=r[0],z=t/decay,f=ns_loading(z),y=beta[0]+beta[1]*f+beta[2]*(f-std::exp(-z));
        double forward=beta[0]+beta[1]*std::exp(-z)+beta[2]*z*std::exp(-z);
        out.rows.push_back({t,r[1],y,r[1]-y,std::exp(-t*y),forward,beta[0],beta[1],beta[2],decay,best});
    }
    return out;
}
inline Report keyrate_curve(const Mat& x,const Vec& p){
    validate(x,1,2,2);require(p.size()>=5&&p.size()%2==1,"Bump and at least two maturity-rate node pairs required");check_finite(p);
    double bump=positive(p[0]);require(bump<=.01,"Bump exceeds 100 basis points");
    Vec time,rate;
    for(std::size_t i=1;i<p.size();i+=2){positive(p[i]);if(!time.empty())require(p[i]>time.back(),"Curve times must increase");time.push_back(p[i]);rate.push_back(p[i+1]);}
    auto pv=[&](const Vec& z){
        double v=0;
        for(const auto& r:x){
            require(r[0]>=0,"Negative cash-flow time");double t=r[0],y=z.front();
            if(t>=time.back())y=z.back();
            else if(t>time.front()){
                auto j=std::size_t(std::upper_bound(time.begin(),time.end(),t)-time.begin());
                double h=(t-time[j-1])/(time[j]-time[j-1]);y=(1-h)*z[j-1]+h*z[j];
            }
            v+=r[1]*std::exp(-t*y);
        }
        return v;
    };
    double base=pv(rate);Report out{{"Node","Maturity","BasePV","UpPV","DownPV","CentralDV01","SecondDerivative"},{}};
    for(std::size_t j=0;j<rate.size();++j){
        Vec up=rate,down=rate;up[j]+=bump;down[j]-=bump;
        double a=pv(up),b=pv(down);
        out.rows.push_back({double(j+1),time[j],base,a,b,(b-a)/(2*bump)*.0001,(a+b-2*base)/(bump*bump)});
    }
    return out;
}
inline Report bond_cashflow_risk(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,2);double spread=p[0],shock=p[1];
    double price=0,first=0,second=0,scenario=0;
    for(const auto& r:x){
        require(r[0]>=0&&r[1]>=0,"Invalid bond cash flow");double v=r[1]*std::exp(-r[0]*(r[2]+spread));
        price+=v;first+=r[0]*v;second+=r[0]*r[0]*v;scenario+=v*std::exp(-shock*r[0]);
    }
    positive(price);
    double approx=price-first*shock+.5*second*shock*shock;
    return scalar_rows({"PV","SpreadDuration","SpreadConvexity","DV01","ShockedPV","QuadraticPV","ApproximationError"},{price,first/price,second/price,first*.0001,scenario,approx,approx-scenario});
}
inline Report cashflow_hedge(const Mat& x,const Vec& p){
    auto d=validate(x,2,3,17);parameters(p,1);double lambda=positive(p[0]);
    // Each row is an exposure scenario; last column is liability exposure.
    --d;Mat A(d,Vec(d,0));Vec b(d,0);
    for(const auto& r:x){for(std::size_t i=0;i<d;++i){
        b[i]+=r[i]*r[d];for(std::size_t j=0;j<d;++j)A[i][j]+=r[i]*r[j];
    }}
    for(std::size_t j=0;j<d;++j)A[j][j]+=lambda;
    Vec w=solve_spd(A,b);
    Report out{{"Scenario","LiabilityExposure","HedgeExposure","ResidualExposure"},{}};
    for(std::size_t j=0;j<d;++j)out.header.push_back("Hedge"+std::to_string(j+1));
    for(std::size_t i=0;i<x.size();++i){
        double y=0;for(std::size_t j=0;j<d;++j)y+=w[j]*x[i][j];
        Vec row={double(i),x[i][d],y,x[i][d]-y};row.insert(row.end(),w.begin(),w.end());out.rows.push_back(row);
    }
    return out;
}
inline Report inflation_indexed(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,2);double lag=p[0],base=positive(p[1]);
    require(lag>=0,"Negative indexation lag");
    // Inputs are contract-aligned reference indices, not contemporaneous CPI.
    Report out{{"PaymentTime","ReferenceTime","IndexRatio","NominalCash","DiscountedCash","CumulativePV"},{}};
    double total=0;
    for(std::size_t i=0;i<x.size();++i){
        const auto& r=x[i];require(r[0]>=lag&&r[1]>=0,"Invalid payment time or real cash flow");
        if(i)require(r[0]>x[i-1][0],"Payments must be ordered");
        double ratio=positive(r[2])/base,cash=r[1]*ratio,v=cash*std::exp(-r[3]*r[0]);total+=v;
        out.rows.push_back({r[0],r[0]-lag,ratio,cash,v,total});
    }
    return out;
}
} }
