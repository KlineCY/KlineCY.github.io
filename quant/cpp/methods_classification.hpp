#pragma once
#include "evidence_common.hpp"
namespace kline { namespace methods_classification {
using namespace evidence;
inline Report prior_adjustment(const Mat& x,const Vec& p){
    validate(x,1,1,1);
    parameters(p,2);
    require(p[0]>0&&p[0]<1&&p[1]>0&&p[1]<1,"Invalid prior");
    double a=p[1]*(1-p[0])/(p[0]*(1-p[1]));
    Report o{{"SourceProbability","TargetProbability"},{}};
    for(auto&r:x){double z=r[0];
    require(z>=0&&z<=1,"Invalid posterior");
    o.rows.push_back({z,a*z/(1-z+a*z)});
    }return o;
    
}

inline Report pairwise_auc(const Mat& x,const Vec& p){
    validate(x,2,2,2);
    parameters(p,0);
    Mat a=x;
    double pos=0,neg=0;
    for(auto&r:a){require(r[1]==0||r[1]==1,"Invalid event");
    pos+=r[1];
    neg+=1-r[1];
    }positive(pos);
    positive(neg);
    std::sort(a.begin(),a.end(),[](const Vec&u,const Vec&v){return u[0]<v[0];
    });
    double b=0,s=0;
    for(std::size_t i=0;
    i<a.size();
    ){std::size_t j=i;
    double P=0,N=0;
    while(j<a.size()&&a[j][0]==a[i][0]){P+=a[j][1];
    N+=1-a[j][1];
    ++j;
    }s+=P*(b+.5*N);
    b+=N;
    i=j;
    }return scalar({"Positive","Negative","AUC"},{pos,neg,s/(pos*neg)});
    
}

inline Report multiclass_brier(const Mat& x,const Vec& p){
    auto d=validate(x,1,3,64);
    parameters(p,0);
    auto K=d-1;
    double s=0;
    for(auto&r:x){auto y=count(r[0],0,K-1);
    double a=0;
    for(std::size_t j=0;
    j<K;
    ++j){double q=r[j+1];
    require(q>=0&&q<=1,"Invalid probability");
    a+=q;
    double z=q-double(j==y);
    s+=z*z;
    }require(std::abs(a-1)<1e-9,"Probabilities must sum to one");
    }double b=1-1./K,v=s/x.size();
    return scalar({"Brier","UniformBaseline","Skill"},{v,b,1-v/b});
    
}

inline Report binary_cost_frontier(const Mat& x,const Vec& p){
    validate(x,1,2,2);
    parameters(p,2);
    double a=positive(p[0]),b=positive(p[1]),t=a/(a+b),tp=0,fp=0,tn=0,fn=0;
    for(auto&r:x){require(r[0]>=0&&r[0]<=1&&(r[1]==0||r[1]==1),"Invalid input");
    bool z=r[0]>=t;
    if(z){if(r[1])++tp;
    else ++fp;
    }else{if(r[1])++fn;
    else ++tn;
    }}return scalar({"Threshold","TP","FP","TN","FN","TotalCost"},{t,tp,fp,tn,fn,a*fp+b*fn});
    
}
} }
