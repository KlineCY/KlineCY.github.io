#pragma once
#include "comprehensive_common.hpp"
namespace kline { namespace comprehensive {
inline Report factor_covariance_estimate(const Mat& x,const Vec& p){
    auto cols=validate(x,3,2,64);parameters(p,4);
    auto k=count(p[0],1,16);require(cols>k&&cols-k<=32,"Invalid factor dimensions");auto d=cols-k,n=x.size();
    double lam=p[1],s=p[2],floor=positive(p[3]);
    require(lam>=0&&s>=0&&s<=1&&double(n)*d*k*k<=5e7,"Invalid factor controls");
    Mat X,F;Vec mean(k,0);
    for(auto&r:x){
        Vec f(r.begin(),r.begin()+k);F.push_back(f);
        for(std::size_t j=0;j<k;++j)mean[j]+=f[j]/n;
        f.insert(f.begin(),1);X.push_back(f);
    }
    Mat sf(k,Vec(k,0));
    for(auto&r:F){Vec z=r;for(std::size_t j=0;j<k;++j)z[j]-=mean[j];add_outer(sf,z,1./(n-1));}
    for(std::size_t i=0;i<k;++i)for(std::size_t j=0;j<k;++j)if(i!=j)sf[i][j]*=1-s;
    Vec alpha(d),specific(d);Mat B;
    for(std::size_t i=0;i<d;++i){
        Vec y=column(x,k+i),w=fit_ridge(X,y,lam,true);alpha[i]=w[0];
        double rss=0;for(std::size_t t=0;t<n;++t)rss+=std::pow(y[t]-dot(X[t],w),2);
        specific[i]=rss/(n-1)+floor;B.emplace_back(w.begin()+1,w.end());
    }
    Report out{{"Asset","Alpha","SpecificVariance"},{}};
    for(std::size_t j=1;j<=k;++j)out.header.push_back("FactorLoading"+std::to_string(j));
    for(std::size_t j=1;j<=d;++j)out.header.push_back("Covariance"+std::to_string(j));
    for(std::size_t i=0;i<d;++i){
        Vec r{double(i+1),alpha[i],specific[i]};r.insert(r.end(),B[i].begin(),B[i].end());Vec z=mul(sf,B[i]);
        for(std::size_t j=0;j<d;++j)r.push_back(dot(z,B[j])+(i==j?specific[i]:0));out.rows.push_back(r);
    }
    return out;
}

inline Report black_litterman_correlated_views(const Mat& x,const Vec& p){
    auto cols=validate(x,3,3,34);parameters(p,4);auto d=count(p[0],1,32),m=count(p[1],1,d);
    double tau=positive(p[2]),delta=positive(p[3]);require(cols==d+2&&x.size()==d+2*m,"Invalid BL block dimensions");
    Mat S,P,O;Vec prior,q;
    for(std::size_t i=0;i<d;++i){
        auto&r=x[i];require(r[0]==0,"Covariance rows must be type zero");S.emplace_back(r.begin()+1,r.end()-1);prior.push_back(r.back());
    }
    for(std::size_t i=0;i<m;++i){
        auto&r=x[d+i];require(r[0]==1,"View rows must be type one");P.emplace_back(r.begin()+1,r.end()-1);q.push_back(r.back());
        auto&v=x[d+m+i];require(v[0]==2,"View covariance rows must be type two");
        O.emplace_back(v.begin()+1,v.begin()+1+m);for(std::size_t j=1+m;j<v.size();++j)require(v[j]==0,"Unused view covariance fields must be zero");
    }
    solve_spd(S,Vec(d,0));solve_spd(O,Vec(m,0));
    Mat V=S;for(auto&r:V)for(double&z:r)z*=tau;
    Mat C;for(auto&r:P)C.push_back(mul(V,r));
    Mat K=O;
    for(std::size_t i=0;i<m;++i)for(std::size_t j=0;j<m;++j)K[i][j]+=dot(P[i],C[j]);
    Vec e=q;for(std::size_t i=0;i<m;++i)e[i]-=dot(P[i],prior);Vec a=solve_spd(K,e),mean=prior;
    for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<m;++j)mean[i]+=C[j][i]*a[j];
    Mat M=V,pred=S;
    for(std::size_t j=0;j<d;++j){
        Vec b(m);for(std::size_t h=0;h<m;++h)b[h]=C[h][j];Vec z=solve_spd(K,b);
        for(std::size_t i=0;i<d;++i)for(std::size_t h=0;h<m;++h)M[i][j]-=C[h][i]*z[h];
    }
    for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<=i;++j){double v=(M[i][j]+M[j][i])/2;M[i][j]=M[j][i]=v;pred[i][j]=pred[j][i]=S[i][j]+v;}
    Vec w=solve_spd(pred,mean);for(double&v:w)v/=delta;
    Report out{{"Asset","PriorMean","PosteriorMean","MeanVariance","PredictiveVariance","UnconstrainedWeight"},{}};
    for(std::size_t j=1;j<=d;++j)out.header.push_back("MeanCovariance"+std::to_string(j));
    for(std::size_t i=0;i<d;++i){Vec r{double(i+1),prior[i],mean[i],M[i][i],pred[i][i],w[i]};r.insert(r.end(),M[i].begin(),M[i].end());out.rows.push_back(r);}
    return out;
}

