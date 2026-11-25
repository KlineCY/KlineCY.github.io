#pragma once
#include "comprehensive_common.hpp"
namespace kline { namespace comprehensive {
inline Report levinson_durbin_path(const Mat& x,const Vec& p){
    validate(x,2,1,1);parameters(p,1);auto P=count(p[0],1,std::min<std::size_t>(x.size()-1,256));
    Vec gamma=column(x,0),phi;double v=positive(gamma[0]);
    Report out{{"Order","Lag","Coefficient","Reflection","InnovationVariance","NormalResidual"},{}};
    for(std::size_t k=1;k<=P;++k){
        double a=gamma[k];for(std::size_t j=1;j<k;++j)a-=phi[j-1]*gamma[k-j];
        double reflection=a/v;require(std::abs(reflection)<1,"Toeplitz covariance not positive definite");
        Vec next(k);next[k-1]=reflection;
        for(std::size_t j=1;j<k;++j)next[j-1]=phi[j-1]-reflection*phi[k-j-1];
        v=positive(v*(1-reflection*reflection));phi=next;
        double err=0;
        for(std::size_t i=1;i<=k;++i){
            double z=-gamma[i];for(std::size_t j=1;j<=k;++j)z+=gamma[i>j?i-j:j-i]*phi[j-1];
            err=std::max(err,std::abs(z));
        }
        for(std::size_t j=0;j<k;++j)out.rows.push_back({double(k),double(j+1),phi[j],reflection,v,err});
    }
    return out;
}

inline Report arx_recursive_fit(const Mat& x,const Vec& p){
    auto ex=validate(x,4,2,18)-2;parameters(p,2);auto P=count(p[0],1,16);double lam=p[1];
    require(lam>=0&&1+P+ex<=32,"Invalid ARX model");
    std::size_t T=0;bool test=false;Vec history;
    for(auto&r:x){
        require(r[0]==0||r[0]==1,"Binary train flag");
        if(!r[0])test=true;
        else{require(!test,"Training must be a prefix");++T;history.push_back(r[1]);}
    }
    require(T>P+1&&T<x.size()&&double(T)*std::pow(1+P+ex,2)<=5e7,"Insufficient ARX training");
    auto design=[&](std::size_t t,const Vec& h){
        Vec z{1};for(std::size_t j=1;j<=P;++j)z.push_back(h[t-j]);
        z.insert(z.end(),x[t].begin()+2,x[t].end());return z;
    };
    Mat X;Vec y;
    for(std::size_t t=P;t<T;++t){X.push_back(design(t,history));y.push_back(history[t]);}
    Vec w=fit_ridge(X,y,lam,true);double rss=0;
    for(std::size_t i=0;i<y.size();++i)rss+=std::pow(y[i]-dot(X[i],w),2);
    Report out{{"Horizon","Actual","Prediction","Error","TrainingRSS","Intercept"},{}};
    for(std::size_t j=1;j<=P;++j)out.header.push_back("AR"+std::to_string(j));
    for(std::size_t j=1;j<=ex;++j)out.header.push_back("Exog"+std::to_string(j));
    for(std::size_t t=T;t<x.size();++t){
        double f=dot(design(t,history),w);history.push_back(f);
        Vec r{double(t-T+1),x[t][1],f,x[t][1]-f,rss};r.insert(r.end(),w.begin(),w.end());out.rows.push_back(r);
    }
    return out;
}

inline Report arma11_profile_grid(const Mat& x,const Vec& p){
    validate(x,3,1,1);parameters(p,6);
    double lo=p[0],hi=p[1],ml=p[3],mh=p[4];
    auto G=count(p[2],2,1000),H=count(p[5],2,1000);
    require(lo<=hi&&ml<=mh&&lo>-1&&hi<1&&ml>-1&&mh<1&&double(G)*H*x.size()<=5e7,"Invalid ARMA grid");
    Vec y=column(x,0),best;double score=std::numeric_limits<double>::infinity(),n=y.size()-1;
    for(std::size_t g=0;g<G;++g)for(std::size_t h=0;h<H;++h){
        double phi=lo+(hi-lo)*g/(G-1),theta=ml+(mh-ml)*h/(H-1);
        Vec a(y.size(),0),b(y.size(),0);double ab=0,bb=0;
        for(std::size_t t=1;t<y.size();++t){
            a[t]=y[t]-phi*y[t-1]-theta*a[t-1];
            b[t]=1-phi-theta*b[t-1];ab+=a[t]*b[t];bb+=b[t]*b[t];
        }
        double mu=ab/positive(bb),s=0;
        for(std::size_t t=1;t<y.size();++t)s+=std::pow(a[t]-b[t]*mu,2);
        double var=s/n;if(!(var>0))continue;
        double f=.5*n*(1+std::log(2*std::acos(-1.)*var));
        if(f<score){score=f;best={phi,theta,mu,var,f,mu+phi*(y.back()-mu)+theta*(a.back()-b.back()*mu),n};}
    }
    require(!best.empty(),"No positive innovation variance");
    return scalar({"AR","MA","Mean","InnovationVariance","NLL","NextForecast","ConditionalObservations"},best);
}

inline Report garch_targeted_grid(const Mat& x,const Vec& p){
    validate(x,3,2,2);parameters(p,6);
    auto G=count(p[2],2,1000),H=count(p[5],2,1000);
    require(p[0]>=0&&p[1]>=p[0]&&p[3]>=0&&p[4]>=p[3]&&double(G)*H*x.size()<=5e7,"Invalid GARCH grid");
    std::size_t T=0;bool test=false;double variance=0;
    for(auto&r:x){
        require(r[0]==0||r[0]==1,"Binary train flag");
        if(!r[0])test=true;
        else{require(!test,"Training must be a prefix");++T;variance+=r[1]*r[1];}
    }
    require(T>=3,"Three training returns required");variance=positive(variance/T);
    double best=std::numeric_limits<double>::infinity(),a=0,b=0,w=0;
    for(std::size_t i=0;i<G;++i)for(std::size_t j=0;j<H;++j){
        double alpha=p[0]+(p[1]-p[0])*i/(G-1),beta=p[3]+(p[4]-p[3])*j/(H-1);
        if(alpha+beta>=1)continue;
        double omega=(1-alpha-beta)*variance,h=variance,f=0;
        for(std::size_t t=0;t<T;++t){
            double r=x[t][1];f+=.5*(std::log(2*std::acos(-1.)*h)+r*r/h);h=omega+alpha*r*r+beta*h;
        }
        if(f<best){best=f;a=alpha;b=beta;w=omega;}
    }
    require(std::isfinite(best),"No stationary candidate");
    Report out{{"Period","Train","Return","ForecastVariance","StandardizedShock","Omega","Alpha","Beta","TrainingNLL"},{}};
    double h=variance;
    for(std::size_t t=0;t<x.size();++t){
        double r=x[t][1];out.rows.push_back({double(t+1),x[t][0],r,h,r/std::sqrt(h),w,a,b,best});h=w+a*r*r+b*h;
    }
    return out;
}

inline Report threshold_ar_fit(const Mat& x,const Vec& p){
    validate(x,5,2,2);parameters(p,5);
    double lo=p[0],hi=p[1],lam=p[3];auto G=count(p[2],2,10000),minn=count(p[4],2,x.size());
    require(hi>=lo&&lam>=0&&double(G)*x.size()<=5e7,"Invalid TAR grid");
    std::size_t T=0;bool test=false;
    for(auto&r:x){require(r[0]==0||r[0]==1,"Binary train flag");if(!r[0])test=true;else{require(!test,"Training must be a prefix");++T;}}
    require(T>=2*minn+1,"Insufficient TAR training");
    double best=std::numeric_limits<double>::infinity(),cut=0,rss=0;Mat beta;
    for(std::size_t g=0;g<G;++g){
        double c=lo+(hi-lo)*g/(G-1);
        std::vector<Mat> X(2);std::vector<Vec> y(2);
        for(std::size_t t=1;t<T;++t){auto s=std::size_t(x[t-1][1]>=c);X[s].push_back({1,x[t-1][1]});y[s].push_back(x[t][1]);}
        if(X[0].size()<minn||X[1].size()<minn)continue;
        Mat w;double ss=0;
        try{
            for(int s=0;s<2;++s){w.push_back(fit_ridge(X[s],y[s],lam,true));for(std::size_t i=0;i<y[s].size();++i)ss+=std::pow(y[s][i]-dot(X[s][i],w[s]),2);}
        }catch(const std::exception&){continue;}
        double f=ss+lam*(w[0][1]*w[0][1]+w[1][1]*w[1][1]);
        if(f<best){best=f;cut=c;rss=ss;beta=w;}
    }
    require(!beta.empty(),"No eligible threshold");
    Report out{{"Period","Train","Regime","Actual","Forecast","Threshold","TrainingRSS","InterceptLow","ARLow","InterceptHigh","ARHigh"},{}};
    for(std::size_t t=1;t<x.size();++t){
        auto s=std::size_t(x[t-1][1]>=cut);double f=beta[s][0]+beta[s][1]*x[t-1][1];
        out.rows.push_back({double(t+1),x[t][0],double(s),x[t][1],f,cut,rss,beta[0][0],beta[0][1],beta[1][0],beta[1][1]});
    }
    return out;
}

inline Report dynamic_factor_sequential(const Mat& x,const Vec& p){
    auto cols=validate(x,1,3,65);require((cols-1)%2==0,"Observation pairs required");
    auto d=(cols-1)/2;parameters(p,4+2*d);
    double phi=p[0],Q=p[1],m=p[2],P=positive(p[3]);
    require(std::abs(phi)<=1&&Q>=0,"Invalid factor dynamics");
    for(std::size_t j=0;j<d;++j)positive(p[4+d+j]);
    double cum=0,work=0;Report out{{"Period","Gap","PriorMean","PriorVariance","PosteriorMean","PosteriorVariance","ObservedCount","InnovationNLL","CumulativeNLL"},{}};
    for(std::size_t t=0;t<x.size();++t){
        auto&r=x[t];auto gap=count(r[0],0,100000);
        require(t==0||gap>0,"Increasing observation time required");work+=gap;require(work<=5e7,"Factor propagation limit");
        for(std::size_t h=0;h<gap;++h){m*=phi;P=phi*phi*P+Q;}
        double pm=m,pv=P,ll=0,n=0;
        for(std::size_t j=0;j<d;++j){
            double flag=r[2+2*j];require(flag==0||flag==1,"Binary observation flag");
            if(flag){
                double b=p[4+j],R=p[4+d+j],S=positive(b*b*P+R),e=r[1+2*j]-b*m;
                ll+=.5*(std::log(2*std::acos(-1.)*S)+e*e/S);
                m+=P*b*e/S;P=P*R/S;++n;
            }
        }
        cum+=ll;out.rows.push_back({double(t+1),double(gap),pm,pv,m,P,n,ll,cum});
    }
    return out;
}

inline Report local_trend_filter_grid(const Mat& x,const Vec& p){
    validate(x,2,3,3);parameters(p,11);
    auto G=count(p[2],2,1000),H=count(p[5],2,1000);
    require(p[0]>=0&&p[1]>=p[0]&&p[3]>=0&&p[4]>=p[3]&&double(G)*H*x.size()<=5e7,"Invalid trend variance grid");
    double R=positive(p[6]),p0=positive(p[9]),p1=positive(p[10]);
    bool test=false;std::size_t T=0,observed=0;
    for(auto&r:x){
        require((r[0]==0||r[0]==1)&&(r[1]==0||r[1]==1),"Binary flags required");
        if(!r[0])test=true;
        else{require(!test,"Training must be a prefix");++T;observed+=std::size_t(r[1]);}
    }
    require(observed>0,"Observed training sample required");
    auto filter=[&](double ql,double qb,std::size_t n,Mat* rows){
        double a=p[7],b=p[8],u=p0,v=0,w=p1,loss=0;
        for(std::size_t t=0;t<n;++t){
            if(t){a+=b;u+=2*v+w+ql;v+=w;w+=qb;}
            double A=a,B=b,S=positive(u+R),ll=0;
            if(x[t][1]){
                double e=x[t][2]-a;ll=.5*(std::log(2*std::acos(-1.)*S)+e*e/S);
                a+=u*e/S;b+=v*e/S;
                double next=w-v*v/S;require(next>=-1e-12*std::max(1.,w),"Negative trend variance");
                w=std::max(0.,next);v=v*R/S;u=u*R/S;
            }
            loss+=ll;
            if(rows)rows->push_back({double(t+1),x[t][0],x[t][1],A,B,S,a,b,u,w,v,ll,ql,qb,0});
        }
        return loss;
    };
    double best=std::numeric_limits<double>::infinity(),ql=0,qb=0;
    for(std::size_t i=0;i<G;++i)for(std::size_t j=0;j<H;++j){
        double a=p[0]+(p[1]-p[0])*i/(G-1),b=p[3]+(p[4]-p[3])*j/(H-1),f=filter(a,b,T,nullptr);
        if(f<best){best=f;ql=a;qb=b;}
    }
    Report out{{"Period","Train","Observed","PriorLevel","PriorSlope","PredictionVariance","FilteredLevel","FilteredSlope","LevelVariance","SlopeVariance","Covariance","InnovationNLL","QLevel","QSlope","TrainingNLL"},{}};
    filter(ql,qb,x.size(),&out.rows);for(auto&r:out.rows)r.back()=best;
    return out;
}

inline Report markov_cumulative_reward(const Mat& x,const Vec& p){
    auto d=validate(x,1,4,35)-3;require(x.size()==d&&d<=32,"Square transition required");
    parameters(p,1);auto H=count(p[0],1,10000);require(double(H)*d*d<=5e7,"Markov work limit");
    Mat P;Vec mu,var,prob;
    for(auto&r:x){
        P.emplace_back(r.begin(),r.begin()+d);for(double v:P.back())require(v>=0,"Nonnegative transition required");
        require(std::abs(sum(P.back())-1)<=1e-10&&r[d+1]>=0&&r[d+2]>=0,"Invalid state distribution");
        mu.push_back(r[d]);var.push_back(r[d+1]);prob.push_back(r[d+2]);
    }
    require(std::abs(sum(prob)-1)<=1e-10,"Initial mass must sum to one");
    Vec a(d,0),b(d,0);Report out{{"Horizon","MeanCumulative","VarianceCumulative","MeanNextReward"},{}};
    for(std::size_t i=1;i<=d;++i)out.header.push_back("StateProbability"+std::to_string(i));
    auto transition=[&](const Vec& z){Vec w(d,0);for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<d;++j)w[j]+=z[i]*P[i][j];return w;};
    for(std::size_t h=1;h<=H;++h){
        for(std::size_t i=0;i<d;++i){b[i]+=2*mu[i]*a[i]+prob[i]*(mu[i]*mu[i]+var[i]);a[i]+=prob[i]*mu[i];}
        double m=sum(a),second=sum(b),v=second-m*m;require(v>=-1e-10*std::max(1.,std::abs(second)),"Negative cumulative variance");
        Vec next=transition(prob),r{double(h),m,std::max(0.,v),dot(next,mu)};r.insert(r.end(),prob.begin(),prob.end());out.rows.push_back(r);
        prob=next;a=transition(a);b=transition(b);
    }
    return out;
}
} }
