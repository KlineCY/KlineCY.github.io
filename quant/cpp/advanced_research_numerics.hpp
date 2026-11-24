#pragma once
#include "advanced_research_common.hpp"
namespace kline { namespace advanced_research {
inline Report gmres_restarted(const Mat& x,const Vec& p){
    auto d=validate(x,1,2,65)-1;parameters(p,3);require(x.size()==d,"Square system required");
    auto m=count(p[0],1,d),K=count(p[2],1,100000);double tol=positive(p[1]);
    require(double(K)*d*(d+m)<=1e8,"GMRES work limit");
    Mat A;Vec b,w(d,0);for(auto&r:x){A.emplace_back(r.begin(),r.end()-1);b.push_back(r.back());}
    auto residual=[&](const Vec& z){Vec r=mul(A,z);for(std::size_t i=0;i<d;++i)r[i]=b[i]-r[i];return r;};
    double target=tol*std::max(1.,std::sqrt(squared(b)));Vec r=residual(w);double rn=std::sqrt(squared(r));std::size_t it=0;
    while(rn>target&&it<K){
        Mat V(m+1,Vec(d,0)),H(m+1,Vec(m,0));Vec cs(m),sn(m),g(m+1,0);g[0]=rn;
        for(std::size_t i=0;i<d;++i)V[0][i]=r[i]/rn;
        std::size_t used=0;
        for(std::size_t j=0;j<m&&it<K;++j){
            Vec z=mul(A,V[j]);double initial=std::sqrt(squared(z));
            for(int pass=0;pass<2;++pass)for(std::size_t i=0;i<=j;++i){double a=dot(V[i],z);H[i][j]+=a;for(std::size_t k=0;k<d;++k)z[k]-=a*V[i][k];}
            H[j+1][j]=std::sqrt(squared(z));bool breakdown=H[j+1][j]<=1e-14*std::max(1.,initial);
            if(!breakdown)for(std::size_t k=0;k<d;++k)V[j+1][k]=z[k]/H[j+1][j];
            for(std::size_t i=0;i<j;++i){double a=cs[i]*H[i][j]+sn[i]*H[i+1][j];H[i+1][j]=-sn[i]*H[i][j]+cs[i]*H[i+1][j];H[i][j]=a;}
            double h=positive(std::hypot(H[j][j],H[j+1][j]));cs[j]=H[j][j]/h;sn[j]=H[j+1][j]/h;H[j][j]=h;H[j+1][j]=0;
            g[j+1]=-sn[j]*g[j];g[j]=cs[j]*g[j];++it;used=j+1;
            if(breakdown||std::abs(g[j+1])<=target)break;
        }
        Vec y(used);for(std::size_t j=used;j-->0;){double a=g[j];for(std::size_t k=j+1;k<used;++k)a-=H[j][k]*y[k];y[j]=a/positive(H[j][j]);}
        for(std::size_t j=0;j<used;++j)for(std::size_t i=0;i<d;++i)w[i]+=V[j][i]*y[j];
        r=residual(w);rn=std::sqrt(squared(r));
    }
    require(rn<=target,"GMRES true residual did not converge");
    Report o{{"Index","Solution","TrueResidual","ResidualNorm","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),w[j],r[j],rn,double(it)});
    return o;
}

inline Report jacobi_eigensystem(const Mat& x,const Vec& p){
    auto d=validate(x,1,1,32);parameters(p,2);require(x.size()==d,"Square symmetric matrix required");
    double tol=positive(p[0]);auto K=count(p[1],1,1000000);require(double(K)*d*d<=1e8,"Jacobi work limit");
    Mat a=x,q=identity(d);double scale=1;
    for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<d;++j){require(std::abs(x[i][j]-x[j][i])<=1e-12*std::max({1.,std::abs(x[i][j]),std::abs(x[j][i])}),"Symmetric input required");scale=std::max(scale,std::abs(x[i][j]));a[i][j]=(x[i][j]+x[j][i])/2;}
    std::size_t it=0;double gap=0;
    for(;it<=K;){
        std::size_t i=0,j=0;gap=0;
        for(std::size_t u=0;u<d;++u)for(std::size_t v=u+1;v<d;++v)if(std::abs(a[u][v])>gap){gap=std::abs(a[u][v]);i=u;j=v;}
        if(gap<=tol*scale)break;require(it<K,"Jacobi eigen iteration limit");
        double t0=(a[j][j]-a[i][i])/(2*a[i][j]),t=std::copysign(1.,t0)/(std::abs(t0)+std::hypot(1.,t0)),c=1/std::hypot(1.,t),s=t*c;
        double v=a[i][j];a[i][i]-=t*v;a[j][j]+=t*v;a[i][j]=a[j][i]=0;
        for(std::size_t k=0;k<d;++k)if(k!=i&&k!=j){double u=a[k][i],w=a[k][j];a[k][i]=a[i][k]=c*u-s*w;a[k][j]=a[j][k]=s*u+c*w;}
        for(std::size_t k=0;k<d;++k){double u=q[k][i],w=q[k][j];q[k][i]=c*u-s*w;q[k][j]=s*u+c*w;}++it;
    }
    std::vector<std::size_t> order(d);std::iota(order.begin(),order.end(),0);std::stable_sort(order.begin(),order.end(),[&](auto i,auto j){return a[i][i]>a[j][j];});
    Mat V(d,Vec(d));Vec ev(d);
    for(std::size_t k=0;k<d;++k){auto j=order[k];ev[k]=a[j][j];std::size_t idx=0;for(std::size_t i=1;i<d;++i)if(std::abs(q[i][j])>std::abs(q[idx][j])+1e-14)idx=i;double sign=q[idx][j]<0?-1:1;for(std::size_t i=0;i<d;++i)V[k][i]=sign*q[i][j];}
    double rec=0,orth=0;
    for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<d;++j){double u=0;for(std::size_t k=0;k<d;++k)u+=ev[k]*V[k][i]*V[k][j];rec=std::max(rec,std::abs(u-x[i][j]));orth=std::max(orth,std::abs(dot(V[i],V[j])-double(i==j)));}
    Report o{{"EigenIndex","Coordinate","Eigenvalue","Eigenvector","ReconstructionResidual","OrthogonalityResidual","Iterations"},{}};
    for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<d;++j)o.rows.push_back({double(i+1),double(j+1),ev[i],V[i][j],rec,orth,double(it)});
    return o;
}

inline Report equality_ridge(const Mat& x,const Vec& p){
    auto d=validate(x,2,3,34)-2;parameters(p,1);double lam=positive(p[0]);
    Mat X,A;Vec y,b;
    for(auto&r:x){Vec z(r.begin()+1,r.end()-1);require(r[0]==0||r[0]==1,"Invalid row type");if(r[0]){A.push_back(z);b.push_back(r.back());}else{X.push_back(z);y.push_back(r.back());}}
    auto m=A.size();require(!X.empty()&&m>=1&&m<=d&&double(X.size())*d*d<=5e7,"Invalid constrained design");
    Mat H(d,Vec(d,0));Vec rhs(d,0);
    for(std::size_t i=0;i<X.size();++i){add_outer(H,X[i],1);for(std::size_t j=0;j<d;++j)rhs[j]+=X[i][j]*y[i];}
    for(std::size_t j=0;j<d;++j)H[j][j]+=lam;
    Vec v=solve_spd(H,rhs);Mat HA;for(auto&r:A)HA.push_back(solve_spd(H,r));Mat S(m,Vec(m));Vec z=mul(A,v);
    for(std::size_t j=0;j<m;++j){z[j]-=b[j];for(std::size_t k=0;k<m;++k)S[j][k]=dot(A[j],HA[k]);}
    Vec eta=solve_spd(S,z),w=v;
    for(std::size_t j=0;j<d;++j)for(std::size_t k=0;k<m;++k)w[j]-=HA[k][j]*eta[k];
    Vec g=mul(H,w),e=mul(A,w);for(std::size_t j=0;j<m;++j)e[j]-=b[j];
    for(std::size_t j=0;j<d;++j){g[j]-=rhs[j];for(std::size_t k=0;k<m;++k)g[j]+=eta[k]*A[k][j];}
    double f=.5*lam*squared(w);for(std::size_t i=0;i<X.size();++i)f+=.5*std::pow(dot(w,X[i])-y[i],2);
    Report o{{"Feature","Coefficient","Objective","EqualityResidual","StationarityResidual"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),w[j],f,inf_norm(e),inf_norm(g)});return o;
}