inline Report active_tracking_frontier(const Mat& x,const Vec& p){
    auto d=validate(x,2,4,34)-2;require(x.size()==d,"Square risk matrix required");parameters(p,1);
    double risk=p[0];require(risk>=0,"Nonnegative tracking risk required");
    Mat S;Vec mu,b,one(d,1);
    for(auto&r:x){S.emplace_back(r.begin(),r.begin()+d);mu.push_back(r[d]);require(r[d+1]>=0,"Nonnegative benchmark weight");b.push_back(r[d+1]);}
    require(std::abs(sum(b)-1)<=1e-10,"Benchmark must sum to one");
    Vec u=solve_spd(S,mu),v=solve_spd(S,one);
    double nu=sum(u)/positive(sum(v));Vec z=mu;for(double&w:z)w-=nu;
    Vec a=solve_spd(S,z);double norm=std::sqrt(std::max(0.,dot(a,z)));
    if(norm<=1e-12*std::max(1.,std::sqrt(squared(mu)))||risk==0)a.assign(d,0);
    else for(double&w:a)w*=risk/norm;
    double te=std::sqrt(std::max(0.,dot(a,mul(S,a)))),ret=dot(mu,a);
    Report out{{"Asset","Benchmark","ActiveWeight","TotalWeight","ActiveReturn","TrackingError","InformationRatio","BudgetResidual"},{}};
    for(std::size_t i=0;i<d;++i)out.rows.push_back({double(i+1),b[i],a[i],b[i]+a[i],ret,te,te>0?ret/te:0,std::abs(sum(a))});
    return out;
}

inline Report mean_semivariance_simplex(const Mat& x,const Vec& p){
    auto d=validate(x,2,2,32);parameters(p,4);
    double target=p[0],gamma=positive(p[1]),tol=positive(p[2]);auto cap=count(p[3],1,100000);
    require(double(cap)*x.size()*d<=1e8,"Semivariance work limit");
    Vec mean(d,0),w(d,1./d),lo(d,0),hi(d,1);double L=0;
    for(auto&r:x){L+=gamma*squared(r)/x.size();for(std::size_t j=0;j<d;++j)mean[j]+=r[j]/x.size();}
    L=std::max(1e-12,L);
    auto eval=[&](const Vec& w){
        Vec g=mean;for(double&v:g)v=-v;double s=0;
        for(auto&r:x){
            double e=std::max(0.,target-dot(r,w));s+=e*e/x.size();
            for(std::size_t j=0;j<d;++j)g[j]-=gamma*e*r[j]/x.size();
        }
        return std::make_pair(s,g);
    };
    double gap=0;auto f=eval(w);
    for(std::size_t it=0;it<=cap;++it){
        Vec z=w;for(std::size_t j=0;j<d;++j)z[j]-=f.second[j]/L;
        Vec next=box_projection(z,lo,hi,1),g(d);
        for(std::size_t j=0;j<d;++j)g[j]=L*(w[j]-next[j]);gap=inf_norm(g);
        if(gap<=tol)break;require(it<cap,"Semivariance optimization did not converge");
        w=next;f=eval(w);
    }
    double r=dot(mean,w),obj=.5*gamma*f.first-r;
    Report out{{"Asset","Weight","MeanReturn","DownsideSemivariance","Objective","ProjectedGradientResidual","BudgetResidual"},{}};
    for(std::size_t j=0;j<d;++j)out.rows.push_back({double(j+1),w[j],r,f.first,obj,gap,std::abs(sum(w)-1)});
    return out;
}

