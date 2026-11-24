#pragma once
#include "evidence_common.hpp"
namespace kline { namespace comparison {
using namespace evidence;
inline Report brier_groups(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,0);
    std::map<double,Vec> groups;
    double n=double(x.size()),base=0,score=0;
    for(const auto& r:x){
        require(r[0]>=0&&r[0]<=1&&(r[1]==0||r[1]==1),"Invalid probability or outcome");
        groups[r[0]].push_back(r[1]);base+=r[1]/n;score+=(r[0]-r[1])*(r[0]-r[1])/n;
    }
    double rel=0,res=0,unc=base*(1-base);
    for(const auto& g:groups){
        double m=mean(g.second),w=double(g.second.size())/n;
        rel+=w*(g.first-m)*(g.first-m);res+=w*(m-base)*(m-base);
    }
    return scalar({"Brier","Reliability","Resolution","Uncertainty","IdentityResidual"},{score,rel,res,unc,score-(rel-res+unc)});
}
inline Report forecast_revisions(const Mat& x,const Vec& p){
    auto d=validate(x,2,3,64);parameters(p,0);
    Report out{{"Revision","MeanRevision","MeanFinalError","CrossMoment","RevisionSecondMoment"},{}};
    for(std::size_t j=1;j+1<d;++j){
        double a=0,b=0,c=0,v=0,n=double(x.size());
        for(const auto& r:x){
            double rev=r[j]-r[j-1],err=r.back()-r[d-2];
            a+=rev/n;b+=err/n;c+=rev*err/n;v+=rev*rev/n;
        }
        out.rows.push_back({double(j),a,b,c,v});
    }
    return out;
}
inline Report encompassing(const Mat& x,const Vec& p){
    validate(x,3,3,3);parameters(p,0);
    double n=double(x.size()),md=0,me=0;
    for(const auto& r:x){md+=(r[2]-r[1])/n;me+=(r[0]-r[1])/n;}
    double xx=0,xy=0;
    for(const auto& r:x){double d=r[2]-r[1]-md,e=r[0]-r[1]-me;xx+=d*d;xy+=d*e;}
    positive(xx);double slope=xy/xx,intercept=me-slope*md,sse=0;
    for(const auto& r:x){double e=r[0]-r[1]-intercept-slope*(r[2]-r[1]);sse+=e*e;}
    double se=std::sqrt(positive(sse)/(n-2)/xx);
    return scalar({"Intercept","Slope","SlopeSE","TZero","TOne"},{intercept,slope,se,slope/se,(slope-1)/se});
}
inline Report murphy_quantile(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,4);
    double a=p[0],lo=p[1],hi=p[2];auto k=count(p[3],2,10001);
    require(a>0&&a<1&&lo<hi&&double(k)*x.size()<=2e7,"Invalid quantile score grid");
    Report out{{"Threshold","ScoreA","ScoreB","Difference"},{}};
    for(std::size_t j=0;j<k;++j){
        double theta=lo+(hi-lo)*double(j)/double(k-1),s[2]={0,0};
        for(const auto& r:x)for(int h=0;h<2;++h){
            double y=r[0],q=r[h+1];
            s[h]+=(double(y<q)-a)*(double(theta<q)-double(theta<y))/double(x.size());
        }
        out.rows.push_back({theta,s[0],s[1],s[0]-s[1]});
    }
    return out;
}
} }
