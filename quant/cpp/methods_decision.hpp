#pragma once
#include "evidence_common.hpp"
namespace kline { namespace methods_decision {
using namespace evidence;
inline Report log_utility_grid(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,3);
    auto k=count(p[2],2,10001);
    require(p[0]<=p[1]&&double(k)*x.size()<=2e7,"Invalid grid");
    double s=0;
    for(auto&r:x){require(r[1]>=0&&r[1]<=1,"Invalid probability");
    s+=r[1];
    }require(std::abs(s-1)<1e-9,"Probability sum");
    Report o{{"Weight","ExpectedLog","CertaintyEquivalent"},{}};
    for(std::size_t j=0;
    j<k;
    ++j){double w=p[0]+(p[1]-p[0])*j/(k-1),v=0;
    for(auto&r:x){if(r[1]>0)v+=r[1]*std::log(positive(1+w*r[0]));
    }o.rows.push_back({w,v,std::expm1(v)});
    }return o;
    
}

inline Report cara_ce(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,1);
    double g=positive(p[0]),m=0,s=0,z=-std::numeric_limits<double>::infinity();
    for(auto&r:x){require(r[1]>=0&&r[1]<=1,"Invalid probability");
    s+=r[1];
    m+=r[0]*r[1];
    if(r[1]>0)z=std::max(z,std::log(r[1])-g*r[0]);
    }require(std::abs(s-1)<1e-9,"Probability sum");
    double q=0;
    for(auto&r:x)if(r[1]>0)q+=std::exp(std::log(r[1])-g*r[0]-z);
    double ce=-(z+std::log(q))/g;
    return scalar({"Mean","CertaintyEquivalent","RiskPremium"},{m,ce,m-ce});
    
}

inline Report regret_matrix(const Mat& x,const Vec& p){
    auto d=validate(x,1,2,64);
    parameters(p,0);
    Vec m(d,0),s(d,0);
    for(auto&r:x){double z=*std::max_element(r.begin(),r.end());
    for(std::size_t j=0;
    j<d;
    ++j){double v=z-r[j];
    m[j]=std::max(m[j],v);
    s[j]+=v;
    }}double b=*std::min_element(m.begin(),m.end());
    Report o{{"Action","MaximumRegret","MeanRegret","Minimax"},{}};
    for(std::size_t j=0;
    j<d;
    ++j)o.rows.push_back({double(j+1),m[j],s[j]/x.size(),double(m[j]==b)});
    return o;
    
}

inline Report value_information(const Mat& x,const Vec& p){
    auto d=validate(x,1,3,64);
    parameters(p,0);
    Vec a(d-1,0);
    double s=0,v=0;
    for(auto&r:x){require(r[0]>=0&&r[0]<=1,"Invalid probability");
    s+=r[0];
    v+=r[0]*(*std::max_element(r.begin()+1,r.end()));
    for(std::size_t j=1;
    j<d;
    ++j)a[j-1]+=r[0]*r[j];
    }require(std::abs(s-1)<1e-9,"Probability sum");
    double b=*std::max_element(a.begin(),a.end());
    return scalar({"BestFixedExpectedPayoff","PerfectInformationPayoff","EVPI"},{b,v,v-b});
    
}
} }
