#pragma once
#include "comprehensive_common.hpp"
namespace kline { namespace comprehensive {
inline Report bitemporal_vintage_join(const Mat& x,const Vec& p){
    validate(x,1,5,5);parameters(p,1);double age=p[0];require(age>=0,"negative age");
    std::map<int,Mat> history;std::set<std::tuple<int,double,double>> keys;
    for(const auto&r:x)if(count(r[0],0,1)==0){
        int id=count(r[1],1,1000000000);require(r[3]>=r[2],"publication before effective time");
        require(keys.insert({id,r[2],r[3]}).second,"duplicate vintage");
        history[id].push_back({r[2],r[3],r[4]});
    }
    require(double(x.size())*x.size()<=1e8,"join too large");
    Report out;out.header={"Entity","DecisionTime","TargetTime","EffectiveTime","AvailableTime","Value","Valid"};
    for(const auto&r:x)if(r[0]==1){
        int id=count(r[1],1,1000000000);require(r[2]<=r[3],"future target");
        Vec best;for(const auto&v:history[id])if(v[0]<=r[2]&&v[1]<=r[3]&&(best.empty()||std::tie(v[0],v[1])>std::tie(best[0],best[1])))best=v;
        bool valid=!best.empty()&&r[2]-best[0]<=age;
        out.rows.push_back({double(id),r[3],r[2],valid?best[0]:0,valid?best[1]:0,valid?best[2]:0,double(valid)});
    }
    require(!out.rows.empty(),"no queries");return out;
}

inline Report watermark_window_ledger(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,2);double h=positive(p[0]),lag=p[1];require(lag>=0,"negative lateness");
    double maxTime=-std::numeric_limits<double>::infinity(),arrival=-std::numeric_limits<double>::infinity();
    std::map<double,Vec> bins;
    for(const auto&r:x){
        require(r[1]>=arrival,"arrival order must increase");arrival=r[1];double start=std::floor(r[0]/h)*h;
        if(bins[start].empty())bins[start]=Vec(4);
        auto&b=bins[start];bool late=start+h<=maxTime-lag;
        b[late?2:0]++;b[late?3:1]+=r[2];maxTime=std::max(maxTime,r[0]);
    }
    Report out;out.header={"WindowStart","WindowEnd","AcceptedCount","Sum","LateCount","LateSum"};
    for(const auto&kv:bins)out.rows.push_back({kv.first,kv.first+h,kv.second[0],kv.second[1],kv.second[2],kv.second[3]});
    return out;
}

inline Report sequence_gap_replay(const Mat& x,const Vec& p){
    validate(x,1,3,3);parameters(p,1);int initial=count(p[0],0,1000000000);
    struct State{long long next=0;double sum=0;std::map<int,double> seen,pending;};
    std::map<int,State> states;Report out;out.header={"Partition","Sequence","Status","ContiguousThrough","Buffered","CommittedCount","CommittedSum"};
    for(const auto&r:x){
        int id=count(r[0],1,1000000000),seq=count(r[1],initial,1000000000);auto it=states.try_emplace(id);
        auto&s=it.first->second;if(it.second)s.next=initial;
        int status=0;auto old=s.seen.find(seq);
        if(old!=s.seen.end())require(old->second==r[2],"conflicting duplicate");
        else{
            s.seen[seq]=r[2];s.pending[seq]=r[2];status=seq==s.next?1:2;
            while(s.next<=1000000000){
                auto hit=s.pending.find(int(s.next));if(hit==s.pending.end())break;
                s.sum+=hit->second;s.pending.erase(hit);++s.next;
            }
        }
        out.rows.push_back({double(id),double(seq),double(status),double(s.next-1),double(s.pending.size()),double(s.next-initial),s.sum});
    }
    return out;
}

inline Report label_window_availability(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,2);double H=positive(p[0]),delay=p[1];require(delay>=0,"negative delay");
    std::map<std::pair<int,double>,double> prices;
    for(const auto&r:x)if(count(r[0],0,1)==0){
        int id=count(r[1],1,1000000000);require(prices.emplace(std::make_pair(id,r[2]),positive(r[3])).second,"duplicate price");
    }
    Report out;out.header={"Asset","Start","End","Label","LabelAvailableTime","TrainingEligible","Valid"};
    for(const auto&r:x)if(r[0]==1){
        int id=count(r[1],1,1000000000);double end=r[2]+H,avail=end+delay;
        auto a=prices.find({id,r[2]}),b=prices.find({id,end});bool ok=a!=prices.end()&&b!=prices.end();
        out.rows.push_back({double(id),r[2],end,ok?b->second/a->second-1:0,avail,double(ok&&avail<=r[3]),double(ok)});
    }
    require(!out.rows.empty(),"no labels");return out;
}

