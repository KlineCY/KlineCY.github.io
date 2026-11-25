#pragma once
#include "finite_state_inference.hpp"
#include "finite_state_credit.hpp"
#include "finite_state_control.hpp"
#include "finite_state_derivatives.hpp"
#include "finite_state_queue.hpp"
namespace fsq {
// Age-expanded online filter. The last segment need not finish at the observation boundary.
inline Vec duration_filter(const Vec& x){
    auto a=duration_input(x);
    int k=a.initial.size(),d=a.duration[0].size(),n=a.observations.size(),m=a.emission[0].size();
    Mat survive(k,Vec(d+1));
    for(int j=0;j<k;++j)for(int l=d-1;l>=0;--l)survive[j][l]=survive[j][l+1]+a.duration[j][l];
    auto advance=[&](const Mat& f){
        Mat g(k,Vec(d));
        for(int j=0;j<k;++j)for(int l=0;l<d;++l)if(f[j][l]>0){
            require(survive[j][l]>0);
            double p=a.duration[j][l]/survive[j][l];
            for(int i=0;i<k;++i)g[i][0]+=f[j][l]*p*a.transition[j][i];
            if(l+1<d)g[j][l+1]+=f[j][l]*(1-p);
        }
        return g;
    };
    Mat f(k,Vec(d));
    for(int j=0;j<k;++j)f[j][0]=a.initial[j];
    double ll=0;
    Vec out;
    for(int t=0;t<n;++t){
        double z=0;
        for(int j=0;j<k;++j)for(int l=0;l<d;++l){f[j][l]*=a.emission[j][a.observations[t]];z+=f[j][l];}
        require(z>0);ll+=std::log(z);
        for(auto& row:f)for(double& v:row)v/=z;
        if(t+1<n)f=advance(f);
    }
    out.push_back(ll);
    for(const auto& row:f)out.push_back(sum(row));
    double age=0;for(int j=0;j<k;++j)for(int l=0;l<d;++l)age+=(l+1)*f[j][l];out.push_back(age);
    Mat g=advance(f);Vec state(k),predict(m);
    for(int j=0;j<k;++j){state[j]=sum(g[j]);for(int i=0;i<m;++i)predict[i]+=state[j]*a.emission[j][i];}
    out.insert(out.end(),state.begin(),state.end());out.insert(out.end(),predict.begin(),predict.end());
    for(const auto& row:f)out.insert(out.end(),row.begin(),row.end());
    return out;
}
// A systemic-state credit mixture with posterior state weights and a TV stress on these weights.
inline Vec credit_posterior_stress(const Vec& x){
    require(x.size()>=8);int k=count(x[0],2,20),n=count(x[1],1,100),obs=count(x[2],0,n);
    int attach=count(x[3],0,20000),detach=count(x[4],1,20000);double radius=x[5];
    require(detach>attach&&x.size()==size_t(6+n+k*(n+1)));probability(radius);
    Vec weights(x.begin()+6,x.begin()+6+n);for(double w:weights)count(w,1,2000);
    Vec data{double(k),double(n),double(obs)},prior,loss;
    size_t at=6+n;
    for(int s=0;s<k;++s){
        double p=x[at++];prior.push_back(p);data.push_back(p);
        Vec tr{double(attach),double(detach)};
        for(int i=0;i<n;++i){double z=x[at++];data.push_back(z);tr.insert(tr.end(),{weights[i],z});}
        loss.push_back(tranche_sensitivity(tr)[0]);
    }
    Vec mix=mixture_count(data),post(mix.begin()+6,mix.begin()+6+k),stress{radius};
    for(int i=0;i<k;++i)stress.insert(stress.end(),{post[i],-loss[i]});
    Vec worst=worst_tv(stress);
    double priorEL=std::inner_product(prior.begin(),prior.end(),loss.begin(),0.);
    double postEL=std::inner_product(post.begin(),post.end(),loss.begin(),0.);
    Vec out{mix[4],priorEL,postEL,-worst[1],-worst[1]-postEL,worst[2]};
    out.insert(out.end(),post.begin(),post.end());out.insert(out.end(),worst.begin()+4,worst.end());out.insert(out.end(),loss.begin(),loss.end());return out;
}
// Finite horizon robust MDP with independent row-wise total variation balls.
inline Vec robust_mdp(const Vec& x){
    require(x.size()>=8);double eps=x[0];probability(eps);
    Vec y(x.begin()+1,x.end());MDP a=mdp_input(y);
    require(a.horizon<=100&&a.n<=10&&a.m<=10);
    Vec robust(a.n),nominal(a.n);
    std::vector<std::vector<int>> pol(a.horizon,std::vector<int>(a.n));
    std::vector<Mat> worst(a.horizon,Mat(a.n,Vec(a.n)));
    for(int t=a.horizon-1;t>=0;--t){
        Vec r(a.n,-std::numeric_limits<double>::infinity()),v=r;
        for(int i=0;i<a.n;++i){
            for(int b=0;b<a.m;++b){
                Vec arg{eps};
                for(int j=0;j<a.n;++j)arg.insert(arg.end(),{a.p[i][b][j],robust[j]});
                Vec w=worst_tv(arg);
                double q=a.reward[i][b]+a.discount*w[1];
                double n=a.reward[i][b]+a.discount*std::inner_product(a.p[i][b].begin(),a.p[i][b].end(),nominal.begin(),0.);
                v[i]=std::max(v[i],n);
                if(q>r[i]+1e-12){r[i]=q;pol[t][i]=b;worst[t][i]=Vec(w.begin()+4,w.end());}
            }
        }
        robust.swap(r);nominal.swap(v);
    }
    Vec mass(a.n),base(a.n);mass[a.initial]=base[a.initial]=1;
    double gain=0,basegain=0,df=1,tv=0;
    for(int t=0;t<a.horizon;++t){
        Vec nm(a.n),nb(a.n);
        for(int i=0;i<a.n;++i){
            int b=pol[t][i];gain+=df*mass[i]*a.reward[i][b];basegain+=df*base[i]*a.reward[i][b];
            for(int j=0;j<a.n;++j){
                nm[j]+=mass[i]*worst[t][i][j];nb[j]+=base[i]*a.p[i][b][j];
                tv+=mass[i]*std::abs(worst[t][i][j]-a.p[i][b][j])/2;
            }
        }
        mass.swap(nm);base.swap(nb);df*=a.discount;
    }
    return {robust[a.initial],nominal[a.initial],basegain,gain,robust[a.initial]-gain,tv,sum(mass),double(pol[0][a.initial])};
}
// Risk-aware discrete exposure. A log-wealth penalty is charged once when the drawdown limit is crossed.
inline Vec drawdown_policy(const Vec& x){
    require(x.size()>=9);int n=count(x[0],1,100),k=count(x[1],2,6),m=count(x[2],2,12),initial=count(x[3],0,k-1),old=count(x[4],0,m-1);
    int H=count(x[5],1,100),cost=count(x[6],0,20);double step=x[7],pen=x[8];
    require(step>0&&step<=1&&pen>=0&&x.size()==size_t(9+m*k+k*k));
    std::vector<std::vector<int>> move(m,std::vector<int>(k));
    size_t at=9;
    for(int b=0;b<m;++b)for(int j=0;j<k;++j)move[b][j]=count(x[at++],-20,20);
    Mat p(k,Vec(k));
    for(int i=0;i<k;++i){for(int j=0;j<k;++j)p[i][j]=x[at++];distribution(p[i]);}
    // The absorbing drawdown state H represents a previously crossed threshold.
    auto index=[&](int i,int a,int h){return (i*m+a)*(H+1)+h;};
    int states=k*m*(H+1);
    auto next=[&](int h,int a,int b,int j){
        int charge=cost*std::abs(a-b);
        // Trading cost occurs before the next return: an intraperiod breach counts.
        int before=h+charge,after=std::max(0,before-move[b][j]);
        if(h==H||before>=H||after>=H)return H;
        return after;
    };
    Vec v(states);
    std::vector<std::vector<int>> pol(n,std::vector<int>(states));
    for(int t=n-1;t>=0;--t){
        Vec u(states,-std::numeric_limits<double>::infinity());
        for(int i=0;i<k;++i)for(int a=0;a<m;++a)for(int h=0;h<=H;++h){
            int s=index(i,a,h);
            for(int b=0;b<m;++b){
                int c=cost*std::abs(a-b);double z=0;
                for(int j=0;j<k;++j){
                    int nh=next(h,a,b,j);
                    z+=p[i][j]*(step*(move[b][j]-c)-pen*(h<H&&nh==H)+v[index(j,b,nh)]);
                }
                if(z>u[s]+1e-12){u[s]=z;pol[t][s]=b;}
            }
        }
        v.swap(u);
    }
    int start=index(initial,old,0);Vec mass(states);mass[start]=1;
    double growth=0,turn=0;
    for(int t=0;t<n;++t){
        Vec u(states);
        for(int i=0;i<k;++i)for(int a=0;a<m;++a)for(int h=0;h<=H;++h){
            int s=index(i,a,h),b=pol[t][s],c=cost*std::abs(a-b);
            turn+=mass[s]*std::abs(a-b);
            for(int j=0;j<k;++j){double z=mass[s]*p[i][j];int nh=next(h,a,b,j);u[index(j,b,nh)]+=z;growth+=z*step*(move[b][j]-c);}
        }
        mass.swap(u);
    }
    double breach=0;
    for(int i=0;i<k;++i)for(int a=0;a<m;++a)breach+=mass[index(i,a,H)];
    double obj=growth-pen*breach;
    return {v[start],double(pol[0][start]),growth,breach,turn,obj,v[start]-obj,sum(mass)};
}
// Arithmetic average payoff in an additive, risk-neutral integer tree at zero rates.
inline Vec asian_additive(const Vec& x){
    require(x.size()==5);int s=count(x[0],1,100000),jump=count(x[1],1,1000),n=count(x[2],1,80);
    double strike=x[3],cp=x[4];require(s>n*jump&&strike>0&&(cp==1||cp==-1));
    // At time t, j is the signed price displacement and a is the sum of signed displacements.
    std::map<std::pair<int,int>,double> f;f[{0,0}]=1;
    for(int t=0;t<n;++t){
        std::map<std::pair<int,int>,double> g;
        for(const auto& z:f)for(int a:{-1,1}){
            int j=z.first.first+a,area=z.first.second+j;
            g[{j,area}]+=.5*z.second;
        }
        f.swap(g);
    }
    double value=0,plain=0,mean=0,var=0,mass=0,prob=0;
    for(auto z:f){
        double avg=s+double(jump)*z.first.second/(n+1),end=s+jump*z.first.first,w=z.second;
        value+=w*std::max(0.,cp*(avg-strike));plain+=w*std::max(0.,cp*(end-strike));
        mean+=w*avg;var+=w*sq(avg-s);mass+=w;prob+=w*(cp*(avg-strike)>0);
    }
    return {value,plain,mean,var,prob,mass,double(f.size())};
}
// Posterior-aware quote decision with hidden two-state liquidity and independent state transition after each event.
inline Vec hidden_queue_choice(const Vec& x){
    require(x.size()==14);int n=count(x[0],1,7),q=count(x[1],0,10);
    double b=x[2],wait=x[3],fill=x[4],market=x[5],shock=x[6];probability(b);
    require(wait>=0&&market>=0&&shock>=0);
    Mat p(2,Vec(4));
    for(int i=0;i<2;++i){p[i]={x[7+3*i],x[8+3*i],x[9+3*i],1-x[7+3*i]-x[8+3*i]-x[9+3*i]};distribution(p[i]);}
    double switchp=x[13];probability(switchp);
    // The posterior after observing an event is propagated through a symmetric regime transition.
    auto update=[&](double prior,int e){
        double a=(1-prior)*p[0][e],c=prior*p[1][e],prob=a+c;
        double post=prob>0?c/prob:prior;
        return std::pair<double,double>{prob,switchp+(1-2*switchp)*post};
    };
    auto evaluate=[&](auto&& self,int t,int j,double prior)->std::pair<double,int>{
        if(t==0)return {market,0};
        double hold=wait;
        for(int e=0;e<4;++e){
            auto [pr,next]=update(prior,e);if(pr==0)continue;
            double v=e==2?market+shock:(e==0&&j==0?fill:self(self,t-1,e==0||e==1?std::max(0,j-1):j,next).first);
            hold+=pr*v;
        }
        return hold<market-1e-12?std::pair<double,int>{hold,1}:std::pair<double,int>{market,0};
    };
    // Fully observable state lower bound: transition happens after observing the current event.
    Mat v(2,Vec(q+1,market));
    for(int t=1;t<=n;++t){
        Mat u(2,Vec(q+1));
        for(int i=0;i<2;++i)for(int j=0;j<=q;++j){
            double hold=wait;
            auto cont=[&](int l){return (1-switchp)*v[i][l]+switchp*v[1-i][l];};
            hold+=p[i][0]*(j==0?fill:cont(j-1))+p[i][1]*cont(std::max(0,j-1))+p[i][2]*(market+shock)+p[i][3]*cont(j);
            u[i][j]=std::min(market,hold);
        }
        v.swap(u);
    }
    auto result=evaluate(evaluate,n,q,b);
    double lower=(1-b)*v[0][q]+b*v[1][q];
    auto [pr,posterior]=update(b,0);
    return {result.first,double(result.second),lower,market,result.first-lower,pr,posterior};
}
// Finite support distributionally robust risk: transport budget and exact piecewise-linear dual.
inline Vec transport_stress(const Vec& x){
    require(x.size()>=5);int m=count(x[0],1,30),n=count(x[1],1,30);double budget=x[2];require(budget>=0&&x.size()==size_t(3+m+n+m*n));
    Vec p(x.begin()+3,x.begin()+3+m);distribution(p);
    Vec payoff(x.begin()+3+m,x.begin()+3+m+n);
    Mat c(m,Vec(n));size_t at=3+m+n;
    Vec cand{0};
    for(int i=0;i<m;++i){
        double low=std::numeric_limits<double>::infinity();
        for(int j=0;j<n;++j){c[i][j]=x[at++];require(c[i][j]>=0);low=std::min(low,c[i][j]);}
        require(low==0);
        for(int j=0;j<n;++j)for(int l=0;l<j;++l)if(c[i][j]!=c[i][l]){
            double a=(payoff[j]-payoff[l])/(c[i][j]-c[i][l]);if(a>0&&std::isfinite(a))cand.push_back(a);
        }
    }
    double best=std::numeric_limits<double>::infinity(),lambda=0;
    for(double a:cand){
        double z=a*budget;
        for(int i=0;i<m;++i){
            double mx=-std::numeric_limits<double>::infinity();
            for(int j=0;j<n;++j)mx=std::max(mx,payoff[j]-a*c[i][j]);
            z+=p[i]*mx;
        }
        if(z<best-1e-11){best=z;lambda=a;}
    }
    // A primal optimizer is obtained by mixing tied active plans with low and high transport cost.
    double cl=0,ch=0,pl=0,ph=0;
    std::vector<int> lo(m),hi(m);
    for(int i=0;i<m;++i){
        double mx=-std::numeric_limits<double>::infinity();
        for(int j=0;j<n;++j)mx=std::max(mx,payoff[j]-lambda*c[i][j]);
        lo[i]=hi[i]=-1;
        for(int j=0;j<n;++j)if(std::abs(payoff[j]-lambda*c[i][j]-mx)<=1e-9*(1+std::abs(mx))){
            if(lo[i]<0||c[i][j]<c[i][lo[i]])lo[i]=j;
            if(hi[i]<0||c[i][j]>c[i][hi[i]])hi[i]=j;
        }
        require(lo[i]>=0&&hi[i]>=0);cl+=p[i]*c[i][lo[i]];ch+=p[i]*c[i][hi[i]];
        pl+=p[i]*payoff[lo[i]];ph+=p[i]*payoff[hi[i]];
    }
    require(cl<=budget+1e-8);
    double a=lambda==0?0:ch>cl?std::clamp((budget-cl)/(ch-cl),0.,1.):0;
    double used=(1-a)*cl+a*ch,primal=(1-a)*pl+a*ph;
    Vec out{best,lambda,primal,used,best-primal};Vec dest(n);
    for(int i=0;i<m;++i){dest[lo[i]]+=(1-a)*p[i];dest[hi[i]]+=a*p[i];}
    out.insert(out.end(),dest.begin(),dest.end());return out;
}
// Second-order jet: exact gradient and Hessian propagation for a two-dimensional nonlinear Gaussian transform.
struct Jet {
    double v;
    std::array<double,2> g{};
    std::array<double,3> h{};
};
inline Jet variable(double v,int i){Jet a{v};a.g[i]=1;return a;}
inline Jet plus(Jet a,Jet b){
    Jet c{a.v+b.v};for(int i=0;i<2;++i)c.g[i]=a.g[i]+b.g[i];for(int i=0;i<3;++i)c.h[i]=a.h[i]+b.h[i];return c;
}
inline Jet mul(Jet a,Jet b){
    Jet c{a.v*b.v};
    for(int i=0;i<2;++i)c.g[i]=a.g[i]*b.v+a.v*b.g[i];
    c.h[0]=a.h[0]*b.v+a.v*b.h[0]+2*a.g[0]*b.g[0];
    c.h[1]=a.h[1]*b.v+a.v*b.h[1]+a.g[0]*b.g[1]+a.g[1]*b.g[0];
    c.h[2]=a.h[2]*b.v+a.v*b.h[2]+2*a.g[1]*b.g[1];return c;
}
inline Jet compose(Jet a,double v,double d,double dd){
    Jet c{v};for(int i=0;i<2;++i)c.g[i]=d*a.g[i];
    c.h[0]=d*a.h[0]+dd*sq(a.g[0]);c.h[1]=d*a.h[1]+dd*a.g[0]*a.g[1];c.h[2]=d*a.h[2]+dd*sq(a.g[1]);return c;
}
inline Jet exponential(Jet a){double z=std::exp(a.v);return compose(a,z,z,z);}
inline Jet logarithm(Jet a){require(a.v>0);return compose(a,std::log(a.v),1/a.v,-1/sq(a.v));}
inline Vec nonlinear_risk_jet(const Vec& x){
    require(x.size()==9);
    double a=x[0],b=x[1],c=x[2],d=x[3],e=x[4],f=x[5],s=x[6],t=x[7],rho=x[8];
    require(s>0&&t>0&&std::abs(rho)<1);
    // g(X,Y)=log(1+exp(c X+d Y))+e XY+f X^2.
    Jet X=variable(a,0),Y=variable(b,1);
    Jet linear=plus(mul(Jet{c},X),mul(Jet{d},Y));
    // Stable softplus with analytic first and second derivatives.
    double z=linear.v,p=z>=0?1/(1+std::exp(-z)):std::exp(z)/(1+std::exp(z));
    Jet g=compose(linear,std::max(z,0.)+std::log1p(std::exp(-std::abs(z))),p,p*(1-p));
    g=plus(g,plus(mul(Jet{e},mul(X,Y)),mul(Jet{f},mul(X,X))));
    double v=s*s,w=t*t,u=rho*s*t;
    double correction=.5*(g.h[0]*v+2*g.h[1]*u+g.h[2]*w);
    double delta= sq(g.g[0])*v+2*g.g[0]*g.g[1]*u+sq(g.g[1])*w;
    // The variance of the centered quadratic Gaussian term is 1/2 tr((H Sigma)^2).
    double h00=g.h[0]*v+g.h[1]*u,h01=g.h[0]*u+g.h[1]*w,h10=g.h[1]*v+g.h[2]*u,h11=g.h[1]*u+g.h[2]*w;
    double quad=.5*(h00*h00+2*h01*h10+h11*h11);
    return {g.v,g.g[0],g.g[1],g.h[0],g.h[1],g.h[2],g.v+correction,delta,quad,delta+quad};
}
}
