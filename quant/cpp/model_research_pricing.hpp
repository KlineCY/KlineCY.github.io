#pragma once
#include "model_research_optimization.hpp"
#include <random>
namespace kline { namespace model_research {
inline Report american_put_pde(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,10);
    double K=positive(p[0]),r=p[1],q=p[2],vol=p[3],T=positive(p[4]),Smax=positive(p[5]);
    auto M=count(p[6],4,2000),N=count(p[7],1,2000),maxit=count(p[9],1,100000);
    double tol=positive(p[8]);
    require(r>=0&&q>=0&&vol>=0&&Smax>K&&double(M)*N*maxit<=3e8,"Invalid PDE parameters or work limit");
    double h=Smax/M,dt=T/N;auto n=M-1;
    Vec lower(n,0),diag(n,0),upper(n,0),left(n),right(n),pay(n),e(n),a(n);
    for(std::size_t j=0;j<n;++j){
        double s=(j+1)*h,b=(r-q)*s/h,v=.5*vol*vol*s*s/(h*h);
        left[j]=dt*(v+std::max(-b,0.));right[j]=dt*(v+std::max(b,0.));
        diag[j]=1+left[j]+right[j]+dt*r;
        if(j)lower[j]=-left[j];if(j+1<n)upper[j]=-right[j];
        pay[j]=std::max(K-s,0.);e[j]=a[j]=pay[j];
    }
    double maxgap=0,used=0;
    for(std::size_t t=1;t<=N;++t){
        Vec b=e,c=a;b[0]+=left[0]*K*std::exp(-r*t*dt);c[0]+=left[0]*K;
        e=tridiagonal(lower,diag,upper,b);
        double gap=0;std::size_t it=0;
        for(;it<maxit;){
            for(std::size_t j=0;j<n;++j){
                double z=c[j];if(j)z-=lower[j]*a[j-1];if(j+1<n)z-=upper[j]*a[j+1];
                a[j]=std::max(pay[j],z/diag[j]);
            }
            gap=0;
            for(std::size_t j=0;j<n;++j){
                double z=diag[j]*a[j]-c[j];if(j)z+=lower[j]*a[j-1];if(j+1<n)z+=upper[j]*a[j+1];
                double w=a[j]-pay[j];
                gap=std::max({gap,std::max(0.,-z),std::max(0.,-w),std::abs(std::min(z,w))});
            }
            ++it;if(gap<=tol)break;
        }
        require(gap<=tol,"American complementarity iteration did not converge");
        maxgap=std::max(maxgap,gap);used=std::max(used,double(it));
    }
    auto interp=[&](const Vec& v,double low,double s){
        require(s>=0&&s<=Smax,"Query outside PDE grid");
        if(s==Smax)return 0.;
        double z=s/h;auto j=std::min(M-1,std::size_t(std::floor(z)));double f=z-j;
        double l=j==0?low:v[j-1],u=j+1==M?0:v[j];
        return (1-f)*l+f*u;
    };
    Report o{{"Spot","AmericanPut","EuropeanPut","EarlyExercisePremium","MaxComplementarityResidual","MaxIterations"},{}};
    for(auto&z:x){double s=z[0],va=interp(a,K,s),ve=interp(e,K*std::exp(-r*T),s);o.rows.push_back({s,va,ve,va-ve,maxgap,used});}
    return o;
}

inline Report localvol_call_pde(const Mat& x,const Vec& p){
    validate(x,5,2,2);parameters(p,5);
    double K=positive(p[0]),r=p[1],q=p[2],T=positive(p[3]);auto N=count(p[4],1,10000);
    require(r>=0&&q>=0&&x[0][0]==0&&x.back()[0]>K&&double(N)*x.size()<=5e7,"Invalid local volatility grid");
    double h=positive(x[1][0]),dt=T/N,Smax=x.back()[0];auto M=x.size()-1,n=M-1;
    for(std::size_t j=0;j<=M;++j){require(std::abs(x[j][0]-j*h)<=1e-10*std::max(1.,x[j][0])&&x[j][1]>=0,"Uniform spot grid and nonnegative volatility required");}
    Vec lower(n,0),diag(n,0),upper(n,0),left(n),right(n),v(n);
    for(std::size_t j=0;j<n;++j){
        double s=x[j+1][0],sig=x[j+1][1],b=(r-q)*s/h,d=.5*sig*sig*s*s/(h*h);
        left[j]=dt*(d+std::max(-b,0.));right[j]=dt*(d+std::max(b,0.));
        diag[j]=1+left[j]+right[j]+r*dt;
        if(j)lower[j]=-left[j];if(j+1<n)upper[j]=-right[j];
        v[j]=std::max(s-K,0.);
    }
    double err=0,high=0;
    for(std::size_t t=1;t<=N;++t){
        high=Smax*std::exp(-q*t*dt)-K*std::exp(-r*t*dt);
        require(high>=0,"Upper asymptotic boundary must be nonnegative");
        Vec b=v;b[n-1]+=right[n-1]*high;
        v=tridiagonal(lower,diag,upper,b);
        for(std::size_t j=0;j<n;++j){
            double z=diag[j]*v[j]-b[j];if(j)z+=lower[j]*v[j-1];if(j+1<n)z+=upper[j]*v[j+1];
            err=std::max(err,std::abs(z));
        }
    }
    Report o{{"Spot","LocalVol","CallPrice","TerminalPayoff","MaxLinearResidual"},{}};
    for(std::size_t j=0;j<=M;++j)o.rows.push_back({x[j][0],x[j][1],j==0?0:j==M?high:v[j-1],std::max(x[j][0]-K,0.),err});
    return o;
}

inline Report asian_independent_control(const Mat& x,const Vec& p){
    validate(x,1,6,6);parameters(p,4);
    auto m=count(p[0],1,2520),pilot=count(p[1],2,2000000),n=count(p[2],2,2000000),seed=count(p[3],0,1000000000);
    require(double(pilot+n)*m*x.size()<=1e8,"Asian simulation work limit");
    struct Moments{
        double n=0,x=0,y=0,xx=0,yy=0,xy=0;
        void push(double a,double b){++n;double da=a-x,db=b-y;x+=da/n;y+=db/n;xx+=da*(a-x);yy+=db*(b-y);xy+=da*(b-y);}
    };
    Report o{{"PlainPrice","ControlPrice","PlainSE","ControlSE","Beta","GeometricAnalytic","MainPaths"},{}};
    for(std::size_t row=0;row<x.size();++row){
        auto&r=x[row];double S=positive(r[0]),K=positive(r[1]),rate=r[2],q=r[3],vol=r[4],T=positive(r[5]);
        require(vol>=0,"Negative volatility");double dt=T/m,disc=std::exp(-rate*T),drift=(rate-q-.5*vol*vol)*dt,sdev=vol*std::sqrt(dt);
        double mu=std::log(S)+(rate-q-.5*vol*vol)*T*(m+1)/(2.*m),v=vol*vol*T*(m+1)*(2.*m+1)/(6.*m*m);
        double geom;
        if(v==0)geom=disc*std::max(std::exp(mu)-K,0.);
        else{double d2=(mu-std::log(K))/std::sqrt(v);geom=disc*(std::exp(mu+v/2)*normal(d2+std::sqrt(v))-K*normal(d2));}
        auto simulate=[&](std::mt19937_64& rng){
            auto uniform=[&](){return (double(rng()>>11)+.5)/9007199254740992.;};
            double logS=std::log(S),sumS=0,sumLog=0;
            for(std::size_t j=0;j<m;++j){
                double z=std::sqrt(-2*std::log(uniform()))*std::cos(2*std::acos(-1.)*uniform());
                logS+=drift+sdev*z;sumS+=std::exp(logS);sumLog+=logS;
            }
            return std::make_pair(disc*std::max(sumS/m-K,0.),disc*std::max(std::exp(sumLog/m)-K,0.));
        };
        std::mt19937_64 rng1(std::uint64_t(seed)+2*row),rng2(std::uint64_t(seed)+2*row+1);
        Moments fit,main;
        for(std::size_t j=0;j<pilot;++j){auto z=simulate(rng1);fit.push(z.first,z.second);}
        double beta=fit.yy>1e-24?fit.xy/fit.yy:0;
        for(std::size_t j=0;j<n;++j){auto z=simulate(rng2);main.push(z.first,z.first-beta*(z.second-geom));}
        o.rows.push_back({main.x,main.y,std::sqrt(std::max(0.,main.xx)/(n-1)/n),std::sqrt(std::max(0.,main.yy)/(n-1)/n),beta,geom,double(n)});
    }
    return o;
}

inline Report cds_hazard_bootstrap(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,4);double rec=p[0],r=p[1],dt=positive(p[2]),tol=positive(p[3]);
    require(rec>=0&&rec<1&&r>=0,"Invalid recovery or discount rate");
    std::size_t prior=0;double surv=1,ann=0,prot=0;
    Report o{{"Maturity","Hazard","Survival","MarketSpread","FittedSpread","SpreadResidual","PremiumAnnuity","ProtectionPV"},{}};
    for(auto&row:x){
        double units=positive(row[0])/dt,round=std::round(units),spread=row[1];
        require(std::abs(units-round)<=1e-9*std::max(1.,units)&&spread>=0,"Maturity must align to accrual grid and spread nonnegative");
        auto end=count(round,1,10000);require(end>prior,"Increasing maturities required");
        struct Leg{double s,a,b;};
        auto calc=[&](double h){
            Leg z{surv,ann,prot};double decay=std::exp(-h*dt);
            for(std::size_t j=prior+1;j<=end;++j){
                double next=z.s*decay,df=std::exp(-r*j*dt);
                z.a+=dt*df*next;z.b+=(1-rec)*df*(z.s-next);z.s=next;
            }
            return z;
        };
        auto f=[&](double h){auto z=calc(h);return z.b-spread*z.a;};
        double lo=0,hi=100/dt;require(f(lo)<=tol&&f(hi)>=-tol,"CDS quote has no nonnegative hazard root");
        double h=0;
        if(std::abs(f(0))>tol){
            for(int k=0;k<100;++k){double m=lo/2+hi/2;if(f(m)<0)lo=m;else hi=m;}
            h=lo/2+hi/2;
        }
        auto z=calc(h);double fitted=z.b/positive(z.a);require(std::abs(fitted-spread)<=tol,"CDS calibration residual");
        surv=z.s;ann=z.a;prot=z.b;prior=end;
        o.rows.push_back({row[0],h,surv,spread,fitted,fitted-spread,ann,prot});
    }
    return o;
}

