#pragma once
#include "comprehensive_common.hpp"
#include "model_research_likelihood.hpp"
#include <array>
#include <functional>
namespace kline { namespace comprehensive {
inline Report normal_hierarchical_grid(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,5);
    double m0=p[0],v0=positive(p[1]),lo=p[2],hi=p[3];int G=count(p[4],1,10000);
    require(lo>=0&&hi>=lo&&(G>1||lo==hi),"invalid tau grid");
    for(const auto&r:x)positive(r[1]);Vec ts,ms,vs,logs;
    for(int g=0;g<G;++g){
        double t=G==1?lo:lo+(hi-lo)*g/(G-1),prec=1/v0,b=m0/v0,c=m0*m0/v0,ld=std::log(v0);
        for(const auto&r:x){double v=r[1]+t;prec+=1/v;b+=r[0]/v;c+=r[0]*r[0]/v;ld+=std::log(v);}
        double V=1/prec,M=V*b;
        ts.push_back(t);ms.push_back(M);vs.push_back(V);
        logs.push_back(-.5*(x.size()*std::log(2*std::acos(-1.))+ld+std::log(prec)+c-b*b/prec));
    }
    double logz=logsumexp(logs);Vec weights;double hm=0,ht=0;
    for(int g=0;g<G;++g){double w=std::exp(logs[g]-logz);weights.push_back(w);hm+=w*ms[g];ht+=w*ts[g];}
    Report out;out.header={"Group","PosteriorMean","PosteriorVariance","HyperMean","HyperTauSquared","LogEvidence"};
    for(size_t i=0;i<x.size();++i){
        double mean=0,second=0;
        for(int g=0;g<G;++g){
            double a=ts[g]/(ts[g]+x[i][1]),m=a*x[i][0]+(1-a)*ms[g],v=a*x[i][1]+(1-a)*(1-a)*vs[g];
            mean+=weights[g]*m;second+=weights[g]*(v+m*m);
        }
        out.rows.push_back({double(i+1),mean,std::max(0.,second-mean*mean),hm,ht,logz-std::log(double(G))});
    }
    return out;
}

inline Report logistic_laplace_posterior(const Mat& x,const Vec& p){
    size_t d=validate(x,1,2,33)-1;parameters(p,3);
    double lam=positive(p[0]),tol=positive(p[1]);int cap=count(p[2],1,10000);
    Mat X;Vec y;for(const auto&r:x){y.push_back(count(r[0],0,1));X.emplace_back(r.begin()+1,r.end());}
    auto eval=[&](const Vec& b){
        auto s=model_research::regularized_state(b,lam);
        for(size_t i=0;i<X.size();++i){
            double z=dot(X[i],b),q=logistic(z);
            s.value+=model_research::softplus(z)-y[i]*z;
            for(size_t j=0;j<d;++j){s.gradient[j]+=(q-y[i])*X[i][j];for(size_t k=0;k<d;++k)s.hessian[j][k]+=q*(1-q)*X[i][j]*X[i][k];}
        }
        return s;
    };
    auto fit=model_research::damped_newton(d,tol,cap,eval);
    Mat V=invert_spd(fit.state.hessian);double evidence=-fit.state.value+.5*d*std::log(lam)-.5*logdet_spd(fit.state.hessian);
    Report out;out.header={"Feature","Mode","PosteriorSD","LogEvidenceApprox","ScoreNorm"};
    for(size_t j=0;j<d;++j)out.rows.push_back({double(j+1),fit.beta[j],std::sqrt(V[j][j]),evidence,inf_norm(fit.state.gradient)});
    return out;
}

inline Report dirichlet_transition_posterior(const Mat& x,const Vec& p){
    validate(x,1,3,3);require(p.size()>=2,"state count and prior required");
    int K=count(p[0],1,32);require(p.size()==size_t(K+1),"one prior per destination");
    Mat A(K,Vec(K));for(int i=0;i<K;++i)for(int j=0;j<K;++j)A[i][j]=positive(p[j+1]);
    for(const auto&r:x){int i=count(r[0],1,K)-1,j=count(r[1],1,K)-1;A[i][j]+=count(r[2],0,1000000000);}
    Vec total(K);for(int i=0;i<K;++i)total[i]=sum(A[i]);
    Report out;out.header={"From","To","PosteriorAlpha","MeanTransition","TransitionVariance","TwoStepPredictive"};
    for(int i=0;i<K;++i)for(int j=0;j<K;++j){
        double m=A[i][j]/total[i],v=m*(1-m)/(total[i]+1),two=0;
        for(int k=0;k<K;++k){
            if(k==i)two+=A[i][i]*(A[i][j]+(j==i))/(total[i]*(total[i]+1));
            else two+=A[i][k]/total[i]*A[k][j]/total[k];
        }
        out.rows.push_back({double(i+1),double(j+1),A[i][j],m,v,two});
    }
    return out;
}

inline Report beta_binomial_hyper_grid(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,6);
    double alo=positive(p[0]),ahi=positive(p[1]),blo=positive(p[3]),bhi=positive(p[4]);
    int G=count(p[2],1,300),H=count(p[5],1,300);
    require(ahi>=alo&&bhi>=blo&&(G>1||alo==ahi)&&(H>1||blo==bhi)&&double(G)*H*x.size()<=1e7,"invalid hyper grid");
    Vec k,n;for(const auto&r:x){n.push_back(count(r[1],0,1000000000));k.push_back(count(r[0],0,int(n.back())));}
    auto lb=[](double a,double b){return std::lgamma(a)+std::lgamma(b)-std::lgamma(a+b);};
    Mat grid;Vec logs;
    for(int i=0;i<G;++i)for(int j=0;j<H;++j){
        double a=G==1?alo:alo+(ahi-alo)*i/(G-1),b=H==1?blo:blo+(bhi-blo)*j/(H-1),ll=0;
        for(size_t r=0;r<x.size();++r)ll+=std::lgamma(n[r]+1)-std::lgamma(k[r]+1)-std::lgamma(n[r]-k[r]+1)+lb(a+k[r],b+n[r]-k[r])-lb(a,b);
        grid.push_back({a,b});logs.push_back(ll);
    }
    double z=logsumexp(logs),ha=0,hb=0;Vec w;
    for(size_t j=0;j<grid.size();++j){w.push_back(std::exp(logs[j]-z));ha+=w.back()*grid[j][0];hb+=w.back()*grid[j][1];}
    Report out;out.header={"Group","PosteriorMean","PosteriorVariance","HyperAlpha","HyperBeta","LogEvidence"};
    for(size_t i=0;i<x.size();++i){
        double m=0,s=0;
        for(size_t j=0;j<grid.size();++j){double a=grid[j][0]+k[i],b=grid[j][1]+n[i]-k[i],t=a+b;m+=w[j]*a/t;s+=w[j]*a*(a+1)/(t*(t+1));}
        out.rows.push_back({double(i+1),m,std::max(0.,s-m*m),ha,hb,z-std::log(double(grid.size()))});
    }
    return out;
}

