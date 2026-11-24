#pragma once
#include "evidence_common.hpp"
namespace kline { namespace methods_macroeconomics {
using namespace evidence;
inline Report debt_dynamics(const Mat& x,const Vec& p){
    validate(x,1,4,4);
    parameters(p,1);
    double b=p[0];
    require(b>=0,"Negative initial debt ratio");
    Report o{{"Period","PreviousDebt","InterestGrowthEffect","PrimaryEffect","StockFlow","DebtRatio"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){auto&r=x[i];
    positive(1+r[0]);
    double g=positive(1+r[1]),e=(r[0]-r[1])/g*b,v=b+e-r[2]+r[3];
    o.rows.push_back({double(i+1),b,e,-r[2],r[3],v});
    b=v;
    }return o;
    
}

inline Report gdp_growth_contributions(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,0);
    double s=0;
    for(auto&r:x){s+=positive(r[0]);
    require(r[1]>=0,"Negative current component");
    }Report o{{"Component","BaseShare","OwnGrowth","Contribution"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){auto&r=x[i];
    double a=r[0]/s,b=r[1]/r[0]-1;
    o.rows.push_back({double(i+1),a,b,(r[1]-r[0])/s});
    }return o;
    
}

inline Report real_fx_decomposition(const Mat& x,const Vec& p){
    validate(x,1,3,3);
    parameters(p,0);
    Report o{{"Period","RealFX","LogNominalChange","LogForeignInflation","LogDomesticInflation","LogRealChange"},{}};
    for(std::size_t i=0;
    i<x.size();
    ++i){auto&r=x[i];
    double a=positive(r[0]),b=positive(r[1]),c=positive(r[2]),u=0,v=0,w=0;
    if(i){u=std::log(a/x[i-1][0]);
    v=std::log(b/x[i-1][1]);
    w=std::log(c/x[i-1][2]);
    }o.rows.push_back({double(i+1),a*b/c,u,v,w,u+v-w});
    }return o;
    
}

inline Report uip_carry(const Mat& x,const Vec& p){
    validate(x,1,4,4);
    parameters(p,0);
    Report o{{"DomesticGross","ForeignInDomesticGross","ArithmeticExcess","LogExcess"},{}};
    for(auto&r:x){double a=positive(1+r[2]),b=positive(1+r[3])*positive(r[1])/positive(r[0]);
    o.rows.push_back({a,b,b-a,std::log(b/a)});
    }return o;
    
}
} }
