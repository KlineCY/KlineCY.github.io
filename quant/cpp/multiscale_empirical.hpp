#pragma once
#include "multiscale_common.hpp"
namespace msq {
inline Vec colmean(const Mat& x){Vec mu(x[0].size());for(const auto& r:x)for(size_t j=0;j<mu.size();++j)mu[j]+=r[j]/x.size();return mu;}
inline Mat covariance(const Mat& x,const Vec& mu){
    int k=mu.size();Mat c(k,Vec(k));for(const auto& r:x)for(int i=0;i<k;++i)for(int j=0;j<k;++j)c[i][j]+=(r[i]-mu[i])*(r[j]-mu[j])/x.size();return c;
}
inline double hac_lrv(const Vec& x,int lag){
    require(lag>=0&&size_t(lag)<x.size());double m=avg(x),v=0;for(double z:x)v+=sq(z-m);
    for(int l=1;l<=lag;++l){double g=0;for(size_t i=l;i<x.size();++i)g+=(x[i]-m)*(x[i-l]-m);v+=2*(1-double(l)/(lag+1))*g;}
    return std::max(0.,v/x.size());
}
inline Vec hj_bound(const Vec& x){
    require(x.size()>=6);int k=integer(x[0],1,50),n=integer(x[1],k+1,10000);double m=x[2];positive(m);require(x.size()==size_t(3+k*n));
    Mat r(n,Vec(k));size_t at=3;for(auto& z:r)for(double& v:z)v=x[at++];
    Vec mu=colmean(r),z=solve(covariance(r,mu),mu);double sr2=dot(mu,z);require(sr2>=-1e-10);
    Vec sdf,err(k);double neg=0;
    for(const auto& a:r){Vec b(k);for(int j=0;j<k;++j)b[j]=a[j]-mu[j];double y=m*(1-dot(z,b));sdf.push_back(y);neg+=y<0;for(int j=0;j<k;++j)err[j]+=y*a[j]/n;}
    double maxerr=0;for(double e:err)maxerr=std::max(maxerr,std::abs(e));
    Vec out{std::sqrt(std::max(0.,sr2)),m*std::sqrt(std::max(0.,sr2)),avg(sdf),std::sqrt(var(sdf)*(n-1)/n),neg/n,maxerr};
    out.insert(out.end(),z.begin(),z.end());out.insert(out.end(),err.begin(),err.end());return out;
}
inline Vec factor_spanning(const Vec& x){
    require(x.size()>=8);int k=integer(x[0],1,40),n=integer(x[1],k+2,10000);require(x.size()==size_t(2+n*(k+1)));
    Mat a(n,Vec(k+1));size_t at=2;for(auto& r:a)for(double& z:r)z=x[at++];
    Vec mu=colmean(a);Mat c=covariance(a,mu),f(k,Vec(k));Vec fm(mu.begin(),mu.begin()+k),cross(k);
    for(int i=0;i<k;++i){cross[i]=c[i][k];for(int j=0;j<k;++j)f[i][j]=c[i][j];}
    Vec beta=solve(f,cross),z=solve(f,fm);double alpha=mu[k]-dot(beta,fm),v=c[k][k]-dot(beta,cross);positive(v);
    double base=dot(fm,z),add=sq(alpha)/v,both=dot(mu,solve(c,mu));
    Vec out{base,both,add,alpha,v,both-base-add};out.insert(out.end(),beta.begin(),beta.end());return out;
}
inline Vec timing_regression(const Vec& x){
    require(x.size()>=8&&x.size()%2==0);int n=x.size()/2;require(n>=4&&n<=10000);
    Mat a(3,Vec(3));Vec b(3),y(n);Mat X(n,Vec(3));
    for(int i=0;i<n;++i){double f=x[2*i];X[i]={1,f,f*f};y[i]=x[2*i+1];
        for(int j=0;j<3;++j){b[j]+=X[i][j]*y[i];for(int k=0;k<3;++k)a[j][k]+=X[i][j]*X[i][k];}}
    Vec beta=solve(a,b),v=solve(a,{0,0,1});double rss=0;for(int i=0;i<n;++i)rss+=sq(y[i]-dot(X[i],beta));
    require(v[2]>0);return {beta[0],beta[1],beta[2],rss,double(n-3),std::sqrt(rss/(n-3)*v[2]),beta[1]+2*beta[2]*x[0],beta[1]+2*beta[2]*x[x.size()-2]};
}
inline Vec sdf_distance(const Vec& x){
    require(x.size()>=7);int k=integer(x[0],1,40),n=integer(x[1],k,10000);require(x.size()==size_t(2+k+n*(k+1)));
    Vec q(x.begin()+2,x.begin()+2+k),error(k);Mat a(k,Vec(k)),pay(n,Vec(k));Vec m(n);size_t at=2+k;
    for(int i=0;i<n;++i){m[i]=x[at++];for(double& z:pay[i])z=x[at++];
        for(int j=0;j<k;++j){error[j]+=m[i]*pay[i][j]/n;for(int l=0;l<k;++l)a[j][l]+=pay[i][j]*pay[i][l]/n;}}
    for(int j=0;j<k;++j)error[j]-=q[j];
    Vec z=solve(a,error),correct(k);double d2=dot(z,error),real=0,negative=0;Vec mnew;
    for(int i=0;i<n;++i){double d=dot(pay[i],z),v=m[i]-d;mnew.push_back(v);real+=d*d/n;negative+=v<0;for(int j=0;j<k;++j)correct[j]+=v*pay[i][j]/n;}
    for(int j=0;j<k;++j)correct[j]-=q[j];
    Vec out{std::sqrt(std::max(0.,d2)),real,d2-real,negative/n};out.insert(out.end(),error.begin(),error.end());out.insert(out.end(),correct.begin(),correct.end());out.insert(out.end(),mnew.begin(),mnew.end());return out;
}
inline Vec joint_sign_stepdown(const Vec& x){
    require(x.size()>=6);int n=integer(x[0],2,18),k=integer(x[1],2,20);require(x.size()==size_t(2+n*k)&&double(1<<n)*n*k<=5e7);
    Vec d(k),obs(k);Mat z(n,Vec(k));size_t at=2;
    for(auto& r:z)for(double& v:r)v=x[at++];
    for(const auto& r:z)for(int j=0;j<k;++j){d[j]+=sq(r[j]);obs[j]+=r[j];}
    for(int j=0;j<k;++j){positive(d[j]);d[j]=std::sqrt(d[j]);obs[j]=std::abs(obs[j]/d[j]);}
    std::vector<int> order(k);std::iota(order.begin(),order.end(),0);
    std::stable_sort(order.begin(),order.end(),[&](int a,int b){return obs[a]>obs[b];});
    Vec raw(k),single(k),step(k);
    int N=1<<n;
    for(int mask=0;mask<N;++mask){Vec t(k);
        for(int i=0;i<n;++i){double sign=(mask&(1<<i))?1:-1;for(int j=0;j<k;++j)t[j]+=sign*z[i][j];}
        for(int j=0;j<k;++j){t[j]=std::abs(t[j]/d[j]);raw[j]+=t[j]+1e-12>=obs[j];}
        double maximum=*std::max_element(t.begin(),t.end()),sub=0;
        for(int j=0;j<k;++j)single[j]+=maximum+1e-12>=obs[j];
        for(int j=k-1;j>=0;--j){int a=order[j];sub=std::max(sub,t[a]);step[a]+=sub+1e-12>=obs[a];}
    }
    for(int j=0;j<k;++j){raw[j]/=N;single[j]/=N;step[j]/=N;}
    for(int j=1;j<k;++j)step[order[j]]=std::max(step[order[j]],step[order[j-1]]);
    Vec out{double(N)};out.insert(out.end(),obs.begin(),obs.end());out.insert(out.end(),raw.begin(),raw.end());out.insert(out.end(),single.begin(),single.end());out.insert(out.end(),step.begin(),step.end());return out;
}
inline Vec fdr_stepup(const Vec& x){
    require(x.size()>=2&&x.size()<=100001);double alpha=x[0];require(alpha>0&&alpha<1);int n=x.size()-1;
    std::vector<std::pair<double,int>> p;double harmonic=0;for(int i=0;i<n;++i){prob(x[i+1]);p.push_back({x[i+1],i});harmonic+=1./(i+1);}
    std::sort(p.begin(),p.end());Vec bh(n),by(n);double b=1,y=1;int rb=0,ry=0;
    for(int j=n-1;j>=0;--j){b=std::min(b,p[j].first*n/(j+1));y=std::min(y,p[j].first*n*harmonic/(j+1));bh[p[j].second]=b;by[p[j].second]=y;rb+=b<=alpha;ry+=y<=alpha;}
    Vec out{double(rb),double(ry),harmonic};out.insert(out.end(),bh.begin(),bh.end());out.insert(out.end(),by.begin(),by.end());return out;
}
inline Vec sharpe_hac(const Vec& x){
    require(x.size()>=5);Vec r(x.begin()+1,x.end());int n=r.size(),lag=integer(x[0],0,n-1);double mu=avg(r),q=dot(r,r)/n,v=q-mu*mu;positive(v);
    double s=std::sqrt(v),a=q/(v*s),b=-mu/(2*v*s);Vec psi;
    for(double z:r)psi.push_back(a*(z-mu)+b*(z*z-q));
    double lr=hac_lrv(psi,lag),se=std::sqrt(lr/n),iid=std::sqrt(hac_lrv(psi,0)/n),sr=mu/s;
    return {sr,mu,s,se,iid,lr,sr-1.959963984540054*se,sr+1.959963984540054*se};
}
inline Vec midrank(const Vec& x){
    std::vector<int> o(x.size());std::iota(o.begin(),o.end(),0);std::stable_sort(o.begin(),o.end(),[&](int a,int b){return x[a]<x[b];});Vec r(x.size());
    for(int a=0;a<int(o.size());){int b=a+1;while(b<int(o.size())&&x[o[a]]==x[o[b]])++b;for(int i=a;i<b;++i)r[o[i]]=(a+b+1)/2.;a=b;}return r;
}
inline Vec rank_ic_panel(const Vec& x){
    require(x.size()>=16);int n=integer(x[0],2,10000),lag=integer(x[1],0,n-1);size_t at=2;Vec ic;
    for(int t=0;t<n;++t){require(at<x.size());int k=integer(x[at++],3,10000);require(at+2*size_t(k)<=x.size());Vec a,b;
        for(int j=0;j<k;++j){a.push_back(x[at++]);b.push_back(x[at++]);}Vec r=midrank(a),s=midrank(b);
        double va=var(r),vb=var(s);positive(va);positive(vb);ic.push_back(cov(r,s)/std::sqrt(va*vb));}
    require(at==x.size());double mu=avg(ic),sd=std::sqrt(var(ic)),se=std::sqrt(hac_lrv(ic,lag)/n),p=0;for(double z:ic)p+=z>0;
    Vec out{mu,sd,sd>0?mu/sd:0,se,p/n,double(n)};out.insert(out.end(),ic.begin(),ic.end());return out;
}
}
