#pragma once
#include "advanced_research_common.hpp"
#include <map>
#include <tuple>
namespace kline { namespace advanced_research {
inline Report tyler_shape(const Mat& x,const Vec& p){
    auto d=validate(x,2,1,24);parameters(p,3);
    double delta=p[0],tol=positive(p[1]);auto K=count(p[2],1,100000);
    require(delta>0&&delta<=1&&double(K)*(double(x.size())*d*d+d*d*d*d)<=1e8,"Invalid Tyler controls");
    for(auto&r:x)positive(squared(r));
    Mat s=identity(d);
    auto update=[&](const Mat& v){
        Mat inv=invert_spd(v),a(d,Vec(d,0));
        for(auto&r:x)add_outer(a,r,(1-delta)*d/(x.size()*positive(dot(r,mul(inv,r)))));
        for(std::size_t j=0;j<d;++j)a[j][j]+=delta;
        double trace=0;for(std::size_t j=0;j<d;++j)trace+=a[j][j];
        for(auto&row:a)for(double&z:row)z*=d/positive(trace);
        return a;
    };
    std::size_t it=0;double gap=0;
    for(;it<K;){auto a=update(s);gap=matrix_error(a,s);++it;if(gap<=tol)break;s=std::move(a);}
    require(gap<=tol,"Tyler fixed point did not converge");
    double trace=0;for(std::size_t j=0;j<d;++j)trace+=s[j][j];
    Report o{{"Row","Column","Shape","Trace","FixedPointResidual","Iterations"},{}};
    for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<d;++j)o.rows.push_back({double(i+1),double(j+1),s[i][j],trace,gap,double(it)});
    return o;
}

inline Report linear_gmm_two_step(const Mat& x,const Vec& p){
    auto cols=validate(x,4,3,33);parameters(p,2);
    auto d=count(p[0],1,cols-2),q=cols-d-1,n=x.size();double ridge=positive(p[1]);
    require(q>=d&&n>q&&q<=16&&double(n)*q*q<=5e7,"Invalid GMM dimensions");
    Mat A(q,Vec(d,0)),X,Z;Vec b(q,0),y;
    for(auto&r:x){
        y.push_back(r[0]);X.emplace_back(r.begin()+1,r.begin()+1+d);Z.emplace_back(r.begin()+1+d,r.end());
        for(std::size_t j=0;j<q;++j){b[j]+=Z.back()[j]*r[0]/n;for(std::size_t k=0;k<d;++k)A[j][k]+=Z.back()[j]*X.back()[k]/n;}
    }
    auto moments=[&](const Vec& beta){
        Mat S(q,Vec(q,0));for(std::size_t i=0;i<n;++i)add_outer(S,Z[i],std::pow(y[i]-dot(X[i],beta),2)/n);return S;
    };
    auto fit=[&](const Mat&W){
        Mat WA(q,Vec(d,0)),M(d,Vec(d,0));Vec wb=mul(W,b),rhs(d,0);
        for(std::size_t i=0;i<q;++i)for(std::size_t j=0;j<d;++j)for(std::size_t k=0;k<q;++k)WA[i][j]+=W[i][k]*A[k][j];
        for(std::size_t i=0;i<d;++i)for(std::size_t k=0;k<q;++k){rhs[i]+=A[k][i]*wb[k];for(std::size_t j=0;j<d;++j)M[i][j]+=A[k][i]*WA[k][j];}
        return std::make_tuple(solve_spd(M,rhs),M,WA);
    };
    auto first=fit(identity(q));Mat S=moments(std::get<0>(first));for(std::size_t j=0;j<q;++j)S[j][j]+=ridge;
    Mat W=invert_spd(S);auto second=fit(W);Vec beta=std::get<0>(second);
    Mat M=std::get<1>(second),WA=std::get<2>(second),S2=moments(beta),Mi=invert_spd(M);
    Vec g=b,v=mul(A,beta);for(std::size_t j=0;j<q;++j)g[j]-=v[j];
    Vec wg=mul(W,g),score(d,0);
    for(std::size_t j=0;j<d;++j)for(std::size_t k=0;k<q;++k)score[j]+=A[k][j]*wg[k];
    Report o{{"Feature","Coefficient","RobustSE","JStatistic","OveridentifyingDF","MomentNorm","ScoreResidual"},{}};
    for(std::size_t j=0;j<d;++j){
        Vec a=mul(WA,Mi[j]);double var=dot(a,mul(S2,a))/n;
        o.rows.push_back({double(j+1),beta[j],std::sqrt(std::max(0.,var)),n*dot(g,wg),double(q-d),std::sqrt(squared(g)),inf_norm(score)});
    }
    return o;
}

inline Report ar1_gls_profile(const Mat& x,const Vec& p){
    auto d=validate(x,3,2,25)-1;parameters(p,3);
    double lo=p[0],hi=p[1];auto g=count(p[2],2,10000),n=x.size();
    require(lo>-1&&hi<1&&lo<=hi&&n>d&&double(g)*n*d*d<=5e7,"Invalid GLS grid");
    double best=std::numeric_limits<double>::infinity(),rho=0,var=0,gap=0;Vec beta;
    for(std::size_t k=0;k<g;++k){
        double a=lo+(hi-lo)*k/(g-1),first=std::sqrt((1-a)*(1+a));Mat X;Vec y;
        for(std::size_t t=0;t<n;++t){
            Vec z(d);for(std::size_t j=0;j<d;++j)z[j]=t?x[t][j+1]-a*x[t-1][j+1]:first*x[t][j+1];
            X.push_back(z);y.push_back(t?x[t][0]-a*x[t-1][0]:first*x[t][0]);
        }
        Vec w=fit_ridge(X,y,0,false),score(d,0);double s=0;
        for(std::size_t t=0;t<n;++t){double e=y[t]-dot(X[t],w);s+=e*e/n;for(std::size_t j=0;j<d;++j)score[j]+=X[t][j]*e;}
        positive(s);double f=.5*n*(1+std::log(2*std::acos(-1.)*s))-.5*std::log1p(-a*a);
        if(f<best){best=f;beta=w;rho=a;var=s;gap=inf_norm(score);}
    }
    Report o{{"Feature","Coefficient","Rho","InnovationVariance","ProfileNLL","ScoreResidual"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),beta[j],rho,var,best,gap});
    return o;
}

