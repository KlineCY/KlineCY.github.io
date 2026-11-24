#pragma once
#include "model_research_common.hpp"
namespace kline { namespace model_research {
struct NewtonState {
    double value;
    Vec gradient;
    Mat hessian;
};
struct NewtonFit {
    Vec beta;
    NewtonState state;
    std::size_t iterations;
};
template<class Evaluate>
inline NewtonFit damped_newton(std::size_t d,double tol,std::size_t max_iter,Evaluate evaluate){
    Vec w(d,0);
    auto state=evaluate(w);
    std::size_t it=0;
    while(inf_norm(state.gradient)>tol&&it<max_iter){
        auto direction=solve_spd(state.hessian,state.gradient);
        double dec=positive(dot(direction,state.gradient));
        bool accepted=false;
        for(int k=0;k<45;++k){
            double step=std::ldexp(1.,-k);
            Vec v=w;
            for(std::size_t j=0;j<d;++j)v[j]-=step*direction[j];
            auto s=evaluate(v);
            if(std::isfinite(s.value)&&s.value<=state.value-1e-4*step*dec+1e-15*std::max(1.,std::abs(state.value))){
                w=v;state=std::move(s);accepted=true;break;
            }
        }
        require(accepted,"Newton line search failed");
        ++it;
    }
    require(inf_norm(state.gradient)<=tol,"Newton did not converge");
    return {w,state,it};
}
inline NewtonState regularized_state(const Vec& w,double lam){
    NewtonState s{.5*lam*squared(w),Vec(w.size()),Mat(w.size(),Vec(w.size(),0))};
    for(std::size_t j=0;j<w.size();++j){s.gradient[j]=lam*w[j];s.hessian[j][j]=lam;}
    return s;
}
inline double logadd(double a,double b){
    double m=std::max(a,b);
    return m+std::log1p(std::exp(std::min(a,b)-m));
}
inline double left_mills(double a){
    if(a>=-30)return std::exp(-.5*a*a)/std::sqrt(2*std::acos(-1.))/positive(normal(a));
    double z=-a,v=0;
    for(int j=60;j>=1;--j)v=j/(z+v);
    return z+v;
}
inline double log_normal_cdf(double a){
    if(a>=-30)return std::log(positive(normal(a)));
    return -.5*a*a-.5*std::log(2*std::acos(-1.))-std::log(left_mills(a));
}

inline Report nb2_exposure(const Mat& x,const Vec& p){
    auto d=validate(x,1,3,18)-2;
    parameters(p,4);
    double alpha=positive(p[0]),lam=p[1],tol=positive(p[2]),k=1/alpha;
    auto K=count(p[3],1,2000);
    require(lam>=0&&double(K)*x.size()*d*d<=5e7,"Invalid penalty or work limit");
    Mat a;Vec offset,y;
    for(auto&r:x){
        a.emplace_back(r.begin(),r.begin()+d);
        offset.push_back(std::log(positive(r[d]))+std::log(alpha));
        y.push_back(double(count(r[d+1],0,10000000)));
    }
    auto evaluate=[&](const Vec&w){
        auto s=regularized_state(w,lam);
        for(std::size_t i=0;i<a.size();++i){
            double z=dot(a[i],w)+offset[i],q=logistic(z),v=y[i];
            s.value+=std::lgamma(k)+std::lgamma(v+1)-std::lgamma(v+k)+k*softplus(z)+v*softplus(-z);
            double g=(v+k)*q-v;
            for(std::size_t j=0;j<d;++j)s.gradient[j]+=g*a[i][j];
            add_outer(s.hessian,a[i],(v+k)*q*(1-q));
        }
        return s;
    };
    auto f=damped_newton(d,tol,K,evaluate);
    Report o{{"Feature","Coefficient","PenalizedNLL","ScoreResidual","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),f.beta[j],f.state.value,inf_norm(f.state.gradient),double(f.iterations)});
    return o;
}

inline Report zip_em(const Mat& x,const Vec& p){
    validate(x,2,1,1);parameters(p,2);
    double tol=positive(p[0]);auto K=count(p[1],1,100000);
    double n=x.size(),zero=0,s=0,fac=0;
    for(auto&r:x){double y=count(r[0],0,10000000);zero+=double(y==0);s+=y;fac+=std::lgamma(y+1);}
    double pos=n-zero;
    require(zero>0&&pos>0&&zero/n>std::exp(-s/n)&&s/pos>1,"Interior ZIP solution required");
    double lam=s/pos,pi=.5*zero/n;
    auto likelihood=[&](double a,double b){return zero*logadd(std::log(b),std::log1p(-b)-a)+pos*(std::log1p(-b)-a)+s*std::log(a)-fac;};
    double ll=likelihood(lam,pi),gap=0;std::size_t it=0;
    for(;it<K;){
        double h=logistic(std::log(pi)-std::log1p(-pi)+lam);
        pi=zero*h/n;lam=s/positive(n-zero*h);
        double next=likelihood(lam,pi);
        require(next>=ll-1e-10*std::max(1.,std::abs(ll)),"EM likelihood decreased");
        ll=next;
        double e=std::exp(-lam),pzero=pi+(1-pi)*e;
        gap=std::max(std::abs(s/lam-pos-zero*(1-pi)*e/pzero),std::abs(zero*(1-e)/pzero-pos/(1-pi)));
        ++it;if(gap<=tol)break;
    }
    require(gap<=tol,"ZIP EM did not converge");
    return scalar({"Rate","StructuralZeroProbability","LogLikelihood","ScoreResidual","Iterations","ModelZeroProbability"},{lam,pi,ll,gap,double(it),pi+(1-pi)*std::exp(-lam)});
}

inline Report cox_breslow_ridge(const Mat& x,const Vec& p){
    auto d=validate(x,2,3,18)-2;parameters(p,3);
    double lam=positive(p[0]),tol=positive(p[1]);auto K=count(p[2],1,2000);
    require(double(K)*x.size()*d*d<=5e7,"Cox work limit");
    Mat rows=x;std::sort(rows.begin(),rows.end(),[](const Vec&a,const Vec&b){return a[0]>b[0];});
    double events=0;
    Mat a;for(auto&r:rows){positive(r[0]);require(r[1]==0||r[1]==1,"Binary event indicator");events+=r[1];a.emplace_back(r.begin()+2,r.end());}
    positive(events);
    auto evaluate=[&](const Vec&w){
        auto s=regularized_state(w,lam);
        double shift=-std::numeric_limits<double>::infinity(),total=0;
        Vec sx(d,0);Mat sxx(d,Vec(d,0));
        for(std::size_t i=0;i<rows.size();){
            std::size_t j=i;double deaths=0,ev=0;Vec ex(d,0);
            while(j<rows.size()&&rows[j][0]==rows[i][0]){
                double z=dot(a[j],w);
                if(z>shift){double f=std::exp(shift-z);total*=f;for(double&v:sx)v*=f;for(auto&r:sxx)for(double&v:r)v*=f;shift=z;}
                double v=std::exp(z-shift);total+=v;
                for(std::size_t k=0;k<d;++k)sx[k]+=v*a[j][k];
                add_outer(sxx,a[j],v);
                if(rows[j][1]){++deaths;ev+=z;for(std::size_t k=0;k<d;++k)ex[k]+=a[j][k];}
                ++j;
            }
            if(deaths){
                s.value+=deaths*(shift+std::log(total))-ev;
                for(std::size_t k=0;k<d;++k){
                    s.gradient[k]+=deaths*sx[k]/total-ex[k];
                    for(std::size_t h=0;h<d;++h)s.hessian[k][h]+=deaths*(sxx[k][h]/total-(sx[k]/total)*(sx[h]/total));
                }
            }
            i=j;
        }
        return s;
    };
    auto f=damped_newton(d,tol,K,evaluate);
    Report o{{"Feature","Coefficient","HazardRatio","PenalizedNLL","ScoreResidual","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),f.beta[j],std::exp(f.beta[j]),f.state.value,inf_norm(f.state.gradient),double(f.iterations)});
    return o;
}

inline Report tobit_known_scale(const Mat& x,const Vec& p){
    auto d=validate(x,2,4,19)-3;parameters(p,4);
    double sig=positive(p[0]),lam=p[1],tol=positive(p[2]),v=sig*sig;
    auto K=count(p[3],1,100000);
    require(lam>=0&&double(K)*(x.size()*d+d*d*d)<=5e7,"Invalid penalty or work limit");
    Mat a,H(d,Vec(d,0));double observed=0;
    for(auto&r:x){
        require(r[d+2]==0||r[d+2]==1,"Binary censor flag");
        require(r[d+2]?r[d+1]==r[d]:r[d+1]>r[d],"Inconsistent censored observation");
        observed+=1-r[d+2];a.emplace_back(r.begin(),r.begin()+d);add_outer(H,a.back(),1);
    }
    positive(observed);for(std::size_t j=0;j<d;++j)H[j][j]+=lam*v;
    auto eval=[&](const Vec&w){
        double nll=.5*lam*squared(w);Vec g=w;for(double&z:g)z*=lam;
        for(std::size_t i=0;i<x.size();++i){
            double mu=dot(a[i],w),h;
            if(x[i][d+2]){double z=(x[i][d]-mu)/sig;nll-=log_normal_cdf(z);h=left_mills(z)/sig;}
            else{double e=(mu-x[i][d+1])/sig;nll+=std::log(sig)+.5*std::log(2*std::acos(-1.))+.5*e*e;h=e/sig;}
            for(std::size_t j=0;j<d;++j)g[j]+=h*a[i][j];
        }
        return std::make_pair(nll,inf_norm(g));
    };
    Vec w(d,0);auto st=eval(w);std::size_t it=0;
    while(st.second>tol&&it<K){
        Vec b(d,0);
        for(std::size_t i=0;i<x.size();++i){
            double mu=dot(a[i],w),y=x[i][d+1];
            if(x[i][d+2])y=mu-sig*left_mills((x[i][d]-mu)/sig);
            for(std::size_t j=0;j<d;++j)b[j]+=a[i][j]*y;
        }
        auto z=solve_spd(H,b);auto next=eval(z);
        require(next.first<=st.first+1e-10*std::max(1.,std::abs(st.first)),"Tobit EM likelihood decreased");
        w=z;st=next;++it;
    }
    require(st.second<=tol,"Tobit EM did not converge");
    Report o{{"Feature","Coefficient","PenalizedNLL","ScoreResidual","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),w[j],st.first,st.second,double(it)});
    return o;
}

inline Report shrinkage_lda(const Mat& x,const Vec& p){
    auto d=validate(x,4,3,18)-2;parameters(p,2);
    double a=p[0],ridge=positive(p[1]);require(a>=0&&a<=1,"Invalid shrinkage");
    Mat mu(2,Vec(d,0));Vec n(2,0);
    for(auto&r:x){require((r[0]==0||r[0]==1)&&(r[1]==0||r[1]==1),"Binary flags required");if(r[0]){auto y=std::size_t(r[1]);++n[y];for(std::size_t j=0;j<d;++j)mu[y][j]+=r[j+2];}}
    require(n[0]>0&&n[1]>0&&sum(n)>2,"Both classes and three training observations required");
    for(std::size_t k=0;k<2;++k)for(double&z:mu[k])z/=n[k];
    Mat S(d,Vec(d,0));double nt=sum(n);
    for(auto&r:x)if(r[0]){Vec z(d);for(std::size_t j=0;j<d;++j)z[j]=r[j+2]-mu[std::size_t(r[1])][j];add_outer(S,z,1/(nt-2));}
    double trace=0;for(std::size_t j=0;j<d;++j)trace+=S[j][j]/d;
    for(std::size_t j=0;j<d;++j)for(std::size_t k=0;k<d;++k)S[j][k]=(1-a)*S[j][k]+(j==k?a*trace+ridge:0);
    Vec diff(d),center(d);for(std::size_t j=0;j<d;++j){diff[j]=mu[1][j]-mu[0][j];center[j]=(mu[1][j]+mu[0][j])/2;}
    Vec w=solve_spd(S,diff);double b=std::log(n[1]/n[0])-dot(w,center);
    Report o{{"Row","Train","Label","Probability","PredictedLabel","LogOdds","TrainingPrior"},{}};
    for(std::size_t i=0;i<x.size();++i){double z=b;for(std::size_t j=0;j<d;++j)z+=x[i][j+2]*w[j];o.rows.push_back({double(i+1),x[i][0],x[i][1],logistic(z),double(z>=0),z,n[1]/nt});}
    return o;
}

inline Report kernel_logistic_fit(const Mat& x,const Vec& p){
    auto d=validate(x,3,3,18)-2;parameters(p,5);
    double gam=positive(p[0]),lam=positive(p[1]),ridge=positive(p[2]),tol=positive(p[3]);auto Kmax=count(p[4],1,2000);
    Mat a;Vec y;std::vector<std::size_t> train;
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];require((r[0]==0||r[0]==1)&&(r[1]==0||r[1]==1),"Binary flags required");a.emplace_back(r.begin()+2,r.end());if(r[0]){train.push_back(i);y.push_back(r[1]);}}
    auto n=train.size();require(n>=2&&n<=64&&sum(y)>0&&sum(y)<n,"Both classes and at most 64 training centers required");
    require(double(Kmax)*n*n*n<=5e7&&double(x.size())*n*d<=5e7,"Kernel work limit");
    auto kernel=[&](const Vec& u,const Vec& v){double s=0;for(std::size_t j=0;j<d;++j){double z=u[j]-v[j];s+=z*z;}return std::exp(-gam*s);};
    Mat K(n,Vec(n));for(std::size_t i=0;i<n;++i)for(std::size_t j=0;j<n;++j)K[i][j]=kernel(a[train[i]],a[train[j]]);
    auto evaluate=[&](const Vec&w){
        auto s=regularized_state(w,ridge);Vec z=mul(K,w);s.value+=.5*lam*dot(w,z);
        for(std::size_t i=0;i<n;++i){
            s.gradient[i]+=lam*z[i];
            for(std::size_t j=0;j<n;++j)s.hessian[i][j]+=lam*K[i][j];
        }
        for(std::size_t i=0;i<n;++i){
            double q=logistic(z[i]);s.value+=softplus(z[i])-y[i]*z[i];
            for(std::size_t j=0;j<n;++j)s.gradient[j]+=(q-y[i])*K[i][j];
            add_outer(s.hessian,K[i],q*(1-q));
        }
        return s;
    };
    auto f=damped_newton(n,tol,Kmax,evaluate);
    Report o{{"Row","Train","Label","Probability","Logit","PenalizedNLL","ScoreResidual","Iterations"},{}};
    for(std::size_t i=0;i<x.size();++i){double z=0;for(std::size_t j=0;j<n;++j)z+=f.beta[j]*kernel(a[i],a[train[j]]);o.rows.push_back({double(i+1),x[i][0],x[i][1],logistic(z),z,f.state.value,inf_norm(f.state.gradient),double(f.iterations)});}
    return o;
}
} }
