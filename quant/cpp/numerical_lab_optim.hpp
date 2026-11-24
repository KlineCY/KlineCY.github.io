#pragma once
#include "numerical_lab_common.hpp"

namespace kline { namespace numerical_lab {
inline Vec box_project(const Vec& z,const Vec& lo,const Vec& hi,double budget){
    require(sum(lo)<=budget+1e-12&&sum(hi)>=budget-1e-12,"Infeasible portfolio budget");
    double a=std::numeric_limits<double>::infinity(),b=-a;
    for(std::size_t j=0;j<z.size();++j){require(lo[j]<=hi[j],"Reversed weight bounds");a=std::min(a,z[j]-hi[j]);b=std::max(b,z[j]-lo[j]);}
    Vec w(z.size());
    for(int k=0;k<90;++k){
        double m=a+(b-a)/2;
        for(std::size_t j=0;j<z.size();++j)w[j]=std::clamp(z[j]-m,lo[j],hi[j]);
        if(sum(w)>budget)a=m;else b=m;
    }
    require(std::abs(sum(w)-budget)<1e-8,"Projection budget residual");
    return w;
}
inline Report box_markowitz(const Mat& x,const Vec& p){
    auto d=validate(x,3,4,32);parameters(p,3);d-=3;
    require(x.size()==d,"One covariance row per asset required");
    double gamma=positive(p[0]),budget=p[1];auto n=count(p[2],1,50000);
    Mat A;Vec mu,lo,hi;
    for(const auto& row:x){mu.push_back(row[0]);lo.push_back(row[1]);hi.push_back(row[2]);A.emplace_back(row.begin()+3,row.end());}
    solve_spd(A,Vec(d,1));
    double L=0;
    for(const auto& row:A){double z=0;for(double v:row)z+=std::abs(v);L=std::max(L,z);}
    double step=1/positive(gamma*L);
    Vec w=box_project(Vec(d,budget/d),lo,hi,budget);
    double error=1;std::size_t it=0;
    for(;it<n;++it){
        Vec grad=mul(A,w),z=w;
        for(std::size_t j=0;j<d;++j)z[j]-=step*(gamma*grad[j]-mu[j]);
        z=box_project(z,lo,hi,budget);
        Vec delta=w;for(std::size_t j=0;j<d;++j)delta[j]=(w[j]-z[j])/step;
        error=norm(delta);w=std::move(z);
        if(error<1e-9)break;
    }
    require(error<1e-7,"Projected-gradient iteration did not converge");
    Vec risk=mul(A,w);double objective=.5*gamma*dot(w,risk)-dot(w,mu);
    Report out{{"Asset","Weight","MarginalVariance","ExpectedReturn","PortfolioVariance","Objective","GradientMappingNorm","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)out.rows.push_back({double(j+1),w[j],risk[j],dot(w,mu),dot(w,risk),objective,error,double(std::min(n,it+1))});
    return out;
}
inline Report turnover_admm(const Mat& x,const Vec& p){
    auto d=validate(x,2,5,32);parameters(p,4);d-=4;
    require(x.size()==d,"One covariance row per asset required");
    double gamma=positive(p[0]),lambda=p[1],rho=positive(p[2]);auto n=count(p[3],1,30000);
    require(lambda>=0,"Negative turnover penalty");
    Vec mu,old,lo,hi;
    Mat A;
    for(const auto& row:x){mu.push_back(row[0]);old.push_back(row[1]);lo.push_back(row[2]);hi.push_back(row[3]);A.emplace_back(row.begin()+4,row.end());}
    solve_spd(A,Vec(d,1));Mat H=A;
    for(std::size_t i=0;i<d;++i){for(double& v:H[i])v*=gamma;H[i][i]+=2*rho;}
    Vec w=box_project(old,lo,hi,1),z=w,v(d,0),u(d,0),h(d,0);
    for(std::size_t i=0;i<d;++i)v[i]=w[i]-old[i];
    double primal=1,dual=1;std::size_t iter=0;
    for(;iter<n;++iter){
        Vec b=mu;
        for(std::size_t i=0;i<d;++i)b[i]+=rho*(z[i]-u[i]+v[i]+old[i]-h[i]);
        w=solve_spd(H,b);
        Vec prevz=z,prevv=v,shift=w;
        for(std::size_t i=0;i<d;++i)shift[i]+=u[i];
        z=box_project(shift,lo,hi,1);
        double r2=0,s2=0;
        for(std::size_t i=0;i<d;++i){
            double a=w[i]-old[i]+h[i];
            v[i]=std::copysign(std::max(0.0,std::abs(a)-lambda/rho),a);
            double r1=w[i]-z[i],r3=w[i]-old[i]-v[i];u[i]+=r1;h[i]+=r3;
            r2+=r1*r1+r3*r3;
            double s=rho*(z[i]-prevz[i]+v[i]-prevv[i]);s2+=s*s;
        }
        primal=std::sqrt(r2);dual=std::sqrt(s2);
        if(primal<1e-9&&dual<1e-9)break;
    }
    require(primal<1e-7&&dual<1e-7,"ADMM residuals exceed tolerance");
    Vec trade=z;for(std::size_t i=0;i<d;++i)trade[i]-=old[i];
    double turnover=researchflow::l1(trade),var=dot(z,mul(A,z)),objective=.5*gamma*var-dot(mu,z)+lambda*turnover;
    Report out{{"Asset","Weight","Trade","TurnoverL1","PortfolioVariance","Objective","PrimalResidual","DualResidual","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)out.rows.push_back({double(j+1),z[j],trade[j],turnover,var,objective,primal,dual,double(std::min(n,iter+1))});
    return out;
}
inline Report equality_quadratic(const Mat& x,const Vec& p){
    auto d=validate(x,2,4,32);parameters(p,2);d-=2;
    require(x.size()==d,"One covariance row per asset required");
    Vec f,mu;Mat A;
    for(const auto& row:x){mu.push_back(row[0]);f.push_back(row[1]);A.emplace_back(row.begin()+2,row.end());}
    Vec one(d,1),a=solve_spd(A,one),b=solve_spd(A,f),z=solve_spd(A,mu);
    double aa=dot(one,a),ab=dot(one,b),bb=dot(f,b),det=positive(aa*bb-ab*ab);
    double e=dot(one,z)-p[0],g=dot(f,z)-p[1];
    double m=(bb*e-ab*g)/det,n=(aa*g-ab*e)/det;
    Vec w=z;for(std::size_t i=0;i<d;++i)w[i]-=m*a[i]+n*b[i];
    Vec risk=mul(A,w);double residual=0;
    for(std::size_t i=0;i<d;++i)residual=std::max(residual,std::abs(risk[i]-mu[i]+m+n*f[i]));
    Report out{{"Asset","Weight","FactorLoading","PortfolioBudget","FactorExposure","StationarityResidual"},{}};
    for(std::size_t i=0;i<d;++i)out.rows.push_back({double(i+1),w[i],f[i],sum(w),dot(w,f),residual});
    return out;
}
inline Report robust_mean_ellipsoid(const Mat& x,const Vec& p){
    auto d=validate(x,2,3,32);parameters(p,2);d-=1;require(x.size()==d,"One covariance row per asset required");
    double gamma=positive(p[0]),radius=p[1];require(radius>=0,"Negative uncertainty radius");
    Vec mu;Mat A;for(const auto& r:x){mu.push_back(r[0]);A.emplace_back(r.begin()+1,r.end());}
    Vec z=solve_spd(A,mu);double signal=std::sqrt(std::max(0.0,dot(mu,z)));
    double scale=signal>radius?(1-radius/signal)/gamma:0;
    Vec w=z;for(double& v:w)v*=scale;
    double vol=std::sqrt(std::max(0.0,dot(w,mul(A,w)))),worst=dot(mu,w)-radius*vol;
    Report out{{"Asset","Weight","Volatility","NominalExpectedReturn","WorstCaseExpectedReturn","Objective"},{}};
    for(std::size_t i=0;i<d;++i)out.rows.push_back({double(i+1),w[i],vol,dot(mu,w),worst,worst-.5*gamma*vol*vol});
    return out;
}
inline Report exposure_frontier(const Mat& x,const Vec& p){
    auto d=validate(x,2,3,32);require(p.size()>=1&&p.size()<=100,"At least one target return required");check_finite(p);d-=1;
    require(x.size()==d,"One covariance row per asset required");
    Vec mu;Mat A;for(const auto& r:x){mu.push_back(r[0]);A.emplace_back(r.begin()+1,r.end());}
    Vec one(d,1),a=solve_spd(A,one),b=solve_spd(A,mu);
    double aa=dot(one,a),ab=dot(one,b),bb=dot(mu,b),det=positive(aa*bb-ab*ab);
    Report out{{"TargetReturn","ExpectedReturn","Variance","GrossExposure"},{}};
    for(std::size_t j=0;j<d;++j)out.header.push_back("Weight"+std::to_string(j+1));
    for(double target:p){
        double c=(bb-ab*target)/det,k=(aa*target-ab)/det;Vec w(d);
        for(std::size_t i=0;i<d;++i)w[i]=c*a[i]+k*b[i];
        Vec row={target,dot(mu,w),dot(w,mul(A,w)),researchflow::l1(w)};
        row.insert(row.end(),w.begin(),w.end());out.rows.push_back(row);
    }
    return out;
}
} }
