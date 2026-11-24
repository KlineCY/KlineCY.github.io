#pragma once
#include "evidence_common.hpp"
namespace kline { namespace applied_allocation {
using namespace evidence;
inline Report brinson_fachler(const Mat& x,const Vec& p){
    validate(x,1,4,4);
    parameters(p,0);
    double wp=0,wb=0,rb=0;
    for(auto&r:x){require(r[0]>=0&&r[1]>=0&&r[2]>=-1&&r[3]>=-1,"Invalid group");
    wp+=r[0];
    wb+=r[1];
    rb+=r[1]*r[3];
    }require(std::abs(wp-1)<1e-9&&std::abs(wb-1)<1e-9,"Weights must sum to one");
    Report o{{"Group","Allocation","Selection","Interaction","Total"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){auto&r=x[i];
    double d=r[0]-r[1],e=r[2]-r[3],a=d*(r[3]-rb),s=r[1]*e,z=d*e;
    o.rows.push_back({double(i+1),a,s,z,a+s+z});
    }return o;
    
}

inline Report carino(const Mat& x,const Vec& p){
    validate(x,1,4,4);
    parameters(p,0);
    auto k=[](double a,double b){positive(1+a);
    positive(1+b);
    double d=a-b;
    return d==0?1/(1+b):std::log1p(d/(1+b))/d;
    };
    double a=0,b=0;
    for(auto&r:x){positive(1+r[0]);
    positive(1+r[1]);
    require(std::abs(r[2]+r[3]-(r[0]-r[1]))<1e-9,"Contributions do not reconcile");
    a+=std::log1p(r[0]);
    b+=std::log1p(r[1]);
    }double K=k(std::expm1(a),std::expm1(b));
    Report o{{"Period","LocalK","GlobalK","LinkedA","LinkedB"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){auto&r=x[i];
    double u=k(r[0],r[1]);
    o.rows.push_back({double(i+1),u,K,r[2]*u/K,r[3]*u/K});
    }return o;
    
}

inline Report daily_leverage(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,2);
    double L=p[0],c=p[1],s=1,v=1;
    require(c>=0&&c<1,"Invalid cost");
    Report o{{"Period","UnderlyingNAV","LeveragedNAV","StaticMultipleNAV","Gap"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){double r=x[i][0];
    s*=positive(1+r);
    v*=positive(1+L*r-c);
    double u=1+L*(s-1);
    o.rows.push_back({double(i+1),s,v,u,v-u});
    }return o;
    
}

inline Report active_wealth(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,0);
    double a=1,b=1;
    Report o{{"Period","PortfolioNAV","BenchmarkNAV","ArithmeticActive","RelativeWealth"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){a*=positive(1+x[i][0]);
    b*=positive(1+x[i][1]);
    o.rows.push_back({double(i+1),a,b,a-b,a/b-1});
    }return o;
    
}
} }
