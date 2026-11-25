#pragma once
#include "comprehensive_common.hpp"
#include "model_research_likelihood.hpp"
namespace kline { namespace comprehensive {
inline Report anderson_rubin_inversion(const Mat& x,const Vec& p){
    auto d=validate(x,4,3,20);require(x.size()>d-1&&p.size()>=2,"insufficient residual degrees of freedom");
    double critical=positive(p[0]);size_t k=d-2,n=x.size();
    Mat Z;for(const auto&r:x){Vec z{1};z.insert(z.end(),r.begin()+2,r.end());Z.push_back(z);}
    Mat H(k+1,Vec(k+1));for(const auto&z:Z)for(size_t a=0;a<=k;++a)for(size_t b=0;b<=k;++b)H[a][b]+=z[a]*z[b];
    Report out;out.header={"Candidate","Statistic","Accept","ResidualVariance"};
    for(size_t j=1;j<p.size();++j){
        double b=p[j];Vec y(n),rhs(k+1);
        for(size_t i=0;i<n;++i){y[i]=x[i][0]-b*x[i][1];for(size_t a=0;a<=k;++a)rhs[a]+=Z[i][a]*y[i];}
        Vec theta=solve_spd(H,rhs);double m=sum(y)/n,restricted=0,unrestricted=0;
        for(size_t i=0;i<n;++i){restricted+=std::pow(y[i]-m,2);unrestricted+=std::pow(y[i]-dot(Z[i],theta),2);}
        double var=positive(unrestricted/(n-k-1));
        double F=std::max(0.,restricted-unrestricted)/k/var;
        out.rows.push_back({b,F,double(F<=critical),var});
    }
    return out;
}

inline Report cluster_wild_sign_test(const Mat& x,const Vec& p){
    validate(x,2,3,3);parameters(p,1);double b=p[0],mx=0,mu=0;
    for(const auto&r:x){mx+=r[1]/x.size();mu+=(r[0]-b*r[1])/x.size();}
    std::map<int,double> g;
    for(const auto&r:x){int c=count(r[2],1,1000000000);g[c]+=(r[1]-mx)*(r[0]-b*r[1]-mu);}
    require(g.size()>=2&&g.size()<=20,"cluster count must be 2 to 20");
    Vec v;for(const auto&z:g)v.push_back(z.second);
    double obs=std::abs(sum(v)),extreme=0;size_t total=size_t(1)<<v.size();
    for(size_t mask=0;mask<total;++mask){
        double z=0;for(size_t j=0;j<v.size();++j)z+=((mask>>j)&1?1:-1)*v[j];
        extreme+=std::abs(z)>=obs-1e-12*std::max(1.,obs);
    }
    return scalar({"NullSlope","ObservedScore","Clusters","Assignments","PValue"},{b,obs,double(v.size()),double(total),extreme/total});
}

inline Report empirical_likelihood_mean(const Mat& x,const Vec& p){
    validate(x,2,1,1);require(!p.empty(),"candidate grid required");
    Report out;out.header={"Candidate","Lambda","LogLikelihoodRatio","WeightSum","MomentResidual","Feasible"};
    for(double m:p){
        Vec z;for(const auto&r:x)z.push_back(r[0]-m);
        double mn=*std::min_element(z.begin(),z.end()),mx=*std::max_element(z.begin(),z.end());
        if(mn==0&&mx==0){out.rows.push_back({m,0,0,1,0,1});continue;}
        if(mn>=0||mx<=0){out.rows.push_back({m,0,0,0,0,0});continue;}
        double lo=-1/mx,hi=-1/mn;
        for(int it=0;it<180;++it){
            double v=lo+(hi-lo)*.5,f=0;
            for(double a:z)f+=a/(1+v*a);
            if(f>0)lo=v;else hi=v;
        }
        double l=lo+(hi-lo)*.5,ratio=0,s=0,err=0;
        for(double a:z){double den=positive(1+l*a),w=1/(x.size()*den);ratio+=2*std::log(den);s+=w;err+=w*a;}
        out.rows.push_back({m,l,std::max(0.,ratio),s,err,1});
    }
    return out;
}

inline Report quantile_regression_admm(const Mat& x,const Vec& p){
    size_t d=validate(x,2,2,33)-1,n=x.size();parameters(p,4);
    double tau=p[0],rho=positive(p[1]),tol=positive(p[2]);int cap=count(p[3],1,100000);
    require(tau>0&&tau<1&&double(n)*d*d*cap<=1e9,"invalid quantile controls");
    Mat X,H(d,Vec(d));Vec y;
    for(const auto&r:x){y.push_back(r[0]);X.emplace_back(r.begin()+1,r.end());for(size_t a=0;a<d;++a)for(size_t b=0;b<d;++b)H[a][b]+=r[a+1]*r[b+1];}
    Vec z(n),u(n),w(d);double pr=0,du=0;int it=0;
    for(;it<cap;++it){
        Vec rhs(d),old=z;
        for(size_t i=0;i<n;++i)for(size_t j=0;j<d;++j)rhs[j]+=X[i][j]*(y[i]-z[i]-u[i]);
        w=solve_spd(H,rhs);Vec dr(d);pr=0;
        for(size_t i=0;i<n;++i){
            double fit=dot(X[i],w),v=y[i]-fit-u[i];
            z[i]=std::max(v-tau/rho,0.)+std::min(v+(1-tau)/rho,0.);
            double r=fit+z[i]-y[i];u[i]+=r;pr=std::max(pr,std::abs(r));
            for(size_t j=0;j<d;++j)dr[j]+=rho*X[i][j]*(z[i]-old[i]);
        }
        du=inf_norm(dr);if(pr<=tol&&du<=tol){++it;break;}
    }
    require(pr<=tol&&du<=tol,"ADMM did not converge");double loss=0;
    for(size_t i=0;i<n;++i){double r=y[i]-dot(X[i],w);loss+=r*(tau-(r<0));}
    Report out;out.header={"Feature","Coefficient","PinballLoss","PrimalResidual","DualResidual","Iterations"};
    for(size_t j=0;j<d;++j)out.rows.push_back({double(j+1),w[j],loss,pr,du,double(it)});
    return out;
}

inline Report aalen_johansen_incidence(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,0);std::map<double,Vec> table;
    for(const auto&r:x){require(r[0]>=0,"negative duration");int e=count(r[1],0,2);if(table[r[0]].empty())table[r[0]]=Vec(3);table[r[0]][e]++;}
    double Y=x.size(),S=1,F1=0,F2=0;Report out;out.header={"Time","AtRisk","Events1","Events2","Censored","Survival","CIF1","CIF2"};
    for(const auto&v:table){
        double c=v.second[0],d1=v.second[1],d2=v.second[2];
        F1+=S*d1/Y;F2+=S*d2/Y;S*=1-(d1+d2)/Y;
        out.rows.push_back({v.first,Y,d1,d2,c,S,F1,F2});Y-=c+d1+d2;
    }
    return out;
}

inline Report interval_censored_discrete_em(const Mat& x,const Vec& p){
    validate(x,1,2,2);require(p.size()>=3,"support required");
    double tol=positive(p[0]);int cap=count(p[1],1,100000);Vec s(p.begin()+2,p.end());
    require(std::is_sorted(s.begin(),s.end())&&std::adjacent_find(s.begin(),s.end())==s.end(),"support must increase");
    require(double(x.size())*s.size()*cap<=1e8,"EM too large");
    std::vector<std::vector<size_t>> idx;
    for(const auto&r:x){
        require(r[0]<=r[1],"reversed interval");std::vector<size_t> a;
        for(size_t j=0;j<s.size();++j)if(s[j]>=r[0]&&s[j]<=r[1])a.push_back(j);
        require(!a.empty(),"interval misses support");idx.push_back(a);
    }
    Vec w(s.size(),1./s.size());double ll=0;int it=0;
    auto step=[&](const Vec& v,Vec& next){
        next.assign(s.size(),0);double value=0;
        for(const auto&a:idx){
            double mass=0;for(size_t j:a)mass+=v[j];positive(mass);value+=std::log(mass);
            for(size_t j:a)next[j]+=v[j]/mass/x.size();
        }
        return value;
    };
    Vec next;ll=step(w,next);
    for(;it<cap;++it){
        double gap=0;for(size_t j=0;j<s.size();++j)gap=std::max(gap,std::abs(next[j]-w[j]));
        w=next;double v=step(w,next);require(v+1e-10>=ll,"EM decreased");ll=v;
        if(gap<=tol){++it;break;}
    }
    require(it<cap,"EM iteration cap reached");
    Report out;out.header={"Support","Mass","CDF","LogLikelihood","Iterations"};double cdf=0;
    for(size_t j=0;j<s.size();++j){cdf+=w[j];out.rows.push_back({s[j],w[j],cdf,ll,double(it)});}
    return out;
}

inline Report overlap_weighted_effects(const Mat& x,const Vec& p){
    size_t d=validate(x,2,3,32)-2;parameters(p,3);
    double lam=p[0],tol=positive(p[1]);int cap=count(p[2],1,10000);require(lam>=0,"negative penalty");
    Mat X;Vec D,y;double treated=0;
    for(const auto&r:x){D.push_back(count(r[0],0,1));treated+=D.back();y.push_back(r[1]);X.emplace_back(r.begin()+2,r.end());}
    require(treated>0&&treated<x.size(),"both treatment groups needed");
    auto eval=[&](const Vec& b){
        auto s=model_research::regularized_state(b,lam);
        for(size_t i=0;i<X.size();++i){
            double z=dot(X[i],b),q=logistic(z);s.value+=model_research::softplus(z)-D[i]*z;
            for(size_t j=0;j<d;++j){s.gradient[j]+=(q-D[i])*X[i][j];for(size_t k=0;k<d;++k)s.hessian[j][k]+=q*(1-q)*X[i][j]*X[i][k];}
        }
        return s;
    };
    auto f=model_research::damped_newton(d,tol,cap,eval);
    Vec mass(2),sq(2),outcome(2);Mat balance(2,Vec(d));
    for(size_t i=0;i<X.size();++i){
        int g=int(D[i]);double w=logistic((g?-1:1)*dot(X[i],f.beta));
        mass[g]+=w;sq[g]+=w*w;outcome[g]+=w*y[i];for(size_t j=0;j<d;++j)balance[g][j]+=w*X[i][j];
    }
    positive(mass[0]);positive(mass[1]);double m0=outcome[0]/mass[0],m1=outcome[1]/mass[1],err=0;
    for(size_t j=0;j<d;++j)err=std::max(err,std::abs(balance[1][j]/mass[1]-balance[0][j]/mass[0]));
    return scalar({"Effect","TreatedMean","ControlMean","TreatedESS","ControlESS","MaxBalanceError","ScoreNorm"},{m1-m0,m1,m0,mass[1]*mass[1]/sq[1],mass[0]*mass[0]/sq[0],err,inf_norm(f.state.gradient)});
}

inline Report rosenbaum_pair_bounds(const Mat& x,const Vec& p){
    validate(x,1,1,1);require(!p.empty(),"gamma grid required");
    int n=0,t=0;for(const auto&r:x){if(r[0]!=0){++n;t+=r[0]>0;}}require(n>0,"no nonzero pair differences");
    auto tail=[&](double q){
        if(t==0)return 1.;Vec v;
        for(int k=t;k<=n;++k)v.push_back(std::lgamma(n+1.)-std::lgamma(k+1.)-std::lgamma(n-k+1.)+k*std::log(q)+(n-k)*std::log1p(-q));
        return std::min(1.,std::exp(logsumexp(v)));
    };
    Report out;out.header={"Gamma","Pairs","PositivePairs","LowerP","UpperP"};
    for(double g:p){require(g>=1&&g<=1e12,"gamma outside range");double q=1/(1+g);out.rows.push_back({g,double(n),double(t),tail(q),tail(1-q)});}
    return out;
}
} }
