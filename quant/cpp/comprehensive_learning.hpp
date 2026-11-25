#pragma once
#include "comprehensive_common.hpp"
#include "model_research_likelihood.hpp"
namespace kline { namespace comprehensive {
inline Report softmax_newton_fit(const Mat& x,const Vec& p){
    auto d=validate(x,2,3,34)-2;parameters(p,4);
    auto K=count(p[0],2,16);auto D=(K-1)*d;
    double lam=positive(p[1]),tol=positive(p[2]);auto cap=count(p[3],1,1000);
    require(D<=32&&double(cap)*x.size()*D*D<=5e7,"Softmax work limit");
    Mat X;std::vector<std::size_t> y;std::vector<std::size_t> train;Vec mass(K,0);
    for(std::size_t i=0;i<x.size();++i){
        auto&r=x[i];require(r[0]==0||r[0]==1,"Binary train flag");
        y.push_back(count(r[1],1,K)-1);X.emplace_back(r.begin()+2,r.end());
        if(r[0]){train.push_back(i);++mass[y.back()];}
    }
    for(double v:mass)positive(v);
    auto scores=[&](const Vec&w,const Vec& a){
        Vec z(K,0);for(std::size_t k=0;k<K-1;++k)for(std::size_t j=0;j<d;++j)z[k]+=w[k*d+j]*a[j];
        return z;
    };
    auto evaluate=[&](const Vec&w){
        auto s=model_research::regularized_state(w,lam);
        for(auto i:train){
            auto&a=X[i];Vec z=scores(w,a);double ld=logsumexp(z);
            s.value+=ld-z[y[i]];for(double&v:z)v=std::exp(v-ld);
            for(std::size_t k=0;k<K-1;++k)for(std::size_t j=0;j<d;++j){
                auto u=k*d+j;s.gradient[u]+=(z[k]-double(y[i]==k))*a[j];
                for(std::size_t l=0;l<K-1;++l)for(std::size_t h=0;h<d;++h)
                    s.hessian[u][l*d+h]+=z[k]*(double(k==l)-z[l])*a[j]*a[h];
            }
        }
        return s;
    };
    auto f=model_research::damped_newton(D,tol,cap,evaluate);
    Report out{{"Row","Train","Label","Predicted"},{}};
    for(std::size_t k=1;k<=K;++k)out.header.push_back("P"+std::to_string(k));
    out.header.insert(out.header.end(),{"PenalizedNLL","ScoreResidual"});
    for(std::size_t k=1;k<K;++k)for(std::size_t j=1;j<=d;++j)
        out.header.push_back("Beta"+std::to_string(k)+"_"+std::to_string(j));
    for(std::size_t i=0;i<x.size();++i){
        Vec z=scores(f.beta,X[i]);double ld=logsumexp(z);
        auto k=std::max_element(z.begin(),z.end())-z.begin();
        Vec r{double(i+1),x[i][0],x[i][1],double(k+1)};
        for(double v:z)r.push_back(std::exp(v-ld));
        r.push_back(f.state.value);r.push_back(inf_norm(f.state.gradient));
        r.insert(r.end(),f.beta.begin(),f.beta.end());out.rows.push_back(r);
    }
    return out;
}

inline Report ranknet_linear_fit(const Mat& x,const Vec& p){
    auto d=validate(x,2,4,35)-3;parameters(p,3);
    double lam=positive(p[0]),tol=positive(p[1]);auto cap=count(p[2],1,1000);
    std::map<double,std::vector<std::size_t>> groups;std::map<double,double> flag;Mat X;
    for(std::size_t i=0;i<x.size();++i){
        auto&r=x[i];require(r[0]==0||r[0]==1,"Binary train flag");
        auto f=flag.emplace(r[1],r[0]);require(f.first->second==r[0],"Group crosses train and test");
        if(r[0])groups[r[1]].push_back(i);
        X.emplace_back(r.begin()+3,r.end());
    }
    double work=0;for(auto&kv:groups)work+=double(kv.second.size())*kv.second.size();
    require(work<=1e6,"Pair construction limit");
    Mat dif;
    for(auto&kv:groups)for(auto i:kv.second)for(auto j:kv.second)if(x[i][2]>x[j][2]){
        Vec v(d);for(std::size_t k=0;k<d;++k)v[k]=X[i][k]-X[j][k];dif.push_back(v);
    }
    require(!dif.empty()&&double(cap)*dif.size()*d*d<=5e7,"Invalid ranking design");
    auto evaluate=[&](const Vec&w){
        auto s=model_research::regularized_state(w,lam);double n=dif.size();
        for(auto&a:dif){
            double z=dot(a,w),b=logistic(-z);s.value+=model_research::softplus(-z)/n;
            for(std::size_t j=0;j<d;++j)s.gradient[j]-=b*a[j]/n;
            add_outer(s.hessian,a,b*(1-b)/n);
        }
        return s;
    };
    auto f=model_research::damped_newton(d,tol,cap,evaluate);
    Report out{{"Row","Train","Group","Relevance","Score","PenalizedLoss","ScoreResidual"},{}};
    for(std::size_t j=1;j<=d;++j)out.header.push_back("Coefficient"+std::to_string(j));
    for(std::size_t i=0;i<x.size();++i){
        Vec r{double(i+1),x[i][0],x[i][1],x[i][2],dot(X[i],f.beta),f.state.value,inf_norm(f.state.gradient)};
        r.insert(r.end(),f.beta.begin(),f.beta.end());out.rows.push_back(r);
    }
    return out;
}

inline Report gamma_log_glm(const Mat& x,const Vec& p){
    auto d=validate(x,2,3,34)-2;parameters(p,4);
    double k=positive(p[0]),lam=positive(p[1]),tol=positive(p[2]);auto cap=count(p[3],1,1000);
    require(double(cap)*x.size()*d*d<=5e7,"Gamma work limit");
    Mat X;std::vector<std::size_t> train;
    for(std::size_t i=0;i<x.size();++i){
        auto&r=x[i];require(r[0]==0||r[0]==1,"Binary train flag");positive(r[1]);
        X.emplace_back(r.begin()+2,r.end());if(r[0])train.push_back(i);
    }
    require(!train.empty(),"Training data required");
    double constant=std::lgamma(k)-k*std::log(k);
    auto evaluate=[&](const Vec&w){
        auto s=model_research::regularized_state(w,lam);
        for(auto i:train){
            double eta=dot(X[i],w),y=x[i][1],a=y*std::exp(-eta);
            s.value+=constant-(k-1)*std::log(y)+k*(eta+a);
            for(std::size_t j=0;j<d;++j)s.gradient[j]+=k*(1-a)*X[i][j];
            add_outer(s.hessian,X[i],k*a);
        }
        return s;
    };
    auto f=model_research::damped_newton(d,tol,cap,evaluate);
    Report out{{"Row","Train","Actual","Mean","Variance","UnitDeviance","PenalizedNLL","ScoreResidual"},{}};
    for(std::size_t j=1;j<=d;++j)out.header.push_back("Coefficient"+std::to_string(j));
    for(std::size_t i=0;i<x.size();++i){
        double mu=positive(std::exp(dot(X[i],f.beta))),a=positive(x[i][1]/mu);
        Vec r{double(i+1),x[i][0],x[i][1],mu,mu*mu/k,2*(a-1-std::log(a)),f.state.value,inf_norm(f.state.gradient)};
        r.insert(r.end(),f.beta.begin(),f.beta.end());out.rows.push_back(r);
    }
    return out;
}

inline Report ordinal_logit_fixed_cuts(const Mat& x,const Vec& p){
    auto d=validate(x,2,3,34)-2;
    require(p.size()>=5,"Ordinal parameters required");parameters(p,p.size());
    auto K=count(p[0],2,16);require(p.size()==K+3,"Incorrect cutpoint count");
    double lam=positive(p[1]),tol=positive(p[2]);auto cap=count(p[3],1,1000);
    Vec cuts(p.begin()+4,p.end());
    for(std::size_t j=1;j<cuts.size();++j)require(cuts[j]>cuts[j-1],"Increasing cutpoints required");
    require(double(cap)*x.size()*d*d<=5e7,"Ordinal work limit");
    Mat X;std::vector<std::size_t> y,train;
    for(std::size_t i=0;i<x.size();++i){
        auto&r=x[i];require(r[0]==0||r[0]==1,"Binary train flag");
        y.push_back(count(r[1],1,K)-1);X.emplace_back(r.begin()+2,r.end());if(r[0])train.push_back(i);
    }
    require(!train.empty(),"Training data required");
    auto logprob=[&](double eta,std::size_t k){
        if(k==0)return -model_research::softplus(eta-cuts[0]);
        if(k+1==K)return -model_research::softplus(cuts.back()-eta);
        double a=cuts[k-1]-eta,b=cuts[k]-eta;
        return -model_research::softplus(-b)-model_research::softplus(a)+std::log(positive(-std::expm1(cuts[k-1]-cuts[k])));
    };
    auto evaluate=[&](const Vec&w){
        auto s=model_research::regularized_state(w,lam);
        for(auto i:train){
            double eta=dot(X[i],w);auto j=y[i];
            double a=j?logistic(cuts[j-1]-eta):0,b=j+1<K?logistic(cuts[j]-eta):1;
            s.value-=logprob(eta,j);
            for(std::size_t k=0;k<d;++k)s.gradient[k]+=(1-a-b)*X[i][k];
            add_outer(s.hessian,X[i],a*(1-a)+b*(1-b));
        }
        return s;
    };
    auto f=model_research::damped_newton(d,tol,cap,evaluate);
    Report out{{"Row","Train","Label","Predicted"},{}};
    for(std::size_t k=1;k<=K;++k)out.header.push_back("P"+std::to_string(k));
    out.header.insert(out.header.end(),{"PenalizedNLL","ScoreResidual"});
    for(std::size_t j=1;j<=d;++j)out.header.push_back("Coefficient"+std::to_string(j));
    for(std::size_t i=0;i<x.size();++i){
        double eta=dot(X[i],f.beta);Vec z(K);for(std::size_t k=0;k<K;++k)z[k]=logprob(eta,k);
        auto k=std::max_element(z.begin(),z.end())-z.begin();
        Vec r{double(i+1),x[i][0],x[i][1],double(k+1)};
        for(double v:z)r.push_back(std::exp(v));
        r.push_back(f.state.value);r.push_back(inf_norm(f.state.gradient));
        r.insert(r.end(),f.beta.begin(),f.beta.end());out.rows.push_back(r);
    }
    return out;
}

inline Report ridge_loocv_gcv_path(const Mat& x,const Vec& p){
    auto d=validate(x,2,2,33)-1,n=x.size();require(!p.empty()&&p.size()<=500,"Penalty path required");
    parameters(p,p.size());require(double(p.size())*n*d*d<=5e7,"GCV work limit");
    Mat X,H(d,Vec(d,0));Vec y,b(d,0);
    for(auto&r:x){
        X.emplace_back(r.begin()+1,r.end());y.push_back(r[0]);
        add_outer(H,X.back(),1);for(std::size_t j=0;j<d;++j)b[j]+=r[0]*r[j+1];
    }
    Report out{{"Penalty","LOOCVMSE","GCV","RSS","EffectiveDF","MaxLeverage"},{}};
    for(std::size_t j=1;j<=d;++j)out.header.push_back("Coefficient"+std::to_string(j));
    for(double lam:p){
        require(lam>=0,"Nonnegative ridge required");Mat A=H;
        for(std::size_t j=0;j<d;++j)A[j][j]+=lam;
        Vec w=solve_spd(A,b);Mat inv=invert_spd(A);
        double rss=0,loo=0,df=0,maxh=0;
        for(std::size_t i=0;i<n;++i){
            double h=dot(X[i],mul(inv,X[i])),e=y[i]-dot(X[i],w);
            require(h>=-1e-10&&h<1-1e-12,"Unit leverage prevents LOOCV");
            rss+=e*e;loo+=std::pow(e/(1-h),2);df+=h;maxh=std::max(maxh,h);
        }
        double gcv=rss/n/std::pow(positive(1-df/n),2);
        Vec r{lam,loo/n,gcv,rss,df,maxh};r.insert(r.end(),w.begin(),w.end());out.rows.push_back(r);
    }
    return out;
}

inline Report label_shift_prior_em(const Mat& x,const Vec& p){
    auto K=validate(x,2,2,16);parameters(p,K+2);
    double tol=positive(p[0]);auto cap=count(p[1],1,100000);
    Vec prior(p.begin()+2,p.end()),w=prior;
    for(double v:prior)positive(v);require(std::abs(sum(prior)-1)<=1e-10,"Source prior must sum to one");
    for(auto&r:x){for(double v:r)require(v>=0&&v<=1,"Invalid posterior");require(std::abs(sum(r)-1)<=1e-10,"Posterior must sum to one");}
    require(double(cap)*x.size()*K<=1e8,"Label shift work limit");
    auto evaluate=[&](const Vec&a,Mat* post){
        Vec mean(K,0);double ll=0;if(post)post->clear();
        for(auto&r:x){
            Vec z(K);for(std::size_t j=0;j<K;++j)z[j]=r[j]*a[j]/prior[j];
            double mass=positive(sum(z));ll+=std::log(mass);
            for(std::size_t j=0;j<K;++j){z[j]/=mass;mean[j]+=z[j]/x.size();}
            if(post)post->push_back(z);
        }
        return std::make_pair(ll,mean);
    };
    auto f=evaluate(w,nullptr);double gap=0;
    for(std::size_t it=0;it<cap;++it){
        Vec next=f.second;auto g=evaluate(next,nullptr);
        require(g.first>=f.first-1e-10*std::max(1.,std::abs(f.first)),"EM likelihood decreased");
        w=next;f=g;Vec e=w;for(std::size_t j=0;j<K;++j)e[j]-=f.second[j];gap=inf_norm(e);
        if(gap<=tol)break;
    }
    require(gap<=tol,"Label shift EM did not converge");
    Mat post;auto final=evaluate(w,&post);
    Report out{{"Row"},{}};
    for(auto prefix:{"OriginalP","AdjustedP","TargetPrior"})for(std::size_t j=1;j<=K;++j)out.header.push_back(std::string(prefix)+std::to_string(j));
    out.header.insert(out.header.end(),{"LogLikelihood","FixedPointResidual"});
    for(std::size_t i=0;i<x.size();++i){
        Vec r{double(i+1)};r.insert(r.end(),x[i].begin(),x[i].end());r.insert(r.end(),post[i].begin(),post[i].end());
        r.insert(r.end(),w.begin(),w.end());r.push_back(final.first);r.push_back(gap);out.rows.push_back(r);
    }
    return out;
}

inline Report mixture_regression_em(const Mat& x,const Vec& p){
    auto d=validate(x,2,2,17)-1;parameters(p,5+2*d);
    double sig=positive(p[0]),lam=positive(p[1]),tol=positive(p[2]),pi=p[4],var=sig*sig;
    auto cap=count(p[3],1,100000);require(pi>0&&pi<1&&double(cap)*x.size()*d*d<=5e7,"Invalid mixture controls");
    Vec a(p.begin()+5,p.begin()+5+d),b(p.begin()+5+d,p.end());Mat X;Vec y;
    for(auto&r:x){X.emplace_back(r.begin()+1,r.end());y.push_back(r[0]);}
    auto evaluate=[&](const Vec&a,const Vec&b,double prior,Vec& responsibility){
        require(prior>0&&prior<1,"Mixture weight reached boundary");
        double f=-.5*lam*(squared(a)+squared(b)),c=-std::log(sig)-.5*std::log(2*std::acos(-1.));
        responsibility.resize(y.size());
        for(std::size_t i=0;i<y.size();++i){
            double u=std::log(prior)+c-.5*std::pow(y[i]-dot(X[i],a),2)/var;
            double v=std::log1p(-prior)+c-.5*std::pow(y[i]-dot(X[i],b),2)/var;
            double ld=logsumexp({u,v});f+=ld;responsibility[i]=std::exp(u-ld);
        }
        return f;
    };
    Vec r;double ll=evaluate(a,b,pi,r),gap=0;
    for(std::size_t it=0;it<cap;++it){
        Mat H1(d,Vec(d,0)),H2=H1;Vec z1(d,0),z2(d,0);
        for(std::size_t i=0;i<y.size();++i){
            add_outer(H1,X[i],r[i]);add_outer(H2,X[i],1-r[i]);
            for(std::size_t j=0;j<d;++j){z1[j]+=r[i]*X[i][j]*y[i];z2[j]+=(1-r[i])*X[i][j]*y[i];}
        }
        for(std::size_t j=0;j<d;++j){H1[j][j]+=lam*var;H2[j][j]+=lam*var;}
        pi=sum(r)/r.size();a=solve_spd(H1,z1);b=solve_spd(H2,z2);
        double next=evaluate(a,b,pi,r);
        require(next>=ll-1e-10*std::max(1.,std::abs(ll)),"Mixture likelihood decreased");ll=next;
        Vec g1(d),g2(d);for(std::size_t j=0;j<d;++j){g1[j]=-lam*a[j];g2[j]=-lam*b[j];}
        for(std::size_t i=0;i<y.size();++i){
            double u=r[i]*(y[i]-dot(X[i],a))/var,v=(1-r[i])*(y[i]-dot(X[i],b))/var;
            for(std::size_t j=0;j<d;++j){g1[j]+=u*X[i][j];g2[j]+=v*X[i][j];}
        }
        gap=std::max({inf_norm(g1),inf_norm(g2),std::abs(sum(r)/r.size()-pi)});
        if(gap<=tol)break;
    }
    require(gap<=tol,"Mixture regression EM did not converge");
    Report out{{"Row","Responsibility1","Responsibility2","MixtureMean","Prior1","PenalizedLogLikelihood","ScoreResidual"},{}};
    for(int k=1;k<=2;++k)for(std::size_t j=1;j<=d;++j)out.header.push_back("Beta"+std::to_string(k)+"_"+std::to_string(j));
    for(std::size_t i=0;i<y.size();++i){
        Vec z{double(i+1),r[i],1-r[i],pi*dot(X[i],a)+(1-pi)*dot(X[i],b),pi,ll,gap};
        z.insert(z.end(),a.begin(),a.end());z.insert(z.end(),b.begin(),b.end());out.rows.push_back(z);
    }
    return out;
}

inline Report multitask_group_lasso(const Mat& x,const Vec& p){
    auto cols=validate(x,2,2,64);parameters(p,4);
    auto K=count(p[0],2,16);require(cols>K,"Feature columns required");auto d=cols-K;
    require(d<=32,"Feature dimension limit");
    double lam=positive(p[1]),tol=positive(p[2]);auto cap=count(p[3],1,100000);
    require(double(x.size())*d*d+double(cap)*d*d*K<=1e8,"Group lasso work limit");
    Mat H(d,Vec(d,0)),C(d,Vec(K,0)),B=C;double yy=0;
    for(auto&r:x){
        Vec z(r.begin()+K,r.end());add_outer(H,z,1);
        for(std::size_t k=0;k<K;++k){
            yy+=r[k]*r[k];for(std::size_t j=0;j<d;++j)C[j][k]+=z[j]*r[k];
        }
    }
    double L=matrix_lipschitz(H);
    auto gradient=[&](const Mat&w){
        Mat G(d,Vec(K,0));
        for(std::size_t i=0;i<d;++i)for(std::size_t k=0;k<K;++k){
            G[i][k]=-C[i][k];for(std::size_t j=0;j<d;++j)G[i][k]+=H[i][j]*w[j][k];
        }
        return G;
    };
    double gap=0;Mat G=gradient(B);
    for(std::size_t it=0;it<=cap;++it){
        gap=0;
        for(std::size_t j=0;j<d;++j){
            double n=std::sqrt(squared(B[j]));Vec z=G[j];
            if(n>0){for(std::size_t k=0;k<K;++k)z[k]+=lam*B[j][k]/n;gap=std::max(gap,std::sqrt(squared(z)));}
            else gap=std::max(gap,std::max(0.,std::sqrt(squared(z))-lam));
        }
        if(gap<=tol)break;require(it<cap,"Group lasso did not converge");
        for(std::size_t j=0;j<d;++j){
            Vec z=B[j];for(std::size_t k=0;k<K;++k)z[k]-=G[j][k]/L;
            double n=std::sqrt(squared(z)),s=n>0?std::max(0.,1-lam/(L*n)):0;
            for(std::size_t k=0;k<K;++k)B[j][k]=s*z[k];
        }
        G=gradient(B);
    }
    double objective=0;
    for(auto&r:x)for(std::size_t k=0;k<K;++k){
        double e=r[k];for(std::size_t j=0;j<d;++j)e-=r[K+j]*B[j][k];objective+=.5*e*e;
    }
    for(auto&r:B)objective+=lam*std::sqrt(squared(r));
    Report out{{"Feature","CoefficientNorm","Objective","KKTResidual"},{}};
    for(std::size_t k=1;k<=K;++k)out.header.push_back("Beta"+std::to_string(k));
    for(std::size_t j=0;j<d;++j){
        Vec r{double(j+1),std::sqrt(squared(B[j])),objective,gap};
        r.insert(r.end(),B[j].begin(),B[j].end());out.rows.push_back(r);
    }
    return out;
}
} }
