#pragma once
#include "comprehensive_common.hpp"
#include <complex>
namespace kline { namespace comprehensive {
inline Report svi_profile_calibration(const Mat& x,const Vec& p){
    validate(x,3,2,2);parameters(p,9);
    auto R=count(p[2],2,1000),S=count(p[5],2,1000),M=count(p[8],2,1000);
    require(p[0]>-1&&p[1]<1&&p[0]<=p[1]&&p[3]>0&&p[4]>=p[3]&&p[7]>=p[6]&&double(R)*S*M*x.size()<=5e7,"Invalid SVI grid");
    double ym=0;for(auto&r:x)ym+=positive(r[1])/x.size();
    double best=std::numeric_limits<double>::infinity(),a=0,b=0,rho=0,sigma=0,m=0;
    for(std::size_t i=0;i<R;++i)for(std::size_t j=0;j<S;++j)for(std::size_t k=0;k<M;++k){
        double r=p[0]+(p[1]-p[0])*i/(R-1),s=p[3]+(p[4]-p[3])*j/(S-1),z=p[6]+(p[7]-p[6])*k/(M-1);
        Vec u;double um=0;
        for(auto&v:x){double d=v[0]-z;u.push_back(r*d+std::hypot(d,s));um+=u.back()/x.size();}
        double uu=0,uy=0;
        for(std::size_t l=0;l<x.size();++l){uu+=std::pow(u[l]-um,2);uy+=(u[l]-um)*(x[l][1]-ym);}
        if(uu<=0)continue;
        double B=std::max(0.,uy/uu),A=ym-B*um;
        if(!(A+B*s*std::sqrt(1-r*r)>0&&B*(1-r)<2&&B*(1+r)<2))continue;
        double f=0;for(std::size_t l=0;l<x.size();++l)f+=std::pow(x[l][1]-A-B*u[l],2);
        if(f<best){best=f;a=A;b=B;rho=r;sigma=s;m=z;}
    }
    require(std::isfinite(best),"No admissible SVI candidate");
    Report out{{"LogMoneyness","MarketVariance","FittedVariance","Residual","A","B","Rho","M","Sigma","DensityFactor","WingLeft","WingRight","TrainingSSE"},{}};
    for(auto&r:x){
        double k=r[0],d=k-m,h=std::hypot(d,sigma),w=positive(a+b*(rho*d+h)),v=b*(rho+d/h),curv=b*sigma*sigma/(h*h*h);
        double g=std::pow(1-k*v/(2*w),2)-v*v/4*(1/w+.25)+curv/2;
        out.rows.push_back({k,r[1],w,r[1]-w,a,b,rho,m,sigma,g,b*(1-rho),b*(1+rho),best});
    }
    return out;
}

inline Report call_curve_convex_projection(const Mat& x,const Vec& p){
    validate(x,2,3,3);parameters(p,3);auto n=x.size();
    double F=positive(p[0]),tol=positive(p[1]);auto cap=count(p[2],1,100000);
    require(n<=64&&double(cap)*n*n<=1e8,"Call projection work limit");
    Vec z(n),w(n),K(n),lo(n);
    for(std::size_t i=0;i<n;++i){
        K[i]=positive(x[i][0]);if(i)require(K[i]>K[i-1],"Increasing strikes required");
        w[i]=std::sqrt(positive(x[i][2]));z[i]=w[i]*x[i][1];lo[i]=std::max(0.,F-K[i]);
    }
    Mat A;Vec b;
    auto constraint=[&](Vec a,double c){
        for(std::size_t j=0;j<n;++j)a[j]/=w[j];
        double h=positive(std::sqrt(squared(a)));for(double&v:a)v/=h;
        A.push_back(a);b.push_back(c/h);
    };
    for(std::size_t i=0;i<n;++i){
        Vec a(n,0);a[i]=1;constraint(a,F);a[i]=-1;constraint(a,-lo[i]);
        if(i+1<n){
            a.assign(n,0);a[i]=-1;a[i+1]=1;constraint(a,0);
            for(double&v:a)v=-v;constraint(a,K[i+1]-K[i]);
        }
        if(i+2<n){
            double h=K[i+1]-K[i],k=K[i+2]-K[i+1];
            a.assign(n,0);a[i]=-1/h;a[i+1]=1/h+1/k;a[i+2]=-1/k;constraint(a,0);
        }
    }
    Vec u=z,lambda(A.size(),0);double violation=0,obj=0,gap=0;std::size_t it=0;
    for(;it<cap;){
        for(std::size_t j=0;j<A.size();++j){
            double next=std::max(0.,dot(A[j],u)+lambda[j]-b[j]),d=lambda[j]-next;
            for(std::size_t k=0;k<n;++k)u[k]+=d*A[j][k];lambda[j]=next;
        }
        Vec v(n),dual(n,0);double lin=0;violation=0;
        for(std::size_t j=0;j<A.size();++j){
            violation=std::max(violation,std::max(0.,dot(A[j],u)-b[j]));
            lin+=lambda[j]*(dot(A[j],z)-b[j]);
            for(std::size_t k=0;k<n;++k)dual[k]+=lambda[j]*A[j][k];
        }
        for(std::size_t k=0;k<n;++k)v[k]=u[k]-z[k];
        obj=.5*squared(v);gap=obj-lin+.5*squared(dual);++it;
        if(violation<=tol&&std::abs(gap)<=tol*std::max(1.,obj))break;
    }
    require(violation<=tol&&std::abs(gap)<=tol*std::max(1.,obj),"Call projection did not converge");
    Report out{{"Strike","RawCall","ProjectedCall","LowerBound","UpperBound","MaxViolation","Objective","DualGap","Iterations"},{}};
    for(std::size_t i=0;i<n;++i)out.rows.push_back({K[i],x[i][1],u[i]/w[i],lo[i],F,violation,obj,gap,double(it)});
    return out;
}

inline Report ho_lee_curve_tree(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,3);auto n=x.size();
    double dt=positive(p[0]),sig=p[1],strike=p[2];
    require(sig>=0&&n<=1000,"Invalid rate tree controls");
    Vec q{1};Report out{{"Step","Node","Shift","ShortRate","StatePrice","TargetDiscount","ModelDiscount","CurveResidual","CapletContribution"},{}};
    for(std::size_t t=0;t<n;++t){
        double D=positive(x[t][0]),Z=0;Vec jump(t+1);
        for(std::size_t j=0;j<=t;++j){jump[j]=(2.*j-t)*sig*std::sqrt(dt);Z+=q[j]*std::exp(-jump[j]*dt);}
        double alpha=(std::log(positive(Z))-std::log(D))/dt;Vec next(t+2,0);double model=0;
        for(std::size_t j=0;j<=t;++j){
            double rate=alpha+jump[j],df=std::exp(-rate*dt),state=q[j]*df;
            next[j]+=.5*state;next[j+1]+=.5*state;model+=state;
            double value=q[j]*std::max(0.,1-(1+strike*dt)*df);
            out.rows.push_back({double(t),double(j),alpha,rate,q[j],D,0,0,value});
        }
        for(std::size_t j=out.rows.size()-(t+1);j<out.rows.size();++j){out.rows[j][6]=model;out.rows[j][7]=std::abs(model-D);}
        q=next;
    }
    return out;
}

inline Report multicurve_forward_bootstrap(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,0);
    double prev=0,A=0,B=0;Report out{{"Maturity","Accrual","Discount","ParSwap","Forward","Annuity","FloatingPV","RepricedSwap","Residual"},{}};
    for(auto&r:x){
        require(r[0]>prev,"Increasing maturity required");prev=r[0];
        double a=positive(r[1]),D=positive(r[2]),K=r[3],w=a*D;
        A+=w;double F=(K*A-B)/w;B+=w*F;
        double model=B/A;out.rows.push_back({r[0],a,D,K,F,A,B,model,std::abs(model-K)});
    }
    return out;
}

