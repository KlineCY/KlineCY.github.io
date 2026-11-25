#pragma once
#include "comprehensive_common.hpp"
namespace kline { namespace comprehensive {
inline Report rrqr_regression(const Mat& x,const Vec& p){
    auto d=validate(x,2,2,33)-1;
    parameters(p,1);
    double tol=positive(p[0]);
    auto n=x.size();
    require(tol<1&&double(n)*d*d<=5e7,"Invalid QR controls");
    Mat a(n,Vec(d)); Vec y(n);
    for(std::size_t i=0;i<n;++i){
        y[i]=x[i][0];
        for(std::size_t j=0;j<d;++j)a[i][j]=x[i][j+1];
    }
    std::vector<std::size_t> ix(d); std::iota(ix.begin(),ix.end(),0);
    double scale=0;
    for(std::size_t j=0;j<d;++j){
        double z=0;for(std::size_t i=0;i<n;++i)z=std::hypot(z,a[i][j]);
        scale=std::max(scale,z);
    }
    std::size_t rank=0;
    for(std::size_t k=0;k<std::min(n,d);++k){
        std::size_t pivot=k;double best=0;
        for(std::size_t j=k;j<d;++j){
            double z=0;for(std::size_t i=k;i<n;++i)z=std::hypot(z,a[i][j]);
            if(z>best){best=z;pivot=j;}
        }
        if(best<=tol*scale)break;
        for(auto& r:a)std::swap(r[k],r[pivot]);
        std::swap(ix[k],ix[pivot]);
        Vec v(n-k);
        for(std::size_t i=k;i<n;++i)v[i-k]=a[i][k]/best;
        v[0]+=std::copysign(1.,v[0]);
        double vv=positive(dot(v,v));
        for(std::size_t j=k;j<d;++j){
            double h=0;for(std::size_t i=k;i<n;++i)h+=v[i-k]*a[i][j];
            for(std::size_t i=k;i<n;++i)a[i][j]-=2*h*v[i-k]/vv;
        }
        double h=0;for(std::size_t i=k;i<n;++i)h+=v[i-k]*y[i];
        for(std::size_t i=k;i<n;++i)y[i]-=2*h*v[i-k]/vv;
        ++rank;
    }
    Vec b(d,0),coef(d,0),grad(d,0);
    for(std::size_t i=rank;i-->0;){
        double z=y[i];for(std::size_t j=i+1;j<rank;++j)z-=a[i][j]*b[j];
        b[i]=z/a[i][i];
    }
    for(std::size_t j=0;j<d;++j)coef[ix[j]]=b[j];
    double ss=0;Vec position(d);
    for(std::size_t j=0;j<d;++j)position[ix[j]]=j+1;
    for(auto& r:x){
        Vec z(r.begin()+1,r.end());double e=r[0]-dot(z,coef);ss+=e*e;
        for(std::size_t j=0;j<d;++j)grad[j]+=z[j]*e;
    }
    Report out{{"Feature","Coefficient","Rank","ResidualNorm","NormalResidual","PermutationIndex"},{}};
    for(std::size_t j=0;j<d;++j)out.rows.push_back({double(j+1),coef[j],double(rank),std::sqrt(ss),inf_norm(grad),position[j]});
    return out;
}

inline Report woodbury_risk_solve(const Mat& x,const Vec& p){
    auto k=validate(x,1,3,34)-2,n=x.size();
    parameters(p,0);
    require(double(n)*k*k<=5e7,"Factor solve work limit");
    Mat K=identity(k);
    Vec rhs(k,0),base(n),diag(n);
    double ld=0;
    for(std::size_t i=0;i<n;++i){
        diag[i]=positive(x[i][0]);base[i]=x[i].back()/diag[i];ld+=std::log(diag[i]);
        Vec b(x[i].begin()+1,x[i].end()-1);
        add_outer(K,b,1/diag[i]);
        for(std::size_t j=0;j<k;++j)rhs[j]+=b[j]*base[i];
    }
    Vec z=solve_spd(K,rhs),w(n),btx(k,0);
    Mat L(k,Vec(k,0));
    for(std::size_t i=0;i<k;++i)for(std::size_t j=0;j<=i;++j){
        double v=K[i][j];for(std::size_t h=0;h<j;++h)v-=L[i][h]*L[j][h];
        L[i][j]=i==j?std::sqrt(positive(v)):v/L[j][j];
        if(i==j)ld+=2*std::log(L[i][j]);
    }
    double quad=0,err=0;
    for(std::size_t i=0;i<n;++i){
        Vec b(x[i].begin()+1,x[i].end()-1);
        w[i]=base[i]-dot(b,z)/diag[i];
        quad+=x[i].back()*w[i];
        for(std::size_t j=0;j<k;++j)btx[j]+=b[j]*w[i];
    }
    for(std::size_t i=0;i<n;++i){
        Vec b(x[i].begin()+1,x[i].end()-1);
        err=std::max(err,std::abs(diag[i]*w[i]+dot(b,btx)-x[i].back()));
    }
    Report out{{"Asset","Solution","LogDet","QuadraticForm","Residual"},{}};
    for(std::size_t i=0;i<n;++i)out.rows.push_back({double(i+1),w[i],ld,quad,err});
    return out;
}

inline Report pcg_jacobi(const Mat& x,const Vec& p){
    auto n=validate(x,1,2,33)-1;
    require(x.size()==n,"Square PCG system required");parameters(p,3);
    double tol=positive(p[0]);auto cap=count(p[1],1,10000);
    require(p[2]==0||p[2]==1,"Preconditioner flag must be binary");
    Mat A;Vec b;
    for(auto&r:x){A.emplace_back(r.begin(),r.end()-1);b.push_back(r.back());}
    solve_spd(A,b);
    Vec w(n,0),r=b,z(n),dir(n);
    auto pre=[&](const Vec& r){Vec z(n);for(std::size_t j=0;j<n;++j)z[j]=p[2]?r[j]/A[j][j]:r[j];return z;};
    double bnorm=std::sqrt(squared(b)),den=std::max(bnorm,1e-300);
    Report out{{"Iteration","ResidualNorm","RelativeResidual","Energy","Converged"},{}};
    auto emit=[&](std::size_t k){
        Vec a=mul(A,w);double e=0;
        for(std::size_t j=0;j<n;++j)e=std::hypot(e,b[j]-a[j]);
        bool done=e/den<=tol;
        out.rows.push_back({double(k),e,e/den,.5*dot(w,a)-dot(w,b),double(done)});
        return done;
    };
    if(emit(0))return out;
    z=pre(r);dir=z;double rz=positive(dot(r,z));
    for(std::size_t k=1;k<=cap;++k){
        Vec q=mul(A,dir);double a=rz/positive(dot(dir,q));
        for(std::size_t j=0;j<n;++j){w[j]+=a*dir[j];r[j]-=a*q[j];}
        if(emit(k))break;
        z=pre(r);double next=dot(r,z);
        if(next<=0)break;
        double beta=next/rz;
        for(std::size_t j=0;j<n;++j)dir[j]=z[j]+beta*dir[j];
        rz=next;
    }
    return out;
}

inline Report lanczos_krylov(const Mat& x,const Vec& p){
    auto n=validate(x,1,2,33)-1;require(x.size()==n,"Square Lanczos matrix required");
    parameters(p,2);auto m=count(p[0],1,n);double tol=positive(p[1]);
    Mat A;Vec q;
    for(auto&r:x){A.emplace_back(r.begin(),r.end()-1);q.push_back(r.back());}
    double scale=0;
    for(std::size_t i=0;i<n;++i)for(std::size_t j=0;j<n;++j){
        require(std::abs(A[i][j]-A[j][i])<=1e-12*std::max({1.,std::abs(A[i][j]),std::abs(A[j][i])}),"Symmetric matrix required");
        scale=std::max(scale,std::abs(A[i][j]));
    }
    double norm=positive(std::sqrt(squared(q)));for(double&v:q)v/=norm;
    Mat basis;Vec prev(n,0);double beta=0;
    Report out{{"Step","Alpha","Beta","OrthogonalityError","ProjectionResidual"},{}};
    for(std::size_t k=0;k<m;++k){
        basis.push_back(q);Vec aq=mul(A,q),r=aq;
        for(std::size_t i=0;i<n;++i)r[i]-=beta*prev[i];
        double alpha=dot(q,r);
        for(std::size_t i=0;i<n;++i)r[i]-=alpha*q[i];
        for(int pass=0;pass<2;++pass)for(auto&v:basis){
            double z=dot(r,v);for(std::size_t i=0;i<n;++i)r[i]-=z*v[i];
        }
        double b=std::sqrt(squared(r)),orth=0,err=0;
        for(std::size_t i=0;i<basis.size();++i)for(std::size_t j=0;j<basis.size();++j)
            orth=std::max(orth,std::abs(dot(basis[i],basis[j])-double(i==j)));
        for(std::size_t i=0;i<n;++i)err=std::hypot(err,aq[i]-alpha*q[i]-beta*prev[i]-r[i]);
        out.rows.push_back({double(k+1),alpha,b,orth,err});
        if(b<=tol*std::max(1.,scale)||k+1==m)break;
        prev=q;q=r;for(double&v:q)v/=b;beta=b;
    }
    return out;
}

inline Report polytope_analytic_center(const Mat& x,const Vec& p){
    auto d=validate(x,2,2,17)-1;parameters(p,d+2);
    double tol=positive(p[0]);auto cap=count(p[1],1,500);
    require(double(x.size())*d*d*cap<=5e7,"Barrier work limit");
    Mat A;Vec b,w(p.begin()+2,p.end());
    for(auto&r:x){A.emplace_back(r.begin(),r.end()-1);b.push_back(r.back());}
    auto objective=[&](const Vec& z){
        double f=0;for(std::size_t i=0;i<A.size();++i)f-=std::log(positive(b[i]-dot(A[i],z)));return f;
    };
    double f=objective(w),gn=0,slack=0;std::size_t iteration=0;
    for(;iteration<=cap;++iteration){
        Vec g(d,0),s(x.size());Mat H(d,Vec(d,0));
        slack=std::numeric_limits<double>::infinity();
        for(std::size_t i=0;i<A.size();++i){
            s[i]=positive(b[i]-dot(A[i],w));slack=std::min(slack,s[i]);
            Vec v=A[i];for(double&z:v)z/=s[i];
            for(std::size_t j=0;j<d;++j)g[j]+=v[j];
            add_outer(H,v,1);
        }
        gn=inf_norm(g);
        if(gn<=tol)break;
        require(iteration<cap,"Analytic center did not converge");
        Vec rhs=g;for(double&v:rhs)v=-v;
        Vec dir=solve_spd(H,rhs);
        double t=1,gd=dot(g,dir);
        require(gd<0,"No barrier descent direction");
        for(std::size_t i=0;i<A.size();++i){
            double a=dot(A[i],dir);if(a>0)t=std::min(t,.99*s[i]/a);
        }
        bool ok=false;
        for(int ls=0;ls<60;++ls){
            Vec z=w;for(std::size_t j=0;j<d;++j)z[j]+=t*dir[j];
            double v=objective(z);
            if(v<=f+1e-4*t*gd){w=z;f=v;ok=true;break;}
            t*=.5;
        }
        require(ok,"Barrier line search failed");
    }
    Report out{{"Variable","Center","Barrier","GradientNorm","MinSlack","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)out.rows.push_back({double(j+1),w[j],f,gn,slack,double(iteration)});
    return out;
}

inline Report simplex_active_set(const Mat& x,const Vec& p){
    auto d=validate(x,1,2,13)-1;require(x.size()==d,"Square QP matrix required");parameters(p,0);
    Mat Q;Vec c;
    for(auto&r:x){Q.emplace_back(r.begin(),r.end()-1);c.push_back(r.back());}
    solve_spd(Q,Vec(d,0));
    Vec best;double obj=std::numeric_limits<double>::infinity(),nu=0,kkt=0;
    for(std::size_t mask=1;mask<(std::size_t(1)<<d);++mask){
        std::vector<std::size_t> S;
        for(std::size_t j=0;j<d;++j)if(mask&(std::size_t(1)<<j))S.push_back(j);
        auto m=S.size();Mat H(m,Vec(m));Vec b(m),one(m,1);
        for(std::size_t i=0;i<m;++i){
            b[i]=c[S[i]];for(std::size_t j=0;j<m;++j)H[i][j]=Q[S[i]][S[j]];
        }
        Vec u=solve_spd(H,one),v=solve_spd(H,b),w(d,0);
        double lambda=-(1+sum(v))/positive(sum(u));bool feasible=true;
        for(std::size_t i=0;i<m;++i){w[S[i]]=-v[i]-lambda*u[i];if(w[S[i]]<-1e-10)feasible=false;}
        if(!feasible)continue;
        Vec g=mul(Q,w);double f=.5*dot(w,g)+dot(c,w),err=0;
        for(std::size_t i=0;i<d;++i){double z=g[i]+c[i]+lambda;err=std::max(err,w[i]>1e-10?std::abs(z):std::max(0.,-z));}
        if(err>1e-8*std::max(1.,inf_norm(c))||std::abs(sum(w)-1)>1e-8)continue;
        if(f<obj){best=w;obj=f;nu=lambda;kkt=err;}
    }
    require(!best.empty(),"No active set with valid certificate");
    Report out{{"Asset","Weight","Multiplier","Objective","StationarityResidual","BudgetResidual","Active"},{}};
    for(std::size_t j=0;j<d;++j)out.rows.push_back({double(j+1),best[j],nu,obj,kkt,std::abs(sum(best)-1),double(best[j]>1e-10)});
    return out;
}

inline Report pchip_monotone(const Mat& x,const Vec& p){
    validate(x,2,2,2);require(!p.empty()&&p.size()<=100000,"Query points required");parameters(p,p.size());
    auto n=x.size();Vec h(n-1),delta(n-1),m(n),nodes;
    for(auto&r:x)nodes.push_back(r[0]);
    for(std::size_t i=0;i+1<n;++i){h[i]=positive(x[i+1][0]-x[i][0]);delta[i]=(x[i+1][1]-x[i][1])/h[i];}
    m.front()=delta.front();m.back()=delta.back();
    for(std::size_t i=1;i+1<n;++i){
        double a=delta[i-1],b=delta[i];
        if(a==0||b==0||std::signbit(a)!=std::signbit(b))m[i]=0;
        else{double u=2*h[i]+h[i-1],v=h[i]+2*h[i-1];m[i]=(u+v)/(u/a+v/b);}
    }
    Report out{{"Query","Value","Derivative","LowerKnot","UpperKnot"},{}};
    for(double q:p){
        require(q>=nodes.front()&&q<=nodes.back(),"Interpolation outside support");
        auto it=std::upper_bound(nodes.begin(),nodes.end(),q);
        auto i=std::min(n-2,std::size_t(it-nodes.begin()-1));
        double t=(q-nodes[i])/h[i],t2=t*t,t3=t2*t,a=x[i][1],b=x[i+1][1];
        double y=(2*t3-3*t2+1)*a+(t3-2*t2+t)*h[i]*m[i]+(-2*t3+3*t2)*b+(t3-t2)*h[i]*m[i+1];
        double der=(6*t2-6*t)*(a-b)/h[i]+(3*t2-4*t+1)*m[i]+(3*t2-2*t)*m[i+1];
        out.rows.push_back({q,y,der,nodes[i],nodes[i+1]});
    }
    return out;
}

inline Report chebyshev_interpolant(const Mat& x,const Vec& p){
    validate(x,2,1,1);auto n=x.size();
    require(n<=128&&!p.empty()&&double(n)*p.size()<=5e7,"Chebyshev work limit");
    parameters(p,p.size());
    Vec c(n,0);
    const double pi=std::acos(-1.);
    for(std::size_t k=0;k<n;++k){
        for(std::size_t j=0;j<n;++j)c[k]+=x[j][0]*std::cos(pi*k*(j+.5)/n);
        c[k]*=(k==0?1.:2.)/n;
    }
    Report out{{"Query","Approximation"},{}};
    for(std::size_t k=0;k<n;++k)out.header.push_back("Coefficient"+std::to_string(k));
    for(double z:p){
        require(z>=-1&&z<=1,"Chebyshev query outside support");
        double b1=0,b2=0;
        for(std::size_t k=n;k-->1;){
            double b=c[k]+2*z*b1-b2;b2=b1;b1=b;
        }
        Vec row{z,c[0]+z*b1-b2};row.insert(row.end(),c.begin(),c.end());out.rows.push_back(row);
    }
    return out;
}
} }