inline Report ctmc_uniformization(const Mat& x,const Vec& p){
    auto d=validate(x,1,1,32);parameters(p,2);require(x.size()==d,"Square generator required");double t=p[0];auto K=count(p[1],0,10000);
    require(t>=0&&double(K+1)*d*d*d<=5e7,"Invalid CTMC work limit");
    double rate=0;
    for(std::size_t i=0;i<d;++i){double s=0;for(std::size_t j=0;j<d;++j){require(i==j?x[i][j]<=0:x[i][j]>=0,"Invalid generator signs");s+=x[i][j];}require(std::abs(s)<=1e-10*std::max(1.,std::abs(x[i][i])),"Generator rows must sum to zero");rate=std::max(rate,-x[i][i]);}
    Mat R=identity(d);if(rate>0)for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<d;++j)R[i][j]+=x[i][j]/rate;
    double a=rate*t;require(a<=600,"Poisson mean outside stable domain");double w=std::exp(-a),mass=w;Mat power=identity(d),P=power;for(auto&r:P)for(double&v:r)v*=w;
    for(std::size_t k=1;k<=K;++k){Mat next(d,Vec(d,0));for(std::size_t i=0;i<d;++i)for(std::size_t l=0;l<d;++l)for(std::size_t j=0;j<d;++j)next[i][j]+=power[i][l]*R[l][j];power=next;w*=a/k;mass+=w;for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<d;++j)P[i][j]+=w*power[i][j];}
    double tail=std::max(0.,1-mass);
    Report o{{"From","To","TransitionProbability","PoissonTail","RowMassResidual"},{}};
    for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<d;++j)o.rows.push_back({double(i+1),double(j+1),P[i][j],tail,std::abs(sum(P[i])-mass)});return o;
}

