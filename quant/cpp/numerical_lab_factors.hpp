#pragma once
#include "numerical_lab_inference.hpp"

namespace kline { namespace numerical_lab {
inline std::map<double,Mat> by_date(const Mat& x){
    std::map<double,Mat> groups;
    for(const auto& row:x)groups[row.front()].push_back(row);
    return groups;
}
inline Vec wls_cross_section(const Mat& x,std::size_t d){
    require(x.size()>d,"Insufficient cross-sectional observations");
    Mat A(d,Vec(d,0));Vec b(d,0);
    for(const auto& r:x){
        double weight=positive(r[1]);Vec z={1};z.insert(z.end(),r.begin()+2,r.end()-1);
        for(std::size_t i=0;i<d;++i){b[i]+=weight*z[i]*r.back();for(std::size_t j=0;j<d;++j)A[i][j]+=weight*z[i]*z[j];}
    }
    return solve_spd(A,b);
}
inline Report fama_macbeth_multi(const Mat& x,const Vec& p){
    auto d=validate(x,6,4,16)-2;parameters(p,1);
    auto groups=by_date(x);require(groups.size()>=3,"Three cross-sectional dates required");
    auto lag=count(p[0],0,groups.size()-1);Mat co;
    for(const auto& g:groups)co.push_back(wls_cross_section(g.second,d));
    Report out{{"Coefficient","MeanPremium","HACStandardError","NormalStatistic","Dates","CrossSectionPremiumSD"},{}};
    for(std::size_t j=0;j<d;++j){
        Vec v=column(co,j);double m=mean(v),se=positive(hac_mean_standard_error(v,int(lag)));
        out.rows.push_back({double(j),m,se,m/se,double(groups.size()),std::sqrt(variance(v))});
    }
    return out;
}
inline Report weighted_neutralization(const Mat& x,const Vec& p){
    auto d=validate(x,4,4,16)-2;parameters(p,0);
    auto groups=by_date(x);
    Report out{{"Date","WithinDateRow","RawSignal","FittedSignal","Residual","Weight"},{}};
    for(const auto& g:groups){
        auto beta=wls_cross_section(g.second,d);
        for(std::size_t j=0;j<g.second.size();++j){
            const auto& r=g.second[j];Vec z={1};z.insert(z.end(),r.begin()+2,r.end()-1);
            double fit=dot(z,beta);
            out.rows.push_back({g.first,double(j),r.back(),fit,r.back()-fit,r[1]});
        }
    }
    return out;
}
inline Report event_market_model(const Mat& x,const Vec& p){
    validate(x,8,3,3);parameters(p,4);
    auto est=count(p[0],3,x.size()-1),gap=count(p[1],0,x.size()-1),pre=count(p[2],0,x.size()-1),post=count(p[3],0,x.size()-1);
    Report out{{"EventIndex","RelativePeriod","ActualReturn","ExpectedReturn","AbnormalReturn","CAR","Alpha","Beta"},{}};
    for(const auto& row:x)require(row[2]==0||row[2]==1,"Binary event flag required");
    std::size_t lastend=0;bool first=true;
    for(std::size_t t=0;t<x.size();++t){
        if(x[t][2]==0)continue;
        require(t>=est+gap+pre&&post<x.size()-t,"Event window exceeds sample");
        require(first||t-pre>lastend,"Overlapping event windows");lastend=t+post;first=false;
        Vec a,b;std::size_t end=t-pre-gap;
        for(std::size_t s=end-est;s<end;++s){a.push_back(x[s][0]);b.push_back(x[s][1]);}
        double beta=covariance(a,b)/positive(variance(b)),alpha=mean(a)-beta*mean(b),car=0;
        for(std::size_t s=t-pre;s<=t+post;++s){
            double expected=alpha+beta*x[s][1],ab=x[s][0]-expected;car+=ab;
            out.rows.push_back({double(t),double(s)-double(t),x[s][0],expected,ab,car,alpha,beta});
        }
    }
    require(!out.rows.empty(),"No events");return out;
}
inline Report independent_double_sort(const Mat& x,const Vec& p){
    validate(x,4,4,4);parameters(p,0);
    Report out{{"Date","Signal1Group","Signal2Group","Count","MeanReturn"},{}};
    for(const auto& g:by_date(x)){
        auto a=column(g.second,1),b=column(g.second,2);
        require(a.size()>=4,"Four observations required per date");
        double m1=quantile(a,.5),m2=quantile(b,.5),s[2][2]={{0,0},{0,0}},n[2][2]={{0,0},{0,0}};
        for(const auto& r:g.second){
            if(r[1]==m1||r[2]==m2)continue;
            int i=r[1]>m1,j=r[2]>m2;s[i][j]+=r[3];++n[i][j];
        }
        for(int i=0;i<2;++i)for(int j=0;j<2;++j){
            require(n[i][j]>0,"Empty double-sort cell");
            out.rows.push_back({g.first,double(i),double(j),n[i][j],s[i][j]/n[i][j]});
        }
    }
    return out;
}
inline Report horizon_rank_ic(const Mat& x,const Vec& p){
    auto d=validate(x,3,3,32);parameters(p,0);
    Report out{{"Date","HorizonColumn","Observations","RankIC"},{}};
    for(const auto& g:by_date(x)){
        auto score=column(g.second,1);
        require(score.size()>=3,"Three assets required per date");
        for(std::size_t j=2;j<d;++j)out.rows.push_back({g.first,double(j-1),double(score.size()),spearman(score,column(g.second,j))});
    }
    return out;
}
} }
