#pragma once
#include "comprehensive_common.hpp"
#include "comprehensive_numerics.hpp"
#include "model_research_likelihood.hpp"
namespace kline { namespace comprehensive {
inline Report queue_competing_hazard_fit(const Mat& x,const Vec& p){
    validate(x,2,3); parameters(p,4);
    double H=positive(p[0]),lambda=p[1],tol=positive(p[2]);
    require(lambda>=0,"lambda must be nonnegative");
    int cap=count(p[3],1,10000);
    size_t d=x[0].size()-2;
    Mat design; Vec duration,event;
    for(const auto& r:x){
        duration.push_back(positive(r[0]));
        event.push_back(count(r[1],0,2));
        design.emplace_back(r.begin()+2,r.end());
    }
    Mat beta; Vec hazards, scores;
    for(int k=1;k<=2;++k){
        auto eval=[&](const Vec& b){
            auto s=model_research::regularized_state(b,lambda);
            for(size_t i=0;i<x.size();++i){
                double z=dot(design[i],b),w=duration[i]*std::exp(z),y=event[i]==k;
                require(std::isfinite(w),"overflow in hazard");
                s.value+=w-y*z;
                for(size_t j=0;j<d;++j)s.gradient[j]+=(w-y)*design[i][j];
                for(size_t j=0;j<d;++j)for(size_t l=0;l<d;++l)
                    s.hessian[j][l]+=w*design[i][j]*design[i][l];
            }
            return s;
        };
        auto sol=model_research::damped_newton(d,tol,cap,eval);
        beta.push_back(sol.beta);scores.push_back(inf_norm(sol.state.gradient));
        hazards.push_back(std::exp(dot(design.back(),sol.beta)));
    }
    double total=sum(hazards),survival=std::exp(-H*total);
    Report out;out.header={"Cause","Coefficient","Hazard","EventProbability","Survival","ScoreNorm"};
    for(int k=0;k<2;++k)for(size_t j=0;j<d;++j)
        out.rows.push_back({double(k+1),beta[k][j],hazards[k],hazards[k]/total*(-std::expm1(-H*total)),survival,scores[k]});
    return out;
}

inline Report trade_signing_asof(const Mat& x,const Vec& p){
    validate(x,2,4);parameters(p,3);
    double lag=p[0],H=positive(p[1]),age=positive(p[2]);require(lag>=0,"negative lag");
    Mat quotes;double prev=-std::numeric_limits<double>::infinity();
    for(const auto&r:x){
        require(r.size()==4&&r[1]>=prev,"invalid time order");prev=r[1];
        int type=count(r[0],0,1);
        if(type==0){require(r[2]>0&&r[3]>=r[2],"invalid quote");quotes.push_back({r[1],(r[2]+r[3])/2});}
        else{positive(r[2]);positive(r[3]);}
    }
    require(!quotes.empty(),"no quotes");
    auto asof=[&](double t){
        auto it=std::upper_bound(quotes.begin(),quotes.end(),t,[](double v,const Vec&r){return v<r[0];});
        if(it==quotes.begin())return Vec{};
        --it;return t-(*it)[0]<=age?*it:Vec{};
    };
    Report out;out.header={"Time","Price","Quantity","Sign","Mid","EffectiveSpread","FutureMid","Markout","Valid"};
    double last=0,tick=0;
    for(const auto&r:x)if(r[0]==1){
        if(last>0&&r[2]!=last)tick=r[2]>last?1:-1;
        last=r[2];Vec now=asof(r[1]-lag),future=asof(r[1]+H);
        double sign=0,mid=0,fm=0;
        if(!now.empty()){mid=now[1];sign=r[2]>mid?1:r[2]<mid?-1:tick;}
        bool valid=sign!=0&&!future.empty()&&r[1]+H<=x.back()[1];
        if(valid)fm=future[1];
        out.rows.push_back({r[1],r[2],r[3],sign,mid,2*sign*(r[2]-mid),fm,valid?sign*(fm-r[2]):0,double(valid)});
    }
    return out;
}

inline Report price_time_matching_replay(const Mat& x,const Vec& p){
    validate(x,1,5);parameters(p,0);require(x.size()<=5000,"replay too large");
    struct Order{int id,side;double price,qty;};
    std::vector<Order> book;std::set<int> ids;
    Report out;out.header={"Event","ID","Filled","Notional","Rested","Cancelled","BestBid","BidDepth","BestAsk","AskDepth","ConservationError"};
    for(size_t t=0;t<x.size();++t){
        const auto&r=x[t];require(r.size()==5,"invalid event width");
        int type=count(r[0],1,3),id=count(r[1],1,1000000000),side=int(r[2]);
        require(r[2]==1||r[2]==-1,"invalid side");
        double qty=positive(r[4]),remain=qty,filled=0,notional=0,rest=0,cancel=0;
        if(type==2){
            auto it=std::find_if(book.begin(),book.end(),[&](const Order&o){return o.id==id&&o.qty>0;});
            require(it!=book.end()&&it->side==side&&qty<=it->qty,"invalid cancellation");
            it->qty-=qty;cancel=qty;remain=0;
        }else{
            require(ids.insert(id).second,"duplicate order ID");
            if(type==1)positive(r[3]);
            while(remain>0){
                size_t best=book.size();
                for(size_t j=0;j<book.size();++j){
                    const auto&o=book[j];if(o.qty<=0||o.side==side)continue;
                    if(type==1&&side*(o.price-r[3])>0)continue;
                    if(best==book.size()||side*o.price<side*book[best].price)best=j;
                }
                if(best==book.size())break;
                auto&o=book[best];double q=std::min(remain,o.qty);
                filled+=q;notional+=q*o.price;remain-=q;o.qty-=q;
            }
            if(type==1&&remain>0){book.push_back({id,side,r[3],remain});rest=remain;}
            else cancel=remain;
        }
        double bid=0,ask=0,bd=0,ad=0;
        for(const auto&o:book)if(o.qty>0){
            if(o.side==1){if(o.price>bid){bid=o.price;bd=0;}if(o.price==bid)bd+=o.qty;}
            else{if(ask==0||o.price<ask){ask=o.price;ad=0;}if(o.price==ask)ad+=o.qty;}
        }
        require(bid==0||ask==0||bid<ask,"crossed resting book");
        out.rows.push_back({double(t+1),double(id),filled,notional,rest,cancel,bid,bd,ask,ad,qty-filled-rest-cancel});
    }
    return out;
}

inline Report market_maker_cara_dp(const Mat& x,const Vec& p){
    validate(x,1,4);parameters(p,7);
    int H=count(p[0],1,500),Q=count(p[1],1,100),G=count(p[6],2,100);
    double gamma=positive(p[2]),dt=positive(p[3]),lo=p[4],hi=p[5];
    require(lo>=0&&hi>=lo,"invalid quote grid");
    require(double(H)*(2*Q+1)*(G+1)*(G+1)*x.size()<=5e7,"DP too large");
    Vec actions{-1};for(int j=0;j<G;++j)actions.push_back(lo+(hi-lo)*j/(G-1));
    Report out;out.header={"Scenario","Remaining","Inventory","BidDistance","AskDistance","CertaintyEquivalent","BuyProbability","SellProbability"};
    for(size_t s=0;s<x.size();++s){
        const auto&r=x[s];require(r.size()==4&&r[0]>=0&&r[1]>0&&r[2]>=0&&r[3]>=0,"invalid market parameters");
        double A=r[0],k=r[1],sigma=r[2],phi=r[3];Vec value(2*Q+1);
        for(int q=-Q;q<=Q;++q)value[q+Q]=gamma*phi*q*q;
        for(int h=1;h<=H;++h){
            Vec next(value.size());
            for(int q=-Q;q<=Q;++q){
                double best=std::numeric_limits<double>::infinity(),bb=-1,ba=-1,bpb=0,bpa=0;
                for(double db:actions)for(double da:actions){
                    if((q==Q&&db>=0)||(q==-Q&&da>=0))continue;
                    double pb=db<0?0:A*dt*std::exp(-k*db),pa=da<0?0:A*dt*std::exp(-k*da);
                    if(pb+pa>1)continue;
                    Vec terms;
                    auto term=[&](int e,double prob,double dist){
                        if(prob>0){double inv=q+e;terms.push_back(std::log(prob)-gamma*dist+.5*gamma*gamma*sigma*sigma*dt*inv*inv+value[q+e+Q]);}
                    };
                    term(0,1-pb-pa,0);term(1,pb,db);term(-1,pa,da);
                    double v=logsumexp(terms);
                    if(v<best-1e-13){best=v;bb=db;ba=da;bpb=pb;bpa=pa;}
                }
                next[q+Q]=best;
                out.rows.push_back({double(s+1),double(h),double(q),bb,ba,-best/gamma,bpb,bpa});
            }
            value=next;
        }
    }
    return out;
}

inline Report transient_impact_schedule(const Mat& x,const Vec& p){
    validate(x,2,3);parameters(p,4);
    double Q=positive(p[0]),eta=p[1],k=positive(p[2]),gamma=p[3];
    require(eta>=0&&gamma>=0,"negative penalty");size_t n=x.size();
    require(n<=12,"active-set dimension limit");Vec prefix(n+1);
    for(size_t i=0;i<n;++i){require(x[i].size()==3&&x[i][2]>=0,"invalid schedule");positive(x[i][1]);
        if(i)require(x[i][0]>x[i-1][0],"times must increase");prefix[i+1]=prefix[i]+x[i][2];}
    Mat A(n,Vec(n+1));
    for(size_t i=0;i<n;++i)for(size_t j=0;j<n;++j)
        A[i][j]=(i==j?x[i][1]:0)+eta*std::exp(-k*std::abs(x[i][0]-x[j][0]))+gamma*prefix[std::min(i,j)];
    auto sol=simplex_active_set(A,{});
    Vec q(n);for(size_t i=0;i<n;++i)q[i]=Q*sol.rows[i][1];
    Report out;out.header={"Time","Quantity","Remaining","TemporaryCost","TransientCost","RiskCost","TotalObjective"};
    double rem=Q,total=0;
    for(size_t i=0;i<n;++i){
        rem-=q[i];double c=.5*x[i][1]*q[i]*q[i],z=0;
        for(size_t j=0;j<n;++j)z+=.5*q[i]*eta*std::exp(-k*std::abs(x[i][0]-x[j][0]))*q[j];
        double risk=.5*gamma*x[i][2]*rem*rem;total+=c+z+risk;
        out.rows.push_back({x[i][0],q[i],rem,c,z,risk,0});
    }
    for(auto&r:out.rows)r.back()=total;
    return out;
}

inline Report hawkes_network_em(const Mat& x,const Vec& p){
    validate(x,1,2);parameters(p,9);
    int K=count(p[0],1,20),cap=count(p[8],1,100000);
    double T=positive(p[1]),beta=positive(p[2]),u=positive(p[5]),v=positive(p[6]),tol=positive(p[7]);
    Vec mu(K,positive(p[3])),exposure(K);Mat A(K,Vec(K,positive(p[4]))),hist;std::vector<int> type;
    require(double(x.size())*K*cap<=1e8,"EM too large");double last=0;Vec state(K);
    for(size_t m=0;m<x.size();++m){
        const auto&r=x[m];require(r.size()==2&&r[0]>last&&r[0]<=T,"invalid event time");
        int c=count(r[1],1,K)-1;
        for(double&z:state)z*=std::exp(-beta*(r[0]-last));
        hist.push_back(state);type.push_back(c);state[c]+=beta;last=r[0];
        exposure[c]+=-std::expm1(-beta*(T-r[0]));
    }
    auto estep=[&](Vec& n0,Mat& nij){
        n0.assign(K,0);nij.assign(K,Vec(K));double ll=0;
        for(size_t m=0;m<hist.size();++m){
            int i=type[m];double intensity=mu[i]+dot(A[i],hist[m]);
            ll+=std::log(intensity);n0[i]+=mu[i]/intensity;
            for(int j=0;j<K;++j)nij[i][j]+=A[i][j]*hist[m][j]/intensity;
        }
        for(int i=0;i<K;++i){
            ll+=u*std::log(mu[i])-(T+v)*mu[i];
            for(int j=0;j<K;++j)ll+=u*std::log(A[i][j])-(exposure[j]+v)*A[i][j];
        }
        return ll;
    };
    Vec n0;Mat nij;double ll=estep(n0,nij),score=0;
    for(int iter=0;iter<cap;++iter){
        for(int i=0;i<K;++i){mu[i]=(n0[i]+u)/(T+v);for(int j=0;j<K;++j)A[i][j]=(nij[i][j]+u)/(exposure[j]+v);}
        double next=estep(n0,nij);require(next+1e-9>=ll,"EM objective decreased");ll=next;score=0;
        for(int i=0;i<K;++i){score=std::max(score,std::abs((n0[i]+u)/mu[i]-T-v));
            for(int j=0;j<K;++j)score=std::max(score,std::abs((nij[i][j]+u)/A[i][j]-exposure[j]-v));}
        if(score<=tol)break;
    }
    Report out;out.header={"Target","Source","Baseline","Excitation","LogPosterior","ScoreNorm","RowSum"};
    for(int i=0;i<K;++i)for(int j=0;j<K;++j)out.rows.push_back({double(i+1),double(j+1),mu[i],A[i][j],ll,score,sum(A[i])});
    return out;
}

inline Report spread_resiliency_profile(const Mat& x,const Vec& p){
    validate(x,3,3);parameters(p,3);
    double lo=positive(p[0]),hi=p[1];int G=count(p[2],2,10000);require(hi>=lo,"invalid grid");
    for(const auto&r:x){require(r.size()==3&&r[0]>=0,"invalid elapsed time");positive(r[2]);}
    Vec best;double obj=std::numeric_limits<double>::infinity();
    for(int j=0;j<G;++j){
        double k=lo+(hi-lo)*j/(G-1);Mat A(2,Vec(2));Vec rhs(2);
        for(const auto&r:x){Vec z{1,std::exp(-k*r[0])};for(int a=0;a<2;++a){rhs[a]+=r[2]*z[a]*r[1];for(int b=0;b<2;++b)A[a][b]+=r[2]*z[a]*z[b];}}
        if(A[0][0]*A[1][1]-A[0][1]*A[0][1]<=1e-14*A[0][0]*A[1][1])continue;
        Vec theta=solve_spd(A,rhs);double sse=0;
        for(const auto&r:x){double e=r[1]-theta[0]-theta[1]*std::exp(-k*r[0]);sse+=r[2]*e*e;}
        if(sse<obj){obj=sse;best={k,theta[0],theta[1],std::log(2)/k,sse};}
    }
    require(!best.empty(),"unidentified profile");
    return {{"Decay","Baseline","Amplitude","HalfLife","WeightedSSE"},{best}};
}

inline Report two_scale_realized_variance(const Mat& x,const Vec& p){
    validate(x,3,1);require(!p.empty(),"stride required");
    size_t n=x.size()-1;double fast=0;
    for(size_t i=1;i<x.size();++i){require(x[i].size()==1,"one log price column");double r=x[i][0]-x[i-1][0];fast+=r*r;}
    Report out;out.header={"Stride","FastRV","AverageSlowRV","AverageCount","NoiseRatio","AdjustedRV"};
    for(double a:p){
        int K=count(a,2,int(n));double slow=0;
        for(size_t i=K;i<x.size();++i){double r=x[i][0]-x[i-K][0];slow+=r*r/K;}
        double m=double(n-K+1)/K,ratio=m/n;
        out.rows.push_back({double(K),fast,slow,m,ratio,(slow-ratio*fast)/(1-ratio)});
    }
    return out;
}
} }
