#pragma once
#include "finite_state_common.hpp"
namespace fsq {
inline Vec poisson_binomial_pmf(const Vec& p){
    require(!p.empty()&&p.size()<=500);
    Vec f(p.size()+1);f[0]=1;
    size_t i=0;
    for(double z:p){probability(z);++i;for(size_t j=i;j>0;--j)f[j]=(1-z)*f[j]+z*f[j-1];f[0]*=1-z;}
    return f;
}
inline Vec count_credit(const Vec& x){
    require(x.size()>=2);double a=x[0];require(a>0&&a<1);
    Vec p(x.begin()+1,x.end()),f=poisson_binomial_pmf(p);
    double mean=0,var=0;for(double z:p){mean+=z;var+=z*(1-z);}
    double acc=0;int q=0;for(;q<int(f.size())-1;++q){acc+=f[q];if(acc>=a-1e-14)break;}
    double es=q;
    for(size_t j=q+1;j<f.size();++j)es+=(j-q)*f[j]/(1-a);
    Vec out{mean,var,double(q),es,sum(f)};out.insert(out.end(),f.begin(),f.end());return out;
}
// Integer loss units: weighted independent Bernoulli convolution.
inline Vec loss_distribution(const Vec& x){
    require(x.size()>=4&&(x.size()-2)%2==0);
    double alpha=x[0];require(alpha>0&&alpha<1);int attachment=count(x[1],0,20000);
    int total=0;Vec f(1,1);double mean=0;
    for(size_t i=2;i<x.size();i+=2){
        int w=count(x[i],1,2000);double p=x[i+1];probability(p);require(total+w<=20000);
        f.resize(total+w+1);
        for(int j=total+w;j>=0;--j)f[j]=(j<=total?(1-p)*f[j]:0)+(j>=w?p*f[j-w]:0);
        total+=w;mean+=w*p;
    }
    double stop=0,m=0,var=0;int q=0;
    for(int j=0;j<=total;++j){m+=j*f[j];var+=sq(j-mean)*f[j];stop+=std::max(j-attachment,0)*f[j];}
    double acc=0;for(;q<total;++q){acc+=f[q];if(acc>=alpha-1e-14)break;}
    double es=q;for(int j=q+1;j<=total;++j)es+=(j-q)*f[j]/(1-alpha);
    Vec out{m,var,double(q),es,stop,sum(f)};out.insert(out.end(),f.begin(),f.end());return out;
}
// Stop-loss sensitivity uses a leave-one-out loss distribution, not a finite difference.
inline Vec tranche_sensitivity(const Vec& x){
    require(x.size()>=4&&x.size()%2==0);
    int a=count(x[0],0,20000),b=count(x[1],1,20000);require(b>a);
    Vec base{.9,double(a)};base.insert(base.end(),x.begin()+2,x.end());
    Vec f=loss_distribution(base);double el=0;
    auto payoff=[&](int l){return double(std::min(std::max(l-a,0),b-a));};
    for(size_t j=6;j<f.size();++j)el+=payoff(int(j-6))*f[j];
    Vec out{el,el/(b-a)};
    for(size_t i=2;i<x.size();i+=2){
        int w=count(x[i],1,2000);Vec other{.9,0};
        for(size_t j=2;j<x.size();j+=2)if(i!=j)other.insert(other.end(),{x[j],x[j+1]});
        double delta=0;
        if(other.size()==2)delta=payoff(w)-payoff(0);
        else{Vec g=loss_distribution(other);for(size_t l=6;l<g.size();++l)delta+=(payoff(int(l-6)+w)-payoff(int(l-6)))*g[l];}
        out.push_back(delta);
    }
    return out;
}
// Conditional independence given a finite systemic factor; posterior factor weights after observed count.
inline Vec mixture_count(const Vec& x){
    require(x.size()>=4);int k=count(x[0],1,50),n=count(x[1],1,500),obs=count(x[2],0,n);
    require(x.size()==size_t(3+k*(n+1)));
    Vec weights,pmf(n+1),post;double mu=0,within=0,between=0;
    Vec means;size_t j=3;
    for(int s=0;s<k;++s){double w=x[j++];probability(w);weights.push_back(w);Vec p(x.begin()+j,x.begin()+j+n);j+=n;Vec f=poisson_binomial_pmf(p);
        double m=sum(p),v=0;for(double z:p)v+=z*(1-z);mu+=w*m;within+=w*v;means.push_back(m);
        for(int i=0;i<=n;++i)pmf[i]+=w*f[i];
        post.push_back(w*f[obs]);
    }
    distribution(weights);for(int s=0;s<k;++s)between+=weights[s]*sq(means[s]-mu);
    require(pmf[obs]>0);for(double& z:post)z/=pmf[obs];
    Vec out{mu,within,between,within+between,pmf[obs],sum(pmf)};
    out.insert(out.end(),post.begin(),post.end());out.insert(out.end(),pmf.begin(),pmf.end());return out;
}
}
