#pragma once
#include "continuous_research_common.hpp"
namespace crq {
// Bivariate linear Gaussian filter and RTS smoother with arbitrary missing coordinate observations.
// At t=0 the prior is supplied directly. For t>0 x_t=F x_{t-1}+noise.
inline Vec kalman_missing(const Vec& x){
    check(x.size()>=18);int n=integer(x[0],1,500);
    check(x.size()==size_t(15+3*n));
    M2 F{{{x[1],x[2]},{x[3],x[4]}}},Q=covariance(x[5],x[6],x[7]),P=covariance(x[10],x[11],x[12]);
    V2 m{x[8],x[9]},R{x[13],x[14]};
    check(spd(Q)&&spd(P)&&R[0]>0&&R[1]>0);
    std::vector<V2> pred(n),filt(n),smooth(n);
    std::vector<M2> pp(n),pf(n),ps(n);
    double ll=0;int seen=0;
    for(int t=0;t<n;++t){
        if(t){m=mult(F,m);P=sym(sum(mult(mult(F,P),trans(F)),Q));}
        check(spd(P));pred[t]=m;pp[t]=P;
        int mask=integer(x[15+3*t],0,3);
        for(int j=0;j<2;++j)if(mask&(1<<j)){
            ++seen;double error=x[16+3*t+j]-m[j],S=P[j][j]+R[j];check(S>0);
            V2 h{};h[j]=1;
            V2 K=scale(mult(P,h),1/S);m=add(m,scale(K,error));
            M2 A=sum(eye(),outer(K,h),-1);
            M2 noise=outer(K,K);for(auto& row:noise)for(double& z:row)z*=R[j];
            P=sym(sum(mult(mult(A,P),trans(A)),noise));
            ll-=.5*(std::log(2*std::acos(-1.)*S)+error*error/S);
        }
        check(spd(P));filt[t]=m;pf[t]=P;
    }
    smooth.back()=filt.back();ps.back()=pf.back();
    for(int t=n-2;t>=0;--t){
        M2 G=mult(mult(pf[t],trans(F)),inv(pp[t+1]));
        smooth[t]=add(filt[t],mult(G,sub(smooth[t+1],pred[t+1])));
        ps[t]=sym(sum(pf[t],mult(mult(G,sum(ps[t+1],pp[t+1],-1)),trans(G))));
        check(spd(ps[t]));
    }
    Vec out{ll,double(seen)};
    for(int t=0;t<n;++t){append(out,filt[t],pf[t]);append(out,smooth[t],ps[t]);}
    return out;
}
// Third-degree spherical-radial cubature and fifth-order tensor Gaussian quadrature.
inline Vec nonlinear_gaussian_moments(const Vec& x){
    check(x.size()==7);
    double a=x[0],b=x[1],s=x[2],t=x[3],rho=x[4],c=x[5],d=x[6];
    check(s>0&&t>0&&std::abs(rho)<1);
    M2 P=covariance(s*s,rho*s*t,t*t),L=cholesky(P);
    double mu=c*a+d*b,var=c*c*P[0][0]+2*c*d*P[0][1]+d*d*P[1][1];
    double E=std::exp(mu+.5*var),Ex=a*a+s*s,cxz=c*s*s+d*P[0][1];
    Vec out{Ex,E,2*std::pow(s,4)+4*a*a*s*s,E*(sq(a+cxz)+s*s-Ex),E*E*std::expm1(var)};
    auto transform=[&](V2 p){return V2{p[0]*p[0],std::exp(c*p[0]+d*p[1])};};
    auto eval=[&](const std::vector<V2>& pts,const Vec& weights){
        std::vector<V2> val;V2 mean{};
        for(size_t i=0;i<pts.size();++i){V2 y=transform(add(V2{a,b},mult(L,pts[i])));val.push_back(y);mean=add(mean,scale(y,weights[i]));}
        M2 cov{};
        for(size_t i=0;i<val.size();++i){M2 z=outer(sub(val[i],mean),sub(val[i],mean));for(auto& row:z)for(double& v:row)v*=weights[i];cov=sum(cov,z);}
        append(out,mean,cov);
    };
    double h=std::sqrt(2.);
    eval({{h,0},{-h,0},{0,h},{0,-h}},{.25,.25,.25,.25});
    std::vector<V2> nodes;Vec weights;
    for(int i=0;i<5;++i)for(int j=0;j<5;++j){nodes.push_back({gh_z[i],gh_z[j]});weights.push_back(gh_w[i]*gh_w[j]);}
    eval(nodes,weights);
    return out;
}
// Scalar nonlinear Gaussian assumed-density filtering by positive quadrature and EKF.
inline Vec gaussian_density_filter(const Vec& x){
    check(x.size()>=7);double m=x[0],v=x[1],phi=x[2],q=x[3],r=x[4];int power=integer(x[5],1,2);
    check(v>0&&q>0&&r>0&&x.size()<=506);
    double em=m,ev=v,ghll=0,ekll=0;Vec out{0,0};
    auto h=[&](double z){return power==1?z:z*z;};
    for(size_t i=6;i<x.size();++i){
        double pm=phi*m,pv=phi*phi*v+q,y=x[i],maxlog=-std::numeric_limits<double>::infinity();
        Vec z(5),weights(5);
        for(int j=0;j<5;++j){z[j]=pm+std::sqrt(pv)*gh_z[j];weights[j]=std::log(gh_w[j])-.5*(std::log(2*std::acos(-1.)*r)+sq(y-h(z[j]))/r);maxlog=std::max(maxlog,weights[j]);}
        double den=0;
        for(double& w:weights){w=std::exp(w-maxlog);den+=w;}
        ghll+=maxlog+std::log(den);m=0;v=0;
        for(int j=0;j<5;++j)m+=z[j]*weights[j]/den;
        for(int j=0;j<5;++j)v+=sq(z[j]-m)*weights[j]/den;
        em*=phi;ev=phi*phi*ev+q;double derivative=power==1?1:2*em;
        double S=derivative*derivative*ev+r,K=ev*derivative/S,e=y-h(em);
        ekll-=.5*(std::log(2*std::acos(-1.)*S)+e*e/S);em+=K*e;ev=sq(1-K*derivative)*ev+K*K*r;
        out.insert(out.end(),{m,v,em,ev});
    }
    out[0]=ghll;out[1]=ekll;return out;
}
// Deterministic scalar-observation ensemble square-root analysis.
inline Vec ensemble_analysis(const Vec& x){
    check(x.size()>=9);int n=integer(x[0],2,1000);check(x.size()==size_t(5+2*n));
    double y=x[1],r=x[2];V2 h{x[3],x[4]};check(r>0&&dot(h,h)>0);
    std::vector<V2> z;V2 mean{};
    for(int i=0;i<n;++i){z.push_back({x[5+2*i],x[6+2*i]});mean=add(mean,scale(z.back(),1./n));}
    M2 P{};for(auto a:z)P=sum(P,outer(sub(a,mean),sub(a,mean)),1./(n-1));
    V2 ph=mult(P,h);double S=dot(h,ph)+r;check(S>0);
    V2 K=scale(ph,1/S),posterior=add(mean,scale(K,y-dot(h,mean)));
    M2 target=sym(sum(P,outer(K,ph),-1));
    double alpha=1/(1+std::sqrt(r/S));std::vector<V2> updated;V2 avg{};
    for(auto a:z){V2 d=sub(a,mean),v=add(posterior,sub(d,scale(K,alpha*dot(h,d))));updated.push_back(v);avg=add(avg,scale(v,1./n));}
    M2 cov{};for(auto a:updated)cov=sum(cov,outer(sub(a,avg),sub(a,avg)),1./(n-1));
    double err=0;for(int i=0;i<2;++i)for(int j=0;j<2;++j)err=std::max(err,std::abs(cov[i][j]-target[i][j]));
    Vec out;append(out,mean,P);append(out,posterior,target);append(out,avg,cov);out.push_back(err);
    for(auto a:updated)out.insert(out.end(),a.begin(),a.end());
    return out;
}
}
