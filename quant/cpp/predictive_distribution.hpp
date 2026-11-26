#pragma once
#include "predictive_kernel.hpp"
namespace pdq {
inline Vec weighted_conformal(const Vec& x){
    require(x.size()>=6&&(x.size()-2)%2==0);double alpha=x[0],test=x[1];require(alpha>0&&alpha<1);positive(test);int n=(x.size()-2)/2;
    std::vector<std::pair<double,double>> a;double total=0,w2=0;
    for(int i=0;i<n;++i){double s=x[2+2*i],w=x[3+2*i];require(s>=0&&w>=0);total+=w;w2+=w*w;a.push_back({s,w});}
    positive(total);double threshold=(1-alpha)*(total+test),cum=0,q=0,finite=0;std::sort(a.begin(),a.end());
    for(auto z:a){cum+=z.second;if(cum>=threshold){q=z.first;finite=1;break;}}
    return {finite,q,total/(total+test),test/(total+test),threshold,total*total/w2};
}
inline Vec ulsif_ratio(const Vec& x){
    require(x.size()>=8);int n=integer(x[0],2,200),m=integer(x[1],2,200),k=integer(x[2],1,50),q=integer(x[3],1,200);double g=x[4],l=x[5];positive(g);positive(l);require(x.size()==size_t(6+n+m+k+q));
    Vec src(x.begin()+6,x.begin()+6+n),target(x.begin()+6+n,x.begin()+6+n+m),centers(x.begin()+6+n+m,x.begin()+6+n+m+k),queries(x.begin()+6+n+m+k,x.end());
    auto basis=[&](double z){Vec v;for(double c:centers)v.push_back(rbf(c,z,g));return v;};
    Mat H=zeros(k,k);Vec h(k);
    for(double z:src){Vec a=basis(z);for(int i=0;i<k;++i)for(int j=0;j<k;++j)H[i][j]+=a[i]*a[j]/n;}
    for(double z:target){Vec a=basis(z);for(int i=0;i<k;++i)h[i]+=a[i]/m;}
    for(int i=0;i<k;++i)H[i][i]+=l;
    Vec a=solve(H,h),b=a;for(double& v:b){v=std::max(0.,v);}
    double obj=.5*dot(a,mv(H,a))-dot(h,a),clipobj=.5*dot(b,mv(H,b))-dot(h,b),sw=0,sw2=0;
    for(double z:src){double w=dot(b,basis(z));sw+=w;sw2+=w*w;}require(sw2>0);
    Vec out{obj,clipobj,sw/n,sw*sw/sw2};out.insert(out.end(),a.begin(),a.end());out.insert(out.end(),b.begin(),b.end());
    for(double z:queries){out.push_back(dot(b,basis(z)));}
    return out;
}
struct Ensemble{int n,d;Vec y,p;Mat X;};
inline Ensemble ensemble(const Vec& x,size_t offset){
    require(x.size()>=offset+2);int n=integer(x[offset],2,300),d=integer(x[offset+1],1,50);require(x.size()==offset+2+d+n*(d+1));
    size_t at=offset+2;Ensemble a{n,d,Vec(d),Vec(n),zeros(n,d)};
    for(double& v:a.y){v=x[at++];}
    for(int i=0;i<n;++i){a.p[i]=x[at++];prob(a.p[i]);for(double& v:a.X[i])v=x[at++];}require(std::abs(sum(a.p)-1)<1e-10);return a;
}
inline Vec energy_score(const Vec& x){
    require(x.size()>=8);double beta=x[0];require(beta>0&&beta<2);auto a=ensemble(x,1);auto dist=[&](const Vec& u,const Vec& v){double z=0;for(int i=0;i<a.d;++i)z+=sq(u[i]-v[i]);return std::pow(z,beta/2);};
    double first=0,second=0;for(int i=0;i<a.n;++i){first+=a.p[i]*dist(a.X[i],a.y);for(int j=i+1;j<a.n;++j)second+=a.p[i]*a.p[j]*dist(a.X[i],a.X[j]);}
    return {first-second,first,second};
}
inline Vec variogram_score(const Vec& x){
    require(x.size()>=9);double power=x[0];require(power>0&&power<2);int d=integer(x[2],2,50);auto a=ensemble(x,1);
    double loss=0;Vec out{0};for(int i=0;i<d;++i)for(int j=i+1;j<d;++j){double observed=std::pow(std::abs(a.y[i]-a.y[j]),power),expected=0;for(int k=0;k<a.n;++k)expected+=a.p[k]*std::pow(std::abs(a.X[k][i]-a.X[k][j]),power);double e=sq(observed-expected);loss+=e;out.insert(out.end(),{observed,expected,e});}out[0]=loss;return out;
}
inline Vec ipcw_brier_curve(const Vec& x){
    require(x.size()>=9);int n=integer(x[0],2,10000),m=integer(x[1],2,1000);require(x.size()==size_t(2+m+n*(3+2*m)));
    Vec times(x.begin()+2,x.begin()+2+m);for(int j=0;j<m;++j)require(times[j]>=0&&(j==0||times[j]>times[j-1]));
    Vec b(m),event(m),alive(m),missing(m),weight(m);size_t at=2+m;
    for(int i=0;i<n;++i){double t=x[at++];require(t>=0);int delta=integer(x[at++],0,1);double ge=x[at++];require(ge>0&&ge<=1);double prev=1,prevG=1;
        for(int j=0;j<m;++j){double p=x[at++],g=x[at++];prob(p);require(g>0&&g<=1&&p<=prev+1e-12&&g<=prevG+1e-12);prev=p;prevG=g;
            if(t<=times[j]&&delta){b[j]+=sq(p)/ge;++event[j];weight[j]+=1/ge;}
            else if(t>times[j]){b[j]+=sq(1-p)/g;++alive[j];weight[j]+=1/g;}
            else ++missing[j];
        }
    }
    double area=0;for(double& v:b){v/=n;}for(int j=1;j<m;++j)area+=(times[j]-times[j-1])*(b[j]+b[j-1])/2;
    Vec out{area/(times.back()-times.front())};for(const auto& a:{b,event,alive,missing,weight})out.insert(out.end(),a.begin(),a.end());return out;
}
inline Vec conformal_risk_grid(const Vec& x){
    require(x.size()>=8);int n=integer(x[0],2,10000),k=integer(x[1],2,1000);double alpha=x[2],B=x[3];positive(B);require(alpha>0&&alpha<B&&x.size()==size_t(4+k+n*k));
    Vec thresholds(x.begin()+4,x.begin()+4+k),s(k),bound(k);for(int j=1;j<k;++j)require(thresholds[j]>thresholds[j-1]);
    size_t at=4+k;for(int i=0;i<n;++i){double prev=B;for(int j=0;j<k;++j){double z=x[at++];require(z>=0&&z<=prev&&z<=B);prev=z;s[j]+=z;}}
    int chosen=-1;for(int j=0;j<k;++j){bound[j]=(s[j]+B)/(n+1);s[j]/=n;if(chosen==-1&&bound[j]<=alpha)chosen=j;}
    Vec out{chosen<0?0.:1.,chosen<0?0.:thresholds[chosen],double(chosen)};out.insert(out.end(),s.begin(),s.end());out.insert(out.end(),bound.begin(),bound.end());return out;
}
}