inline Report variance_strip_quadrature(const Mat& x,const Vec& p){
    validate(x,3,3,3);parameters(p,3);double F=positive(p[0]),T=positive(p[1]),tol=positive(p[2]);
    Vec K,f;bool center=false;double err=0;
    for(auto&r:x){
        double k=positive(r[0]),c=r[1],put=r[2];
        require((K.empty()||k>K.back())&&c>=0&&put>=0,"Invalid option strip");
        double e=std::abs(c-put-F+k);err=std::max(err,e);
        require(e<=tol*std::max(1.,F),"Forward parity violated");
        center|=k==F;K.push_back(k);f.push_back((k<F?put:k>F?c:.5*(c+put))/(k*k));
    }
    require(center&&K.front()<F&&K.back()>F,"Strip must bracket and include forward");
    double s=0;Report out{{"LowerStrike","UpperStrike","LeftIntegrand","RightIntegrand","VarianceContribution","CumulativeVariance","ParityResidual"},{}};
    for(std::size_t i=1;i<K.size();++i){
        double v=(K[i]-K[i-1])*(f[i]+f[i-1])/T;s+=v;
        out.rows.push_back({K[i-1],K[i],f[i-1],f[i],v,s,err});
    }
    return out;
}

inline Report quanto_lognormal_option(const Mat& x,const Vec& p){
    validate(x,1,10,10);parameters(p,0);
    Report out{{"Call","Put","QuantoForward","AdjustedDrift","DiscountedUnderlying","ParityResidual"},{}};
    for(auto&r:x){
        double S=positive(r[0]),K=positive(r[1]),rd=r[2],rf=r[3],q=r[4],s=r[5],v=r[6],rho=r[7],T=positive(r[8]),fx=positive(r[9]);
        require(s>=0&&v>=0&&std::abs(rho)<=1,"Invalid quanto volatility or correlation");
        double drift=rf-q-rho*s*v,F=positive(S*std::exp(drift*T)),df=positive(fx*std::exp(-rd*T)),c,put;
        if(s==0){c=df*std::max(F-K,0.);put=df*std::max(K-F,0.);}
        else{
            double z=s*std::sqrt(T),d1=std::log(F/K)/z+z/2,d2=d1-z;
            c=df*(F*normal(d1)-K*normal(d2));put=df*(K*normal(-d2)-F*normal(-d1));
        }
        out.rows.push_back({c,put,F,drift,df*F,c-put-df*(F-K)});
    }
    return out;
}

