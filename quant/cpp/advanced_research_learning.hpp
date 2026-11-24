#pragma once
#include "advanced_research_common.hpp"
#include "model_research_likelihood.hpp"
namespace kline { namespace advanced_research {
inline Report masked_als(const Mat& x,const Vec& p){
    validate(x,2,4,4);parameters(p,4);
    auto k=count(p[0],1,8),K=count(p[3],1,100000);double lam=positive(p[1]),tol=positive(p[2]);
    double scale=1;
    std::size_t m=0,n=0;
    for(auto&r:x){m=std::max(m,count(r[0],1,128));n=std::max(n,count(r[1],1,128));require(r[2]==0||r[2]==1,"Binary training mask required");if(r[2])scale=std::max(scale,std::abs(r[3]));}
    require(k<=std::min(m,n)&&double(K)*(x.size()*k*k+(m+n)*k*k*k)<=2e8,"ALS work limit");
    std::vector<std::vector<std::pair<std::size_t,double>>> row(m),col(n);
    std::vector<std::vector<bool>> seen(m,std::vector<bool>(n,false));std::size_t train=0;
    for(auto&r:x){auto i=std::size_t(r[0])-1,j=std::size_t(r[1])-1;require(!seen[i][j],"Duplicate matrix coordinate");seen[i][j]=true;if(r[2]){row[i].push_back({j,r[3]});col[j].push_back({i,r[3]});++train;}}
    require(train>0,"No training observations");
    Mat U(m,Vec(k,0)),V(n,Vec(k,0));
    for(std::size_t j=0;j<n;++j)for(std::size_t h=0;h<k;++h)V[j][h]=h==0?1:.1*std::sin(double((j+1)*(h+1)));
    auto update=[&](const auto& groups,const Mat& other,Mat& target){
        for(std::size_t i=0;i<groups.size();++i){
            Mat H=identity(k);for(std::size_t h=0;h<k;++h)H[h][h]=lam;
            Vec b(k,0);for(auto&item:groups[i]){const auto& a=other[item.first];add_outer(H,a,1);for(std::size_t h=0;h<k;++h)b[h]+=a[h]*item.second;}
            target[i]=solve_spd(H,b);
        }
    };
    auto eval=[&](){double f=0,gap=0;Mat g=U,h=V;
        for(auto&r:g)for(double&v:r)v*=lam;for(auto&r:h)for(double&v:r)v*=lam;
        for(auto&r:U)f+=.5*lam*squared(r);for(auto&r:V)f+=.5*lam*squared(r);
        for(std::size_t i=0;i<m;++i)for(auto&item:row[i]){
            auto j=item.first;double e=dot(U[i],V[j])-item.second;f+=.5*e*e;
            for(std::size_t a=0;a<k;++a){g[i][a]+=e*V[j][a];h[j][a]+=e*U[i][a];}
        }
        for(auto&r:g)gap=std::max(gap,inf_norm(r));for(auto&r:h)gap=std::max(gap,inf_norm(r));
        return std::make_pair(f,gap);
    };
    auto prev=eval();std::size_t it=0;
    while(prev.second>tol*scale&&it<K){update(row,V,U);update(col,U,V);auto now=eval();require(now.first<=prev.first+1e-10*std::max(1.,prev.first),"ALS objective increased");prev=now;++it;}
    require(prev.second<=tol*scale,"ALS did not converge");
    Report o{{"Row","Column","Train","Value","Prediction","Objective","StationarityResidual","Iterations"},{}};
    for(auto&r:x)o.rows.push_back({r[0],r[1],r[2],r[3],dot(U[std::size_t(r[0])-1],V[std::size_t(r[1])-1]),prev.first,prev.second,double(it)});
    return o;
}

inline Report probit_ridge_fit(const Mat& x,const Vec& p){
    auto d=validate(x,2,2,17)-1;parameters(p,3);
    double lam=positive(p[0]),tol=positive(p[1]);auto K=count(p[2],1,2000);
    require(double(K)*x.size()*d*d<=5e7,"Probit work limit");
    Mat a;Vec sign;
    for(auto&r:x){require(r.back()==0||r.back()==1,"Binary label required");a.emplace_back(r.begin(),r.end()-1);sign.push_back(2*r.back()-1);}
    auto evaluate=[&](const Vec& w){
        auto s=model_research::regularized_state(w,lam);
        for(std::size_t i=0;i<x.size();++i){
            double z=sign[i]*dot(a[i],w),v=model_research::left_mills(z);
            s.value-=model_research::log_normal_cdf(z);
            for(std::size_t j=0;j<d;++j)s.gradient[j]-=sign[i]*v*a[i][j];
            add_outer(s.hessian,a[i],std::max(0.,v*(z+v)));
        }
        return s;
    };
    auto fit=model_research::damped_newton(d,tol,K,evaluate);
    Report o{{"Feature","Coefficient","PenalizedNLL","ScoreResidual","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),fit.beta[j],fit.state.value,inf_norm(fit.state.gradient),double(fit.iterations)});
    return o;
}

inline Report poisson_mixture_weight(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,4);
    double low=positive(p[0]),high=positive(p[1]),tol=positive(p[2]);auto K=count(p[3],1,100000);
    require(high>low&&double(K)*x.size()<=5e7,"Invalid Poisson mixture controls");
    Vec a,b;for(auto&r:x){double y=count(r[0],0,10000000),z=std::lgamma(y+1);a.push_back(y*std::log(low)-low-z);b.push_back(y*std::log(high)-high-z);}
    auto score=[&](double w){double g=0;for(std::size_t i=0;i<a.size();++i){double m=std::max(a[i],b[i]),u=std::exp(a[i]-m),v=std::exp(b[i]-m),den=w*u+(1-w)*v;if(den==0)g+=w==0?1e300:-1e300;else g+=(u-v)/den;}return g;};
    double w=.5,gap=0;std::size_t it=0;
    if(score(0)<=0)w=0;
    else if(score(1)>=0)w=1;
    else{
        for(;it<K;){
            double z=std::log(w)-std::log1p(-w),s=0;
            for(std::size_t i=0;i<a.size();++i)s+=logistic(z+a[i]-b[i]);
            w=s/a.size();++it;
            require(w>0&&w<1,"Mixture EM reached boundary unexpectedly");
            gap=std::abs(score(w));if(gap<=tol)break;
        }
        require(gap<=tol,"Mixture EM did not converge");
    }
    double ll=0;for(std::size_t i=0;i<a.size();++i)ll+=w==0?b[i]:w==1?a[i]:model_research::logadd(std::log(w)+a[i],std::log1p(-w)+b[i]);
    return scalar({"MixtureWeightLow","MixtureWeightHigh","LogLikelihood","ProjectedScoreResidual","Iterations"},{w,1-w,ll,gap,double(it)});
}

inline Report bernoulli_hmm_filter(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,5);for(double v:p)require(v>0&&v<1,"Interior HMM probabilities required");
    double init=p[0],a=p[1],b=p[2],e0=p[3],e1=p[4];
    auto n=x.size();Vec prior(n),f(n),scale(n),smooth(n),logp(n);double ll=0;
    auto emit=[&](std::size_t t,double e){return x[t][0]?e:1-e;};
    for(std::size_t t=0;t<n;++t){
        require(x[t][0]==0||x[t][0]==1,"Binary observation required");
        prior[t]=t?a*(1-f[t-1])+(1-b)*f[t-1]:init;
        double lo=(1-prior[t])*emit(t,e0),hi=prior[t]*emit(t,e1);
        scale[t]=positive(lo+hi);f[t]=hi/scale[t];logp[t]=std::log(scale[t]);ll+=logp[t];
    }
    double l=1,h=1;
    for(std::size_t t=n;t-->0;){
        smooth[t]=f[t]*h/positive((1-f[t])*l+f[t]*h);
        if(t){double u=emit(t,e0)*l/scale[t],v=emit(t,e1)*h/scale[t];l=(1-a)*u+a*v;h=b*u+(1-b)*v;}
    }
    Report o{{"Time","Observation","PriorHigh","FilteredHigh","SmoothedHigh","LogPredictiveProbability","LogLikelihood"},{}};
    for(std::size_t t=0;t<n;++t)o.rows.push_back({double(t+1),x[t][0],prior[t],f[t],smooth[t],logp[t],ll});
    return o;
}

inline Report recursive_least_squares(const Mat& x,const Vec& p){
    auto d=validate(x,1,2,33)-1;parameters(p,2);double gamma=p[0],lam=positive(p[1]);
    require(gamma>0&&gamma<=1&&double(x.size())*d*d<=5e7,"Invalid RLS controls");
    Mat P=identity(d),H=identity(d);for(std::size_t j=0;j<d;++j){P[j][j]/=lam;H[j][j]*=lam;}
    Vec w(d,0);
    Report o{{"Time","Actual","Prediction","Innovation","PosteriorPrediction","InformationTrace"},{}};
    for(std::size_t j=0;j<d;++j)o.header.push_back("Beta"+std::to_string(j+1));
    for(std::size_t t=0;t<x.size();++t){
        Vec a(x[t].begin(),x[t].end()-1),v=mul(P,a);double y=x[t].back(),pred=dot(a,w),err=y-pred,den=positive(gamma+dot(a,v));
        for(std::size_t j=0;j<d;++j)w[j]+=v[j]*err/den;
        double trace=0;
        for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<d;++j){
            P[i][j]=(P[i][j]-v[i]*v[j]/den)/gamma;
            H[i][j]=gamma*H[i][j]+a[i]*a[j];if(i==j){positive(P[i][j]);trace+=H[i][j];}
        }
        Vec r{double(t+1),y,pred,err,dot(a,w),trace};r.insert(r.end(),w.begin(),w.end());o.rows.push_back(r);
    }
    return o;
}

inline Report dirichlet_multinomial_grid(const Mat& x,const Vec& p){
    auto d=validate(x,2,2,32);parameters(p,3+d);
    double lo=positive(p[0]),hi=positive(p[1]);auto g=count(p[2],2,10000);
    require(hi>=lo&&double(g)*x.size()*d<=5e7,"Invalid concentration grid");
    Vec m(p.begin()+3,p.end());for(double v:m)positive(v);require(std::abs(sum(m)-1)<=1e-10,"Base probabilities must sum to one");
    Vec n;double factorial=0;for(auto&r:x){double s=0;for(double v:r){count(v,0,10000000);s+=v;factorial-=std::lgamma(v+1);}n.push_back(s);factorial+=std::lgamma(s+1);}
    positive(sum(n));double best=std::numeric_limits<double>::infinity(),kap=lo;
    for(std::size_t k=0;k<g;++k){
        double a=std::exp(std::log(lo)+(std::log(hi)-std::log(lo))*k/(g-1)),ll=factorial;
        for(std::size_t i=0;i<x.size();++i){ll+=std::lgamma(a)-std::lgamma(a+n[i]);for(std::size_t j=0;j<d;++j)ll+=std::lgamma(a*m[j]+x[i][j])-std::lgamma(a*m[j]);}
        if(-ll<best){best=-ll;kap=a;}
    }
    Report o{{"Group","Concentration","TrainingNLL"},{}};
    for(std::size_t j=0;j<d;++j)o.header.push_back("PosteriorMean"+std::to_string(j+1));
    for(std::size_t i=0;i<x.size();++i){Vec r{double(i+1),kap,best};for(std::size_t j=0;j<d;++j)r.push_back((kap*m[j]+x[i][j])/(kap+n[i]));o.rows.push_back(r);}
    return o;
}
} }