inline Report particle_sv_filter(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,7);
    double mu=p[0],phi=p[1],sig=p[2],init=p[3],threshold=p[6];
    int N=count(p[4],2,200000);int seed=count(p[5],0,2147483647);
    require(std::abs(phi)<1&&sig>=0&&init>=0&&threshold>0&&threshold<=1&&double(N)*x.size()<=5e7,"invalid filter controls");
    std::mt19937 gen(seed);std::normal_distribution<double> normal01(0,1);std::uniform_real_distribution<double> uniform(0,1);
    Vec h(N),w(N,1./N);for(double&v:h)v=mu+(init==0?0:init*normal01(gen));
    Report out;out.header={"Time","FilteredLogVariance","FilteredVariance","ESS","LogPredictiveDensity","Resampled"};
    for(size_t t=0;t<x.size();++t){
        Vec logs(N);double r=x[t][0];
        for(int i=0;i<N;++i){
            h[i]=mu+phi*(h[i]-mu)+(sig==0?0:sig*normal01(gen));
            require(std::abs(h[i])<700,"log variance overflow");
            logs[i]=std::log(w[i])-.5*(std::log(2*std::acos(-1.))+h[i]+r*r*std::exp(-h[i]));
        }
        double z=logsumexp(logs),mh=0,mv=0,sq=0;
        for(int i=0;i<N;++i){w[i]=std::exp(logs[i]-z);mh+=w[i]*h[i];mv+=w[i]*std::exp(h[i]);sq+=w[i]*w[i];}
        double ess=1/sq;bool resample=ess<threshold*N-1e-10;
        out.rows.push_back({double(t+1),mh,mv,ess,z,double(resample)});
        if(resample){
            Vec next(N);double u=uniform(gen)/N,c=w[0];int j=0;
            for(int i=0;i<N;++i){double q=u+double(i)/N;while(j<N-1&&q>c)c+=w[++j];next[i]=h[j];}
            h=next;std::fill(w.begin(),w.end(),1./N);
        }
    }
    return out;
}

