#pragma once
#include "finite_state_common.hpp"
namespace fsq {
// Segment likelihood with exact right boundary; every state run is a completed segment.
struct DurationModel {
    Vec initial;
    Mat transition,duration,emission;
    std::vector<int> observations;
};
inline DurationModel duration_input(const Vec& x){
    require(x.size()>=4);
    int k=count(x[0],2,5),d=count(x[1],1,20),m=count(x[2],2,10),n=count(x[3],1,300);
    require(x.size()==size_t(4+k+k*k+k*d+k*m+n));
    size_t at=4;
    auto take=[&](int z){Vec a(x.begin()+at,x.begin()+at+z);at+=z;distribution(a);return a;};
    DurationModel a;
    a.initial=take(k);
    for(int i=0;i<k;++i){Vec p=take(k);require(p[i]==0);a.transition.push_back(p);}
    for(int i=0;i<k;++i)a.duration.push_back(take(d));
    for(int i=0;i<k;++i)a.emission.push_back(take(m));
    for(int i=0;i<n;++i)a.observations.push_back(count(x[at++],0,m-1));
    return a;
}
struct DurationResult {double loglik,logbest,expectedSegments;Vec posteriorLast,path,occupancy;};
inline DurationResult duration_smooth(const DurationModel& a){
    int n=a.observations.size(),k=a.initial.size(),d=a.duration[0].size();
    double neg=-std::numeric_limits<double>::infinity();
    Mat f(n+1,Vec(k,neg)),v=f,b=f;
    std::vector<std::vector<int>> prev(n+1,std::vector<int>(k,-1)),length=prev;
    // Log segment emissions are accumulated from the right; no subtraction of infinities.
    auto segments=[&](int t,int j){
        Vec s(std::min(d,t)+1,neg);s[0]=0;
        for(int l=1;l<=std::min(d,t);++l)s[l]=s[l-1]+logp(a.emission[j][a.observations[t-l]]);
        for(size_t l=1;l<s.size();++l)s[l]+=logp(a.duration[j][l-1]);
        return s;
    };
    for(int t=1;t<=n;++t)for(int j=0;j<k;++j){
        Vec seg=segments(t,j);
        for(int l=1;l<int(seg.size());++l){
            int start=t-l;
            if(start==0){
                double z=logp(a.initial[j])+seg[l];f[t][j]=logadd(f[t][j],z);
                if(z>v[t][j]){v[t][j]=z;prev[t][j]=-1;length[t][j]=l;}
            }else for(int i=0;i<k;++i){
                double tr=logp(a.transition[i][j])+seg[l];
                f[t][j]=logadd(f[t][j],f[start][i]+tr);
                double z=v[start][i]+tr;
                if(z>v[t][j]){v[t][j]=z;prev[t][j]=i;length[t][j]=l;}
            }
        }
    }
    double ll=lse(f[n]);require(std::isfinite(ll));
    for(int j=0;j<k;++j)b[n][j]=0;
    for(int t=n-1;t>=1;--t)for(int i=0;i<k;++i)for(int j=0;j<k;++j){
        double e=0;
        for(int l=1;l<=std::min(d,n-t);++l){
            e+=logp(a.emission[j][a.observations[t+l-1]]);
            b[t][i]=logadd(b[t][i],logp(a.transition[i][j])+logp(a.duration[j][l-1])+e+b[t+l][j]);
        }
    }
    Vec occ(k,0),last(k),path(n);
    double ns=0;
    for(int t=1;t<=n;++t)for(int j=0;j<k;++j){
        Vec seg=segments(t,j);
        for(int l=1;l<int(seg.size());++l){
            int start=t-l;double pre=neg;
            if(start==0)pre=logp(a.initial[j]);
            else for(int i=0;i<k;++i)pre=logadd(pre,f[start][i]+logp(a.transition[i][j]));
            double mass=std::exp(pre+seg[l]+b[t][j]-ll);
            ns+=mass;occ[j]+=l*mass;
        }
    }
    for(int j=0;j<k;++j)last[j]=std::exp(f[n][j]-ll);
    int j=int(std::max_element(v[n].begin(),v[n].end())-v[n].begin()),t=n;
    double best=v[n][j];
    while(t>0){int l=length[t][j];require(l>0);for(int z=t-l;z<t;++z)path[z]=j;int i=prev[t][j];t-=l;j=i;}
    return {ll,best,ns,last,path,occ};
}
inline Vec duration_study(const Vec& x){
    auto a=duration_input(x);auto r=duration_smooth(a);
    Vec out{r.loglik,r.logbest,r.expectedSegments};
    out.insert(out.end(),r.posteriorLast.begin(),r.posteriorLast.end());
    out.insert(out.end(),r.occupancy.begin(),r.occupancy.end());
    out.insert(out.end(),r.path.begin(),r.path.end());
    return out;
}
// Exact penalized piecewise constant regression with a minimum segment length.
inline Vec segmentation(const Vec& x){
    require(x.size()>=4);
    int m=count(x[0],1,300),n=int(x.size())-2;
    double penalty=x[1];require(penalty>=0&&n>=m&&n<=3000);
    Vec s(n+1),q(n+1);
    for(int i=1;i<=n;++i){s[i]=s[i-1]+x[i+1];q[i]=q[i-1]+sq(x[i+1]);}
    auto cost=[&](int a,int b){return std::max(0.,q[b]-q[a]-sq(s[b]-s[a])/(b-a));};
    Vec f(n+1,std::numeric_limits<double>::infinity());f[0]=-penalty;
    std::vector<int> prev(n+1,-1),parts;
    for(int t=m;t<=n;++t)for(int a=0;a<=t-m;++a){
        double v=f[a]+cost(a,t)+penalty;
        if(v<f[t]-1e-11){f[t]=v;prev[t]=a;}
    }
    require(prev[n]>=0);double sse=0;Vec fit(n);
    for(int t=n;t>0;){int a=prev[t];require(a>=0);sse+=cost(a,t);double u=(s[t]-s[a])/(t-a);for(int i=a;i<t;++i)fit[i]=u;parts.push_back(t);t=a;}
    std::reverse(parts.begin(),parts.end());
    Vec out{f[n],sse,double(parts.size())};
    for(int t:parts)out.push_back(t);
    out.insert(out.end(),fit.begin(),fit.end());return out;
}
// Integrated beta-binomial emission evidence for a Bernoulli change point.
inline double beta_log(double a,double b){return std::lgamma(a)+std::lgamma(b)-std::lgamma(a+b);}
inline Vec bayes_break(const Vec& x){
    require(x.size()>=4);double a=x[0],b=x[1];require(a>0&&b>0);
    int n=int(x.size())-2;require(n>=2&&n<=5000);
    Vec s(n+1);
    for(int i=1;i<=n;++i){require(x[i+1]==0||x[i+1]==1);s[i]=s[i-1]+x[i+1];}
    auto ll=[&](int l,int r){double k=s[r]-s[l];return beta_log(a+k,b+r-l-k)-beta_log(a,b);};
    double null=ll(0,n);Vec score;
    for(int t=1;t<n;++t)score.push_back(ll(0,t)+ll(t,n)-std::log(n-1.));
    double alt=lse(score);int mode=int(std::max_element(score.begin(),score.end())-score.begin())+1;
    double posterior=std::exp(alt-logadd(null,alt)),mean=0;
    Vec out{null,alt,alt-null,posterior,double(mode),0};
    for(int t=1;t<n;++t){double p=std::exp(score[t-1]-alt);out.push_back(p);mean+=t*p;}
    out[5]=mean;return out;
}
// Two-way fixed effects by alternating orthogonal projections on an unbalanced bipartite panel.
inline Vec two_way_within(const Vec& x){
    require(x.size()>=7&&(x.size()-2)%4==0);
    int ni=count(x[0],2,2000),nt=count(x[1],2,2000),n=int((x.size()-2)/4);
    require(n>ni+nt-1&&n<=100000);
    std::vector<int> id(n),time(n),ci(ni),ct(nt);
    Vec a(n),b(n);
    std::map<std::pair<int,int>,int> seen;
    for(int j=0;j<n;++j){id[j]=count(x[2+4*j],0,ni-1);time[j]=count(x[3+4*j],0,nt-1);require(++seen[{id[j],time[j]}]==1);ci[id[j]]++;ct[time[j]]++;a[j]=x[4+4*j];b[j]=x[5+4*j];}
    for(int v:ci)require(v>0);
    for(int v:ct)require(v>0);
    // The group graph must be connected for the stated residual degrees of freedom.
    std::vector<int> parent(ni+nt);std::iota(parent.begin(),parent.end(),0);
    auto root=[&](int i){while(parent[i]!=i)i=parent[i];return i;};
    for(int j=0;j<n;++j)parent[root(id[j])]=root(ni+time[j]);
    for(int i=1;i<ni+nt;++i)require(root(i)==root(0));
    auto center=[&](Vec& v,const std::vector<int>& group,const std::vector<int>& counts){
        Vec mean(counts.size());for(int j=0;j<n;++j)mean[group[j]]+=v[j];
        for(size_t i=0;i<mean.size();++i)mean[i]/=counts[i];
        for(int j=0;j<n;++j)v[j]-=mean[group[j]];
    };
    auto error=[&](const Vec& v){Vec p(ni),q(nt);for(int j=0;j<n;++j){p[id[j]]+=v[j];q[time[j]]+=v[j];}
        double e=0;for(double z:p)e=std::max(e,std::abs(z));for(double z:q)e=std::max(e,std::abs(z));return e;};
    double scale=1;for(double z:a)scale=std::max(scale,std::abs(z));for(double z:b)scale=std::max(scale,std::abs(z));
    int it=0;double err=0;
    for(;it<20000;++it){center(a,id,ci);center(a,time,ct);center(b,id,ci);center(b,time,ct);err=std::max(error(a),error(b));if(err<=1e-12*scale)break;}
    require(it<20000);double aa=std::inner_product(a.begin(),a.end(),a.begin(),0.);
    require(aa>1e-20*scale*scale&&n>ni+nt);
    double beta=std::inner_product(a.begin(),a.end(),b.begin(),0.)/aa,rss=0;
    for(int j=0;j<n;++j)rss+=sq(b[j]-beta*a[j]);
    return {beta,rss,double(n-ni-nt),std::sqrt(rss/(n-ni-nt)/aa),err};
}
}