inline Report overlapping_label_uniqueness(const Mat& x,const Vec& p){
    validate(x,1,2,2);parameters(p,0);std::vector<std::pair<int,int>> ranges;int hi=0;
    for(const auto&r:x){int a=count(r[0],0,1000000),b=count(r[1],a,1000000);ranges.emplace_back(a,b);hi=std::max(hi,b);}
    Vec diff(hi+2),prefix(hi+2);
    for(auto z:ranges){diff[z.first]++;diff[z.second+1]--;}
    double c=0;for(int t=0;t<=hi;++t){c+=diff[t];prefix[t+1]=prefix[t]+(c>0?1/c:0);}
    Vec u;for(auto z:ranges)u.push_back((prefix[z.second+1]-prefix[z.first])/(z.second-z.first+1));
    double mass=positive(sum(u));Report out;out.header={"Event","Start","End","Length","AverageUniqueness","NormalizedWeight"};
    for(size_t i=0;i<ranges.size();++i){auto z=ranges[i];out.rows.push_back({double(i+1),double(z.first),double(z.second),double(z.second-z.first+1),u[i],x.size()*u[i]/mass});}
    return out;
}

inline Report train_only_winsor_standardize(const Mat& x,const Vec& p){
    size_t d=validate(x,2,2,33)-1;parameters(p,3);double lo=p[0],hi=p[1],floor=positive(p[2]);require(lo>=0&&hi<=1&&lo<=hi,"invalid quantiles");
    Mat train(d);for(const auto&r:x)if(count(r[0],0,1))for(size_t j=0;j<d;++j)train[j].push_back(r[j+1]);
    require(!train[0].empty(),"empty training set");Vec a(d),b(d),m(d),s(d);
    for(size_t j=0;j<d;++j){
        auto&v=train[j];std::sort(v.begin(),v.end());auto quant=[&](double q){double pos=(v.size()-1)*q;size_t k=size_t(pos);return v[k]+(pos-k)*(v[std::min(k+1,v.size()-1)]-v[k]);};
        a[j]=quant(lo);b[j]=quant(hi);for(double&z:v)z=std::clamp(z,a[j],b[j]);m[j]=sum(v)/v.size();
        for(double z:v)s[j]+=std::pow(z-m[j],2)/v.size();s[j]=std::sqrt(s[j]);
    }
    Report out;out.header={"Row","Train","Feature","Original","Lower","Upper","Clipped","TrainingMean","TrainingSD","Standardized"};
    for(size_t i=0;i<x.size();++i)for(size_t j=0;j<d;++j){
        double z=std::clamp(x[i][j+1],a[j],b[j]);out.rows.push_back({double(i+1),x[i][0],double(j+1),x[i][j+1],a[j],b[j],z,m[j],s[j],s[j]<=floor?0:(z-m[j])/s[j]});
    }
    return out;
}

inline Report corporate_action_return_ledger(const Mat& x,const Vec& p){
    validate(x,1,4,4);parameters(p,0);double index=100,prev=0,time=-std::numeric_limits<double>::infinity();
    Report out;out.header={"Time","Close","Split","Dividend","GrossReturn","TotalReturnIndex"};
    for(size_t i=0;i<x.size();++i){
        const auto&r=x[i];require(r[0]>time&&r[3]>=0,"invalid action timing or dividend");time=r[0];double price=positive(r[1]),split=positive(r[2]);
        double gross=i?(split*price+r[3])/prev:1;index*=gross;prev=price;
        out.rows.push_back({r[0],price,split,r[3],gross,index});
    }
    return out;
}

inline Report multi_currency_cash_reconciliation(const Mat& x,const Vec& p){
    validate(x,1,5,5);parameters(p,0);double prev=0,idx=100,last=-std::numeric_limits<double>::infinity();
    Report out;out.header={"Time","AssetBase","CashBase","WealthBase","ExternalFlow","InvestmentPnL","FlowAdjustedReturn","CumulativeIndex"};
    for(size_t i=0;i<x.size();++i){
        const auto&r=x[i];require(r[0]>last,"time must increase");last=r[0];double fx=positive(r[3]),a=r[1]*fx,c=r[2]*fx,V=positive(a+c),f=r[4];
        if(i==0)require(f==0,"initial flow must be zero");
        double pnl=i?V-prev-f:0,ret=i?pnl/prev:0;positive(1+ret);idx*=1+ret;
        out.rows.push_back({r[0],a,c,V,f,pnl,ret,idx});prev=V;
    }
    return out;
}
} }
