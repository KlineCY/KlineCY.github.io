#pragma once
#include "model_research_optimization.hpp"
#include "depth_factorconstruction.hpp"
namespace kline { namespace model_research {
inline Report rolling_elastic_portfolio(const Mat& x,const Vec& p){
    auto d=validate(x,8,4,19)-3;parameters(p,7);
    double frac=p[1],l1=p[2],l2=positive(p[3]),cost=p[4],tol=positive(p[5]);
    auto K=count(p[6],1,100000);
    require(frac>0&&frac<=.5&&l1>=0&&cost>=0&&cost<.5,"Invalid tail, penalty or cost");
    auto A=std::size_t(1);while(A<x.size()&&x[A][0]==x[0][0])++A;
    require(A>=2&&A<=32&&x.size()%A==0,"Balanced asset panel required");auto n=x.size()/A;
    auto look=count(p[0],2,n-1);
    require(double(n-look)*(double(look)*A+K)*d*d<=1e8,"Rolling fit work limit");
    for(std::size_t t=0;t<n;++t)for(std::size_t j=0;j<A;++j){
        const auto&r=x[t*A+j];
        require(r[0]==x[t*A][0]&&r[1]==x[j][1]&&r[0]==std::floor(r[0])&&r[1]==std::floor(r[1])&&r[2]>=-1,"Invalid date, asset or return");
        if(j)require(x[j][1]>x[j-1][1],"Increasing asset IDs required");
        if(t)require(r[0]>x[(t-1)*A][0],"Increasing dates required");
    }
    Report o{{"Date","GrossReturn","NetReturn","FeeFraction","Turnover","NAV","Drawdown","GrossExposure","NetExposure","ModelKKT"},{}};
    for(std::size_t j=0;j<A;++j)o.header.push_back("Weight"+std::to_string(j+1));
    for(std::size_t j=0;j<A;++j)o.header.push_back("Score"+std::to_string(j+1));
    Vec old(A,0);double nav=1,peak=1;
    for(std::size_t t=look;t<n;++t){
        Mat train;train.reserve(look*A);
        for(std::size_t k=(t-look)*A;k<t*A;++k){Vec a(x[k].begin()+3,x[k].end());a.push_back(x[k][2]);train.push_back(a);}
        auto fit=fista_elastic(train,{l1,l2,tol,double(K)});Vec beta;
        for(auto&r:fit.rows)beta.push_back(r[1]);
        Mat rank;Vec score,ret,w;
        for(std::size_t j=0;j<A;++j){auto&r=x[t*A+j];Vec a(r.begin()+3,r.end());double s=dot(a,beta);score.push_back(s);rank.push_back({s});ret.push_back(r[2]);}
        auto tails=depth_factorconstruction::tie_aware_tail_weights(rank,{frac});
        for(auto&r:tails.rows)w.push_back(r[4]);
        auto step=researchflow::trade_step(old,w,ret,cost);
        nav=positive(nav*(1+step.net));peak=std::max(peak,nav);
        Vec r{x[t*A][0],step.gross,step.net,step.fee,step.turnover,nav,1-nav/peak,researchflow::l1(w),sum(w),fit.rows[0][3]};
        r.insert(r.end(),w.begin(),w.end());r.insert(r.end(),score.begin(),score.end());
        o.rows.push_back(r);old=std::move(step.drift);
    }
    return o;
}

inline Report rolling_covariance_qp(const Mat& x,const Vec& p){
    auto d=validate(x,3,1,32);parameters(p,8);
    auto look=count(p[0],2,x.size()-1),K=count(p[7],1,100000);
    double shrink=p[1],ridge=positive(p[2]),gamma=positive(p[3]),cap=p[4],cost=p[5],tol=positive(p[6]);
    require(shrink>=0&&shrink<=1&&cap>=1./d&&cap<=1&&cost>=0&&cost<1,"Invalid shrinkage, cap or cost");
    require(double(x.size()-look)*(double(look)*d*d+double(K)*d*(d+80)+d*d*d)<=2e8,"Rolling optimization work limit");
    for(auto&r:x)for(double v:r)require(v>=-1,"Invalid simple return");
    Report o{{"Time","GrossReturn","NetReturn","NAV","FeeFraction","Turnover","ForecastVariance","ProjectedGradient","BudgetGap"},{}};
    for(std::size_t j=0;j<d;++j)o.header.push_back("Weight"+std::to_string(j+1));
    Vec old(d,0);double nav=1;
    for(std::size_t t=look;t<x.size();++t){
        Vec mu(d,0);for(std::size_t s=t-look;s<t;++s)for(std::size_t j=0;j<d;++j)mu[j]+=x[s][j]/look;
        Mat s=researchflow::covariance_window(x,t-look,t,shrink,ridge),a=s;
        for(std::size_t j=0;j<d;++j){a[j].push_back(mu[j]);a[j].push_back(0);a[j].push_back(cap);}
        auto fit=box_covariance_qp(a,{1,gamma,tol,double(K)});Vec w;for(auto&r:fit.rows)w.push_back(r[1]);
        auto step=researchflow::trade_step(old,w,x[t],cost);nav=positive(nav*(1+step.net));
        Vec r{double(t+1),step.gross,step.net,nav,step.fee,step.turnover,dot(w,mul(s,w)),fit.rows[0][5],fit.rows[0][4]};
        r.insert(r.end(),w.begin(),w.end());o.rows.push_back(r);old=std::move(step.drift);
    }
    return o;
}

inline Report risk_budget_newton(const Mat& x,const Vec& p){
    auto d=validate(x,1,2,33)-1;parameters(p,2);require(x.size()==d,"Square covariance required");
    double tol=positive(p[0]);auto K=count(p[1],1,2000);require(double(K)*d*d*d<=5e7,"Risk budget work limit");
    Mat s;Vec b,w(d);
    for(auto&r:x){s.emplace_back(r.begin(),r.begin()+d);b.push_back(positive(r.back()));}
    require(std::abs(sum(b)-1)<1e-10,"Risk budgets must sum to one");
    solve_spd(s,Vec(d,0));
    for(std::size_t j=0;j<d;++j)w[j]=std::sqrt(b[j]/positive(s[j][j]));
    auto objective=[&](const Vec& a){double f=.5*dot(a,mul(s,a));for(std::size_t j=0;j<d;++j)f-=b[j]*std::log(positive(a[j]));return f;};
    auto grad=[&](const Vec&a){Vec g=mul(s,a);for(std::size_t j=0;j<d;++j)g[j]-=b[j]/a[j];return g;};
    double f=objective(w);Vec g=grad(w);std::size_t it=0;
    while(inf_norm(g)>tol&&it<K){
        Mat H=s;for(std::size_t j=0;j<d;++j)H[j][j]+=b[j]/(w[j]*w[j]);
        Vec dir=solve_spd(H,g);double dec=positive(dot(dir,g));bool ok=false;
        for(int k=0;k<50;++k){
            double step=std::ldexp(1.,-k);Vec a=w;bool feasible=true;
            for(std::size_t j=0;j<d;++j){a[j]-=step*dir[j];feasible=feasible&&a[j]>0;}
            if(!feasible)continue;
            double z=objective(a);
            if(z<=f-1e-4*step*dec+1e-15*std::max(1.,std::abs(f))){w=a;f=z;g=grad(w);ok=true;break;}
        }
        require(ok,"Risk budget line search failed");++it;
    }
    double gap=inf_norm(g);require(gap<=tol,"Risk budget did not converge");
    double z=positive(sum(w));for(double&v:w)v/=z;Vec sw=mul(s,w);double var=positive(dot(w,sw));
    Report o{{"Asset","Weight","TargetRiskBudget","ActualRiskBudget","VarianceContribution","PortfolioVariance","ScoreResidual","Iterations"},{}};
    for(std::size_t j=0;j<d;++j)o.rows.push_back({double(j+1),w[j],b[j],w[j]*sw[j]/var,w[j]*sw[j],var,gap,double(it)});
    return o;
}

inline Report synthetic_control_fit(const Mat& x,const Vec& p){
    auto d=validate(x,3,3,34)-2;parameters(p,3);
    double lam=positive(p[0]),tol=positive(p[1]);auto K=count(p[2],1,100000);
    Mat H(d,Vec(d,0));Vec b(d,0);std::size_t pre=0,post=0;
    for(auto&r:x){
        require(r[0]==0||r[0]==1,"Binary post flag");
        if(r[0]){++post;continue;}require(post==0,"Pre period after post period");
        ++pre;Vec a(r.begin()+2,r.end());add_outer(H,a,1);for(std::size_t j=0;j<d;++j)b[j]+=a[j]*r[1];
    }
    require(pre>=2&&post>=1&&double(pre)*d*d<=5e7,"Insufficient periods or work limit");
    for(std::size_t j=0;j<d;++j){H[j][j]+=lam;H[j].push_back(b[j]);H[j].push_back(0);H[j].push_back(1);}
    auto fit=box_covariance_qp(H,{1,1,tol,double(K)});Vec w;for(auto&r:fit.rows)w.push_back(r[1]);
    double err=0;for(std::size_t t=0;t<pre;++t){Vec a(x[t].begin()+2,x[t].end());double e=x[t][1]-dot(w,a);err+=e*e/pre;}
    Report o{{"Time","Post","Treated","Synthetic","Gap","PreRMSE","ProjectedGradient"},{}};
    for(std::size_t j=0;j<d;++j)o.header.push_back("DonorWeight"+std::to_string(j+1));
    for(std::size_t t=0;t<x.size();++t){Vec a(x[t].begin()+2,x[t].end());double y=dot(w,a);Vec r{double(t+1),x[t][0],x[t][1],y,x[t][1]-y,std::sqrt(err),fit.rows[0][5]};r.insert(r.end(),w.begin(),w.end());o.rows.push_back(r);}
    return o;
}

inline Report crossfit_dml_ridge(const Mat& x,const Vec& p){
    auto d=validate(x,4,4,19)-3;parameters(p,1);double lam=positive(p[0]);auto m=d+1;
    std::map<double,std::vector<std::size_t>> folds;Mat z;
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];require(r[0]==std::floor(r[0]),"Integer fold ID required");folds[r[0]].push_back(i);Vec a{1};a.insert(a.end(),r.begin()+3,r.end());z.push_back(a);}
    require(folds.size()>=2&&folds.size()<=100&&double(folds.size())*x.size()*m*m<=5e7,"Invalid folds or work limit");
    Vec md(x.size()),my(x.size()),v(x.size()),u(x.size());
    for(auto&fold:folds){
        Mat H(m,Vec(m,0));Vec bd(m,0),by(m,0);std::size_t n=0;
        for(std::size_t i=0;i<x.size();++i)if(x[i][0]!=fold.first){++n;add_outer(H,z[i],1);for(std::size_t j=0;j<m;++j){bd[j]+=z[i][j]*x[i][1];by[j]+=z[i][j]*x[i][2];}}
        require(n>=2,"Insufficient fold training data");for(std::size_t j=1;j<m;++j)H[j][j]+=lam;
        Vec a=solve_spd(H,bd),b=solve_spd(H,by);
        for(auto i:fold.second){md[i]=dot(z[i],a);my[i]=dot(z[i],b);v[i]=x[i][1]-md[i];u[i]=x[i][2]-my[i];}
    }
    double den=positive(squared(v)),theta=dot(v,u)/den,s=0;Vec score;
    for(std::size_t i=0;i<x.size();++i){double a=(u[i]-theta*v[i])*v[i];score.push_back(a);s+=a*a;}
    double se=std::sqrt(s)/den;
    Report o{{"Row","Fold","Treatment","Outcome","PredictedTreatment","PredictedOutcome","TreatmentResidual","OutcomeResidual","Score","Theta","IIDStandardError"},{}};
    for(std::size_t i=0;i<x.size();++i)o.rows.push_back({double(i+1),x[i][0],x[i][1],x[i][2],md[i],my[i],v[i],u[i],score[i],theta,se});
    return o;
}