inline Report maximum_diversification_kkt(const Mat& x,const Vec& p){
    auto d=validate(x,1,1,32);require(x.size()==d,"Square covariance required");parameters(p,2);
    double tol=positive(p[0]);auto cap=count(p[1],1,100000);require(double(cap)*d*d<=5e7,"Diversification work limit");
    Vec sig(d),y(d,0);for(std::size_t i=0;i<d;++i)sig[i]=std::sqrt(positive(x[i][i]));
    solve_spd(x,Vec(d,0));double gap=0;
    for(std::size_t it=0;it<=cap;++it){
        Vec g=mul(x,y);gap=0;
        for(std::size_t i=0;i<d;++i){double z=g[i]-sig[i];gap=std::max(gap,y[i]>0?std::abs(z):std::max(0.,-z));}
        if(gap<=tol)break;require(it<cap,"Diversification optimizer did not converge");
        for(std::size_t i=0;i<d;++i){double z=sig[i];for(std::size_t j=0;j<d;++j)if(j!=i)z-=x[i][j]*y[j];y[i]=std::max(0.,z/x[i][i]);}
    }
    Vec w=y;double mass=positive(sum(y));for(double&v:w)v/=mass;
    double var=positive(dot(w,mul(x,w))),dr=dot(sig,w)/std::sqrt(var);
    Report out{{"Asset","Weight","AssetVolatility","DiversificationRatio","PortfolioVariance","ScaledSolution","KKTResidual"},{}};
    for(std::size_t i=0;i<d;++i)out.rows.push_back({double(i+1),w[i],sig[i],dr,var,y[i],gap});
    return out;
}

inline Report entropic_risk_allocation(const Mat& x,const Vec& p){
    auto d=validate(x,2,2,32);parameters(p,3);
    double eta=positive(p[0]),tol=positive(p[1]);auto cap=count(p[2],1,100000);
    require(double(cap)*x.size()*d<=1e8,"Entropic work limit");
    double L=0;Vec mean(d,0),w(d,1./d),lo(d,0),hi(d,1);
    for(auto&r:x){L=std::max(L,eta*squared(r));for(std::size_t j=0;j<d;++j)mean[j]+=r[j]/x.size();}
    L=std::max(L,1e-12);
    auto evaluate=[&](const Vec& w){
        Vec z;for(auto&r:x)z.push_back(-eta*dot(r,w));
        double ld=logsumexp(z),f=(ld-std::log(double(x.size())))/eta,mass=0;Vec g(d,0);
        for(std::size_t i=0;i<x.size();++i){
            double q=std::exp(z[i]-ld);mass=std::max(mass,q);
            for(std::size_t j=0;j<d;++j)g[j]-=q*x[i][j];
        }
        return std::make_tuple(f,mass,g);
    };
    auto f=evaluate(w);double gap=0;
    for(std::size_t it=0;it<=cap;++it){
        Vec z=w;for(std::size_t j=0;j<d;++j)z[j]-=std::get<2>(f)[j]/L;
        Vec next=box_projection(z,lo,hi,1),g(d);
        for(std::size_t j=0;j<d;++j)g[j]=L*(w[j]-next[j]);gap=inf_norm(g);
        if(gap<=tol)break;require(it<cap,"Entropic optimizer did not converge");w=next;f=evaluate(w);
    }
    Report out{{"Asset","Weight","EntropicRisk","MeanReturn","WorstTiltMass","ProjectedGradientResidual","BudgetResidual"},{}};
    for(std::size_t j=0;j<d;++j)out.rows.push_back({double(j+1),w[j],std::get<0>(f),dot(mean,w),std::get<1>(f),gap,std::abs(sum(w)-1)});
    return out;
}

