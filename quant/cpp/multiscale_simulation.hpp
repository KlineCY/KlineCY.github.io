#pragma once
#include "multiscale_common.hpp"
namespace msq {
inline Vec mlmc_plan(const Vec& x){
    require(x.size()>=3&&x.size()%2==1);double e=x[0];positive(e);
    double h=0;for(size_t i=1;i<x.size();i+=2){require(x[i]>=0&&x[i+1]>0);h+=std::sqrt(x[i]*x[i+1]);}
    Vec n;double v=0,c=0;
    for(size_t i=1;i<x.size();i+=2){
        double k=std::max(2.,std::ceil(h*std::sqrt(x[i]/x[i+1])/sq(e)));require(k<=1e9);
        n.push_back(k);v+=x[i]/k;c+=x[i+1]*k;
    }
    Vec out{v,c,sq(h/e),h};out.insert(out.end(),n.begin(),n.end());return out;
}
inline Vec mlmc_estimate(const Vec& x){
    require(x.size()>=7);int l=integer(x[0],1,30);size_t p=1;double est=0,v=0,c=0;Vec means,vars,counts;
    for(int j=0;j<l;++j){
        require(p+2<=x.size());int n=integer(x[p++],2,100000);double cost=x[p++];positive(cost);require(p+2*size_t(n)<=x.size());
        Vec d;for(int i=0;i<n;++i){double f=x[p++],g=x[p++];if(j==0)require(g==0);d.push_back(f-g);}
        double m=avg(d),z=var(d);est+=m;v+=z/n;c+=cost*n;means.push_back(m);vars.push_back(z);counts.push_back(n);
    }
    require(p==x.size());Vec out{est,v,std::sqrt(v),c};out.insert(out.end(),means.begin(),means.end());out.insert(out.end(),vars.begin(),vars.end());out.insert(out.end(),counts.begin(),counts.end());return out;
}
inline Vec asian_coupling(const Vec& x){
    require(x.size()>=7);double s=x[0],k=x[1],r=x[2],sigma=x[3],T=x[4];positive(s);positive(k);positive(T);require(sigma>=0);
    int n=int(x.size())-5;require(n>=2&&n%2==0&&n<=10000);double h=T/n,drift=(r-.5*sigma*sigma)*h,vol=sigma*std::sqrt(h),y=std::log(s),a=0,b=0,coarse=y;
    for(int j=0;j<n;j+=2){y+=drift+vol*x[5+j];a+=std::exp(y);y+=drift+vol*x[6+j];a+=std::exp(y);
        coarse+=2*drift+vol*(x[5+j]+x[6+j]);b+=std::exp(coarse);}
    a/=n;b/=n/2;double df=std::exp(-r*T),f=df*std::max(a-k,0.),g=df*std::max(b-k,0.);
    return {a,b,f,g,f-g,std::exp(y),std::exp(coarse),std::exp(y)-std::exp(coarse)};
}
inline Vec asian_moments(const Vec& x){
    require(x.size()>=5&&(x.size()-3)%2==0);double S=x[0],r=x[1],sigma=x[2];positive(S);require(sigma>=0);
    int n=int((x.size()-3)/2);require(n<=1000);Vec t(n),w(n),m(n);double mt=0,ea=0,vg=0,va=0,cross=0;
    for(int i=0;i<n;++i){t[i]=x[3+2*i];w[i]=x[4+2*i];require(t[i]>0&&w[i]>=0&&(i==0||t[i]>t[i-1]));mt+=w[i]*t[i];m[i]=S*std::exp(r*t[i]);ea+=w[i]*m[i];}
    require(std::abs(sum(w)-1)<1e-10);
    Vec g(n);
    for(int i=0;i<n;++i)for(int j=0;j<n;++j){double c=sq(sigma)*std::min(t[i],t[j]);vg+=w[i]*w[j]*c;g[i]+=w[j]*c;va+=w[i]*w[j]*m[i]*m[j]*std::expm1(c);}
    double eg=S*std::exp((r-.5*sq(sigma))*mt+.5*vg),vG=sq(eg)*std::expm1(vg);
    for(int i=0;i<n;++i)cross+=w[i]*m[i]*eg*std::expm1(g[i]);
    double beta=vG>0?cross/vG:0,remaining=std::max(0.,va-beta*cross);
    return {ea,va,eg,vG,cross,beta,remaining};
}
inline Vec stratified_call(const Vec& x){
    require(x.size()>=5);double S=x[0],K=x[1],r=x[2],sig=x[3],T=x[4];positive(S);positive(K);positive(sig);positive(T);
    require(x.size()<=105);double s=sig*std::sqrt(T),m=std::log(S)+(r-.5*sig*sig)*T,k=(std::log(K)-m)/s,df=std::exp(-r*T);
    Vec cuts{-std::numeric_limits<double>::infinity()};
    for(size_t i=5;i<x.size();++i){require(x[i]>cuts.back()&&std::abs(x[i])<=8);cuts.push_back(x[i]);}
    cuts.push_back(std::numeric_limits<double>::infinity());Vec out{0,0,0,0};double e=0,e2=0,within=0,mass=0;
    for(size_t i=1;i<cuts.size();++i){
        double a=cuts[i-1],b=cuts[i],p=normalmass(a,b);positive(p);double lo=std::max(a,k),u=0,v=0;
        if(lo<b){double z0=normalmass(lo,b),z1=std::exp(m+.5*s*s)*normalmass(lo-s,b-s),z2=std::exp(2*m+2*s*s)*normalmass(lo-2*s,b-2*s);
            u=df*(z1-K*z0);v=df*df*(z2-2*K*z1+K*K*z0);}
        double mu=u/p,second=v/p,variance=std::max(0.,second-mu*mu);e+=u;e2+=v;within+=p*variance;mass+=p;
        out.insert(out.end(),{p,mu,variance});
    }
    out[0]=e;out[1]=std::max(0.,e2-e*e);out[2]=within;out[3]=mass;return out;
}
inline Vec greek_estimators(const Vec& x){
    require(x.size()>=7);double S=x[0],K=x[1],r=x[2],sigma=x[3],T=x[4];positive(S);positive(K);positive(sigma);positive(T);
    double st=std::sqrt(T),v=sigma*st,df=std::exp(-r*T);Mat values(6);
    for(size_t i=5;i<x.size();++i){double z=x[i],p=S*std::exp((r-.5*sigma*sigma)*T+v*z),pay=df*std::max(p-K,0.);bool itm=p>K;
        values[0].push_back(pay);values[1].push_back(itm?df*p/S:0);values[2].push_back(pay*z/(S*v));
        values[3].push_back(itm?df*p*(st*z-sigma*T):0);values[4].push_back(pay*((z*z-1)/sigma-st*z));values[5].push_back(itm?df*z/(S*v):0);}
    Vec out;for(const auto& a:values){out.push_back(avg(a));out.push_back(var(a));}
    double d1=(std::log(S/K)+(r+.5*sigma*sigma)*T)/v,d2=d1-v;
    out.insert(out.end(),{S*cdf(d1)-K*df*cdf(d2),cdf(d1),S*pdf(d1)*st,df*pdf(d2)/(S*v)});return out;
}
inline Vec bridge_path(const Vec& x){
    require(x.size()>=10&&(x.size()-6)%2==0);double h=x[0],sigma=x[1],r=x[2],K=x[3],S=x[4],cp=x[5];
    positive(h);positive(sigma);positive(K);positive(S);require(cp==1||cp==-1);
    double prev=0,logsurv=0,last=S;bool alive=S>h;Vec p;
    for(size_t i=6;i<x.size();i+=2){double t=x[i],s=x[i+1];positive(s);require(t>prev);double a=0;
        if(last>h&&s>h){double z=2*std::log(last/h)*std::log(s/h)/(sq(sigma)*(t-prev));a=-std::expm1(-z);logsurv+=std::log(a);}
        else alive=false;
        p.push_back(a);last=s;prev=t;}
    double survival=alive?std::exp(logsurv):0,vanilla=std::exp(-r*prev)*std::max(cp*(last-K),0.);
    Vec out{alive?vanilla:0,vanilla*survival,survival,vanilla};out.insert(out.end(),p.begin(),p.end());return out;
}
inline Vec frozen_control(const Vec& x){
    require(x.size()>=10&&(x.size()-2)%2==0);int n=integer(x[0],2,100000);double known=x[1];int total=int((x.size()-2)/2);require(total>=n+2);
    Vec yp,cp,yt,ct;
    for(int i=0;i<total;++i){double y=x[2+2*i],c=x[3+2*i];if(i<n){yp.push_back(y);cp.push_back(c);}else{yt.push_back(y);ct.push_back(c);}}
    double vp=var(cp);positive(vp);double beta=cov(yp,cp)/vp;Vec adjusted;
    for(size_t i=0;i<yt.size();++i)adjusted.push_back(yt[i]-beta*(ct[i]-known));
    double a=var(yt),b=var(adjusted);return {beta,avg(yt),avg(adjusted),a,b,a-b,avg(ct),double(yt.size())};
}
}