inline Report heston_fourier_inversion(const Mat& x,const Vec& p){
    validate(x,1,10,10);parameters(p,2);double upper=positive(p[0]);auto n=count(p[1],100,200000);
    require(double(n)*x.size()<=5e6,"Fourier work limit");
    using C=std::complex<double>;const C I(0,1);const double pi=std::acos(-1.);
    Report out{{"Call","Put","P1","P2","Forward","MartingaleResidual","IntegrationDifference","ParityResidual"},{}};
    for(auto&r:x){
        double S=positive(r[0]),K=positive(r[1]),rate=r[2],q=r[3],T=positive(r[4]),kap=positive(r[5]),theta=positive(r[6]),xi=r[7],rho=r[8],v=positive(r[9]);
        require(xi>=0&&std::abs(rho)<1&&kap>rho*xi,"Invalid Fourier parameter domain");
        double F=positive(S*std::exp((rate-q)*T)),df=std::exp(-rate*T),k=std::log(K/F),P1=0,P2=0,err=0,mart=0;
        if(xi==0){
            double variance=theta*T+(v-theta)*(-std::expm1(-kap*T))/kap,z=std::sqrt(positive(variance));
            P1=normal(-k/z+z/2);P2=normal(-k/z-z/2);
        }else{
            auto phi=[&](C u){
                C b=kap-rho*xi*I*u,d=std::sqrt(b*b+xi*xi*(u*u+I*u));
                if(d.real()<0)d=-d;
                C g=(b-d)/(b+d),e=std::exp(-d*T);
                C D=(b-d)/(xi*xi)*(1.-e)/(1.-g*e);
                C a=kap*theta/(xi*xi)*((b-d)*T-2.*std::log((1.-g*e)/(1.-g)));
                return std::exp(a+D*v);
            };
            mart=std::abs(phi(-I)-C(1));
            auto integrate=[&](std::size_t m){
                double h=upper/m,a=0,b=0;
                for(std::size_t j=0;j<m;++j){
                    double u=(j+.5)*h;C w=std::exp(-I*u*k)/(I*u);
                    a+=(w*phi(C(u,-1))).real();b+=(w*phi(C(u,0))).real();
                }
                return Vec{.5+h*a/pi,.5+h*b/pi};
            };
            Vec a=integrate(n),b=integrate(n/2);
            P1=a[0];P2=a[1];err=std::abs(df*(F*(a[0]-b[0])-K*(a[1]-b[1])));
        }
        double call=df*(F*P1-K*P2),put=df*(K*(1-P2)-F*(1-P1));
        out.rows.push_back({call,put,P1,P2,F,mart,err,call-put-df*(F-K)});
    }
    return out;
}

inline Report cir_zero_curve_grid(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,10);double r0=p[0];
    auto G=count(p[3],2,1000),H=count(p[6],2,1000),J=count(p[9],2,1000);
    require(r0>=0&&p[1]>0&&p[2]>=p[1]&&p[4]>0&&p[5]>=p[4]&&p[7]>=0&&p[8]>=p[7]&&double(G)*H*J*x.size()<=5e7,"Invalid CIR grid");
    double prev=0;for(auto&r:x){require(r[0]>prev,"Increasing maturity required");prev=r[0];positive(r[1]);}
    auto logprice=[&](double T,double k,double theta,double s){
        if(s<1e-7)return -theta*T-(r0-theta)*(-std::expm1(-k*T))/k;
        double g=std::hypot(k,std::sqrt(2.)*s),e=std::exp(-g*T);
        double d=positive(k+g+(g-k)*e),B=-2*std::expm1(-g*T)/d;
        double A=2*k*theta/(s*s)*(std::log(2*g/d)+(k-g)*T/2);
        return A-B*r0;
    };
    double best=std::numeric_limits<double>::infinity(),k=0,theta=0,sig=0;
    for(std::size_t i=0;i<G;++i)for(std::size_t j=0;j<H;++j)for(std::size_t h=0;h<J;++h){
        double a=p[1]+(p[2]-p[1])*i/(G-1),b=p[4]+(p[5]-p[4])*j/(H-1),c=p[7]+(p[8]-p[7])*h/(J-1),f=0;
        for(auto&r:x)f+=std::pow(logprice(r[0],a,b,c)-std::log(r[1]),2);
        if(f<best){best=f;k=a;theta=b;sig=c;}
    }
    require(std::isfinite(best),"No finite CIR candidate");
    Report out{{"Maturity","MarketDiscount","ModelDiscount","LogResidual","Kappa","Theta","Sigma","LogSSE"},{}};
    for(auto&r:x){double v=logprice(r[0],k,theta,sig);out.rows.push_back({r[0],r[1],std::exp(v),v-std::log(r[1]),k,theta,sig,best});}
    return out;
}
} }
