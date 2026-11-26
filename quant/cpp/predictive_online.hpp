#pragma once
#include "predictive_kernel.hpp"
namespace pdq {
inline Vec delayed_ogd(const Vec& x){
    require(x.size()>=8&&(x.size()-4)%4==0);double eta=x[0],lo=x[1],hi=x[2],w=x[3];positive(eta);require(lo<hi&&w>=lo&&w<=hi);
    int n=(x.size()-4)/4;require(n<=10000);std::vector<std::tuple<int,int,double>> pending;Vec out;double loss=0;
    for(int t=0;t<n;++t){
        std::sort(pending.begin(),pending.end());
        int count=0;for(auto p:pending){if(std::get<0>(p)>t)break;w=std::clamp(w-eta*std::get<2>(p),lo,hi);++count;}
        pending.erase(pending.begin(),pending.begin()+count);
        double a=x[4+4*t],y=x[5+4*t];int delay=integer(x[6+4*t],1,1000000);double wt=x[7+4*t];require(wt>=0);
        double pred=w*a,l=.5*wt*sq(pred-y);loss+=l;pending.push_back({t+delay,t,wt*(pred-y)*a});
        out.insert(out.end(),{w,pred,l,double(count)});
    }
    out.insert(out.begin(),{loss,w,double(pending.size())});return out;
}
inline Vec fixed_share(const Vec& x){
    require(x.size()>=5);int k=integer(x[0],2,100);double eta=x[1],alpha=x[2];positive(eta);prob(alpha);require((x.size()-3)%k==0);
    int n=(x.size()-3)/k;require(n>=1&&n*k<=100000);Vec p(k,1./k),cum(k),out;double loss=0;
    for(int t=0;t<n;++t){Vec l(x.begin()+3+t*k,x.begin()+3+(t+1)*k);for(int j=0;j<k;++j){prob(l[j]);cum[j]+=l[j];}
        double z=dot(p,l);loss+=z;out.push_back(z);out.insert(out.end(),p.begin(),p.end());double minl=*std::min_element(l.begin(),l.end());
        for(int j=0;j<k;++j){p[j]*=std::exp(-eta*(l[j]-minl));}
        double den=sum(p);positive(den);
        for(double& a:p)a=(1-alpha)*a/den+alpha/k;
    }
    double best=*std::min_element(cum.begin(),cum.end());out.insert(out.begin(),{loss,best,loss-best});out.insert(out.end(),p.begin(),p.end());return out;
}
inline Vec recursive_ridge(const Vec& x){
    require(x.size()>=6);int d=integer(x[0],1,30);double lambda=x[1],rho=x[2];positive(lambda);require(rho>0&&rho<=1&&(x.size()-3)%(d+1)==0);
    int n=(x.size()-3)/(d+1);require(n>=1&&n<=5000);Mat P=zeros(d,d);Vec b(d),out;for(int j=0;j<d;++j)P[j][j]=1/lambda;
    double rss=0;
    for(int t=0;t<n;++t){
        Vec a(x.begin()+3+t*(d+1),x.begin()+3+t*(d+1)+d);double y=x[3+t*(d+1)+d],pred=dot(b,a);rss+=sq(y-pred);
        Vec u=mv(P,a);double den=rho+dot(a,u);positive(den);double e=y-pred;
        for(int i=0;i<d;++i)b[i]+=u[i]*e/den;
        for(int i=0;i<d;++i)for(int j=0;j<d;++j)P[i][j]=(P[i][j]-u[i]*u[j]/den)/rho;
        out.insert(out.end(),{pred,e,den});out.insert(out.end(),b.begin(),b.end());
    }
    out.insert(out.begin(),rss);for(auto& r:P)out.insert(out.end(),r.begin(),r.end());return out;
}
inline Vec passive_aggressive(const Vec& x){
    require(x.size()>=5);int d=integer(x[0],1,100);double C=x[1];positive(C);require((x.size()-2)%(d+1)==0);
    int n=(x.size()-2)/(d+1);require(n>=1&&n*d<=100000);Vec w(d),out;double loss=0,errors=0;
    for(int t=0;t<n;++t){Vec a(x.begin()+2+t*(d+1),x.begin()+2+t*(d+1)+d);double y=x[2+t*(d+1)+d];require(y==1||y==-1);
        double margin=y*dot(w,a),l=std::max(0.,1-margin),norm=dot(a,a),tau=norm>0?std::min(C,l/norm):0;
        loss+=l;errors+=margin<=0;for(int j=0;j<d;++j)w[j]+=tau*y*a[j];
        out.insert(out.end(),{margin,l,tau});out.insert(out.end(),w.begin(),w.end());
    }
    out.insert(out.begin(),{loss,errors});return out;
}
inline Vec adaptive_miscoverage(const Vec& x){
    require(x.size()>=6);double target=x[0],eta=x[1],a=x[2],lo=x[3],hi=x[4];require(target>0&&target<1&&lo>=0&&hi<=1&&lo<hi&&a>=lo&&a<=hi);positive(eta);
    Vec out;double err=0,correction=0,initial=a;
    for(size_t t=5;t<x.size();++t){int e=integer(x[t],0,1);double raw=a+eta*(target-e),next=std::clamp(raw,lo,hi);correction+=next-raw;out.insert(out.end(),{a,double(e),next,next-raw});err+=e;a=next;}
    double n=x.size()-5;
    out.insert(out.begin(),{err/n,a,correction,a-initial-eta*(n*target-err)-correction});return out;
}
inline Vec exp3_observed(const Vec& x){
    require(x.size()>=6&&(x.size()-3)%2==0);int k=integer(x[0],2,100);double gamma=x[1],eta=x[2];require(gamma>0&&gamma<=1);positive(eta);
    Vec logw(k),out;double gain=0;
    for(size_t t=3;t<x.size();t+=2){
        int a=integer(x[t],0,k-1);double r=x[t+1];prob(r);double m=*std::max_element(logw.begin(),logw.end());Vec p(k);
        for(int j=0;j<k;++j){p[j]=std::exp(logw[j]-m);}
        double den=sum(p);positive(den);
        for(double& z:p){z=(1-gamma)*z/den+gamma/k;}
        double estimated=r/p[a];
        logw[a]+=eta*estimated;gain+=r;out.insert(out.end(),{double(a),r,p[a],estimated});out.insert(out.end(),p.begin(),p.end());
    }
    out.insert(out.begin(),gain);return out;
}
}
