#pragma once
#include "numerical_lab_common.hpp"

namespace kline { namespace numerical_lab {
inline Vec european_grid(double K,double r,double q,double sigma,double T,double limit,std::size_t M,std::size_t N,bool call,bool barrier){
    double ds=limit/M,dt=T/N;
    Vec v(M+1),a(M-1),b(M-1),c(M-1),rhs(M-1);
    for(std::size_t j=0;j<=M;++j)v[j]=std::max(0.0,call?j*ds-K:K-j*ds);
    auto boundaries=[&](double tau){
        return std::pair<double,double>{call?0:K*std::exp(-r*tau),
            barrier?0:(call?std::max(0.0,limit*std::exp(-q*tau)-K*std::exp(-r*tau)):0)};
    };
    v[M]=boundaries(0).second;
    // Two implicit half steps smooth the initial payoff kink, followed by CN.
    for(std::size_t n=0;n<=N;++n){
        double h=n<2?dt/2:dt,theta=n<2?1:.5;
        double tau=n<2?(n+1)*dt/2:n*dt;
        auto bd=boundaries(tau);
        for(std::size_t j=1;j<M;++j){
            double x=double(j),diff=.5*sigma*sigma*x*x,drift=.5*(r-q)*x;
            double l=diff-drift,d=-2*diff-r,u=diff+drift;
            std::size_t i=j-1;
            a[i]=-h*theta*l;b[i]=1-h*theta*d;c[i]=-h*theta*u;
            rhs[i]=v[j]+h*(1-theta)*(l*v[j-1]+d*v[j]+u*v[j+1]);
        }
        rhs.front()-=a.front()*bd.first;rhs.back()-=c.back()*bd.second;
        auto z=tridiagonal(a,b,c,rhs);
        v[0]=bd.first;v[M]=bd.second;
        for(std::size_t j=1;j<M;++j)v[j]=z[j-1];
    }
    return v;
}
inline double interp(const Vec& v,double S,double limit){
    require(S>=0&&S<=limit,"Spot outside pricing grid");
    double j=S/limit*(v.size()-1);
    if(j>=v.size()-1)return v.back();
    auto k=std::size_t(j);
    return v[k]+(j-k)*(v[k+1]-v[k]);
}
inline Report pde_european(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,8);
    double S=positive(p[0]),K=positive(p[1]),r=p[2],q=p[3],sigma=positive(p[4]),T=positive(p[5]),limit=positive(p[6]);
    require((p[7]==0||p[7]==1)&&limit>std::max(S,K),"Invalid European grid domain");
    bool call=p[7]==1;
    double exact=bs(S,K,r,q,sigma,T,call);
    Report out{{"SpaceSteps","TimeSteps","GridPrice","AnalyticPrice","AbsError"},{}};
    double work=0;
    for(const auto& row:x){
        auto M=count(row[0],10,4000),N=count(row[1],2,4000);
        work+=double(M)*N;require(work<=3e7,"PDE work limit exceeded");
        auto v=european_grid(K,r,q,sigma,T,limit,M,N,call,false);
        double price=interp(v,S,limit);
        out.rows.push_back({double(M),double(N),price,exact,std::abs(price-exact)});
    }
    return out;
}
inline Report pde_barrier(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,8);
    double K=positive(p[0]),r=p[1],q=p[2],sigma=positive(p[3]),T=positive(p[4]),B=positive(p[5]);
    auto M=count(p[6],20,4000),N=count(p[7],2,4000);
    require(B>K&&double(M)*N<=2e7,"Invalid up-and-out grid");
    auto v=european_grid(K,r,q,sigma,T,B,M,N,true,true);
    Report out{{"Spot","KnockOutCall","VanillaCall","KnockOutDiscount"},{}};
    for(const auto& row:x){
        double S=positive(row[0]),e=bs(S,K,r,q,sigma,T,true),k=S>=B?0:interp(v,S,B);
        require(k>=-1e-8&&k<=e+1e-5*std::max(1.0,e),"Barrier grid violates price bounds");
        out.rows.push_back({S,k,e,e-k});
    }
    return out;
}
inline Report american_psor(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,9);
    double K=positive(p[0]),r=p[1],q=p[2],sig=positive(p[3]),T=positive(p[4]),Smax=positive(p[5]);
    auto M=count(p[6],20,1000),N=count(p[7],2,2000);double omega=p[8];
    require(r>=0&&Smax>K&&omega>0&&omega<2&&double(M)*N<=1e6,"Invalid American solver parameters");
    double ds=Smax/M,dt=T/N;
    Vec v(M+1),g(M+1),l(M+1),d(M+1),u(M+1);
    for(std::size_t j=0;j<=M;++j)v[j]=g[j]=std::max(K-j*ds,0.0);
    for(std::size_t j=1;j<M;++j){
        double z=double(j),a=.5*sig*sig*z*z,b=.5*(r-q)*z;
        l[j]=-dt*(a-b);u[j]=-dt*(a+b);d[j]=1+dt*(2*a+r);
        require(l[j]<=0&&u[j]<=0,"PSOR monotonicity needs nonpositive off-diagonal entries");
    }
    double max_residual=0;std::size_t iterations=0;
    for(std::size_t n=0;n<N;++n){
        Vec rhs=v;double residual=1;
        for(std::size_t it=0;it<10000;++it){
            for(std::size_t j=1;j<M;++j){
                double z=(rhs[j]-l[j]*v[j-1]-u[j]*v[j+1])/d[j];
                v[j]=std::max(g[j],v[j]+omega*(z-v[j]));
            }
            residual=0;
            for(std::size_t j=1;j<M;++j)residual=std::max(residual,std::abs(std::min(v[j]-g[j],l[j]*v[j-1]+d[j]*v[j]+u[j]*v[j+1]-rhs[j])));
            ++iterations;require(double(iterations)*M<=5e7,"PSOR iteration budget exceeded");
            if(residual<=1e-9*std::max(1.0,K))break;
        }
        require(residual<=1e-9*std::max(1.0,K),"PSOR complementarity did not converge");
        max_residual=std::max(max_residual,residual);
    }
    Report out{{"Spot","AmericanPut","EuropeanPut","IntrinsicValue","MaxComplementarityResidual","Sweeps"},{}};
    for(const auto& row:x){
        double s=positive(row[0]);require(s<Smax,"Spot outside pricing grid");
        out.rows.push_back({s,interp(v,s,Smax),bs(s,K,r,q,sig,T,false),std::max(K-s,0.0),max_residual,double(iterations)});
    }
    return out;
}
inline Report svi_slice(const Mat& x,const Vec& p){
    validate(x,3,2,2);parameters(p,2);
    double m=p[0],s=positive(p[1]);
    // Fixed location and curvature reduce the raw SVI fit to linear least squares.
    Mat g(3,Vec(3,0));Vec b(3,0);
    for(const auto& r:x){
        positive(r[1]);double z=r[0]-m;Vec f={1,z,std::hypot(z,s)};
        for(std::size_t i=0;i<3;++i){b[i]+=f[i]*r[1];for(std::size_t j=0;j<3;++j)g[i][j]+=f[i]*f[j];}
    }
    Vec fit=solve_spd(g,b);double a=fit[0],c=fit[1],B=positive(fit[2]),rho=c/B;
    require(std::abs(rho)<1&&a+B*s*std::sqrt(1-rho*rho)>0&&B*(1+std::abs(rho))<=2,"Inadmissible fitted SVI parameters");
    Report out{{"LogMoneyness","ObservedVariance","FittedVariance","Residual","DensityFactor","a","b","rho"},{}};
    for(const auto& r:x){
        double z=r[0]-m,h=std::hypot(z,s),w=a+B*(rho*z+h),wp=B*(rho+z/h),wpp=B*s*s/(h*h*h);
        double gf=std::pow(1-r[0]*wp/(2*w),2)-wp*wp/4*(1/w+.25)+wpp/2;
        out.rows.push_back({r[0],r[1],w,r[1]-w,gf,a,B,rho});
    }
    return out;
}
inline Report calendar_grid(const Mat& x,const Vec& p){
    auto d=validate(x,2,3);parameters(p,1);double tol=p[0];require(tol>=0,"Negative variance tolerance");
    Report out{{"LogMoneyness","NearSlice","FarSlice","VarianceChange","Violation"},{}};
    for(std::size_t i=0;i<x.size();++i){
        const auto& row=x[i];if(i)require(row[0]>x[i-1][0],"Moneyness grid must strictly increase");
        for(std::size_t j=1;j<d;++j)positive(row[j]);
        for(std::size_t j=1;j+1<d;++j){
            double change=row[j+1]-row[j];
            out.rows.push_back({row[0],double(j),double(j+1),change,change<-tol?1.0:0.0});
        }
    }
    return out;
}
} }
