#pragma once
#include "model_research_portfolio.hpp"
namespace kline { namespace model_research {
inline Report nested_elastic_selection(const Mat& x,const Vec& p){
    auto d=validate(x,8,4,19)-3;
    require(p.size()>=8&&p.size()%2==0,"Six controls followed by L1/L2 pairs");
    parameters(p,p.size());
    double frac=p[2],cost=p[3],tol=positive(p[4]);auto K=count(p[5],1,100000),C=(p.size()-6)/2;
    require(frac>0&&frac<=.5&&cost>=0&&cost<.5&&C<=32,"Invalid portfolio or candidate count");
    std::size_t A=1;while(A<x.size()&&x[A][0]==x[0][0])++A;
    require(A>=2&&A<=32&&x.size()%A==0,"Balanced panel required");auto n=x.size()/A;
    auto look=count(p[0],2,n-2),val=count(p[1],1,n-look-1);
    require(double(n-look-val)*(double(C)*val+1)*(double(look)*A+K)*d*d<=2e8,"Nested fit work limit");
    for(std::size_t j=0;j<C;++j)require(p[6+2*j]>=0&&p[7+2*j]>0,"Invalid candidate penalty");
    for(std::size_t t=0;t<n;++t)for(std::size_t j=0;j<A;++j){
        auto&r=x[t*A+j];require(r[0]==x[t*A][0]&&r[1]==x[j][1]&&r[0]==std::floor(r[0])&&r[1]==std::floor(r[1])&&r[2]>=-1,"Invalid panel row");
        if(j)require(x[j][1]>x[j-1][1],"Increasing asset IDs required");
        if(t)require(r[0]>x[(t-1)*A][0],"Increasing dates required");
    }
    auto fit=[&](std::size_t t,std::size_t k){
        Mat a;for(std::size_t j=(t-look)*A;j<t*A;++j){Vec z(x[j].begin()+3,x[j].end());z.push_back(x[j][2]);a.push_back(z);}
        auto r=fista_elastic(a,{p[6+2*k],p[7+2*k],tol,double(K)});Vec w;for(auto&v:r.rows)w.push_back(v[1]);return std::make_pair(w,r.rows[0][3]);
    };
    auto predict=[&](std::size_t t,const Vec& w){
        Vec s;for(std::size_t j=0;j<A;++j){Vec a(x[t*A+j].begin()+3,x[t*A+j].end());s.push_back(dot(a,w));}return s;
    };
    Report o{{"Date","GrossReturn","NetReturn","NAV","Fee","Turnover","SelectedCandidate","L1","L2","InnerMSE","OuterKKT","ValidationStart","ValidationEnd"},{}};
    for(std::size_t j=0;j<A;++j)o.header.push_back("Weight"+std::to_string(j+1));
    for(std::size_t j=0;j<A;++j)o.header.push_back("Score"+std::to_string(j+1));
    Vec old(A,0);double nav=1;
    for(std::size_t t=look+val;t<n;++t){
        double best=std::numeric_limits<double>::infinity();std::size_t selected=0;
        for(std::size_t k=0;k<C;++k){
            double loss=0;
            for(std::size_t s=t-val;s<t;++s){
                auto w=fit(s,k);Vec z=predict(s,w.first);
                for(std::size_t j=0;j<A;++j){double e=x[s*A+j][2]-z[j];loss+=e*e/(double(val)*A);}
            }
            if(loss<best){best=loss;selected=k;}
        }
        auto b=fit(t,selected);Vec score=predict(t,b.first),ret,w;Mat rank;
        for(std::size_t j=0;j<A;++j){rank.push_back({score[j]});ret.push_back(x[t*A+j][2]);}
        auto tails=depth_factorconstruction::tie_aware_tail_weights(rank,{frac});for(auto&r:tails.rows)w.push_back(r[4]);
        auto step=researchflow::trade_step(old,w,ret,cost);nav=positive(nav*(1+step.net));
        Vec r{x[t*A][0],step.gross,step.net,nav,step.fee,step.turnover,double(selected+1),p[6+2*selected],p[7+2*selected],best,b.second,x[(t-val)*A][0],x[(t-1)*A][0]};
        r.insert(r.end(),w.begin(),w.end());r.insert(r.end(),score.begin(),score.end());o.rows.push_back(r);old=std::move(step.drift);
    }
    return o;
}

inline Report cvar_primal_dual(const Mat& x,const Vec& p){
    auto d=validate(x,2,1,32);parameters(p,5);
    double alpha=p[0],lam=positive(p[1]),cap=p[2],tol=positive(p[3]);auto K=count(p[4],1,100000),n=x.size();
    require(alpha>0&&alpha<1&&cap>=1./d&&cap<=1&&n<=512&&double(K)*(n*d+80*(n+d))<=3e8,"Invalid CVaR domain or work limit");
    double pc=std::min(1.,1/((1-alpha)*n));
    Vec wl(d,0),wu(d,cap),pl(n,0),pu(n,pc),w(d,1./d),q(n,1./n),bar=w,avgw(d,0),avgq(n,0);
    auto transpose=[&](const Vec& v){Vec z(d,0);for(std::size_t i=0;i<n;++i)for(std::size_t j=0;j<d;++j)z[j]+=v[i]*x[i][j];return z;};
    auto tail=[&](const Vec& a){Vec loss=mul(x,a);for(double&v:loss)v=-v;std::sort(loss.begin(),loss.end(),std::greater<double>());double rem=1,s=0;for(double v:loss){double m=std::min(rem,pc);s+=m*v;rem-=m;if(rem<=1e-14)break;}return s;};
    auto primal=[&](const Vec&a){return tail(a)+.5*lam*squared(a);};
    auto dual=[&](const Vec&v){Vec z=transpose(v),a=z;for(double&u:a)u/=lam;a=box_projection(a,wl,wu,1);return .5*lam*squared(a)-dot(z,a);};
    double normR=0;for(auto&r:x)for(double v:r)normR=std::hypot(normR,v);
    double step=normR>0?.95/normR:1.,upper=primal(w),lower=dual(q);Vec best=w;
    std::size_t it=0;
    while(upper-lower>tol&&it<K){
        Vec z=mul(x,bar);for(std::size_t i=0;i<n;++i)z[i]=q[i]-step*z[i];q=box_projection(z,pl,pu,1);
        Vec a=transpose(q);
        for(std::size_t j=0;j<d;++j)a[j]=(w[j]+step*a[j])/(1+step*lam);
        a=box_projection(a,wl,wu,1);
        for(std::size_t j=0;j<d;++j)bar[j]=2*a[j]-w[j];
        w=a;++it;
        for(std::size_t j=0;j<d;++j)avgw[j]+=(w[j]-avgw[j])/it;
        for(std::size_t i=0;i<n;++i)avgq[i]+=(q[i]-avgq[i])/it;
        if(it%10==0||it==K){
            for(const Vec* v:{&w,&avgw}){double f=primal(*v);if(f<upper){upper=f;best=*v;}}
            lower=std::max({lower,dual(q),dual(avgq)});
        }
    }
    require(upper-lower<=tol&&upper-lower>=-1e-8,"CVaR primal-dual gap not certified");
    double cv=tail(best),gap=std::max(0.,upper-lower);
    Report o{{"Asset","Weight","CVaR","PrimalObjective","DualLowerBound","PrimalDualGap","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),best[j],cv,upper,lower,gap,double(it)});
    return o;
}

inline Report turnover_admm_qp(const Mat& x,const Vec& p){
    auto d=validate(x,1,5,36)-4;parameters(p,6);
    require(x.size()==d,"Square covariance required");
    double B=p[0],gamma=positive(p[1]),cost=p[2],rho=positive(p[3]),tol=positive(p[4]);auto K=count(p[5],1,100000);
    require(cost>=0&&double(K)*d*d*d<=1e8,"Invalid cost or work limit");
    Mat s;Vec mu,l,h,old;
    for(auto&r:x){s.emplace_back(r.begin(),r.begin()+d);mu.push_back(r[d]);l.push_back(r[d+1]);h.push_back(r[d+2]);old.push_back(r[d+3]);}
    solve_spd(s,Vec(d,0));Mat H=s;
    for(std::size_t i=0;i<d;++i){for(double&v:H[i])v*=gamma;H[i][i]+=rho;}
    Vec one(d,1),v=solve_spd(H,one),z=box_projection(old,l,h,B),u(d,0),w=z;
    double den=positive(sum(v)),pr=0,du=0,gap=0,bg=0;std::size_t it=0;
    for(;it<K;){
        Vec rhs=mu;
        for(std::size_t j=0;j<d;++j)rhs[j]+=rho*(z[j]-u[j]);
        w=solve_spd(H,rhs);double eta=(sum(w)-B)/den;
        Vec prev=z;
        for(std::size_t j=0;j<d;++j){
            w[j]-=eta*v[j];z[j]=std::clamp(old[j]+soft(w[j]+u[j]-old[j],cost/rho),l[j],h[j]);u[j]+=w[j]-z[j];
        }
        pr=du=gap=0;Vec g=mul(s,z);
        for(std::size_t j=0;j<d;++j){
            pr=std::max(pr,std::abs(w[j]-z[j]));du=std::max(du,rho*std::abs(z[j]-prev[j]));
            gap=std::max(gap,std::abs(gamma*g[j]-mu[j]+eta+rho*u[j]));
            double a=z[j]<old[j]?-cost:z[j]>old[j]?cost:-cost;
            double b=z[j]==old[j]?cost:a,dual=rho*u[j];
            double e=l[j]==h[j]?0:z[j]==l[j]?std::max(0.,dual-b):z[j]==h[j]?std::max(0.,a-dual):std::max({0.,a-dual,dual-b});
            gap=std::max(gap,e);
        }
        bg=std::abs(sum(z)-B);++it;
        if(std::max({pr,du,gap,bg})<=tol)break;
    }
    require(std::max({pr,du,gap,bg})<=tol,"Turnover ADMM did not converge");
    double obj=.5*gamma*dot(z,mul(s,z))-dot(mu,z);
    for(std::size_t j=0;j<d;++j)obj+=cost*std::abs(z[j]-old[j]);
    Report o{{"Asset","Weight","Previous","Trade","Objective","BudgetGap","PrimalResidual","DualResidual","KKTResidual","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),z[j],old[j],z[j]-old[j],obj,bg,pr,du,gap,double(it)});
    return o;
}

inline Report rolling_factor_risk_qp(const Mat& x,const Vec& p){
    auto cols=validate(x,4,2,48);parameters(p,8);
    auto k=count(p[0],1,std::min<std::size_t>(16,cols-1)),A=cols-k,look=count(p[1],3,x.size()-1),K=count(p[7],1,100000);
    double lam=positive(p[2]),floor=positive(p[3]),cap=p[4],cost=p[5],tol=positive(p[6]);
    require(A<=32&&cap>=1./A&&cap<=1&&cost>=0&&cost<1,"Invalid factor portfolio constraints");
    auto m=k+1;
    require(double(x.size()-look)*(double(look)*(m*m+A*m)+double(A)*m*m*m+double(A)*A*k*k+double(K)*A*(A+80))<=2e8,"Factor risk work limit");
    Mat f,r;for(auto&a:x){f.emplace_back(a.begin(),a.begin()+k);r.emplace_back(a.begin()+k,a.end());for(double v:r.back())require(v>=-1,"Invalid asset return");}
    Report o{{"Time","GrossReturn","NetReturn","NAV","Fee","Turnover","FactorVariance","SpecificVariance","TotalVariance","ProjectedGradient"},{}};
    for(std::size_t i=0;i<A;++i)o.header.push_back("Weight"+std::to_string(i+1));
    for(std::size_t i=0;i<A;++i)for(std::size_t j=0;j<k;++j)o.header.push_back("Beta"+std::to_string(i+1)+"_"+std::to_string(j+1));
    Vec old(A,0);double nav=1;
    for(std::size_t t=look;t<x.size();++t){
        Mat H(m,Vec(m,0)),rhs(A,Vec(m,0)),design;
        for(std::size_t s=t-look;s<t;++s){
            Vec z{1};z.insert(z.end(),f[s].begin(),f[s].end());design.push_back(z);add_outer(H,z,1);
            for(std::size_t i=0;i<A;++i)for(std::size_t j=0;j<m;++j)rhs[i][j]+=z[j]*r[s][i];
        }
        for(std::size_t j=1;j<m;++j)H[j][j]+=lam;
        Mat B(A,Vec(k));Vec specific(A,floor);
        for(std::size_t i=0;i<A;++i){
            Vec w=solve_spd(H,rhs[i]);for(std::size_t j=0;j<k;++j)B[i][j]=w[j+1];
            double v=0;for(std::size_t s=0;s<look;++s){double e=r[t-look+s][i]-dot(w,design[s]);v+=e*e/look;}
            specific[i]+=v;
        }
        Mat S=researchflow::covariance_window(f,t-look,t,0,floor),cov(A,Vec(A,0));
        for(std::size_t i=0;i<A;++i){Vec sb=mul(S,B[i]);for(std::size_t j=0;j<=i;++j){double v=dot(B[j],sb)+(i==j?specific[i]:0);cov[i][j]=cov[j][i]=v;}}
        Mat a=cov;for(auto&row:a){row.push_back(0);row.push_back(0);row.push_back(cap);}
        auto opt=box_covariance_qp(a,{1,1,tol,double(K)});Vec w;for(auto&v:opt.rows)w.push_back(v[1]);
        Vec beta(k,0);double idio=0;for(std::size_t i=0;i<A;++i){idio+=w[i]*w[i]*specific[i];for(std::size_t j=0;j<k;++j)beta[j]+=w[i]*B[i][j];}
        double fv=dot(beta,mul(S,beta));auto step=researchflow::trade_step(old,w,r[t],cost);nav=positive(nav*(1+step.net));
        Vec row{double(t+1),step.gross,step.net,nav,step.fee,step.turnover,fv,idio,fv+idio,opt.rows[0][5]};
        row.insert(row.end(),w.begin(),w.end());for(auto&b:B)row.insert(row.end(),b.begin(),b.end());o.rows.push_back(row);old=std::move(step.drift);
    }
    return o;
}

inline Report integer_execution_dp(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,1);auto Q=count(p[0],1,5000),n=x.size();
    require(double(n)*(Q+1)*(Q+1)<=1e8,"Execution DP work limit");
    Vec eta(n),risk(n);std::vector<std::size_t> cap(n);
    for(std::size_t t=0;t<n;++t){eta[t]=positive(x[t][1])/positive(x[t][0]);risk[t]=x[t][2];require(risk[t]>=0,"Negative inventory risk");cap[t]=count(x[t][3],0,1000000000);}
    double inf=std::numeric_limits<double>::infinity();Mat V(n+1,Vec(Q+1,inf));
    std::vector<std::vector<std::size_t>> policy(n,std::vector<std::size_t>(Q+1,0));
    V[n][0]=0;
    for(std::size_t t=n;t-->0;){
        for(std::size_t state=0;state<=Q;++state){
            for(std::size_t q=0;q<=std::min(state,cap[t]);++q){
                double rem=state-q,v=eta[t]*q*q+risk[t]*rem*rem+V[t+1][state-q];
                if(v<V[t][state]){V[t][state]=v;policy[t][state]=q;}
            }
        }
    }
    require(std::isfinite(V[0][Q]),"Execution target infeasible");
    Report o{{"Period","Trade","Remaining","TemporaryCost","RiskCost","CumulativeCost","InitialOptimalCost","BellmanResidual"},{}};
    std::size_t state=Q;double cumulative=0;
    for(std::size_t t=0;t<n;++t){
        auto q=policy[t][state],rem=state-q;double a=eta[t]*q*q,b=risk[t]*rem*rem;
        double gap=std::abs(V[t][state]-(a+b+V[t+1][rem]));cumulative+=a+b;
        o.rows.push_back({double(t+1),double(q),double(rem),a,b,cumulative,V[0][Q],gap});state=rem;
    }
    require(state==0,"Execution did not complete");return o;
}

inline Report inventory_quote_dp(const Mat& x,const Vec& p){
    validate(x,1,4,4);require(p.size()>=4,"Inventory bound, terminal risk, fee and quote candidates required");parameters(p,p.size());
    auto Q=count(p[0],1,2000),n=x.size(),states=2*Q+1;double terminal=p[1],fee=p[2];
    require(terminal>=0&&fee>=0&&p.size()<=103&&double(n)*states*(p.size()-3)<=5e7,"Invalid quote controls");
    for(std::size_t j=3;j<p.size();++j)positive(p[j]);
    for(auto&r:x)require(r[0]>=0&&r[0]<=1&&r[1]>=0&&r[1]<=1&&r[2]>=0&&r[3]>=0,"Invalid fill or risk parameters");
    Mat V(n+1,Vec(states,0)),act(n,Vec(states,0)),spread=act,prob=act;
    for(std::size_t i=0;i<states;++i){double q=double(i)-Q;V[n][i]=-terminal*q*q;}
    auto candidate=[&](std::size_t t,std::size_t i,int side,double delta,double z){
        double q=double(i)-Q,qn=q+side,r=x[t][3];auto next=std::size_t(int(i)+side);
        return z*(delta-fee-r*qn*qn+V[t+1][next])+(1-z)*(-r*q*q+V[t+1][i]);
    };
    for(std::size_t t=n;t-->0;){
        for(std::size_t i=0;i<states;++i){
            double q=double(i)-Q,best=V[t+1][i]-x[t][3]*q*q;
            for(int side:{-1,1}){
                if((side<0&&i==0)||(side>0&&i+1==states))continue;
                double base=x[t][side>0?0:1];
                for(std::size_t j=3;j<p.size();++j){
                    double delta=p[j],z=base*std::exp(-x[t][2]*delta),v=candidate(t,i,side,delta,z);
                    if(v>best){best=v;act[t][i]=side;spread[t][i]=delta;prob[t][i]=z;}
                }
            }
            V[t][i]=best;
        }
    }
    Report o{{"Period","Inventory","Action","Spread","FillProbability","Value","ExpectedNextInventory","BellmanResidual"},{}};
    for(std::size_t t=0;t<n;++t)for(std::size_t i=0;i<states;++i){
        double q=double(i)-Q,a=act[t][i],d=spread[t][i],z=prob[t][i];
        double ref=a==0?V[t+1][i]-x[t][3]*q*q:candidate(t,i,int(a),d,z);
        o.rows.push_back({double(t+1),q,a,d,z,V[t][i],q+a*z,std::abs(V[t][i]-ref)});
    }
    return o;
}
} }