inline Report random_intercept_reml(const Mat& x,const Vec& p){
    validate(x,3,3,3);parameters(p,3);
    double lo=p[0],hi=p[1];auto g=count(p[2],2,10000);
    require(lo>=0&&hi>=lo,"Invalid variance ratio grid");
    struct Group{double n=0,s=0,ss=0;};std::map<double,Group> groups;
    for(auto&r:x){require(r[0]==std::floor(r[0])&&r[2]==1,"Integer group and intercept design required");auto&v=groups[r[0]];++v.n;v.s+=r[1];v.ss+=r[1]*r[1];}
    require(groups.size()>=2&&double(g)*groups.size()<=5e7,"Invalid group count");
    double best=std::numeric_limits<double>::infinity(),mean=0,var=0,ratio=0,n=x.size();
    for(std::size_t k=0;k<g;++k){
        double a=lo+(hi-lo)*k/(g-1),den=0,num=0,ld=0;
        for(auto&kv:groups){auto v=kv.second;double h=1+a*v.n;den+=v.n/h;num+=v.s/h;ld+=std::log(h);}
        double m=num/positive(den),ss=0;
        for(auto&kv:groups){auto v=kv.second;double centered=v.s-v.n*m;ss+=std::max(0.,v.ss-v.s*v.s/v.n)+centered*centered/(v.n*(1+a*v.n));}
        double e=positive(ss/(n-1)),f=.5*((n-1)*(1+std::log(2*std::acos(-1.)*e))+ld+std::log(den));
        if(f<best){best=f;mean=m;var=e;ratio=a;}
    }
    Report o{{"Group","FixedIntercept","RandomIntercept","ResidualVariance","VarianceRatio","RestrictedNLL"},{}};
    for(auto&kv:groups){auto v=kv.second;double b=ratio*(v.s-v.n*mean)/(1+ratio*v.n);o.rows.push_back({kv.first,mean,b,var,ratio,best});}
    return o;
}

inline Report sur_feasible_gls(const Mat& x,const Vec& p){
    auto cols=validate(x,3,4,34);parameters(p,3);
    auto d1=count(p[0],1,16),d2=count(p[1],1,16),d=d1+d2,n=x.size();
    double ridge=positive(p[2]);require(cols==d+2&&n>std::max(d1,d2)&&double(n)*d*d<=5e7,"Invalid SUR dimensions");
    Mat X,Z;Vec y,z;
    for(auto&r:x){y.push_back(r[0]);z.push_back(r[1]);X.emplace_back(r.begin()+2,r.begin()+2+d1);Z.emplace_back(r.begin()+2+d1,r.end());}
    Vec a=fit_ridge(X,y,0,false),b=fit_ridge(Z,z,0,false);Mat S(2,Vec(2,0));
    for(std::size_t i=0;i<n;++i)add_outer(S,{y[i]-dot(X[i],a),z[i]-dot(Z[i],b)},1./n);
    S[0][0]+=ridge;S[1][1]+=ridge;Mat W=invert_spd(S),H(d,Vec(d,0));Vec rhs(d,0);
    auto design=[&](std::size_t i){Mat D(2,Vec(d,0));for(std::size_t j=0;j<d1;++j)D[0][j]=X[i][j];for(std::size_t j=0;j<d2;++j)D[1][d1+j]=Z[i][j];return D;};
    for(std::size_t t=0;t<n;++t){
        Mat D=design(t);Vec Wy=mul(W,{y[t],z[t]});
        for(std::size_t i=0;i<d;++i){
            rhs[i]+=D[0][i]*Wy[0]+D[1][i]*Wy[1];
            for(std::size_t j=0;j<d;++j)for(std::size_t k=0;k<2;++k)for(std::size_t h=0;h<2;++h)H[i][j]+=D[k][i]*W[k][h]*D[h][j];
        }
    }
    Vec beta=solve_spd(H,rhs),score=mul(H,beta);for(std::size_t j=0;j<d;++j)score[j]-=rhs[j];
    Report o{{"Equation","Feature","Coefficient","ResidualCov11","ResidualCov12","ResidualCov22","ScoreResidual"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({j<d1?1.:2.,double(j<d1?j+1:j-d1+1),beta[j],S[0][0],S[0][1],S[1][1],inf_norm(score)});
    return o;
}

inline Report james_stein_means(const Mat& x,const Vec& p){
    validate(x,3,1,1);parameters(p,2);double v=positive(p[0]),target=p[1],dist=0;
    for(auto&r:x)dist+=(r[0]-target)*(r[0]-target);
    double a=dist==0?0:std::max(0.,1-(x.size()-2)*v/dist);
    Report o{{"Index","RawEstimate","ShrunkEstimate","ShrinkMultiplier","RawSquaredDistance","KnownVariance"},{}};
    for(std::size_t j=0;j<x.size();++j)o.rows.push_back({double(j+1),x[j][0],target+a*(x[j][0]-target),a,dist,v});
    return o;
}
} }
