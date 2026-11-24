#pragma once
#include "numerical_lab_common.hpp"
#include <random>
namespace kline { namespace numerical_lab {
struct LinearFit {Mat X,inv;Vec y,beta,residual,leverage;};
inline LinearFit ols(const Mat& x,std::size_t skip=0){
    const auto d=validate(x,4,skip+2,20)-skip;
    require(x.size()>d,"Residual degrees of freedom required");
    Mat X;Vec y;
    for(const auto& row:x){Vec v={1};v.insert(v.end(),row.begin()+skip,row.end()-1);X.push_back(v);y.push_back(row.back());}
    Mat A(d,Vec(d,0));Vec b(d,0);
    for(std::size_t t=0;t<X.size();++t){
        for(std::size_t i=0;i<d;++i){b[i]+=X[t][i]*y[t];for(std::size_t j=0;j<d;++j)A[i][j]+=X[t][i]*X[t][j];}
    }
    Vec beta=solve_spd(A,b);Mat inv(d,Vec(d,0));
    for(std::size_t j=0;j<d;++j){Vec e(d,0);e[j]=1;Vec z=solve_spd(A,e);for(std::size_t i=0;i<d;++i)inv[i][j]=z[i];}
    Vec residual,leverage;
    for(std::size_t i=0;i<X.size();++i){residual.push_back(y[i]-dot(X[i],beta));leverage.push_back(dot(X[i],mul(inv,X[i])));}
    return {X,inv,y,beta,residual,leverage};
}
inline void add_outer(Mat& A,const Vec& x,const Vec& y,double w=1){
    for(std::size_t i=0;i<x.size();++i)for(std::size_t j=0;j<y.size();++j)A[i][j]+=w*x[i]*y[j];
}
inline Report sandwich_report(const LinearFit& f,const Mat& meat,double scale){
    const auto d=f.beta.size();
    Report out{{"Coefficient","Estimate","StandardError","NormalStatistic","Lower95Normal","Upper95Normal","Observations","ResidualDF"},{}};
    for(std::size_t i=0;i<d;++i){
        double v=dot(f.inv[i],mul(meat,f.inv[i]))*scale;
        double se=std::sqrt(positive(v));
        out.rows.push_back({double(i),f.beta[i],se,f.beta[i]/se,f.beta[i]-1.959963984540054*se,f.beta[i]+1.959963984540054*se,double(f.X.size()),double(f.X.size()-d)});
    }
    return out;
}
inline Report multivariate_hc3(const Mat& x,const Vec& p){
    parameters(p,0);auto f=ols(x);auto d=f.beta.size();Mat M(d,Vec(d,0));
    for(std::size_t t=0;t<x.size();++t){
        double r=f.residual[t]/positive(1-f.leverage[t]);
        add_outer(M,f.X[t],f.X[t],r*r);
    }
    return sandwich_report(f,M,1);
}
inline Report multivariate_hac(const Mat& x,const Vec& p){
    parameters(p,1);auto f=ols(x);auto d=f.beta.size(),L=count(p[0],0,x.size()-1);
    require(double(x.size())*(L+1)*d*d<=1e8,"HAC work limit exceeded");
    Mat M(d,Vec(d,0)),scores=f.X;
    for(std::size_t t=0;t<x.size();++t){for(double& v:scores[t])v*=f.residual[t];add_outer(M,scores[t],scores[t]);}
    for(std::size_t k=1;k<=L;++k){
        double w=1-double(k)/(L+1);
        for(std::size_t t=k;t<x.size();++t){add_outer(M,scores[t],scores[t-k],w);add_outer(M,scores[t-k],scores[t],w);}
    }
    return sandwich_report(f,M,1);
}
inline Report multivariate_cluster(const Mat& x,const Vec& p){
    parameters(p,0);auto f=ols(x,1);auto d=f.beta.size();std::map<double,Vec> score;
    for(std::size_t i=0;i<x.size();++i){
        auto& v=score[x[i][0]];if(v.empty())v=Vec(d,0);
        for(std::size_t j=0;j<d;++j)v[j]+=f.X[i][j]*f.residual[i];
    }
    require(score.size()>=2,"At least two clusters required");
    Mat M(d,Vec(d,0));for(const auto& v:score)add_outer(M,v.second,v.second);
    double n=double(x.size()),G=double(score.size());
    return sandwich_report(f,M,G/(G-1)*(n-1)/(n-d));
}
inline Report wild_coefficient(const Mat& x,const Vec& p){
    parameters(p,4);auto f=ols(x);auto d=f.beta.size();
    auto j=count(p[0],0,d-1),B=count(p[2],100,20000),seed=count(p[3],0,4294967295ULL);
    double null=p[1],delta=f.beta[j]-null,den=positive(f.inv[j][j]);
    require(double(B)*x.size()*d<=1e8,"Bootstrap work limit exceeded");
    Vec restricted=f.beta;for(std::size_t k=0;k<d;++k)restricted[k]-=f.inv[k][j]*delta/den;
    Vec fitted,err;
    for(std::size_t i=0;i<x.size();++i){double m=dot(f.X[i],restricted);fitted.push_back(m);err.push_back(f.y[i]-m);}
    std::mt19937 rng(static_cast<std::uint32_t>(seed));std::size_t exceed=0;
    // Refit only the tested coefficient using the jth row of the fixed OLS operator.
    Vec influence;for(const auto& v:f.X)influence.push_back(dot(f.inv[j],v));
    for(std::size_t k=0;k<B;++k){
        double stat=0;
        for(std::size_t i=0;i<x.size();++i)stat+=influence[i]*(fitted[i]+((rng()&1U)?1:-1)*err[i]);
        if(std::abs(stat-null)>=std::abs(delta)-1e-12)++exceed;
    }
    return scalar_rows({"Coefficient","Estimate","Null","ObservedAbsDifference","Replications","Exceedances","MonteCarloP"},{double(j),f.beta[j],null,std::abs(delta),double(B),double(exceed),double(exceed+1)/(B+1)});
}
inline Report joint_wald_hc0(const Mat& x,const Vec& p){
    auto f=ols(x);auto d=f.beta.size();
    require(!p.empty()&&p.size()%2==0&&p.size()/2<=d,"Coefficient-null pairs required");check_finite(p);
    Mat M(d,Vec(d,0));for(std::size_t i=0;i<x.size();++i)add_outer(M,f.X[i],f.X[i],f.residual[i]*f.residual[i]);
    std::vector<std::size_t> ix;Vec diff;
    for(std::size_t i=0;i<p.size();i+=2){auto j=count(p[i],0,d-1);require(std::find(ix.begin(),ix.end(),j)==ix.end(),"Repeated coefficient restriction");ix.push_back(j);diff.push_back(f.beta[j]-p[i+1]);}
    Mat V(ix.size(),Vec(ix.size(),0));
    for(std::size_t i=0;i<ix.size();++i)for(std::size_t j=0;j<ix.size();++j)V[i][j]=dot(f.inv[ix[i]],mul(M,f.inv[ix[j]]));
    double stat=dot(diff,solve_spd(V,diff));
    return scalar_rows({"Restrictions","WaldChiSquareStatistic","Observations"},{double(ix.size()),stat,double(x.size())});
}
} }