inline Report gaussian_tail_importance(const Mat& x,const Vec& p){
    validate(x,1,1,1);parameters(p,4);
    double mu=p[0],sig=positive(p[1]);int N=count(p[2],2,2000000),seed=count(p[3],0,2147483647);
    require(double(N)*x.size()<=5e7,"simulation too large");
    std::mt19937 gen(seed);std::normal_distribution<double> normal01(0,1);Vec noise(N);for(double&v:noise)v=normal01(gen);
    Report out;out.header={"Threshold","ImportanceEstimate","MonteCarloSE","ExactProbability","Tilt","EffectiveSampleSize"};
    for(const auto&r:x){
        double a=(r[0]-mu)/sig,theta=std::max(0.,a);require(std::abs(a)<=12,"threshold outside numerical range");
        double mean=0,m2=0,total=0,sq=0;int n=0;
        for(double e:noise){
            double z=theta+e,u=z>a?std::exp(-theta*z+.5*theta*theta):0;total+=u;sq+=u*u;
            ++n;double d=u-mean;mean+=d/n;m2+=d*(u-mean);
        }
        out.rows.push_back({r[0],mean,std::sqrt(std::max(0.,m2)/(N-1)/N),.5*std::erfc(a/std::sqrt(2.)),theta,sq>0?total*total/sq:0});
    }
    return out;
}

inline Report bayesian_multivariate_regression(const Mat& x,const Vec& p){
    size_t cols=validate(x,1,2,33);require(p.size()>=3,"response count and covariance required");
    size_t k=count(p[0],1,16);double lam=positive(p[1]);require(cols>k&&p.size()==2+k*k,"invalid dimensions");
    size_t d=cols-k,n=x.size();Mat S(k,Vec(k));
    for(size_t i=0;i<k;++i)for(size_t j=0;j<k;++j)S[i][j]=p[2+i*k+j];
    Mat Sinv=invert_spd(S),H(d,Vec(d)),XY(d,Vec(k)),YY(k,Vec(k));
    for(size_t j=0;j<d;++j)H[j][j]=lam;
    for(const auto&r:x){
        for(size_t i=0;i<d;++i){for(size_t j=0;j<d;++j)H[i][j]+=r[i]*r[j];for(size_t j=0;j<k;++j)XY[i][j]+=r[i]*r[d+j];}
        for(size_t i=0;i<k;++i)for(size_t j=0;j<k;++j)YY[i][j]+=r[d+i]*r[d+j];
    }
    Mat V=invert_spd(H),M(d,Vec(k));
    for(size_t j=0;j<k;++j){Vec b(d);for(size_t i=0;i<d;++i)b[i]=XY[i][j];Vec z=solve_spd(H,b);for(size_t i=0;i<d;++i)M[i][j]=z[i];}
    double trace=0;
    for(size_t i=0;i<k;++i)for(size_t j=0;j<k;++j){double e=YY[i][j];for(size_t a=0;a<d;++a)e-=M[a][i]*XY[a][j];trace+=Sinv[j][i]*e;}
    double evidence=-.5*n*k*std::log(2*std::acos(-1.))-.5*n*logdet_spd(S)+.5*k*(d*std::log(lam)-logdet_spd(H))-.5*trace;
    Report out;out.header={"Feature","Response","PosteriorMean","PosteriorVariance","FeatureCovariance","LogEvidence"};
    for(size_t i=0;i<d;++i)for(size_t j=0;j<k;++j)out.rows.push_back({double(i+1),double(j+1),M[i][j],V[i][i]*S[j][j],V[i][i],evidence});
    return out;
}

inline Report beta_bandit_finite_horizon(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,2);int H=count(p[0],1,25);double delta=p[1];require(delta>0&&delta<=1,"invalid discount");
    Report out;out.header={"Scenario","Horizon","Action","ExpectedReward","FirstArmValue","SecondArmValue"};
    for(size_t row=0;row<x.size();++row){
        std::array<int,4>s;for(int i=0;i<4;++i)s[i]=count(x[row][i],1,1000000);
        std::map<std::array<int,5>,double> cache;
        std::function<double(std::array<int,4>,int)> value;
        auto action=[&](std::array<int,4> a,int h,int arm){
            int j=2*arm;double m=double(a[j])/(a[j]+a[j+1]);auto plus=a,minus=a;
            ++plus[j];++minus[j+1];return m+delta*(m*value(plus,h-1)+(1-m)*value(minus,h-1));
        };
        value=[&](std::array<int,4>a,int h){
            if(h==0)return 0.;std::array<int,5> key{a[0],a[1],a[2],a[3],h};
            auto it=cache.find(key);if(it!=cache.end())return it->second;
            double v=std::max(action(a,h,0),action(a,h,1));cache[key]=v;return v;
        };
        double a=action(s,H,0),b=action(s,H,1);
        out.rows.push_back({double(row+1),double(H),a>=b-1e-12?1.:2.,std::max(a,b),a,b});
    }
    return out;
}
} }
