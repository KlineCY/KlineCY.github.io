#pragma once
#include "model_research_common.hpp"
namespace kline { namespace model_research {
inline Report fista_elastic(const Mat& x,const Vec& p){
    auto d=validate(x,1,2,33)-1;
    parameters(p,4);
    double l1=p[0],l2=p[1],tol=positive(p[2]);
    auto K=count(p[3],1,100000);
    require(l1>=0&&l2>0&&double(K)*d*d<=5e7,"Invalid penalty or iteration budget");
    Mat H(d,Vec(d,0));
    Vec b(d,0);
    for(const auto&r:x){
        Vec a(r.begin(),r.end()-1);
        add_outer(H,a,1);
        for(std::size_t j=0;j<d;++j)b[j]+=a[j]*r.back();
    }
    for(std::size_t j=0;j<d;++j)H[j][j]+=l2;
    double L=matrix_lipschitz(H);
    Vec w(d,0),z=w;
    double t=1,gap=0;
    std::size_t it=0;
    for(;it<K;){
        Vec g=mul(H,z),v(d);
        for(std::size_t j=0;j<d;++j)v[j]=soft(z[j]-(g[j]-b[j])/L,l1/L);
        Vec q=mul(H,v);gap=0;
        for(std::size_t j=0;j<d;++j){
            double a=q[j]-b[j];
            double e=v[j]==0?std::max(0.,std::abs(a)-l1):std::abs(a+l1*std::copysign(1.,v[j]));
            gap=std::max(gap,e);
        }
        double next=(1+std::sqrt(1+4*t*t))/2;
        for(std::size_t j=0;j<d;++j)z[j]=v[j]+(t-1)/next*(v[j]-w[j]);
        w=v;t=next;++it;
        if(gap<=tol)break;
    }
    require(gap<=tol,"FISTA did not converge");
    double obj=l1*researchflow::l1(w)+.5*l2*squared(w);
    for(const auto&r:x){double e=-r.back();for(std::size_t j=0;j<d;++j)e+=r[j]*w[j];obj+=.5*e*e;}
    Report o{{"Feature","Coefficient","Objective","KKTResidual","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),w[j],obj,gap,double(it)});
    return o;
}

inline Report fused_admm(const Mat& x,const Vec& p){
    validate(x,2,1,1);parameters(p,4);
    double lam=p[0],rho=positive(p[1]),tol=positive(p[2]);
    auto K=count(p[3],1,100000),n=x.size();
    require(lam>=0&&double(K)*n<=5e7,"Invalid penalty or work limit");
    Vec y=column(x,0),w=y,z(n-1,0),u(n-1,0);
    Vec lower(n,-rho),upper(n,-rho),diag(n,1+2*rho);
    lower[0]=upper[n-1]=0;diag[0]=diag[n-1]=1+rho;
    auto dt=[&](const Vec& a){Vec b(n,0);for(std::size_t j=0;j+1<n;++j){b[j]-=a[j];b[j+1]+=a[j];}return b;};
    double pr=0,du=0,gap=0;std::size_t it=0;
    for(;it<K;){
        Vec v(n-1);
        for(std::size_t j=0;j+1<n;++j)v[j]=rho*(z[j]-u[j]);
        Vec b=dt(v);
        for(std::size_t j=0;j<n;++j)b[j]+=y[j];
        w=tridiagonal(lower,diag,upper,b);
        Vec dz(n-1),r(n-1);gap=0;
        for(std::size_t j=0;j+1<n;++j){
            double a=w[j+1]-w[j],old=z[j];
            z[j]=soft(a+u[j],lam/rho);r[j]=a-z[j];u[j]+=r[j];dz[j]=rho*(z[j]-old);
            double vj=rho*u[j],e=z[j]==0?std::max(0.,std::abs(vj)-lam):std::abs(vj-lam*std::copysign(1.,z[j]));
            gap=std::max(gap,e);
        }
        pr=inf_norm(r);du=inf_norm(dt(dz));
        Vec g=dt(u);
        for(std::size_t j=0;j<n;++j)gap=std::max(gap,std::abs(w[j]-y[j]+rho*g[j]));
        ++it;
        if(std::max({pr,du,gap})<=tol)break;
    }
    require(std::max({pr,du,gap})<=tol,"ADMM did not converge");
    double obj=0;
    for(std::size_t j=0;j<n;++j){obj+=.5*(w[j]-y[j])*(w[j]-y[j]);if(j)obj+=lam*std::abs(w[j]-w[j-1]);}
    Report o{{"Index","Observed","Fitted","Difference","Objective","PrimalResidual","DualResidual","KKTResidual","Iterations"},{}};
    for(std::size_t j=0;j<n;++j)o.rows.push_back({double(j+1),y[j],w[j],j?w[j]-w[j-1]:0,obj,pr,du,gap,double(it)});
    return o;
}

inline Report box_covariance_qp(const Mat& x,const Vec& p){
    auto d=validate(x,1,4,35)-3;parameters(p,4);
    require(x.size()==d,"Square covariance required");
    double budget=p[0],gamma=positive(p[1]),tol=positive(p[2]);
    auto K=count(p[3],1,100000);
    require(double(K)*d*(d+80)<=5e7,"Optimization work limit");
    Mat s;Vec mu,l,u;
    for(auto&r:x){s.emplace_back(r.begin(),r.begin()+d);mu.push_back(r[d]);l.push_back(r[d+1]);u.push_back(r[d+2]);}
    solve_spd(s,Vec(d,0));
    double L=gamma*matrix_lipschitz(s);
    Vec w=box_projection(Vec(d,budget/d),l,u,budget),g(d);
    auto gradient=[&](const Vec& a){Vec v=mul(s,a);for(std::size_t j=0;j<d;++j)v[j]=gamma*v[j]-mu[j];return v;};
    double gap=0;std::size_t it=0;
    for(;it<K;){
        g=gradient(w);Vec v(d);
        for(std::size_t j=0;j<d;++j)v[j]=w[j]-g[j]/L;
        Vec z=box_projection(v,l,u,budget);gap=0;
        for(std::size_t j=0;j<d;++j)gap=std::max(gap,L*std::abs(w[j]-z[j]));
        ++it;
        if(gap<=tol)break;
        w=z;
    }
    require(gap<=tol,"Projected gradient did not converge");
    double obj=.5*gamma*dot(w,mul(s,w))-dot(mu,w),bg=std::abs(sum(w)-budget);
    Report o{{"Asset","Weight","Gradient","Objective","BudgetGap","ProjectedGradient","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),w[j],g[j],obj,bg,gap,double(it)});
    return o;
}

inline Report entropy_balance_fit(const Mat& x,const Vec& p){
    auto d=validate(x,2,2,17)-1;
    parameters(p,d+2);Vec target(p.begin(),p.begin()+d);
    double tol=positive(p[d]);auto K=count(p[d+1],1,2000);
    require(double(K)*x.size()*d*d<=5e7,"Calibration work limit");
    Mat a;Vec q;
    for(auto&r:x){q.push_back(positive(r[0]));a.emplace_back(r.begin()+1,r.end());}
    double den=positive(sum(q));for(double&v:q)v/=den;
    struct State{Vec w,g;Mat H;double obj;};
    auto eval=[&](const Vec& theta,bool curvature){
        State s{Vec(x.size()),Vec(d,0),Mat(d,Vec(d,0)),0};
        double m=-std::numeric_limits<double>::infinity();
        for(std::size_t i=0;i<x.size();++i){s.w[i]=std::log(q[i])+dot(a[i],theta);m=std::max(m,s.w[i]);}
        double total=0;
        for(double&v:s.w){v=std::exp(v-m);total+=v;}
        for(std::size_t i=0;i<x.size();++i){s.w[i]/=total;for(std::size_t j=0;j<d;++j)s.g[j]+=s.w[i]*a[i][j];}
        if(curvature)for(std::size_t i=0;i<x.size();++i){Vec z=a[i];for(std::size_t j=0;j<d;++j)z[j]-=s.g[j];add_outer(s.H,z,s.w[i]);}
        for(std::size_t j=0;j<d;++j)s.g[j]-=target[j];
        s.obj=m+std::log(total)-dot(theta,target);return s;
    };
    Vec theta(d,0);State s=eval(theta,true);std::size_t it=0;
    while(inf_norm(s.g)>tol&&it<K){
        Vec dir=solve_spd(s.H,s.g);double descent=positive(dot(dir,s.g));bool ok=false;
        for(int k=0;k<40;++k){
            double step=std::ldexp(1.,-k);Vec v=theta;
            for(std::size_t j=0;j<d;++j)v[j]-=step*dir[j];
            auto trial=eval(v,false);
            if(trial.obj<=s.obj-1e-4*step*descent+1e-15*std::max(1.,std::abs(s.obj))){theta=v;s=eval(theta,true);ok=true;break;}
        }
        require(ok,"Entropy line search failed");++it;
    }
    double gap=inf_norm(s.g);require(gap<=tol,"Moment target not reached");
    double kl=0,ess=1/positive(squared(s.w));
    for(std::size_t i=0;i<x.size();++i)if(s.w[i]>0)kl+=s.w[i]*std::log(s.w[i]/q[i]);
    Report o{{"Observation","Weight","BaseProbability","KL","MomentResidual","EffectiveN","Iterations"},{}};
    for(std::size_t i=0;i<x.size();++i)o.rows.push_back({double(i+1),s.w[i],q[i],kl,gap,ess,double(it)});
    return o;
}

inline Report nnls_coordinate(const Mat& x,const Vec& p){
    auto d=validate(x,1,2,33)-1;parameters(p,3);
    double lam=positive(p[0]),tol=positive(p[1]);auto K=count(p[2],1,100000);
    require(double(K)*d*d<=5e7,"Coordinate work limit");
    Mat H(d,Vec(d,0));Vec b(d,0),w(d,0);
    for(auto&r:x){Vec a(r.begin(),r.end()-1);add_outer(H,a,1);for(std::size_t j=0;j<d;++j)b[j]+=a[j]*r.back();}
    for(std::size_t j=0;j<d;++j)H[j][j]+=lam;
    Vec g=b;for(double&v:g)v=-v;
    double gap=0;std::size_t it=0;
    for(;it<K;){
        for(std::size_t j=0;j<d;++j){
            double v=std::max(0.,w[j]-g[j]/H[j][j]),delta=v-w[j];w[j]=v;
            for(std::size_t k=0;k<d;++k)g[k]+=H[k][j]*delta;
        }
        g=mul(H,w);gap=0;
        for(std::size_t j=0;j<d;++j){g[j]-=b[j];gap=std::max(gap,w[j]>0?std::abs(g[j]):std::max(0.,-g[j]));}
        ++it;if(gap<=tol)break;
    }
    require(gap<=tol,"NNLS did not converge");
    double obj=.5*lam*squared(w);
    for(auto&r:x){double e=-r.back();for(std::size_t j=0;j<d;++j)e+=r[j]*w[j];obj+=.5*e*e;}
    Report o{{"Feature","Coefficient","Gradient","Objective","KKTResidual","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),w[j],g[j],obj,gap,double(it)});
    return o;
}

inline Report graph_smoother(const Mat& x,const Vec& p){
    auto d=validate(x,1,3,34)-2;parameters(p,1);
    require(x.size()==d&&p[0]>0,"Square graph and positive penalty required");
    Mat H(d,Vec(d,0));Vec y(d),w(d),b(d);double lam=p[0];
    for(std::size_t i=0;i<d;++i){
        y[i]=x[i][d];w[i]=x[i][d+1];require(w[i]>=0,"Negative observation weight");
        b[i]=w[i]*y[i];H[i][i]=w[i];
        require(x[i][i]==0,"Self loops excluded");
        for(std::size_t j=0;j<d;++j){
            double a=x[i][j];require(a>=0&&std::abs(a-x[j][i])<=1e-12*std::max({1.,a,std::abs(x[j][i])}),"Undirected nonnegative graph required");
            H[i][i]+=lam*a;if(i!=j)H[i][j]=-lam*a;
        }
    }
    Vec v=solve_spd(H,b),g=mul(H,v),pen(d,0);double obj=0;
    for(std::size_t i=0;i<d;++i){
        for(std::size_t j=0;j<d;++j)pen[i]+=.25*lam*x[i][j]*(v[i]-v[j])*(v[i]-v[j]);
        obj+=.5*w[i]*(v[i]-y[i])*(v[i]-y[i])+pen[i];
    }
    Report o{{"Node","Fitted","ObservationResidual","GraphPenaltyContribution","StationarityResidual","Objective"},{}};
    for(std::size_t i=0;i<d;++i)o.rows.push_back({double(i+1),v[i],v[i]-y[i],pen[i],g[i]-b[i],obj});
    return o;
}
} }