inline Report empirical_bayes_alpha_grid(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,3);auto G=count(p[2],2,100000);
    require(p[0]>=0&&p[1]>=p[0]&&double(G)*x.size()<=5e7,"Invalid EB grid");
    for(auto&r:x)positive(r[1]);double best=std::numeric_limits<double>::infinity(),tau=0,m=0;
    for(std::size_t j=0;j<G;++j){
        double t=p[0]+(p[1]-p[0])*j/(G-1),num=0,den=0;
        for(auto&r:x){double v=r[1]+t;num+=r[0]/v;den+=1/v;}
        double a=num/positive(den),f=0;
        for(auto&r:x){double v=r[1]+t;f+=.5*(std::log(2*std::acos(-1.)*v)+std::pow(r[0]-a,2)/v);}
        if(f<best){best=f;tau=t;m=a;}
    }
    Report out{{"Asset","RawAlpha","SamplingVariance","PriorMean","PriorVariance","PosteriorMean","PosteriorVariance","Shrinkage","NLL"},{}};
    for(std::size_t i=0;i<x.size();++i){
        double a=x[i][0],s=x[i][1],g=tau/(tau+s);
        out.rows.push_back({double(i+1),a,s,m,tau,m+g*(a-m),g*s,1-g,best});
    }
    return out;
}

inline Report rolling_panel_ridge_longshort(const Mat& x,const Vec& p){
    auto d=validate(x,4,4,35)-3;parameters(p,4);
    auto L=count(p[0],1,10000);double lam=p[1],cost=p[2],gross=positive(p[3]);
    require(lam>=0&&cost>=0&&cost<1&&gross<=4&&cost*gross<1,"Invalid strategy controls");
    std::vector<Mat> days;Vec dates;
    for(auto&r:x){
        require(r[2]>=-1,"Return below -100 percent");
        if(dates.empty()||r[0]!=dates.back()){
            require(dates.empty()||r[0]>dates.back(),"Date must increase");dates.push_back(r[0]);days.emplace_back();
        }
        days.back().push_back(r);
    }
    auto n=days[0].size();require(n>=2&&n<=64&&days.size()>L,"Invalid panel shape");
    std::set<double> ids;for(auto&r:days[0])require(ids.insert(r[1]).second,"Duplicate asset");
    require(double(days.size()-L)*L*n*d*d<=5e7,"Rolling fit work limit");
    std::vector<Mat> X;Mat y,ret;
    for(auto&day:days){
        require(day.size()==n,"Incomplete panel");
        Vec mean(d,0),r(n);double avg=0;
        for(std::size_t i=0;i<n;++i){
            require(day[i][1]==days[0][i][1],"Asset ordering mismatch");
            r[i]=day[i][2];avg+=r[i]/n;
            for(std::size_t j=0;j<d;++j)mean[j]+=day[i][j+3]/n;
        }
        Mat z(n,Vec(d));Vec target=r;
        for(std::size_t i=0;i<n;++i){
            target[i]-=avg;for(std::size_t j=0;j<d;++j)z[i][j]=day[i][j+3]-mean[j];
        }
        X.push_back(z);y.push_back(target);ret.push_back(r);
    }
    Vec old(n,0);double nav=1;
    Report out{{"Date","Asset","Score","Weight","RealizedReturn","Contribution","GrossReturn","NetReturn","Turnover","Fee","NAV"},{}};
    for(std::size_t j=1;j<=d;++j)out.header.push_back("FeatureCoefficient"+std::to_string(j));
    for(std::size_t t=L;t<days.size();++t){
        Mat design;Vec target;
        for(std::size_t s=t-L;s<t;++s)for(std::size_t i=0;i<n;++i){design.push_back(X[s][i]);target.push_back(y[s][i]);}
        Vec beta=fit_ridge(design,target,lam,false),score=mul(X[t],beta),w=score;
        double center=sum(score)/n;for(double&v:w)v-=center;
        double scale=l1(w);if(scale<=1e-14)w.assign(n,0);else for(double&v:w)v*=gross/scale;
        auto step=trade_step(old,w,ret[t],cost);nav=positive(nav*(1+step.net));
        for(std::size_t i=0;i<n;++i){
            Vec r{dates[t],days[t][i][1],score[i],w[i],ret[t][i],w[i]*ret[t][i],step.gross,step.net,step.turnover,step.fee,nav};
            r.insert(r.end(),beta.begin(),beta.end());out.rows.push_back(r);
        }
        old=step.drift;
    }
    return out;
}
} }
