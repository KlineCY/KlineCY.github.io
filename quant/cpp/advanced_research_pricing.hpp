#pragma once
#include "advanced_research_common.hpp"
#include <random>
namespace kline { namespace advanced_research {
inline Report merton_jump_mixture(const Mat& x,const Vec& p){
    validate(x,1,9,9);parameters(p,1);auto N=count(p[0],0,10000);
    require(double(N+1)*x.size()<=5e7,"Jump mixture work limit");
    Report o{{"Call","Put","PoissonMass","CallTailBound","PutTailBound","ParityResidual"},{}};
    auto mass=[&](double a){require(a>=0&&a<=600,"Poisson mean outside stable domain");double w=std::exp(-a),s=w;for(std::size_t n=1;n<=N;++n){w*=a/n;s+=w;}return std::clamp(s,0.,1.);};
    for(auto&r:x){
        double S=positive(r[0]),K=positive(r[1]),rate=r[2],q=r[3],vol=r[4],lambda=r[5],mj=r[6],sj=r[7],T=positive(r[8]);
        require(vol>=0&&lambda>=0&&sj>=0,"Nonnegative volatility and intensity required");
        double ej=positive(std::exp(mj+.5*sj*sj)),a=lambda*T,tilt=a*ej,pm=mass(a),tm=mass(tilt),w=std::exp(-a);
        double c=0,put=0,disc=std::exp(-rate*T),base=std::log(S)+(rate-q-lambda*(ej-1)-.5*vol*vol)*T;
        for(std::size_t n=0;n<=N;++n){
            if(n)w*=a/n;if(w==0)continue;
            double m=base+n*mj,v=vol*vol*T+n*sj*sj;
            double f=positive(std::exp(m+v/2)),cc,pp;
            if(v==0){cc=std::max(f-K,0.);pp=std::max(K-f,0.);}
            else{double s=std::sqrt(v),d2=(m-std::log(K))/s,d1=d2+s;cc=f*normal(d1)-K*normal(d2);pp=K*normal(-d2)-f*normal(-d1);}
            c+=w*disc*std::max(0.,cc);put+=w*disc*std::max(0.,pp);
        }
        double cb=S*std::exp(-q*T)*(1-tm),pb=K*disc*(1-pm),par=c-put-(S*std::exp(-q*T)-K*disc);
        o.rows.push_back({c,put,pm,cb,pb,par});
    }
    return o;
}

inline Report vasicek_bond_option(const Mat& x,const Vec& p){
    validate(x,1,7,7);parameters(p,0);
    Report o{{"DiscountExpiry","DiscountMaturity","BondForward","LogForwardVariance","Call","Put","ParityResidual"},{}};
    for(auto&r:x){
        double r0=r[0],k=positive(r[1]),theta=r[2],sig=r[3],s=positive(r[4]),t=positive(r[5]),K=positive(r[6]);
        require(sig>=0&&t>s&&k>=1e-6,"Invalid Vasicek option domain");
        auto B=[&](double h){return -std::expm1(-k*h)/k;};
        auto P=[&](double h){double b=B(h),a=(theta-sig*sig/(2*k*k))*(b-h)-sig*sig*b*b/(4*k);return positive(std::exp(a-b*r0));};
        double ps=P(s),pt=P(t),f=pt/ps,v=sig*sig*B(t-s)*B(t-s)*(-std::expm1(-2*k*s))/(2*k);
        double c,put;
        if(v==0){c=ps*std::max(f-K,0.);put=ps*std::max(K-f,0.);}
        else{double d1=(std::log(f/K)+v/2)/std::sqrt(v),d2=d1-std::sqrt(v);c=ps*(f*normal(d1)-K*normal(d2));put=ps*(K*normal(-d2)-f*normal(-d1));}
        o.rows.push_back({ps,pt,f,v,c,put,c-put-(pt-K*ps)});
    }
    return o;
}

inline Report callable_rate_lattice(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,3);
    double face=positive(p[0]),vol=p[1],dt=positive(p[2]);auto n=x.size();
    require(vol>=0&&n<=300,"Invalid lattice size or spacing");
    for(std::size_t t=0;t<n;++t){require(x[t][0]==double(t+1)&&x[t][2]>=0&&x[t][3]>=0,"Invalid cash flow schedule");}
    Vec straight(n+1,face),callable=straight;
    std::vector<Mat> level(n);
    for(std::size_t t=n;t-->0;){
        Vec a(t+1),b(t+1);
        for(std::size_t j=0;j<=t;++j){
            double rate=x[t][1]+vol*(2.*j-t),df=positive(std::exp(-rate*dt));
            a[j]=df*(x[t][2]+.5*(straight[j]+straight[j+1]));
            double continuation=df*(x[t][2]+.5*(callable[j]+callable[j+1]));
            double cap=t?x[t-1][3]:0;
            b[j]=cap>0?std::min(cap,continuation):continuation;
            level[t].push_back({double(t),double(j),rate,a[j],b[j],a[j]-b[j],double(cap>0&&cap<continuation)});
        }
        straight=a;callable=b;
    }
    Report o{{"Period","Node","ShortRate","StraightValue","CallableValue","CallOptionValue","ExerciseFlag"},{}};
    for(auto&l:level)for(auto&r:l)o.rows.push_back(r);
    return o;
}

inline Report structural_credit_fit(const Mat& x,const Vec& p){
    validate(x,1,5,5);parameters(p,3);
    double lo=positive(p[0]),hi=positive(p[1]),tol=positive(p[2]);require(hi>lo&&x.size()<=10000,"Invalid asset volatility bracket");
    Report o{{"AssetValue","AssetVol","EquityModel","EquityVolModel","DefaultProbability","RiskNeutralDistance","PriceResidual","VolResidual"},{}};
    for(auto&r:x){
        double E=positive(r[0]),ev=positive(r[1]),D=positive(r[2]),rate=r[3],T=positive(r[4]),pv=positive(D*std::exp(-rate*T));
        auto metrics=[&](double V,double vol){
            double z=vol*std::sqrt(T),d1=(std::log(V/D)+rate*T+.5*z*z)/z,d2=d1-z,price=V*normal(d1)-pv*normal(d2);
            return Vec{price,V*normal(d1)*vol/E,d2};
        };
        auto asset=[&](double vol){
            double a=E,b=E+pv;
            for(int i=0;i<100;++i){double m=a/2+b/2;if(metrics(m,vol)[0]<E)a=m;else b=m;}return a/2+b/2;
        };
        auto f=[&](double vol){return metrics(asset(vol),vol)[1]-ev;};
        double a=lo,b=hi,fa=f(a),fb=f(b);
        require(fa==0||fb==0||(fa<0&&fb>0)||(fa>0&&fb<0),"Asset volatility root not bracketed");
        if(fa==0)b=a;else if(fb==0)a=b;
        for(int i=0;i<100;++i){double m=a/2+b/2,g=f(m);if(g==0){a=b=m;break;}if((g>0)==(fa>0)){a=m;fa=g;}else b=m;}
        double vol=a/2+b/2,V=asset(vol);Vec z=metrics(V,vol);
        require(std::abs(z[0]-E)<=tol*std::max(1.,E)&&std::abs(z[1]-ev)<=tol*std::max(1.,ev),"Credit calibration residual");
        o.rows.push_back({V,vol,z[0],z[1],normal(-z[2]),z[2],z[0]-E,z[1]-ev});
    }
    return o;
}

inline Report kirk_spread(const Mat& x,const Vec& p){
    validate(x,1,8,8);parameters(p,1);double tol=positive(p[0]);
    Report o{{"ImpliedCorrelation","EffectiveVol","Call","Put","PriceResidual","ParityResidual"},{}};
    for(auto&r:x){
        double f=positive(r[0]),g=positive(r[1]),K=r[2],v=r[3],w=r[4],rate=r[5],T=positive(r[6]),market=r[7];
        require(K>=0&&v>=0&&w>=0&&market>=0,"Invalid Kirk inputs");double h=g+K,b=g/h,df=positive(std::exp(-rate*T));
        auto val=[&](double rho){
            double var=(v-b*w)*(v-b*w)+2*b*v*w*(1-rho),s=std::sqrt(std::max(0.,var)),c,put;
            if(s==0){c=df*std::max(f-h,0.);put=df*std::max(h-f,0.);}
            else{double z=s*std::sqrt(T),d1=std::log(f/h)/z+z/2,d2=d1-z;c=df*(f*normal(d1)-h*normal(d2));put=df*(h*normal(-d2)-f*normal(-d1));}
            return Vec{s,c,put};
        };
        double rho=0;
        if(v*w>0){
            double a=-1,z=1;require(market<=val(a)[1]+tol&&market>=val(z)[1]-tol,"Kirk target outside price range");
            for(int i=0;i<100;++i){double m=a/2+z/2;if(val(m)[1]>market)a=m;else z=m;}rho=a/2+z/2;
        }
        Vec z=val(rho);require(std::abs(z[1]-market)<=tol,"Kirk calibration residual");
        o.rows.push_back({rho,z[0],z[1],z[2],z[1]-market,z[1]-z[2]-df*(f-h)});
    }
    return o;
}

inline Report heston_full_truncation(const Mat& x,const Vec& p){
    validate(x,1,10,10);parameters(p,3);auto m=count(p[0],1,2520),n=count(p[1],2,2000000),seed=count(p[2],0,1000000000);
    require(double(m)*n*x.size()<=1e8,"Heston simulation work limit");
    struct Moment{double n=0,m=0,s=0;void push(double x){++n;double d=x-m;m+=d/n;s+=d*(x-m);}double se()const{return std::sqrt(std::max(0.,s)/(n-1)/n);}};
    Report o{{"Call","Put","CallSE","PutSE","DiscountedSpot","SpotSE","MartingaleResidual","NegativeVarianceFraction"},{}};
    for(std::size_t row=0;row<x.size();++row){
        auto&r=x[row];double S=positive(r[0]),K=positive(r[1]),rate=r[2],q=r[3],v0=r[4],k=r[5],theta=r[6],xi=r[7],rho=r[8],T=positive(r[9]);
        require(v0>=0&&k>=0&&theta>=0&&xi>=0&&std::abs(rho)<=1,"Invalid Heston parameters");
        std::mt19937_64 rng(std::uint64_t(seed)+row);std::normal_distribution<double> norm(0,1);
        double dt=T/m,disc=positive(std::exp(-rate*T)),neg=0,orth=std::sqrt(std::max(0.,1-rho*rho));Moment c,put,s;
        for(std::size_t i=0;i<n;++i){
            double a=std::log(S),v=v0;
            for(std::size_t j=0;j<m;++j){
                double z=norm(rng),u=norm(rng),vp=std::max(0.,v),root=std::sqrt(vp*dt);
                a+=(rate-q-.5*vp)*dt+root*(rho*z+orth*u);v+=k*(theta-vp)*dt+xi*root*z;
                neg+=double(v<0);
            }
            double st=positive(std::exp(a));c.push(disc*std::max(st-K,0.));put.push(disc*std::max(K-st,0.));s.push(disc*st);
        }
        o.rows.push_back({c.m,put.m,c.se(),put.se(),s.m,s.se(),s.m-S*std::exp(-q*T),neg/(double(n)*m)});
    }
    return o;
}
} }
