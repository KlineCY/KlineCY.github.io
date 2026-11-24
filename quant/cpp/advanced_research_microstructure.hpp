#pragma once
#include "advanced_research_common.hpp"
namespace kline { namespace advanced_research {
inline Report acd_duration_grid(const Mat& x,const Vec& p){
    validate(x,3,1,1);parameters(p,4);auto nt=count(p[0],2,x.size()-1),g=count(p[3],2,100);
    double am=p[1],bm=p[2];require(am>=0&&am<1&&bm>=0&&bm<1&&double(g)*g*nt<=5e7,"Invalid ACD grid");
    double mean=0;for(std::size_t i=0;i<x.size();++i){positive(x[i][0]);if(i<nt)mean+=x[i][0]/nt;}
    auto eval=[&](double a,double b,std::size_t n,Report* out,double training){
        double w=(1-a-b)*mean,psi=mean,ll=0;
        for(std::size_t t=0;t<n;++t){double d=x[t][0],z=d/positive(psi),v=std::log(psi)+z;ll+=v;if(out)out->rows.push_back({double(t+1),double(t<nt),d,psi,z,v,a,b,w,training});psi=w+a*d+b*psi;}
        return ll;
    };
    double best=std::numeric_limits<double>::infinity(),a=0,b=0;
    for(std::size_t i=0;i<g;++i)for(std::size_t j=0;j<g;++j){double u=am*i/(g-1),v=bm*j/(g-1);if(u+v>=1)continue;double f=eval(u,v,nt,nullptr,0);if(f<best){best=f;a=u;b=v;}}
    require(std::isfinite(best),"No finite ACD grid candidate");
    Report o{{"Time","Train","Duration","ExpectedDuration","StandardizedDuration","ConditionalNLL","Alpha","Beta","Omega","TrainingNLL"},{}};
    eval(a,b,x.size(),&o,best);return o;
}

inline Report flow_propagator_ridge(const Mat& x,const Vec& p){
    validate(x,4,2,2);parameters(p,3);auto nt=count(p[0],3,x.size()-1),L=count(p[1],1,std::min<std::size_t>(32,nt-1));double lam=positive(p[2]);
    require(double(nt)*L*L<=5e7,"Propagator work limit");
    auto z=[&](std::size_t t){Vec a;for(std::size_t j=1;j<=L;++j)a.push_back(x[t-j][0]);return a;};
    Mat X;Vec y;
    for(std::size_t t=L;t<nt;++t){X.push_back(z(t));y.push_back(x[t][1]);}
    Vec beta=fit_ridge(X,y,lam,false),cum=beta;for(std::size_t j=1;j<L;++j)cum[j]+=cum[j-1];
    Report o{{"Time","Actual","Forecast","Error"},{}};
    for(const auto&prefix:{"FlowLag","Beta","CumulativeKernel"})for(std::size_t j=0;j<L;++j)o.header.push_back(std::string(prefix)+std::to_string(j+1));
    for(std::size_t t=nt;t<x.size();++t){Vec a=z(t);double f=dot(a,beta);Vec r{double(t+1),x[t][1],f,x[t][1]-f};r.insert(r.end(),a.begin(),a.end());r.insert(r.end(),beta.begin(),beta.end());r.insert(r.end(),cum.begin(),cum.end());o.rows.push_back(r);}
    return o;
}

inline Report queue_fill_uniformization(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,5);auto q=count(p[0],1,500),M=count(p[3],q,500),K=count(p[4],0,10000);
    double birth=p[1],death=positive(p[2]),nu=birth+death;require(birth>=0&&double(x.size())*(K+1)*M<=5e7,"Invalid queue model controls");
    Report o{{"Time","InitialQueue","FillProbability","OutstandingProbability","TruncationTail","MassResidual"},{}};
    for(auto&r:x){double t=r[0],a=nu*t;require(t>=0&&a<=600,"Invalid horizon");Vec v(M+1,0);v[q]=1;double w=std::exp(-a),mass=w,filled=0,outstanding=w;
        for(std::size_t k=1;k<=K;++k){Vec z(M+1,0);z[0]=v[0];for(std::size_t j=1;j<=M;++j){double up=j<M?birth/nu:0,down=death/nu;z[j-1]+=down*v[j];z[j]+=(1-up-down)*v[j];if(j<M)z[j+1]+=up*v[j];}v=z;w*=a/k;mass+=w;filled+=w*v[0];outstanding+=w*(sum(v)-v[0]);}
        o.rows.push_back({t,double(q),filled,outstanding,std::max(0.,1-mass),std::abs(filled+outstanding-mass)});
    }return o;
}

inline Report bivariate_hawkes_likelihood(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,8);double T=positive(p[0]),beta=positive(p[1]);Vec mu{positive(p[2]),positive(p[3])};Mat A{{p[4],p[5]},{p[6],p[7]}};
    for(auto&r:A)for(double v:r)require(v>=0,"Nonnegative excitation required");
    double rho=.5*(A[0][0]+A[1][1]+std::sqrt(std::pow(A[0][0]-A[1][1],2)+4*A[0][1]*A[1][0]));
    require(rho<1,"Stationary Hawkes spectral radius required");
    Vec h(2,0),comp{mu[0]*T,mu[1]*T};double last=0,ll=0;Mat rows;
    for(std::size_t k=0;k<x.size();++k){double t=x[k][0];auto type=count(x[k][1],1,2)-1;require(t>last&&t<=T,"Strict event times in observation window required");double decay=std::exp(-beta*(t-last));for(double&v:h)v*=decay;
        Vec z=mul(A,h);for(std::size_t j=0;j<2;++j){z[j]+=mu[j];comp[j]+=A[j][type]*(-std::expm1(-beta*(T-t)));}
        double v=std::log(positive(z[type]));ll+=v;rows.push_back({double(k+1),t,double(type+1),z[0],z[1],v});h[type]+=beta;last=t;
    }
    ll-=sum(comp);Report o{{"Event","Time","Type","Intensity1","Intensity2","EventLogIntensity","Compensator1","Compensator2","LogLikelihood","SpectralRadius"},{}};
    for(auto&r:rows){r.insert(r.end(),{comp[0],comp[1],ll,rho});o.rows.push_back(r);}return o;
}

