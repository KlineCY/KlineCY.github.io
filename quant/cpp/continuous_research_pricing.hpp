#pragma once
#include "continuous_research_common.hpp"
namespace crq {
// European Black-Scholes log-grid: pure Crank-Nicolson versus Rannacher start.
// Exact Black-Scholes boundaries are used to isolate interior discretization error.
inline Vec bs_rannacher(const Vec& x){
    check(x.size()==9);
    double s=x[0],k=x[1],r=x[2],q=x[3],vol=x[4],T=x[5],width=x[6];
    int M=integer(x[7],4,4000),N=integer(x[8],1,20000);
    check(M%2==0&&s>0&&k>0&&vol>0&&T>0&&width>0&&width<=10&&r>=0&&long(M)*N<=20000000);
    double h=2*width/M,dt=T/N,drift=r-q-.5*vol*vol;
    double a=.5*vol*vol/(h*h)-drift/(2*h),b=-vol*vol/(h*h)-r,c=.5*vol*vol/(h*h)+drift/(2*h);
    check(a>=0&&c>=0);
    Vec spot(M+1);for(int j=0;j<=M;++j)spot[j]=s*std::exp((j-M/2)*h);
    auto evolve=[&](bool rannacher){
        Vec v(M+1);for(int j=0;j<=M;++j)v[j]=std::max(spot[j]-k,0.);
        double time=0,error=0;
        auto step=[&](double d,double theta){
            double tau=time+d,lo=bs(spot.front(),k,r,q,vol,tau,1),hi=bs(spot.back(),k,r,q,vol,tau,1);
            Vec D(M-1,1-theta*d*b),L(M-2,-theta*d*a),U(M-2,-theta*d*c),rhs(M-1);
            for(int j=1;j<M;++j)rhs[j-1]=v[j]+(1-theta)*d*(a*v[j-1]+b*v[j]+c*v[j+1]);
            rhs.front()+=theta*d*a*lo;rhs.back()+=theta*d*c*hi;
            Vec w=tri_solve(D,L,U,rhs);error=std::max(error,residual(tri_mult(D,L,U,w),rhs));
            v[0]=lo;v[M]=hi;for(int j=1;j<M;++j)v[j]=w[j-1];time=tau;
        };
        if(rannacher){step(dt/2,1);step(dt/2,1);}
        else step(dt,.5);
        for(int t=1;t<N;++t)step(dt,.5);
        int j=M/2;double dx=(v[j+1]-v[j-1])/(2*h),dxx=(v[j+1]-2*v[j]+v[j-1])/(h*h);
        return Vec{v[j],dx/s,(dxx-dx)/(s*s),error};
    };
    Vec cn=evolve(false),ra=evolve(true);
    double exact=bs(s,k,r,q,vol,T,1),st=vol*std::sqrt(T),d1=(std::log(s/k)+(r-q+.5*vol*vol)*T)/st;
    double delta=std::exp(-q*T)*cdf(d1),gamma=std::exp(-q*T)*pdf(d1)/(s*st);
    return {cn[0],ra[0],exact,cn[0]-exact,ra[0]-exact,cn[1],ra[1],delta,cn[2],ra[2],gamma,std::max(cn[3],ra[3])};
}
// Exact adjoint of a tridiagonal linear solve with respect to all matrix and RHS entries.
inline Vec tridiagonal_adjoint(const Vec& x){
    check(!x.empty());int n=integer(x[0],2,5000);
    check(x.size()==size_t(5*n-1));
    size_t at=1;auto take=[&](int m){Vec a(x.begin()+at,x.begin()+at+m);at+=m;return a;};
    Vec d=take(n),l=take(n-1),u=take(n-1),b=take(n),c=take(n);
    for(int i=0;i<n;++i)check(std::abs(d[i])>(i?std::abs(l[i-1]):0)+(i+1<n?std::abs(u[i]):0)+1e-14);
    Vec y=tri_solve(d,l,u,b),adj=tri_solve(d,u,l,c);
    double objective=std::inner_product(c.begin(),c.end(),y.begin(),0.);
    Vec out{objective};out.insert(out.end(),y.begin(),y.end());out.insert(out.end(),adj.begin(),adj.end());
    for(int i=0;i<n;++i)out.push_back(-adj[i]*y[i]);
    for(int i=0;i<n-1;++i)out.push_back(-adj[i+1]*y[i]);
    for(int i=0;i<n-1;++i)out.push_back(-adj[i]*y[i+1]);
    out.insert(out.end(),adj.begin(),adj.end());
    out.push_back(residual(tri_mult(d,l,u,y),b));
    out.push_back(residual(tri_mult(d,u,l,adj),c));return out;
}
// Spread option by conditioning on the second normal factor, then integrating a lognormal call.
inline Vec spread_conditional(const Vec& x){
    check(x.size()==11);double s1=x[0],s2=x[1],k=x[2],v1=x[3],v2=x[4],rho=x[5],r=x[6],q1=x[7],q2=x[8],T=x[9];
    int n=integer(x[10],20,100000);check(n%2==0&&s1>0&&s2>0&&k>=0&&v1>0&&v2>0&&T>0&&std::abs(rho)<1);
    double a=v1*std::sqrt(T),b=v2*std::sqrt(T),sig=a*std::sqrt(1-rho*rho);
    auto f=[&](double z){
        double S2=s2*std::exp((r-q2-.5*v2*v2)*T+b*z),strike=S2+k;
        double logmu=std::log(s1)+(r-q1-.5*v1*v1)*T+a*rho*z;
        double d2=(logmu-std::log(strike))/sig,d1=d2+sig;
        double call=std::exp(logmu+.5*sig*sig)*cdf(d1)-strike*cdf(d2);
        return std::exp(-r*T)*pdf(z)*std::max(0.,call);
    };
    auto simpson=[&](int N){double h=18./N,total=f(-9)+f(9);for(int j=1;j<N;++j)total+=(j%2?4:2)*f(-9+h*j);return h*total/3;};
    double a1=simpson(n),a2=simpson(2*n),exchange=0;
    if(k==0){
        double st=std::sqrt((v1*v1+v2*v2-2*rho*v1*v2)*T);
        double d1=(std::log(s1/s2)+(q2-q1)*T+.5*st*st)/st,d2=d1-st;
        exchange=s1*std::exp(-q1*T)*cdf(d1)-s2*std::exp(-q2*T)*cdf(d2);
    }
    return {a1,a2,std::abs(a2-a1)/15,exchange,double(k==0),k==0?a2-exchange:0};
}
// Strong Euler error against exact OU using the same Brownian motion (no Monte Carlo).
inline Vec ou_coupling(const Vec& x){
    check(x.size()==6);double x0=x[0],mu=x[1],k=x[2],vol=x[3],T=x[4];int n=integer(x[5],1,1000000);
    check(k>0&&vol>=0&&T>0);
    double h=T/n,a=std::exp(-k*h),b=1-k*h,qe=vol*vol*(-std::expm1(-2*k*h))/(2*k),qu=vol*vol*h;
    double cross=vol*vol*(-std::expm1(-k*h))/k;
    double m1=x0,m2=x0,v1=0,v2=0,c=0;
    for(int j=0;j<n;++j){m1=mu+a*(m1-mu);m2=mu+b*(m2-mu);v1=a*a*v1+qe;v2=b*b*v2+qu;c=a*b*c+cross;}
    double mse=sq(m1-m2)+v1+v2-2*c;check(mse>=-1e-10*(1+v1+v2));
    double exactmu=mu+std::exp(-k*T)*(x0-mu),exactv=vol*vol*(-std::expm1(-2*k*T))/(2*k);
    return {m1,m2,v1,v2,c,std::sqrt(std::max(0.,mse)),m2-m1,v2-v1,exactmu,exactv};
}
}