inline Report rolling_conformal_ridge(const Mat& x,const Vec& p){
    auto d=validate(x,5,2,17)-1;parameters(p,4);
    auto n=count(p[0],2,x.size()-2),m=count(p[1],1,x.size()-n-1);
    double lam=positive(p[2]),alpha=p[3];require(alpha>0&&alpha<1,"Invalid alpha");
    auto k=std::size_t(std::ceil((m+1)*(1-alpha)));require(k>=1&&k<=m,"Finite conformal threshold unavailable");
    auto h=d+1;require(double(x.size()-n-m)*(double(n)*h*h+h*h*h+double(m)*h)<=1e8,"Rolling conformal work limit");
    Mat z;for(auto&r:x){Vec a{1};a.insert(a.end(),r.begin(),r.end()-1);z.push_back(a);}
    Report o{{"Time","Actual","Prediction","Threshold","Lower","Upper","Covered","TrainStart","TrainEnd","CalibrationStart","CalibrationEnd"},{}};
    for(std::size_t t=n+m;t<x.size();++t){
        Mat H(h,Vec(h,0));Vec b(h,0);
        for(std::size_t i=t-m-n;i<t-m;++i){add_outer(H,z[i],1);for(std::size_t j=0;j<h;++j)b[j]+=z[i][j]*x[i].back();}
        for(std::size_t j=1;j<h;++j)H[j][j]+=lam;
        Vec w=solve_spd(H,b),score;
        for(std::size_t i=t-m;i<t;++i)score.push_back(std::abs(x[i].back()-dot(w,z[i])));
        std::sort(score.begin(),score.end());double q=score[k-1],f=dot(w,z[t]),y=x[t].back(),lo=f-q,hi=f+q;
        double eps=16*std::numeric_limits<double>::epsilon()*std::max({1.,std::abs(y),std::abs(f),q});
        o.rows.push_back({double(t+1),y,f,q,lo,hi,double(y>=lo-eps&&y<=hi+eps),double(t-m-n+1),double(t-m),double(t-m+1),double(t)});
    }
    return o;
}
} }
