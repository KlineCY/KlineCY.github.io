#pragma once
#include "evidence_common.hpp"
namespace kline { namespace evidence {
inline Report call_implied_atoms(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,1);double D=positive(p[0]);
    require(x[0][0]==0&&x.back()[1]==0,"Zero strike and zero final call required");
    Report o{{"State","StatePrice","RiskNeutralProbability","MeanContribution"},{}};
    double prev=-D;
    for(std::size_t j=0;j<x.size();++j){
        require(x[j][1]>=0,"Negative option price");
        double next=j+1<x.size()?(x[j+1][1]-x[j][1])/positive(x[j+1][0]-x[j][0]):0;
        require(next>=prev-1e-12*D&&next<=1e-12*D,"Call curve violates convexity or monotonicity");
        double mass=std::max(0.0,next-prev);
        o.rows.push_back({x[j][0],mass,mass/D,x[j][0]*mass/D});prev=next;
    }
    return o;
}

inline Report complete_replication(const Mat& x,const Vec& p){
    auto d=validate(x,2,3,32)-1;parameters(p,d);require(x.size()==d,"Square payoff matrix required");
    Mat X;Vec h;
    for(const auto& r:x){X.emplace_back(r.begin(),r.end()-1);h.push_back(r.back());}
    Mat A(d,Vec(d,0));Vec b(d,0);
    for(std::size_t t=0;t<d;++t)for(std::size_t i=0;i<d;++i){b[i]+=X[t][i]*h[t];for(std::size_t j=0;j<d;++j)A[i][j]+=X[t][i]*X[t][j];}
    Vec w=solve_spd(A,b),z=solve_spd(A,p),q=mul(X,z);
    for(double a:q)positive(a);
    double value=dot(w,p);require(std::abs(value-dot(q,h))<=1e-8*std::max(1.0,std::abs(value)),"Replication pricing residual");
    Report o{{"Index","AssetHolding","StatePrice","ReplicatedPayoff","ClaimValue"},{}};
    for(std::size_t i=0;i<d;++i)o.rows.push_back({double(i+1),w[i],q[i],dot(X[i],w),value});
    return o;
}

inline Report incomplete_claim_bounds(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,2);double D=positive(p[1]),F=p[0]/D;
    require(x.size()<=5000,"Finite-state enumeration limit exceeded");
    for(std::size_t i=0;i<x.size();++i)require(x[i][0]>=0&&(i==0||x[i][0]>x[i-1][0]),"Increasing nonnegative states required");
    require(F>x.front()[0]&&F<x.back()[0],"Forward must be strictly inside state support");
    double lo=std::numeric_limits<double>::infinity(),hi=-lo;
    for(std::size_t i=0;i<x.size();++i){
        if(x[i][0]==F){lo=std::min(lo,D*x[i][1]);hi=std::max(hi,D*x[i][1]);}
        if(x[i][0]>F)break;
        for(std::size_t j=i+1;j<x.size();++j)if(x[j][0]>=F){
            double a=(x[j][0]-F)/(x[j][0]-x[i][0]),v=D*(a*x[i][1]+(1-a)*x[j][1]);
            lo=std::min(lo,v);hi=std::max(hi,v);
        }
    }
    return scalar({"LowerClosure","UpperClosure","Forward","Discount"},{lo,hi,F,D});
}

inline Report parity_quote_intersection(const Mat& x,const Vec& p){
    validate(x,1,5,5);parameters(p,1);double D=positive(p[0]),lo=-std::numeric_limits<double>::infinity(),hi=-lo;
    Report o{{"Strike","ForwardLower","ForwardUpper","CommonLower","CommonUpper","Nonempty"},{}};
    for(const auto& r:x){
        positive(r[0]);require(r[1]>=0&&r[2]>=r[1]&&r[3]>=0&&r[4]>=r[3],"Invalid bid-ask interval");
        double a=r[0]+(r[1]-r[4])/D,b=r[0]+(r[2]-r[3])/D;
        lo=std::max(lo,a);hi=std::min(hi,b);o.rows.push_back({r[0],a,b,0,0,0});
    }
    for(auto& r:o.rows){r[3]=lo;r[4]=hi;r[5]=double(lo<=hi);}
    return o;
}
} }
