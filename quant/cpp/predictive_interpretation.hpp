#pragma once
#include "predictive_kernel.hpp"
namespace pdq {
inline Vec shapley_interaction(const Vec& x){
    require(x.size()>=8);int d=integer(x[0],2,10),n=integer(x[1],1,200);require(x.size()==size_t(3+2*d+d*(d-1)/2+n*d));
    double intercept=x[2];Vec b(x.begin()+3,x.begin()+3+d),q(x.begin()+3+d,x.begin()+3+2*d);
    size_t at=3+2*d;Mat A=zeros(d,d);for(int i=0;i<d;++i)for(int j=i+1;j<d;++j)A[i][j]=x[at++];
    Mat bg=zeros(n,d);for(auto& row:bg)for(double& z:row)z=x[at++];
    auto f=[&](const Vec& z){double v=intercept+dot(b,z);for(int i=0;i<d;++i)for(int j=i+1;j<d;++j)v+=A[i][j]*z[i]*z[j];return v;};
    int N=1<<d;Vec v(N),phi(d),fac(d+1,1);for(int i=1;i<=d;++i)fac[i]=i*fac[i-1];
    for(int mask=0;mask<N;++mask){for(const auto& row:bg){Vec z=row;for(int j=0;j<d;++j)if(mask&(1<<j))z[j]=q[j];v[mask]+=f(z)/n;}}
    for(int mask=0;mask<N;++mask){int k=0;for(int j=0;j<d;++j)k+=bool(mask&(1<<j));for(int j=0;j<d;++j)if(!(mask&(1<<j)))phi[j]+=fac[k]*fac[d-k-1]/fac[d]*(v[mask|(1<<j)]-v[mask]);}
    Vec out{v[0],v.back(),sum(phi),v.back()-v[0]-sum(phi)};out.insert(out.end(),phi.begin(),phi.end());return out;
}
inline Vec ale_bilinear(const Vec& x){
    require(x.size()>=10);int k=integer(x[0],2,100),n=integer(x[1],k,100000);double a=x[2],b=x[3],c=x[4];require(x.size()==size_t(6+k+2*n));
    Vec edge(x.begin()+5,x.begin()+6+k),count(k),diff(k),out;for(int j=1;j<=k;++j)require(edge[j]>edge[j-1]);
    size_t at=6+k;for(int i=0;i<n;++i){double z=x[at++],w=x[at++];require(z>=edge.front()&&z<=edge.back());int j=std::min(k-1,int(std::upper_bound(edge.begin(),edge.end(),z)-edge.begin())-1);double l=edge[j],u=edge[j+1];
        diff[j]+=a*(u-l)+b*(u*u-l*l)+c*w*(u-l);++count[j];}
    Vec level(k);double accum=0;for(int j=0;j<k;++j){positive(count[j]);diff[j]/=count[j];accum+=diff[j];level[j]=accum;}
    double center=dot(count,level)/n;for(double& z:level){z-=center;}
    out={center,dot(count,level)/n};for(const auto& a0:{count,diff,level})out.insert(out.end(),a0.begin(),a0.end());return out;
}
inline Vec conditional_permutation(const Vec& x){
    require(x.size()>=9&&(x.size()-1)%4==0);double beta=x[0];int n=(x.size()-1)/4;require(n<=100000);
    struct Moments{double n=0,s=0,q=0;};std::map<int,Moments> groups;Moments all;Vec p(n),y(n),b(n);std::vector<int> id(n);double base=0;
    for(int i=0;i<n;++i){size_t j=1+4*i;id[i]=integer(x[j],0,1000000);p[i]=x[j+1];y[i]=x[j+2];b[i]=x[j+3];base+=sq(y[i]-b[i]-beta*p[i])/n;
        for(Moments* m:{&groups[id[i]],&all}){++m->n;m->s+=p[i];m->q+=sq(p[i]);}}
    double marginal=0,conditional=0;Vec out;
    auto expected=[&](double z,const Moments& m){return z*z-2*z*beta*m.s/m.n+beta*beta*m.q/m.n;};
    for(int i=0;i<n;++i){double z=y[i]-b[i],a=expected(z,all),c=expected(z,groups[id[i]]);marginal+=a/n;conditional+=c/n;out.push_back(c);}
    out.insert(out.begin(),{base,marginal,conditional,marginal-base,conditional-base});return out;
}
inline Vec knockoff_plus(const Vec& x){
    require(x.size()>=3);double q=x[0];require(q>0&&q<1);Vec w(x.begin()+1,x.end());std::set<double> abs;for(double v:w)if(v!=0)abs.insert(std::abs(v));
    double t=0,fdp=0,chosen=0;int positives=0,negatives=0;
    for(double z:abs){int p=0,n=0;for(double a:w){p+=a>=z;n+=a<=-z;}double f=(1.+n)/std::max(1,p);if(f<=q){t=z;fdp=f;positives=p;negatives=n;chosen=1;break;}}
    Vec out{chosen,t,double(positives),double(negatives),fdp};for(double v:w)out.push_back(chosen&&v>=t?1.:0.);return out;
}
inline Vec local_linear_smoother(const Vec& x){
    require(x.size()>=6&&(x.size()-2)%2==0);double q=x[0],h=x[1];positive(h);int n=(x.size()-2)/2;require(n<=100000);
    Vec z(n),y(n),a(n);double best=std::numeric_limits<double>::infinity();
    for(int i=0;i<n;++i){z[i]=x[2+2*i]-q;y[i]=x[3+2*i];a[i]=.5*sq(z[i]/h);best=std::min(best,a[i]);}
    for(double& v:a){v=std::exp(best-v);}
    double sw=sum(a);positive(sw);double m=dot(a,z)/sw,ym=dot(a,y)/sw,v=0,c=0;for(int i=0;i<n;++i){v+=a[i]*sq(z[i]-m);c+=a[i]*(z[i]-m)*(y[i]-ym);}positive(v);
    double slope=c/v,pred=ym-slope*m;Vec w(n);for(int i=0;i<n;++i)w[i]=a[i]/sw-m*a[i]*(z[i]-m)/v;
    Vec out{pred,slope,sw*sw/dot(a,a),sum(w),dot(w,z)};out.insert(out.end(),w.begin(),w.end());return out;
}
inline Vec group_calibration(const Vec& x){
    require(x.size()>=7);int n=integer(x[0],2,5000),k=integer(x[1],1,100);double eps=x[2];require(eps>0&&eps<1&&x.size()==size_t(3+n*(2+k)));
    Vec p(n),y(n),count(k);Mat G=zeros(k,n);size_t at=3;
    for(int i=0;i<n;++i){p[i]=x[at++];prob(p[i]);y[i]=x[at++];require(y[i]==0||y[i]==1);for(int j=0;j<k;++j){G[j][i]=integer(x[at++],0,1);count[j]+=G[j][i];}}
    for(double v:count){positive(v);}
    auto brier=[&](){double a=0;for(int i=0;i<n;++i)a+=sq(y[i]-p[i])/n;return a;};
    double start=brier(),worst=0;int it=0;Vec res(k);
    auto residual=[&](){Vec e(n);for(int i=0;i<n;++i)e[i]=y[i]-p[i];for(int j=0;j<k;++j)res[j]=dot(G[j],e)/count[j];int j=0;for(int a=1;a<k;++a)if(std::abs(res[a])>std::abs(res[j])+1e-14)j=a;return j;};
    while(it<100000){int j=residual();if(std::abs(res[j])<=eps)break;for(int i=0;i<n;++i)if(G[j][i])p[i]=std::clamp(p[i]+res[j],0.,1.);++it;}
    residual();for(double v:res)worst=std::max(worst,std::abs(v));require(worst<=eps+1e-12);
    Vec out{start,brier(),worst,double(it)};out.insert(out.end(),p.begin(),p.end());out.insert(out.end(),res.begin(),res.end());return out;
}
}
