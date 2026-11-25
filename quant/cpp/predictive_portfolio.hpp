#pragma once
#include "predictive_neural.hpp"
namespace pdq {
inline Vec simplex(Vec z){
    double mx=*std::max_element(z.begin(),z.end());for(double& v:z)v-=mx;
    Vec s=z;std::sort(s.begin(),s.end(),std::greater<double>());double a=0,tau=0;
    for(size_t i=0;i<s.size();++i){a+=s[i];if(s[i]>(a-1)/(i+1))tau=(a-1)/(i+1);}
    for(double& v:z)v=std::max(0.,v-tau);return z;
}
inline Vec stacking_simplex(const Vec& x){
    require(x.size()>=8);int n=integer(x[0],2,200),k=integer(x[1],2,12);double l=x[2];positive(l);require(x.size()==size_t(3+n*(k+1)));
    Mat P=zeros(n,k),A=zeros(k,k);Vec y(n),b(k),w(k,1./k);size_t at=3;double trace=0;
    for(int i=0;i<n;++i){for(double& v:P[i])v=x[at++];y[i]=x[at++];}
    for(int j=0;j<k;++j){for(int i=0;i<n;++i)b[j]+=P[i][j]*y[i]/n;for(int h=0;h<k;++h){for(int i=0;i<n;++i)A[j][h]+=P[i][j]*P[i][h]/n;if(j==h){A[j][h]+=l;trace+=A[j][h];}}}
    positive(trace);double r=0;
    for(int t=0;t<100000;++t){
        Vec g=mv(A,w),z(k);for(int j=0;j<k;++j)z[j]=w[j]-(g[j]-b[j])/trace;z=simplex(z);r=0;for(int j=0;j<k;++j)r=std::max(r,std::abs(z[j]-w[j]));w=z;if(r<1e-13)break;
    }
    require(r<1e-9);Vec fit=mv(P,w);double loss=0;for(int i=0;i<n;++i)loss+=sq(fit[i]-y[i])/n;
    Vec out{loss,l*dot(w,w),loss+l*dot(w,w),sum(w)};out.insert(out.end(),w.begin(),w.end());out.insert(out.end(),fit.begin(),fit.end());return out;
}
inline Vec neutral_layer_vjp(const Vec& x){
    require(x.size()>=10);int n=integer(x[0],2,40);double gamma=x[1],eta=x[2];positive(gamma);require(eta>=0&&x.size()==size_t(3+3*n+n*n));
    Vec mu(x.begin()+3,x.begin()+3+n),old(x.begin()+3+n,x.begin()+3+2*n),r(x.begin()+3+2*n,x.begin()+3+3*n),ones(n,1.),b(n);
    Mat A=zeros(n,n);size_t at=3+3*n;Mat L=zeros(n,n);for(auto& row:L)for(double& v:row)v=x[at++];
    for(int i=0;i<n;++i)for(int j=0;j<n;++j)A[i][j]=gamma*dot(L[i],L[j])+(i==j?eta:0);
    for(int i=0;i<n;++i)b[i]=mu[i]+eta*old[i];
    Vec a=solve(A,b),u=solve(A,ones);positive(sum(u));double nu=sum(a)/sum(u);Vec w(n),g=solve(A,r);
    double c=sum(g)/sum(u);for(int i=0;i<n;++i){w[i]=a[i]-nu*u[i];g[i]-=c*u[i];}
    Vec out{dot(w,r),nu,sum(w),sum(g)};out.insert(out.end(),w.begin(),w.end());out.insert(out.end(),g.begin(),g.end());return out;
}
inline Vec capped_linear(const Vec& c,const Vec& caps){
    int n=c.size();std::vector<int> ids(n);std::iota(ids.begin(),ids.end(),0);std::stable_sort(ids.begin(),ids.end(),[&](int a,int b){return c[a]<c[b];});Vec w(n);double left=1;
    for(int j:ids){w[j]=std::min(left,caps[j]);left-=w[j];}require(left<1e-10);return w;
}
inline Vec spo_plus(const Vec& x){
    require(x.size()>=6&&x.size()%3==0);int n=x.size()/3;require(n<=1000);
    Vec c(x.begin(),x.begin()+n),p(x.begin()+n,x.begin()+2*n),caps(x.begin()+2*n,x.end()),z(n);
    for(int i=0;i<n;++i){prob(caps[i]);z[i]=2*p[i]-c[i];}require(sum(caps)>=1);
    Vec a=capped_linear(c,caps),b=capped_linear(p,caps),d=capped_linear(z,caps),grad(n);
    double regret=dot(c,b)-dot(c,a),surrogate=dot(z,a)-dot(z,d);for(int i=0;i<n;++i)grad[i]=2*(a[i]-d[i]);
    Vec out{regret,surrogate,dot(c,a),dot(c,b)};for(const auto& v:{a,b,d,grad})out.insert(out.end(),v.begin(),v.end());return out;
}
inline Vec policy_dr(const Vec& x){
    require(x.size()>=10&&(x.size()-2)%8==0);double cap=x[0],floor=x[1];positive(cap);require(floor>0&&floor<=1);
    int n=(x.size()-2)/8;require(n>=2);Vec d,ipw,dr;double wsum=0,w2=0,ips=0,clipped=0,cc=0;
    for(int t=0;t<n;++t){
        size_t i=2+8*t;int a=integer(x[i],0,1);double p=x[i+1],q=x[i+2],r=x[i+3],m0=x[i+4],m1=x[i+5],e0=x[i+6],e1=x[i+7];
        require(p>=floor&&p<=1-floor);prob(q);prob(r);prob(m0);prob(m1);require(e0==0||e0==1);require(e1==0||e1==1);
        require((q==1||e0==1)&&(q==0||e1==1));double target=a?q:1-q,logging=a?p:1-p,w=target/logging,base=(1-q)*m0+q*m1,hat=a?m1:m0;
        d.push_back(base);ipw.push_back(w*r);dr.push_back(base+w*(r-hat));wsum+=w;w2+=w*w;ips+=w*r;
        double v=std::min(w,cap);clipped+=v*r;cc+=v;
    }
    positive(wsum);positive(w2);positive(cc);Vec out{avg(d),avg(ipw),avg(dr),ips/wsum,clipped/cc,wsum*wsum/w2,std::sqrt(var(dr)/n)};
    out.insert(out.end(),dr.begin(),dr.end());return out;
}
inline Vec soft_portfolio_vjp(const Vec& x){
    require(x.size()>=10);int n=integer(x[0],2,60),m=integer(x[1],2,1000);double temp=x[2],gamma=x[3],cost=x[4],eps=x[5];positive(temp);require(gamma>=0&&cost>=0&&eps>0&&x.size()==size_t(6+2*n+m*n));
    Vec score(x.begin()+6,x.begin()+6+n),old(x.begin()+6+n,x.begin()+6+2*n);for(double v:old)prob(v);require(std::abs(sum(old)-1)<1e-9);
    for(double& v:score)v/=temp;Vec w=softmax(score),r(m),g(n);Mat R=zeros(m,n);size_t at=6+2*n;
    for(auto& row:R)for(double& v:row)v=x[at++];
    for(int t=0;t<m;++t)r[t]=dot(w,R[t]);double mean=avg(r),v=0,turn=0;
    for(int t=0;t<m;++t){v+=sq(r[t]-mean)/m;for(int j=0;j<n;++j)g[j]+=R[t][j]*(1-gamma*(r[t]-mean))/m;}
    for(int j=0;j<n;++j){double d=w[j]-old[j],s=std::sqrt(d*d+eps*eps);turn+=s-eps;g[j]-=cost*d/s;}
    double u=dot(g,w);for(int j=0;j<n;++j)g[j]=w[j]*(g[j]-u)/temp;
    Vec out{mean-gamma*v/2-cost*turn,mean,v,turn,sum(g)};out.insert(out.end(),w.begin(),w.end());out.insert(out.end(),g.begin(),g.end());return out;
}
inline Vec ensemble_risk(const Vec& x){
    require(x.size()>=7);int n=integer(x[0],1,50),k=integer(x[1],2,100);require(x.size()==size_t(2+n+k*(1+2*n)));
    Vec w(x.begin()+2,x.begin()+2+n),p(k),mu(k),v(k),out;size_t at=2+n;
    for(int j=0;j<k;++j){p[j]=x[at++];prob(p[j]);Vec a(n),d(n);for(double& z:a)z=x[at++];for(double& z:d){z=x[at++];require(z>=0);}mu[j]=dot(w,a);for(int i=0;i<n;++i)v[j]+=sq(w[i])*d[i];}
    require(std::abs(sum(p)-1)<1e-10);double m=dot(p,mu),within=dot(p,v),between=0;for(int j=0;j<k;++j)between+=p[j]*sq(mu[j]-m);
    out={m,within,between,within+between};out.insert(out.end(),mu.begin(),mu.end());out.insert(out.end(),v.begin(),v.end());return out;
}
}