inline Report parzen_realized_kernel(const Mat& x,const Vec& p){
    validate(x,2,1,1);parameters(p,1);auto H=count(p[0],0,x.size()-1);require(double(H+1)*x.size()<=5e7,"Realized kernel work limit");
    double rv=0,adj=0;for(auto&r:x)rv+=r[0]*r[0];
    for(std::size_t h=1;h<=H;++h){double z=double(h)/(H+1),w=z<=.5?1-6*z*z+6*z*z*z:2*std::pow(1-z,3),g=0;for(std::size_t t=h;t<x.size();++t)g+=x[t][0]*x[t-h][0];adj+=2*w*g;}
    double rk=rv+adj;return scalar({"Bandwidth","RealizedVariance","RealizedKernel","AutocovarianceAdjustment","NegativeFlag"},{double(H),rv,rk,adj,double(rk<0)});
}

inline Report var_structural_irf(const Mat& x,const Vec& p){
    validate(x,2,4,4);require(x.size()==2,"Two-variable VAR required");parameters(p,1);auto H=count(p[0],0,10000);
    Mat A{{x[0][0],x[0][1]},{x[1][0],x[1][1]}},S{{x[0][2],x[0][3]},{x[1][2],x[1][3]}};solve_spd(S,Vec(2,0));
    double a=std::sqrt(positive(S[0][0])),b=S[1][0]/a,c=std::sqrt(positive(S[1][1]-b*b));Mat psi{{a,0},{b,c}},cum(2,Vec(2,0)),var(2,Vec(2,0));
    Report o{{"Horizon","Response","Shock","ImpulseResponse","CumulativeResponse","FEVD"},{}};
    for(std::size_t h=0;h<=H;++h){for(std::size_t i=0;i<2;++i)for(std::size_t j=0;j<2;++j){cum[i][j]+=psi[i][j];var[i][j]+=psi[i][j]*psi[i][j];}
        for(std::size_t i=0;i<2;++i)for(std::size_t j=0;j<2;++j)o.rows.push_back({double(h),double(i+1),double(j+1),psi[i][j],cum[i][j],var[i][j]/positive(sum(var[i]))});
        Mat next(2,Vec(2,0));for(std::size_t i=0;i<2;++i)for(std::size_t j=0;j<2;++j)for(std::size_t k=0;k<2;++k)next[i][j]+=A[i][k]*psi[k][j];psi=next;
    }return o;
}
} }