inline Report sinkhorn_log(const Mat& x,const Vec& p){
    auto n=validate(x,1,2,33)-1;require(p.size()==n+3,"Missing target masses");parameters(p,p.size());
    double eps=positive(p[0]),tol=positive(p[1]);auto K=count(p[2],1,100000),m=x.size();
    require(m<=128&&double(K)*m*n<=5e7,"Sinkhorn work limit");
    Vec a,b(p.begin()+3,p.end());for(auto&r:x)a.push_back(positive(r.back()));for(double v:b)positive(v);
    require(std::abs(sum(a)-1)<=1e-10&&std::abs(sum(b)-1)<=1e-10,"Marginals must sum to one");
    auto lse=[](const Vec& z){double h=*std::max_element(z.begin(),z.end()),v=0;for(double u:z)v+=std::exp(u-h);return h+std::log(positive(v));};
    Vec f(m,0),g(n,0);Mat P(m,Vec(n));double gap=0,obj=0;std::size_t it=0;
    for(;it<K;){
        for(std::size_t i=0;i<m;++i){Vec z(n);for(std::size_t j=0;j<n;++j)z[j]=(g[j]-x[i][j])/eps;f[i]=eps*(std::log(a[i])-lse(z));}
        for(std::size_t j=0;j<n;++j){Vec z(m);for(std::size_t i=0;i<m;++i)z[i]=(f[i]-x[i][j])/eps;g[j]=eps*(std::log(b[j])-lse(z));}
        double shift=f[0];for(double&v:f)v-=shift;for(double&v:g)v+=shift;
        Vec aa(m,0),bb(n,0);obj=0;gap=0;
        for(std::size_t i=0;i<m;++i)for(std::size_t j=0;j<n;++j){double z=(f[i]+g[j]-x[i][j])/eps,v=std::exp(z);P[i][j]=v;aa[i]+=v;bb[j]+=v;if(v>0)obj+=v*(x[i][j]+eps*(z-1));}
        for(std::size_t i=0;i<m;++i)gap=std::max(gap,std::abs(aa[i]-a[i]));for(std::size_t j=0;j<n;++j)gap=std::max(gap,std::abs(bb[j]-b[j]));
        ++it;if(gap<=tol)break;
    }
    require(gap<=tol,"Sinkhorn marginals did not converge");
    Report o{{"Source","Target","Coupling","Cost","EntropyRegularizedObjective","MarginalResidual","Iterations"},{}};
    for(std::size_t i=0;i<m;++i)for(std::size_t j=0;j<n;++j)o.rows.push_back({double(i+1),double(j+1),P[i][j],x[i][j],obj,gap,double(it)});return o;
}

inline Report trust_region_quadratic(const Mat& x,const Vec& p){
    auto d=validate(x,1,2,33)-1;parameters(p,2);require(x.size()==d,"Square Hessian required");double radius=positive(p[0]),tol=positive(p[1]);
    Mat H;Vec g;for(auto&r:x){H.emplace_back(r.begin(),r.end()-1);g.push_back(r.back());}solve_spd(H,Vec(d,0));
    auto step=[&](double nu){Mat a=H;Vec b=g;for(std::size_t j=0;j<d;++j){a[j][j]+=nu;b[j]=-b[j];}return solve_spd(a,b);};
    Vec s=step(0);double nu=0;
    if(std::sqrt(squared(s))>radius){
        double lo=0,hi=1;for(int i=0;i<100&&std::sqrt(squared(step(hi)))>radius;++i)hi*=2;
        require(std::sqrt(squared(step(hi)))<=radius,"Trust region multiplier bracket failed");
        for(int i=0;i<100;++i){double m=lo/2+hi/2;if(std::sqrt(squared(step(m)))>radius)lo=m;else hi=m;}nu=hi;s=step(nu);
    }
    double n=std::sqrt(squared(s)),obj=dot(g,s)+.5*dot(s,mul(H,s)),comp=std::abs(nu*(n-radius));Vec r=mul(H,s);
    for(std::size_t j=0;j<d;++j)r[j]+=g[j]+nu*s[j];
    require(n<=radius+tol&&comp<=tol*std::max(1.,std::abs(obj)),"Trust region certificate failed");
    Report o{{"Index","Step","Multiplier","StepNorm","Objective","StationarityResidual","ComplementarityResidual"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),s[j],nu,n,obj,inf_norm(r),comp});return o;
}
} }