inline Report nelson_siegel_calibration(const Mat& x,const Vec& p){
    validate(x,3,3,3);parameters(p,4);
    double lo=positive(p[0]),hi=positive(p[1]),lam=p[3];auto g=count(p[2],2,10000);
    require(hi>=lo&&lam>=0&&double(g)*x.size()<=5e7,"Invalid curve grid");
    for(auto&r:x){positive(r[0]);positive(r[2]);}
    auto z=[](double t,double tau){double a=t/tau,b=-std::expm1(-a)/a;return Vec{1,b,b-std::exp(-a)};};
    double best=std::numeric_limits<double>::infinity(),tau=lo;Vec w;
    for(std::size_t k=0;k<g;++k){
        double t=std::exp(std::log(lo)+(std::log(hi)-std::log(lo))*k/(g-1));
        Mat H(3,Vec(3,0));Vec b(3,0);
        for(auto&r:x){Vec a=z(r[0],t);add_outer(H,a,r[2]);for(std::size_t j=0;j<3;++j)b[j]+=r[2]*a[j]*r[1];}
        H[1][1]+=lam;H[2][2]+=lam;
        Vec a=solve_spd(H,b);double f=lam*(a[1]*a[1]+a[2]*a[2]);
        for(auto&r:x){double e=r[1]-dot(a,z(r[0],t));f+=r[2]*e*e;}
        if(f<best){best=f;tau=t;w=a;}
    }
    require(w.size()==3,"No curve fit");
    Report o{{"Maturity","ObservedRate","FittedRate","Residual","Level","Slope","Curvature","Tau","PenalizedSSE"},{}};
    for(auto&r:x){double v=dot(w,z(r[0],tau));o.rows.push_back({r[0],r[1],v,r[1]-v,w[0],w[1],w[2],tau,best});}
    return o;
}

