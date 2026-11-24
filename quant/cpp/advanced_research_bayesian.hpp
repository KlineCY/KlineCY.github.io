#pragma once
#include "advanced_research_common.hpp"
namespace kline { namespace advanced_research {
inline Report bayesian_linear_nig(const Mat& x,const Vec& p){
    auto d=validate(x,2,3,34)-2;parameters(p,3);double lam=positive(p[0]),a=positive(p[1]),b=positive(p[2]);Mat X,H(d,Vec(d,0));Vec y,rhs(d,0);
    require(double(x.size())*d*d*d<=5e7,"Bayesian prediction work limit");
    for(auto&r:x){require(r[0]==0||r[0]==1,"Binary train flag");if(r[0]){X.emplace_back(r.begin()+1,r.end()-1);y.push_back(r.back());add_outer(H,X.back(),1);for(std::size_t j=0;j<d;++j)rhs[j]+=X.back()[j]*y.back();}}
    require(!X.empty(),"No training data");for(std::size_t j=0;j<d;++j)H[j][j]+=lam;
    Vec w=solve_spd(H,rhs);a+=.5*X.size();b+=.5*lam*squared(w);
    for(std::size_t i=0;i<X.size();++i)b+=.5*std::pow(y[i]-dot(X[i],w),2);
    Report o{{"Row","Train","Actual","PosteriorMean","PredictiveScale","PredictiveDF","Alpha","Beta"},{}};
    for(std::size_t i=0;i<x.size();++i){Vec z(x[i].begin()+1,x[i].end()-1);double v=b/a*(1+dot(z,solve_spd(H,z)));o.rows.push_back({double(i+1),x[i][0],x[i].back(),dot(z,w),std::sqrt(positive(v)),2*a,a,b});}return o;
}

inline Report gaussian_process_grid(const Mat& x,const Vec& p){
    auto d=validate(x,3,3,18)-2;parameters(p,6);double l0=positive(p[0]),l1=positive(p[1]),v0=positive(p[3]),v1=positive(p[4]);auto gl=count(p[2],2,50),gv=count(p[5],2,50);
    require(l1>=l0&&v1>=v0,"Invalid GP grid");Mat A,X;Vec y;
    for(auto&r:x){require(r[0]==0||r[0]==1,"Binary train flag");A.emplace_back(r.begin()+1,r.end()-1);if(r[0]){X.push_back(A.back());y.push_back(r.back());}}
    auto n=X.size();require(n>=2&&n<=64&&double(gl)*gv*n*n*n+double(x.size())*n*n*n<=1e8,"GP work limit");
    auto kernel=[&](const Vec&a,const Vec&b,double l){double z=0;for(std::size_t j=0;j<d;++j)z+=std::pow((a[j]-b[j])/l,2);return std::exp(-.5*z);};
    double best=std::numeric_limits<double>::infinity(),ell=l0,noise=v0;Vec alpha;Mat C;
    for(std::size_t i=0;i<gl;++i)for(std::size_t j=0;j<gv;++j){
        double l=std::exp(std::log(l0)+(std::log(l1)-std::log(l0))*i/(gl-1)),v=std::exp(std::log(v0)+(std::log(v1)-std::log(v0))*j/(gv-1));
        Mat s(n,Vec(n)),L(n,Vec(n,0));
        for(std::size_t a=0;a<n;++a)for(std::size_t b=0;b<n;++b)s[a][b]=kernel(X[a],X[b],l)+(a==b?v:0);
        double ld=0;for(std::size_t a=0;a<n;++a)for(std::size_t b=0;b<=a;++b){double z=s[a][b];for(std::size_t k=0;k<b;++k)z-=L[a][k]*L[b][k];L[a][b]=a==b?std::sqrt(positive(z)):z/L[b][b];if(a==b)ld+=2*std::log(L[a][b]);}
        Vec w=solve_spd(s,y);double f=.5*(dot(y,w)+ld+n*std::log(2*std::acos(-1.)));
        if(f<best){best=f;ell=l;noise=v;alpha=w;C=s;}
    }
    Report o{{"Row","Train","Actual","PosteriorMean","LatentVariance","ObservationVariance","LengthScale","NoiseVariance","TrainingNLL"},{}};
    for(std::size_t i=0;i<x.size();++i){Vec k(n);for(std::size_t j=0;j<n;++j)k[j]=kernel(A[i],X[j],ell);double v=1-dot(k,solve_spd(C,k));require(v>=-1e-10,"Negative GP variance");v=std::max(0.,v);o.rows.push_back({double(i+1),x[i][0],x[i].back(),dot(k,alpha),v,v+noise,ell,noise,best});}return o;
}

inline Report student_t_regression_em(const Mat& x,const Vec& p){
    auto d=validate(x,2,2,25)-1;parameters(p,5);double nu=positive(p[0]),sig=positive(p[1]),lam=positive(p[2]),tol=positive(p[3]),s2=sig*sig;auto K=count(p[4],1,100000);
    require(double(K)*x.size()*d*d<=5e7,"Student t work limit");Mat X;Vec y;for(auto&r:x){X.emplace_back(r.begin(),r.end()-1);y.push_back(r.back());}
    auto eval=[&](const Vec&w){double f=.5*lam*squared(w);Vec g=w;for(double&v:g)v*=lam;
        for(std::size_t i=0;i<x.size();++i){double e=y[i]-dot(X[i],w);f+=.5*(nu+1)*std::log1p(e*e/(nu*s2));for(std::size_t j=0;j<d;++j)g[j]-=(nu+1)*e*X[i][j]/(nu*s2+e*e);}
        return std::make_pair(f,inf_norm(g));};
    Vec w=fit_ridge(X,y,lam*s2,false);auto st=eval(w);std::size_t it=0;
    while(st.second>tol&&it<K){Mat H(d,Vec(d,0));Vec b(d,0);for(std::size_t i=0;i<x.size();++i){double e=(y[i]-dot(X[i],w))/sig,z=(nu+1)/(nu+e*e);add_outer(H,X[i],z);for(std::size_t j=0;j<d;++j)b[j]+=z*X[i][j]*y[i];}
        for(std::size_t j=0;j<d;++j)H[j][j]+=lam*s2;Vec a=solve_spd(H,b);auto z=eval(a);require(z.first<=st.first+1e-10*std::max(1.,st.first),"Student t objective increased");w=a;st=z;++it;}
    require(st.second<=tol,"Student t EM did not converge");
    Report o{{"Feature","Coefficient","PenalizedObjective","ScoreResidual","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),w[j],st.first,st.second,double(it)});return o;
}

inline Report hmm_transition_em(const Mat& x,const Vec& p){
    validate(x,2,1,1);parameters(p,6);double init=p[0],e0=p[1],e1=p[2],lam=positive(p[3]),tol=positive(p[4]);auto K=count(p[5],1,100000),n=x.size();
    require(init>0&&init<1&&e0>0&&e0<1&&e1>0&&e1<1&&e0!=e1&&double(K)*n<=5e7,"Invalid HMM fitting controls");
    for(auto&r:x)require(r[0]==0||r[0]==1,"Binary observations required");
    auto em=[&](std::size_t t,std::size_t j){double e=j?e1:e0;return x[t][0]?e:1-e;};
    struct State{Mat N;double ll,obj,gap;};
    auto eval=[&](double a,double b){
        Mat P{{1-a,a},{b,1-b}},f(n,Vec(2)),back(n,Vec(2,1)),N(2,Vec(2,0));Vec c(n);double ll=0;
        for(std::size_t t=0;t<n;++t){for(std::size_t j=0;j<2;++j)f[t][j]=em(t,j)*(t?f[t-1][0]*P[0][j]+f[t-1][1]*P[1][j]:j?init:1-init);c[t]=positive(sum(f[t]));for(double&v:f[t])v/=c[t];ll+=std::log(c[t]);}
        for(std::size_t t=n-1;t-->0;)for(std::size_t i=0;i<2;++i){back[t][i]=0;for(std::size_t j=0;j<2;++j){double v=P[i][j]*em(t+1,j)*back[t+1][j]/c[t+1];back[t][i]+=v;N[i][j]+=f[t][i]*v;}}
        double obj=ll+lam*(std::log(a)+std::log1p(-a)+std::log(b)+std::log1p(-b));
        double g=std::max(std::abs((N[0][1]+lam)/a-(N[0][0]+lam)/(1-a)),std::abs((N[1][0]+lam)/b-(N[1][1]+lam)/(1-b)));
        return State{N,ll,obj,g};
    };
    double a=.5,b=.5;auto st=eval(a,b);std::size_t it=0;
    while(st.gap>tol&&it<K){double u=(st.N[0][1]+lam)/(sum(st.N[0])+2*lam),v=(st.N[1][0]+lam)/(sum(st.N[1])+2*lam);auto z=eval(u,v);require(z.obj>=st.obj-1e-10*std::max(1.,std::abs(st.obj)),"HMM MAP objective decreased");a=u;b=v;st=z;++it;}
    require(st.gap<=tol,"HMM EM did not converge");return scalar({"Transition01","Transition10","LogLikelihood","PenalizedObjective","ScoreResidual","Iterations"},{a,b,st.ll,st.obj,st.gap,double(it)});
}

inline Report bayesian_online_change(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,4);double h=p[0],m0=p[1],v0=positive(p[2]),s=positive(p[3]);auto n=x.size();
    require(h>0&&h<1&&double(n)*n<=1e7,"Invalid online change controls");Vec pref(n+1,0),logp;Report o{{"Time","Observation","ChangeProbability","ExpectedRunLength","MAPRunLength","LogPredictiveProbability","MassResidual"},{}};
    auto density=[&](double y,double m,double v){return -.5*(std::log(2*std::acos(-1.)*v)+(y-m)*(y-m)/v);};
    for(std::size_t t=0;t<n;++t){double y=x[t][0];pref[t+1]=pref[t]+y;Vec next(t+1);
        next[0]=density(y,m0,s+v0)+(t?std::log(h):0);
        for(std::size_t r=1;r<=t;++r){double v=1/(1/v0+r/s),m=v*(m0/v0+(pref[t]-pref[t-r])/s);next[r]=logp[r-1]+std::log1p(-h)+density(y,m,s+v);}
        double z=*std::max_element(next.begin(),next.end()),den=0;for(double v:next)den+=std::exp(v-z);double ll=z+std::log(positive(den)),mass=0,mean=0;std::size_t best=0;
        for(std::size_t r=0;r<next.size();++r){next[r]-=ll;double p=std::exp(next[r]);mass+=p;mean+=(r+1)*p;if(next[r]>next[best])best=r;}
        o.rows.push_back({double(t+1),y,std::exp(next[0]),mean,double(best+1),ll,std::abs(mass-1)});logp=next;
    }return o;
}

inline Report dynamic_regression_kalman(const Mat& x,const Vec& p){
    auto d=validate(x,1,2,33)-1;parameters(p,3);double q=p[0],R=positive(p[1]),p0=positive(p[2]);require(q>=0&&double(x.size())*d*d<=5e7,"Invalid dynamic regression controls");
    Vec w(d,0);Mat P=identity(d);for(std::size_t j=0;j<d;++j)P[j][j]=p0;
    Report o{{"Time","Actual","Prediction","Innovation","InnovationVariance","FilteredPrediction","PosteriorTrace"},{}};
    for(std::size_t j=0;j<d;++j)o.header.push_back("Beta"+std::to_string(j+1));
    for(std::size_t t=0;t<x.size();++t){Vec a(x[t].begin(),x[t].end()-1);for(std::size_t j=0;j<d;++j)P[j][j]+=q;Vec v=mul(P,a),K=v;double S=positive(dot(a,v)+R),pred=dot(a,w),e=x[t].back()-pred;
        for(std::size_t j=0;j<d;++j){K[j]/=S;w[j]+=K[j]*e;}
        double trace=0;for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<d;++j){P[i][j]=P[i][j]-K[i]*v[j]-v[i]*K[j]+S*K[i]*K[j];if(i==j){positive(P[i][j]);trace+=P[i][j];}}
        Vec r{double(t+1),x[t].back(),pred,e,S,dot(a,w),trace};r.insert(r.end(),w.begin(),w.end());o.rows.push_back(r);
    }return o;
}
} }
