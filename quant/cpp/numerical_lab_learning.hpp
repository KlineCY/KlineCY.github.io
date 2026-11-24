#pragma once
#include "numerical_lab_common.hpp"

namespace kline { namespace numerical_lab {
inline double sigmoid(double z){if(z>=0)return 1/(1+std::exp(-z));double e=std::exp(z);return e/(1+e);}
inline double softplus(double z){return std::max(z,0.0)+std::log1p(std::exp(-std::abs(z)));}
inline Report damped_logistic(const Mat& x,const Vec& p){
    auto d=validate(x,4,2,16);parameters(p,2);double lam=positive(p[0]);auto maxit=count(p[1],1,1000);
    require(double(x.size())*d*d*maxit<=1e8,"Logistic work limit exceeded");
    Mat X;Vec y;double ones=0;
    for(const auto& row:x){require(row.back()==0||row.back()==1,"Binary outcome required");Vec v={1};v.insert(v.end(),row.begin(),row.end()-1);X.push_back(v);y.push_back(row.back());ones+=row.back();}
    require(ones>0&&ones<x.size(),"Both outcome classes required");
    double n=double(x.size());Vec w(d,0);
    auto loss=[&](const Vec& z){double v=0;for(std::size_t i=0;i<X.size();++i){double a=dot(X[i],z);v+=(softplus(a)-a*y[i])/n;}for(std::size_t j=1;j<d;++j)v+=.5*lam*z[j]*z[j];return v;};
    double error=1;std::size_t iter=0;
    for(;iter<maxit;++iter){
        Mat H(d,Vec(d,0));Vec g(d,0);
        for(std::size_t i=0;i<X.size();++i){double q=sigmoid(dot(X[i],w)),a=q*(1-q);
            for(std::size_t j=0;j<d;++j){g[j]+=X[i][j]*(q-y[i])/n;for(std::size_t k=0;k<d;++k)H[j][k]+=X[i][j]*X[i][k]*a/n;}}
        for(std::size_t j=1;j<d;++j){g[j]+=lam*w[j];H[j][j]+=lam;}
        error=norm(g);if(error<1e-9)break;
        Vec step=solve_spd(H,g);double old=loss(w),t=1,descent=dot(g,step);bool accepted=false;
        for(int k=0;k<60;++k){
            Vec z=w;for(std::size_t j=0;j<d;++j)z[j]-=t*step[j];
            if(loss(z)<=old-1e-4*t*descent){w=std::move(z);accepted=true;break;}
            t*=.5;
        }
        require(accepted,"Logistic line search failed");
    }
    require(error<1e-7,"Logistic first-order residual too large");
    Report out{{"Coefficient","Estimate","MeanPenalizedLoss","GradientNorm","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)out.rows.push_back({double(j),w[j],loss(w),error,double(iter)});
    return out;
}
inline Report ridge_press(const Mat& x,const Vec& p){
    auto d=validate(x,4,2,24);parameters(p,1);double lam=positive(p[0]);
    Mat X;Vec y;for(const auto& r:x){Vec v={1};v.insert(v.end(),r.begin(),r.end()-1);X.push_back(v);y.push_back(r.back());}
    Mat A(d,Vec(d,0));Vec b(d,0);
    for(std::size_t t=0;t<x.size();++t)for(std::size_t i=0;i<d;++i){
        b[i]+=X[t][i]*y[t];for(std::size_t j=0;j<d;++j)A[i][j]+=X[t][i]*X[t][j];}
    for(std::size_t i=1;i<d;++i)A[i][i]+=lam;
    Vec w=solve_spd(A,b);Mat inv(d,Vec(d,0));
    for(std::size_t j=0;j<d;++j){Vec e(d,0);e[j]=1;auto v=solve_spd(A,e);for(std::size_t i=0;i<d;++i)inv[i][j]=v[i];}
    Report out{{"Row","Actual","Fitted","Leverage","LeaveOneOutPrediction","LeaveOneOutSquaredError"},{}};
    for(std::size_t t=0;t<x.size();++t){
        double pred=dot(X[t],w),h=dot(X[t],mul(inv,X[t])),err=(y[t]-pred)/positive(1-h);
        out.rows.push_back({double(t),y[t],pred,h,y[t]-err,err*err});
    }
    return out;
}
inline Report recursive_least_squares(const Mat& x,const Vec& p){
    auto d=validate(x,2,2,24);parameters(p,2);double forget=p[0],prior=positive(p[1]);require(forget>0&&forget<=1,"Invalid forgetting factor");
    Mat P(d,Vec(d,0));for(std::size_t j=0;j<d;++j)P[j][j]=prior;
    Vec w(d,0);Report out{{"Period","PriorPrediction","Actual","Innovation","InnovationVariance","PosteriorPrediction"},{}};
    for(std::size_t j=0;j<d;++j)out.header.push_back("Coefficient"+std::to_string(j));
    for(std::size_t t=0;t<x.size();++t){
        Vec z={1};z.insert(z.end(),x[t].begin(),x[t].end()-1);double y=x[t].back(),pred=dot(w,z);
        Vec q=mul(P,z);double den=positive(forget+dot(z,q)),e=y-pred;
        for(std::size_t j=0;j<d;++j)w[j]+=q[j]*e/den;
        for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<d;++j)P[i][j]=(P[i][j]-q[i]*q[j]/den)/forget;
        Vec row={double(t),pred,y,e,den/forget,dot(w,z)};row.insert(row.end(),w.begin(),w.end());out.rows.push_back(row);
    }
    return out;
}
inline Report gaussian_discriminant(const Mat& x,const Vec& p){
    auto d=validate(x,6,2,16)-1;parameters(p,3);auto n=count(p[0],4,x.size()-1);double shrink=p[1],ridge=positive(p[2]);
    require(shrink>=0&&shrink<=1,"Invalid shrinkage");
    Mat means(2,Vec(d,0));Vec counts(2,0);
    for(const auto& r:x)require(r.back()==0||r.back()==1,"Binary outcome required");
    for(std::size_t i=0;i<n;++i){auto k=std::size_t(x[i].back());++counts[k];for(std::size_t j=0;j<d;++j)means[k][j]+=x[i][j];}
    for(std::size_t k=0;k<2;++k){require(counts[k]>=2,"Each training class needs two observations");for(double& v:means[k])v/=counts[k];}
    Mat A(d,Vec(d,0));for(std::size_t t=0;t<n;++t){
        auto k=std::size_t(x[t].back());for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<d;++j)
        A[i][j]+=(x[t][i]-means[k][i])*(x[t][j]-means[k][j])/(n-2);}
    double mu=0;for(std::size_t i=0;i<d;++i)mu+=A[i][i]/d;
    for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<d;++j)A[i][j]=(1-shrink)*A[i][j]+(i==j?shrink*mu+ridge:0);
    Vec delta(d);for(std::size_t j=0;j<d;++j)delta[j]=means[1][j]-means[0][j];
    Vec beta=solve_spd(A,delta);double a=std::log(counts[1]/counts[0])-.5*(dot(means[1],beta)+dot(means[0],beta));
    Report out{{"Row","Actual","LogOdds","Probability","Prediction","Brier","LogLoss"},{}};
    for(std::size_t t=n;t<x.size();++t){
        Vec z(x[t].begin(),x[t].end()-1);double l=a+dot(z,beta),q=sigmoid(l),y=x[t].back();
        out.rows.push_back({double(t),y,l,q,q>=.5?1.0:0.0,(q-y)*(q-y),softplus(l)-y*l});
    }
    return out;
}
inline Report pca_jacobi(const Mat& x,const Vec& p){
    auto d=validate(x,3,2,24);parameters(p,2);auto train=count(p[0],2,x.size()-1),keep=count(p[1],1,d);
    require(double(x.size())*d*d<=1e8,"PCA work limit exceeded");
    Vec m(d,0);for(std::size_t t=0;t<train;++t)for(std::size_t j=0;j<d;++j)m[j]+=x[t][j]/train;
    Mat A(d,Vec(d,0)),Q(d,Vec(d,0));for(std::size_t i=0;i<d;++i)Q[i][i]=1;
    for(std::size_t t=0;t<train;++t)for(std::size_t i=0;i<d;++i)for(std::size_t j=0;j<d;++j)A[i][j]+=(x[t][i]-m[i])*(x[t][j]-m[j])/(train-1);
    double trace=0;for(std::size_t i=0;i<d;++i)trace+=A[i][i];positive(trace);
    double residual=trace;for(std::size_t iter=0;iter<100*d*d;++iter){
        std::size_t a=0,b=1;residual=0;
        for(std::size_t i=0;i<d;++i)for(std::size_t j=i+1;j<d;++j)if(std::abs(A[i][j])>residual){a=i;b=j;residual=std::abs(A[i][j]);}
        if(residual<=1e-12*trace)break;
        double angle=.5*std::atan2(2*A[a][b],A[b][b]-A[a][a]),c=std::cos(angle),s=std::sin(angle);
        double aa=A[a][a],bb=A[b][b],ab=A[a][b];
        A[a][a]=c*c*aa-2*c*s*ab+s*s*bb;A[b][b]=s*s*aa+2*c*s*ab+c*c*bb;A[a][b]=A[b][a]=0;
        for(std::size_t i=0;i<d;++i){
            if(i!=a&&i!=b){double u=A[i][a],v=A[i][b];A[i][a]=A[a][i]=c*u-s*v;A[i][b]=A[b][i]=s*u+c*v;}
            double u=Q[i][a],v=Q[i][b];Q[i][a]=c*u-s*v;Q[i][b]=s*u+c*v;
        }
    }
    require(residual<=1e-10*trace,"Jacobi eigensolver failed");
    std::vector<std::size_t> order(d);std::iota(order.begin(),order.end(),0);
    std::stable_sort(order.begin(),order.end(),[&](auto a,auto b){return A[a][a]>A[b][b];});
    Report out{{"Row","ReconstructionSquaredError","RetainedVarianceShare"},{}};
    for(std::size_t j=0;j<keep;++j)out.header.push_back("PC"+std::to_string(j+1));
    double explained=0;for(std::size_t j=0;j<keep;++j)explained+=A[order[j]][order[j]]/trace;
    for(std::size_t t=train;t<x.size();++t){
        Vec z=x[t],scores,recon(d,0);for(std::size_t j=0;j<d;++j)z[j]-=m[j];
        for(std::size_t k=0;k<keep;++k){
            auto j=order[k];Vec v(d);for(std::size_t i=0;i<d;++i)v[i]=Q[i][j];
            double a=dot(z,v);scores.push_back(a);for(std::size_t i=0;i<d;++i)recon[i]+=a*v[i];
        }
        for(std::size_t j=0;j<d;++j)z[j]-=recon[j];
        Vec row={double(t),dot(z,z),explained};row.insert(row.end(),scores.begin(),scores.end());out.rows.push_back(row);
    }
    return out;
}
} }