inline Report risk_neutral_entropy_fit(const Mat& x,const Vec& p){
    validate(x,3,2,2);
    require(p.size()>=6&&p.size()%2==0,"Discount, forward, strike-price pairs, tolerance and iterations required");
    parameters(p,p.size());double D=positive(p[0]),F=positive(p[1]),tol=positive(p[p.size()-2]);auto m=(p.size()-4)/2;
    require(m<=15&&x.size()>=m+2,"Insufficient state support or too many strikes");
    double scale=std::max(F,x.back()[0]);Vec target{F/scale};
    for(std::size_t j=0;j<m;++j){double K=positive(p[2+2*j]),price=p[3+2*j];require(K>x.front()[0]&&K<x.back()[0]&&price>=0&&(j==0||K>p[2*j]),"Invalid strike or price");target.push_back(price/(D*scale));}
    target.push_back(tol);target.push_back(p.back());Mat data;
    for(std::size_t i=0;i<x.size();++i){
        double s=positive(x[i][0]),w=positive(x[i][1]);require(i==0||s>x[i-1][0],"Increasing terminal states required");
        Vec r{w,s/scale};for(std::size_t j=0;j<m;++j)r.push_back(std::max(s-p[2+2*j],0.)/scale);data.push_back(r);
    }
    auto fit=entropy_balance_fit(data,target);double forward=0,kl=fit.rows[0][3],gap=0;Vec calls(m,0);
    for(std::size_t i=0;i<x.size();++i){
        double q=fit.rows[i][1];forward+=q*x[i][0];for(std::size_t j=0;j<m;++j)calls[j]+=D*q*std::max(x[i][0]-p[2+2*j],0.);
    }
    gap=std::abs(forward-F);for(std::size_t j=0;j<m;++j)gap=std::max(gap,std::abs(calls[j]-p[3+2*j]));
    Report o{{"State","Probability","StatePrice","Forward","KL","MaxMomentResidual"},{}};
    for(std::size_t j=0;j<m;++j)o.header.push_back("CallPrice"+std::to_string(j+1));
    for(std::size_t i=0;i<x.size();++i){double q=fit.rows[i][1];Vec r{x[i][0],q,D*q,forward,kl,gap};r.insert(r.end(),calls.begin(),calls.end());o.rows.push_back(r);}
    return o;
}
} }
