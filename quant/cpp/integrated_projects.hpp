#pragma once
#include "comprehensive_common.hpp"
#include "comprehensive_dynamics.hpp"
#include "comprehensive_pricing.hpp"
#include "comprehensive_microstructure.hpp"
#include "model_research_optimization.hpp"
namespace kline { namespace integrated {
using namespace comprehensive;

inline Report nested_panel_risk_strategy(const Mat& x,const Vec& p){
    auto d=validate(x,8,4,19)-3;
    require(p.size()>=8,"Training, validation, risk, shrinkage, floor, cost, turnover and penalty path required");
    parameters(p,p.size());
    auto L=count(p[0],2,1000),V=count(p[1],1,100);
    double gamma=positive(p[2]),shrink=p[3],floor=positive(p[4]),cost=p[5],turn=p[6];
    require(shrink>=0&&shrink<=1&&cost>=0&&cost<1&&turn>=0,"Invalid risk or cost controls");
    Vec penalties(p.begin()+7,p.end());
    require(penalties.size()<=20,"Penalty path limit");
    for(double lam:penalties)positive(lam);
    std::vector<Mat> days;Vec dates;
    for(auto&r:x){
        require(r[2]>=-1,"Return below -100 percent");
        if(dates.empty()||r[0]!=dates.back()){
            require(dates.empty()||r[0]>dates.back(),"Increasing dates required");
            dates.push_back(r[0]);days.emplace_back();
        }
        days.back().push_back(r);
    }
    auto n=days.front().size(),T=days.size();
    require(n>=2&&n<=32&&T>L+V&&double(T)*V*L*n*d*d*penalties.size()<=5e7,"Invalid panel size or estimation work limit");
    std::set<double> ids;
    for(auto&r:days.front())require(ids.insert(r[1]).second,"Duplicate asset");
    std::vector<Mat> X;Mat Y,R;
    for(auto&day:days){
        require(day.size()==n,"Incomplete panel");
        Vec avg(d,0),ret(n);double mean=0;
        for(std::size_t i=0;i<n;++i){
            require(day[i][1]==days.front()[i][1],"Asset order mismatch");
            ret[i]=day[i][2];mean+=ret[i]/n;
            for(std::size_t j=0;j<d;++j)avg[j]+=day[i][j+3]/n;
        }
        Mat z(n,Vec(d));Vec y=ret;
        for(std::size_t i=0;i<n;++i){
            y[i]-=mean;
            for(std::size_t j=0;j<d;++j)z[i][j]=day[i][j+3]-avg[j];
        }
        X.push_back(z);Y.push_back(y);R.push_back(ret);
    }
    auto fit=[&](std::size_t end,double lam){
        Mat design;Vec target;
        for(std::size_t t=end-L;t<end;++t)for(std::size_t i=0;i<n;++i){
            design.push_back(X[t][i]);target.push_back(Y[t][i]);
        }
        return fit_ridge(design,target,lam,false);
    };
    Vec old(n,0);double nav=1,peak=1;
    Report out{{"Date","Asset","SelectedPenalty","ValidationMSE","Score","Weight","PretradeWeight","RealizedReturn","Contribution","PredictedVariance","Objective","KKTResidual","GrossReturn","NetReturn","Turnover","Fee","NAV","Drawdown","GrossExposure","NetExposure"},{}};
    for(std::size_t j=1;j<=d;++j)out.header.push_back("Coefficient"+std::to_string(j));
    for(std::size_t t=L+V;t<T;++t){
        double best=std::numeric_limits<double>::infinity(),lam=penalties.front();
        for(double candidate:penalties){
            double mse=0;
            for(std::size_t v=t-V;v<t;++v){
                Vec b=fit(v,candidate),prediction=mul(X[v],b);
                for(std::size_t i=0;i<n;++i)mse+=std::pow(Y[v][i]-prediction[i],2)/(V*n);
            }
            if(mse<best){best=mse;lam=candidate;}
        }
        Vec beta=fit(t,lam),score=mul(X[t],beta);
        Mat S=covariance_window(R,t-L,t,shrink,floor),H=S,problem;
        for(std::size_t i=0;i<n;++i){
            H[i][i]+=turn/gamma;
            Vec row=H[i];row.insert(row.end(),{score[i]+turn*old[i],-1./n,1./n});problem.push_back(row);
        }
        auto opt=model_research::box_covariance_qp(problem,{0,gamma,1e-9,10000});
        Vec w;for(auto&r:opt.rows)w.push_back(r[1]);
        double risk=dot(w,mul(S,w)),obj=.5*gamma*risk-dot(score,w);
        for(std::size_t i=0;i<n;++i)obj+=.5*turn*std::pow(w[i]-old[i],2);
        auto step=trade_step(old,w,R[t],cost);nav=positive(nav*(1+step.net));peak=std::max(peak,nav);
        for(std::size_t i=0;i<n;++i){
            Vec row{dates[t],days[t][i][1],lam,best,score[i],w[i],old[i],R[t][i],w[i]*R[t][i],risk,obj,opt.rows[i][5],step.gross,step.net,step.turnover,step.fee,nav,1-nav/peak,l1(w),sum(w)};
            row.insert(row.end(),beta.begin(),beta.end());out.rows.push_back(row);
        }
        old=step.drift;
    }
    return out;
}

inline Report repaired_option_strip(const Mat& x,const Vec& p){
    validate(x,3,3,3);parameters(p,4);
    double F=positive(p[0]),T=positive(p[1]),tol=positive(p[2]);
    auto cap=count(p[3],1,100000);
    auto repaired=call_curve_convex_projection(x,{F,tol,double(cap)});
    Mat strip;Vec f;double cumulative=0;
    Report out{{"Strike","RawCall","Weight","RepairedCall","ReconstructedPut","OTMIntegrand","IntervalContribution","CumulativeVariance","ProjectionObjective","DualGap","MaxViolation"},{}};
    for(std::size_t i=0;i<x.size();++i){
        double K=x[i][0],C=std::max(0.,repaired.rows[i][2]),P=std::max(0.,C-F+K);
        strip.push_back({K,C,P});f.push_back((K<F?P:K>F?C:(C+P)/2)/(K*K));
        double contribution=i?(K-x[i-1][0])*(f[i]+f[i-1])/T:0;
        cumulative+=contribution;
        out.rows.push_back({K,x[i][1],x[i][2],C,P,f[i],contribution,cumulative,repaired.rows[i][6],repaired.rows[i][7],repaired.rows[i][5]});
    }
    auto reference=variance_strip_quadrature(strip,{F,T,std::max(1e-8,10*tol)});
    require(std::abs(reference.rows.back()[5]-cumulative)<=1e-10*std::max(1.,cumulative),"Variance strip reconciliation");
    return out;
}

inline Report factor_filter_market_neutral(const Mat& x,const Vec& p){
    validate(x,2,7,7);parameters(p,11);
    double threshold=p[8],gross=positive(p[9]),cost=p[10];
    require(threshold>=0&&gross<=1&&cost>=0&&cost<1,"Invalid filter strategy controls");
    Mat observations;
    for(auto&r:x){require(r[5]>=-1&&r[6]>=-1,"Return below -100 percent");observations.emplace_back(r.begin(),r.begin()+5);}
    auto filtered=dynamic_factor_sequential(observations,Vec(p.begin(),p.begin()+8));
    Vec old(2,0);double nav=1,peak=1;
    Report out{{"Period","LaggedState","LaggedVariance","Weight1","Weight2","Return1","Return2","GrossReturn","NetReturn","Turnover","Fee","NAV","Drawdown","ObservedCount"},{}};
    for(std::size_t t=1;t<x.size();++t){
        double m=filtered.rows[t-1][4],v=filtered.rows[t-1][5],z=v>0?m/std::sqrt(v):0;
        double a=z>threshold?gross/2:z< -threshold?-gross/2:0;
        Vec w{a,-a},ret{x[t][5],x[t][6]};
        auto step=trade_step(old,w,ret,cost);nav=positive(nav*(1+step.net));peak=std::max(peak,nav);
        out.rows.push_back({double(t+1),m,v,w[0],w[1],ret[0],ret[1],step.gross,step.net,step.turnover,step.fee,nav,1-nav/peak,filtered.rows[t-1][6]});
        old=step.drift;
    }
    return out;
}

inline Report order_replay_cost_report(const Mat& x,const Vec& p){
    validate(x,2,5,5);parameters(p,2);
    double horizon=positive(p[0]),age=positive(p[1]);
    auto replay=price_time_matching_replay(x,{});
    Mat events,filled;
    for(std::size_t i=0;i<x.size();++i){
        auto&r=replay.rows[i];double t=i+1;
        if(r[2]>0){events.push_back({1,t,r[3]/r[2],r[2]});filled.push_back({t,x[i][2],r[2],r[3]});}
        if(r[6]>0&&r[8]>0)events.push_back({0,t,r[6],r[8]});
    }
    require(!filled.empty(),"At least one fill required");
    auto signed_trades=trade_signing_asof(events,{0,horizon,age});
    require(signed_trades.rows.size()==filled.size(),"Trade count mismatch");
    Report out{{"Event","TrueSide","EstimatedSide","Quantity","VWAP","VisibleMid","EffectiveSpread","Markout","MarkoutValid","Notional","NotionalResidual"},{}};
    for(std::size_t i=0;i<filled.size();++i){
        auto&r=signed_trades.rows[i];auto&a=filled[i];
        out.rows.push_back({a[0],a[1],r[3],a[2],r[1],r[4],r[5],r[7],r[8],a[3],r[1]*r[2]-a[3]});
    }
    return out;
}
} }
