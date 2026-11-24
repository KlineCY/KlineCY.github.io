#pragma once
#include "advanced_research_common.hpp"
#include "model_research_optimization.hpp"
#include <tuple>
namespace kline { namespace advanced_research {
inline Report ellipsoid_robust_qp(const Mat& x,const Vec& p){
    auto d=validate(x,1,4,35)-3;parameters(p,5);require(x.size()==d,"Square covariance required");
    double B=positive(p[0]),gamma=positive(p[1]),delta=p[2],tol=positive(p[3]);auto K=count(p[4],1,100000);
    require(delta>=0&&double(K)*d*(d+80)<=5e7,"Invalid robust optimization controls");
    Mat S;Vec mu,l,u;for(auto&r:x){S.emplace_back(r.begin(),r.begin()+d);mu.push_back(r[d]);l.push_back(r[d+1]);u.push_back(r[d+2]);}solve_spd(S,Vec(d,0));
    auto value=[&](const Vec& w){double v=positive(dot(w,mul(S,w)));return .5*gamma*v-dot(mu,w)+delta*std::sqrt(v);};
    auto grad=[&](const Vec& w){Vec v=mul(S,w);double z=gamma+delta/std::sqrt(positive(dot(w,v)));for(std::size_t j=0;j<d;++j)v[j]=z*v[j]-mu[j];return v;};
    Vec w=box_projection(Vec(d,B/d),l,u,B);double f=value(w),step=1/(gamma*matrix_lipschitz(S)),gap=0;std::size_t it=0;
    for(;it<K;){
        Vec g=grad(w),z(d);bool accepted=false;
        for(int k=0;k<60;++k){
            for(std::size_t j=0;j<d;++j)z[j]=w[j]-step*g[j];
            z=box_projection(z,l,u,B);Vec h(d);for(std::size_t j=0;j<d;++j)h[j]=z[j]-w[j];
            double fv=value(z);gap=inf_norm(h)/step;
            if(fv<=f+dot(g,h)+.5*squared(h)/step+1e-14*std::max(1.,std::abs(f))){accepted=true;if(gap>tol){w=z;f=fv;}break;}
            step*=.5;
        }
        require(accepted,"Robust line search failed");++it;if(gap<=tol)break;step=std::min(1/(gamma*matrix_lipschitz(S)),step*1.2);
    }
    require(gap<=tol,"Robust projected gradient did not converge");
    double var=dot(w,mul(S,w)),m=dot(w,mu);
    Report o{{"Asset","Weight","NominalMean","RobustMean","Variance","Objective","ProjectedGradient","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),w[j],m,m-delta*std::sqrt(var),var,f,gap,double(it)});
    return o;
}

inline Report affine_box_admm(const Mat& x,const Vec& p){
    auto cols=validate(x,2,5,44);require(p.size()>=6,"Missing affine controls");parameters(p,p.size());
    auto m=count(p[0],1,8),d=cols-3-m;require(d==x.size()&&d<=32&&m<=d&&p.size()==5+m,"Invalid affine dimensions");
    double gamma=positive(p[1]),rho=positive(p[2]),tol=positive(p[3]);auto K=count(p[4],1,100000);
    require(double(K)*(d*d*d+m*m*m)<=1e8,"Affine ADMM work limit");
    Mat S,A(m,Vec(d)),H;Vec mu,l,u,b(p.begin()+5,p.end());
    for(std::size_t i=0;i<d;++i){auto&r=x[i];S.emplace_back(r.begin(),r.begin()+d);mu.push_back(r[d]);l.push_back(r[d+1]);u.push_back(r[d+2]);require(l.back()<=u.back(),"Invalid bounds");for(std::size_t j=0;j<m;++j)A[j][i]=r[d+3+j];}
    solve_spd(S,Vec(d,0));H=S;for(std::size_t i=0;i<d;++i){for(double&v:H[i])v*=gamma;H[i][i]+=rho;}
    Mat HA;for(auto&r:A)HA.push_back(solve_spd(H,r));Mat Q(m,Vec(m));for(std::size_t i=0;i<m;++i)for(std::size_t j=0;j<m;++j)Q[i][j]=dot(A[i],HA[j]);
    solve_spd(Q,Vec(m,0));Vec z(d),dual(d,0),w(d),eta(m);
    for(std::size_t j=0;j<d;++j)z[j]=std::clamp(0.,l[j],u[j]);
    double pr=0,dr=0,ar=0,gap=0;std::size_t it=0;
    for(;it<K;){
        Vec rhs=mu;for(std::size_t j=0;j<d;++j)rhs[j]+=rho*(z[j]-dual[j]);
        w=solve_spd(H,rhs);Vec e=mul(A,w);for(std::size_t j=0;j<m;++j)e[j]-=b[j];eta=solve_spd(Q,e);
        Vec prev=z;pr=dr=gap=ar=0;
        for(std::size_t j=0;j<d;++j){
            for(std::size_t k=0;k<m;++k)w[j]-=HA[k][j]*eta[k];
            z[j]=std::clamp(w[j]+dual[j],l[j],u[j]);dual[j]+=w[j]-z[j];pr=std::max(pr,std::abs(w[j]-z[j]));dr=std::max(dr,rho*std::abs(z[j]-prev[j]));
        }
        Vec g=mul(S,z),az=mul(A,z);
        for(std::size_t j=0;j<m;++j)ar=std::max(ar,std::abs(az[j]-b[j]));
        for(std::size_t j=0;j<d;++j){
            double a=gamma*g[j]-mu[j]+rho*dual[j];for(std::size_t k=0;k<m;++k)a+=A[k][j]*eta[k];
            double v=rho*dual[j],e2=l[j]==u[j]?0:z[j]==l[j]?std::max(0.,v):z[j]==u[j]?std::max(0.,-v):std::abs(v);
            gap=std::max({gap,std::abs(a),e2});
        }
        ++it;if(std::max({pr,dr,ar,gap})<=tol)break;
    }
    require(std::max({pr,dr,ar,gap})<=tol,"Affine box ADMM did not converge");
    double obj=.5*gamma*dot(z,mul(S,z))-dot(mu,z);
    Report o{{"Asset","Weight","Objective","AffineResidual","PrimalResidual","DualResidual","KKTResidual","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),z[j],obj,ar,pr,dr,gap,double(it)});
    return o;
}

inline Report scenario_minimax_variance(const Mat& x,const Vec& p){
    auto d=validate(x,2,2,33)-1;parameters(p,3);
    double cap=p[0],tol=positive(p[1]);auto K=count(p[2],1,5000);
    require(cap>=1./d&&cap<=1&&x.size()%d==0,"Invalid scenario shape");auto m=x.size()/d;
    require(m>=2&&m<=16&&double(K)*d*(d+80)*1000<=2e9,"Scenario optimization work limit");
    std::vector<Mat> S;
    for(std::size_t k=0;k<m;++k){Mat a;for(std::size_t j=0;j<d;++j){auto&r=x[k*d+j];require(r[0]==double(k+1),"Sequential scenario IDs required");a.emplace_back(r.begin()+1,r.end());}solve_spd(a,Vec(d,0));S.push_back(a);}
    Vec prob(m,1./m),best(d,1./d);
    double upper=std::numeric_limits<double>::infinity(),lower=-upper;std::size_t it=0;
    for(;it<K;){
        Mat a(d,Vec(d,0));for(std::size_t k=0;k<m;++k)for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<d;++j)a[i][j]+=prob[k]*S[k][i][j];
        Mat data=a;for(auto&r:data){r.push_back(0);r.push_back(0);r.push_back(cap);}
        auto fit=model_research::box_covariance_qp(data,{1,2,std::min(tol*.01,1e-10),1000});
        Vec w;for(auto&r:fit.rows)w.push_back(r[1]);
        Vec losses;for(auto&s:S)losses.push_back(dot(w,mul(s,w)));
        auto worst=std::size_t(std::max_element(losses.begin(),losses.end())-losses.begin());
        if(losses[worst]<upper){upper=losses[worst];best=w;}
        Vec g=mul(a,w);for(double&v:g)v*=2;
        Vec ordered=g;std::sort(ordered.begin(),ordered.end());double rem=1,lin=0;
        for(double v:ordered){double mass=std::min(rem,cap);lin+=mass*v;rem-=mass;if(rem<=1e-14)break;}
        lower=std::max(lower,dot(w,mul(a,w))+lin-dot(g,w));++it;
        if(upper-lower<=tol)break;
        double step=2./(it+2);for(double&v:prob)v*=1-step;prob[worst]+=step;
    }
    require(upper-lower<=tol&&upper-lower>=-1e-8,"Scenario minimax gap not certified");
    Report o{{"Asset","Weight","WorstVariance","DualLowerBound","Gap","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),best[j],upper,lower,std::max(0.,upper-lower),double(it)});
    return o;
}

inline Report multiasset_execution_lq(const Mat& x,const Vec& p){
    auto d=validate(x,1,3,65);require(d%2==1,"Two matrices and inventory required");d=(d-1)/2;parameters(p,2);
    auto T=count(p[0],2,1000),m=d*(T-1);double risk=p[1];
    require(d==x.size()&&m<=64&&risk>=0,"Invalid LQ dimensions");
    Mat H,S;Vec q;
    for(auto&r:x){H.emplace_back(r.begin(),r.begin()+d);S.emplace_back(r.begin()+d,r.begin()+2*d);q.push_back(r.back());}
    solve_spd(H,Vec(d,0));solve_spd(S,Vec(d,0));Mat M(m,Vec(m,0));Vec rhs(m,0),hq=mul(H,q);
    for(std::size_t t=0;t<T-1;++t)for(std::size_t i=0;i<d;++i){
        auto a=t*d+i;if(t==0)rhs[a]=hq[i];
        for(std::size_t j=0;j<d;++j){
            M[a][t*d+j]=2*H[i][j]+risk*S[i][j];if(t)M[a][(t-1)*d+j]=-H[i][j];if(t+1<T-1)M[a][(t+1)*d+j]=-H[i][j];
        }
    }
    Vec z=solve_spd(M,rhs),g=mul(M,z);for(std::size_t j=0;j<m;++j)g[j]-=rhs[j];double gap=inf_norm(g);
    Mat path{q};for(std::size_t t=0;t<T-1;++t)path.emplace_back(z.begin()+t*d,z.begin()+(t+1)*d);path.push_back(Vec(d,0));
    Vec cost(T),rv(T);double total=0;Mat trades;
    for(std::size_t t=0;t<T;++t){Vec a(d);for(std::size_t j=0;j<d;++j)a[j]=path[t][j]-path[t+1][j];trades.push_back(a);cost[t]=dot(a,mul(H,a));rv[t]=risk*dot(path[t+1],mul(S,path[t+1]));total+=cost[t]+rv[t];}
    Report o{{"Period","Asset","Trade","Remaining","TemporaryCost","InventoryRisk","TotalObjective","StationarityResidual"},{}};
    for(std::size_t t=0;t<T;++t)for(std::size_t j=0;j<d;++j)o.rows.push_back({double(t+1),double(j+1),trades[t][j],path[t+1][j],cost[t],rv[t],total,gap});
    return o;
}

inline Report log_optimal_box(const Mat& x,const Vec& p){
    auto d=validate(x,2,2,33)-1;parameters(p,3);
    double cap=p[0],tol=positive(p[1]);auto K=count(p[2],1,100000);
    require(cap>=1./d&&cap<=1&&double(K)*(x.size()*d+80*d)<=1e8,"Invalid log optimal controls");
    Mat r;Vec prob;
    for(auto&a:x){r.emplace_back(a.begin(),a.end()-1);prob.push_back(positive(a.back()));for(double v:r.back())require(v>-1,"Return must exceed -100 percent");}
    require(std::abs(sum(prob)-1)<=1e-10,"Scenario probabilities must sum to one");
    auto eval=[&](const Vec&w){double f=0,minwealth=std::numeric_limits<double>::infinity();Vec g(d,0);
        for(std::size_t i=0;i<r.size();++i){double v=positive(1+dot(r[i],w));f-=prob[i]*std::log(v);minwealth=std::min(minwealth,v);for(std::size_t j=0;j<d;++j)g[j]-=prob[i]*r[i][j]/v;}
        return std::make_tuple(f,minwealth,g);
    };
    Vec l(d,0),u(d,cap),w(d,1./d);auto st=eval(w);double step=1,gap=0;std::size_t it=0;
    for(;it<K;){
        bool ok=false;Vec g=std::get<2>(st),z(d);
        for(int k=0;k<60;++k){
            for(std::size_t j=0;j<d;++j)z[j]=w[j]-step*g[j];z=box_projection(z,l,u,1);
            Vec h(d);for(std::size_t j=0;j<d;++j)h[j]=z[j]-w[j];gap=inf_norm(h)/step;auto next=eval(z);
            if(std::get<0>(next)<=std::get<0>(st)+dot(g,h)+.5*squared(h)/step+1e-14){ok=true;if(gap>tol){w=z;st=next;}break;}step*=.5;
        }
        require(ok,"Log growth line search failed");++it;if(gap<=tol)break;step=std::min(1e6,step*1.5);
    }
    require(gap<=tol,"Log growth projected gradient did not converge");
    Report o{{"Asset","Weight","ExpectedLogGrowth","MinimumWealthFactor","ProjectedGradient","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),w[j],-std::get<0>(st),std::get<1>(st),gap,double(it)});
    return o;
}

inline Report stochastic_execution_dp(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,2);auto Q=count(p[0],1,3000),n=x.size();double terminal=p[1];
    require(terminal>=0&&double(n)*(Q+1)*(Q+1)<=1e8,"Invalid stochastic execution work limit");
    std::vector<std::size_t> cap;for(auto&r:x){require(r[0]>=0&&r[0]<=1&&r[2]>0&&r[3]>=0,"Invalid fill or cost");cap.push_back(count(r[1],0,1000000000));}
    Mat V(n+1,Vec(Q+1,0));std::vector<std::vector<std::size_t>> act(n,std::vector<std::size_t>(Q+1));
    for(std::size_t j=0;j<=Q;++j)V[n][j]=terminal*j*j;
    auto value=[&](std::size_t t,std::size_t q,std::size_t a){double p=x[t][0],eta=x[t][2],risk=x[t][3],r=q-a;return p*(eta*a*a+risk*r*r+V[t+1][q-a])+(1-p)*(risk*q*q+V[t+1][q]);};
    for(std::size_t t=n;t-->0;)for(std::size_t q=0;q<=Q;++q){
        double best=std::numeric_limits<double>::infinity();
        for(std::size_t a=0;a<=std::min(q,cap[t]);++a){double v=value(t,q,a);if(v<best){best=v;act[t][q]=a;}}V[t][q]=best;
    }
    Report o{{"Period","Inventory","Order","FillProbability","ExpectedTrade","Value","BellmanResidual"},{}};
    for(std::size_t t=0;t<n;++t)for(std::size_t q=0;q<=Q;++q){auto a=act[t][q];o.rows.push_back({double(t+1),double(q),double(a),x[t][0],x[t][0]*a,V[t][q],std::abs(V[t][q]-value(t,q,a))});}
    return o;
}
} }
