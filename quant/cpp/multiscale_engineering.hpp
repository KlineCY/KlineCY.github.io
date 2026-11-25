#pragma once
#include "multiscale_common.hpp"
namespace msq {
inline Vec membership_vintage(const Vec& x){
    require(x.size()>=8);int n=integer(x[0],1,1000),q=integer(x[1],1,1000),m=integer(x[2],1,100000);
    require(x.size()==size_t(3+q+4*m)&&n*q<=100000);
    struct Row{double e,a;int flag;};std::vector<std::vector<Row>> r(n);std::set<std::tuple<int,double,double>> seen;
    for(int j=0;j<m;++j){size_t i=3+q+4*j;int a=integer(x[i],0,n-1),f=integer(x[i+3],0,1);
        require(seen.insert({a,x[i+1],x[i+2]}).second);r[a].push_back({x[i+1],x[i+2],f});}
    Vec out;
    for(int t=0;t<q;++t){double now=x[3+t];require(t==0||now>x[2+t]);Vec v(n);int c=0;
        for(int i=0;i<n;++i){bool found=false;double e=0,a=0;for(auto z:r[i])if(z.e<=now&&z.a<=now&&(!found||std::tie(z.e,z.a)>std::tie(e,a))){found=true;e=z.e;a=z.a;v[i]=z.flag;}c+=int(v[i]);}
        out.push_back(c);out.insert(out.end(),v.begin(),v.end());}
    return out;
}
inline Vec delist_wealth(const Vec& x){
    require(x.size()>=7);int n=integer(x[0],1,1000),t=integer(x[1],1,1000);double cash=x[2];require(cash>=0&&x.size()==size_t(3+2*n+3*n*t));
    Vec qty(n),p(n);std::vector<bool> live(n,true);double init=cash,div=0,delist=0;Vec out{0,0,0,0,0,0};
    for(int i=0;i<n;++i){qty[i]=x[3+2*i];p[i]=x[4+2*i];require(qty[i]>=0);positive(p[i]);init+=qty[i]*p[i];}
    positive(init);size_t at=3+2*n;
    for(int j=0;j<t;++j){double stale=0,active=0;
        for(int i=0;i<n;++i){int type=integer(x[at++],0,2);double price=x[at++],d=x[at++];require(d>=0);
            if(!live[i]){require(type==0&&price==0&&d==0);continue;}
            if(type==0){require(price==0&&d==0);++stale;}
            else{double v=qty[i]*d;cash+=v;div+=v;
                if(type==1){positive(price);p[i]=price;}
                else{require(price>=0);double pay=qty[i]*price;cash+=pay;delist+=pay;qty[i]=0;p[i]=0;live[i]=false;}}
            if(live[i])++active;
        }
        double wealth=cash+dot(qty,p);out.insert(out.end(),{wealth,cash,active,stale});}
    double wealth=cash+dot(qty,p);out[0]=wealth;out[1]=cash;out[2]=wealth-init;out[3]=div;out[4]=delist;out[5]=wealth/init-1;return out;
}
inline Vec delayed_orders(const Vec& x){
    require(x.size()>=12);int n=integer(x[0],1,1000),m=integer(x[1],1,10000);require(x.size()==size_t(2+5*n+5*m)&&n*m<=1000000);
    struct Order{double arrival,end,q,limit,filled=0,notional=0,first=-1,last=-1;int side;};std::vector<Order> o;
    for(int i=0;i<n;++i){size_t j=2+5*i;double signal=x[j],delay=x[j+1],end=x[j+2],q=x[j+3],limit=x[j+4];require(signal>=0&&delay>=0&&end>=signal+delay&&q!=0&&limit>0);
        double arrival=signal+delay;require(i==0||arrival>=o.back().arrival);o.push_back({arrival,end,std::abs(q),limit,0,0,-1,-1,q>0?1:-1});}
    double prev=-std::numeric_limits<double>::infinity(),buy=0,sell=0,notional=0;
    for(int i=0;i<m;++i){size_t j=2+5*n+5*i;double t=x[j],bid=x[j+1],ask=x[j+2],b=x[j+3],a=x[j+4];require(t>=0&&t>prev&&bid>0&&ask>=bid&&a>=0&&b>=0);prev=t;
        for(auto& z:o){if(z.arrival>t)break;if(t>z.end||z.filled>=z.q)continue;
            double price=z.side==1?ask:bid;if((z.side==1&&price>z.limit)||(z.side==-1&&price<z.limit))continue;
            double& size=z.side==1?a:b;double q=std::min(z.q-z.filled,size);if(q<=0)continue;
            z.filled+=q;size-=q;z.notional+=q*price;notional+=q*price;if(z.first<0)z.first=t;z.last=t;
            if(z.side==1)buy+=q;else sell+=q;
        }
    }
    Vec out{buy,sell,notional};for(auto z:o)out.insert(out.end(),{z.filled,z.q-z.filled,z.notional,z.filled?z.notional/z.filled:0,z.first,z.last});return out;
}
inline Vec fee_rebalance(const Vec& x){
    require(x.size()>=6&&(x.size()-2)%4==0);double cash=x[0],fee=x[1];require(cash>=0&&fee>=0&&fee<1);
    int n=int((x.size()-2)/4);Vec q(n),p(n),w(n),next(n),h(n);
    double v=cash;
    for(int i=0;i<n;++i){q[i]=x[2+4*i];p[i]=x[3+4*i];w[i]=x[4+4*i];next[i]=x[5+4*i];require(q[i]>=0);positive(p[i]);positive(next[i]);prob(w[i]);h[i]=q[i]*p[i];v+=h[i];}
    double ws=sum(w);require(ws<=1+1e-12);positive(v);
    auto turnover=[&](double wealth){double z=0;for(int i=0;i<n;++i)z+=std::abs(w[i]*wealth-h[i]);return z;};
    double lo=0,hi=v;
    for(int it=0;it<100;++it){double mid=(lo+hi)/2;if(mid+fee*turnover(mid)>v)hi=mid;else lo=mid;}
    double post=(lo+hi)/2,tr=turnover(post),cost=fee*tr,remain=post*(1-ws),end=remain;
    Vec out{v,post,cost,tr,remain,0,v-post-cost};
    for(int i=0;i<n;++i){double newq=w[i]*post/p[i];end+=newq*next[i];out.push_back(newq);}
    out[5]=end;return out;
}
inline Vec matured_ridge(const Vec& x){
    require(x.size()>=12);double now=x[0],query=x[1],lambda=x[2],embargo=x[3];int n=integer(x[4],1,10000);require(lambda>=0&&embargo>=0&&x.size()==size_t(5+6*n));
    Vec a,b;double last=-std::numeric_limits<double>::infinity();
    for(int i=0;i<n;++i){size_t j=5+6*i;double t=x[j],f=x[j+1],end=x[j+2],avail=x[j+3];require(t>last&&f<=t&&end>=t&&avail>=end);last=t;
        if(t<now&&avail<=now-embargo){a.push_back(x[j+4]);b.push_back(x[j+5]);}}
    require(a.size()>=2);double mu=avg(a),ym=avg(b),scale=std::sqrt(var(a)*(a.size()-1)/a.size());positive(scale);
    Vec z;for(double y:a)z.push_back((y-mu)/scale);
    double beta=0;for(size_t i=0;i<z.size();++i)beta+=z[i]*(b[i]-ym);
    beta/=dot(z,z)+lambda;double rss=0;for(size_t i=0;i<z.size();++i)rss+=sq(b[i]-ym-beta*z[i]);
    return {ym+beta*(query-mu)/scale,double(a.size()),mu,scale,ym,beta,beta/scale,rss};
}
inline Vec settlement_cash(const Vec& x){
    require(x.size()>=7&&(x.size()-3)%4==0);double cash=x[0],shares=x[1],lag=x[2];require(cash>=0&&shares>=0&&lag>=0);
    double initial=cash,reserved=0,pending=0,prev=-std::numeric_limits<double>::infinity(),fee=0;std::map<double,double> due;
    Vec out{0,0,0,0,0,0};
    auto settle=[&](double t){while(!due.empty()&&due.begin()->first<=t){double v=due.begin()->second;cash+=v;pending-=v;due.erase(due.begin());}};
    // Reserve every unsettled debit separately, even if same-day credits offset it.
    std::map<double,double> debit;
    auto release=[&](double t){while(!debit.empty()&&debit.begin()->first<=t){reserved-=debit.begin()->second;debit.erase(debit.begin());}};
    int accepted=0,rejected=0;
    for(size_t i=3;i<x.size();i+=4){double t=x[i],q=x[i+1],p=x[i+2],f=x[i+3];require(t>=prev&&q!=0&&p>0&&f>=0);prev=t;
        settle(t);release(t);double cf=-q*p-f;bool ok=(shares+q>=-1e-10&&std::max(0.,-cf)<=cash-reserved+1e-10);
        if(ok){shares+=q;fee+=f;due[t+lag]+=cf;pending+=cf;if(cf<0){reserved-=cf;debit[t+lag]-=cf;}++accepted;settle(t);release(t);}
        else ++rejected;
        out.insert(out.end(),{ok?1.:0.,cash,reserved,cash-reserved,pending,shares});}
    settle(std::numeric_limits<double>::infinity());release(std::numeric_limits<double>::infinity());
    out[0]=cash;out[1]=shares;out[2]=fee;out[3]=accepted;out[4]=rejected;out[5]=cash-initial;return out;
}
inline Vec trade_revisions(const Vec& x){
    require(x.size()>=10);int n=integer(x[0],1,10000),m=integer(x[1],1,1000);require(x.size()==size_t(2+6*n+3*m)&&n*m<=1000000);
    struct Rev{double avail,event,p,q;bool live;};std::map<int,std::vector<Rev>> r;std::set<std::pair<int,double>> seen;
    for(int i=0;i<n;++i){size_t j=2+6*i;int id=integer(x[j],0,1000000);double a=x[j+1],t=x[j+2],p=x[j+3],q=x[j+4];int live=integer(x[j+5],0,1);
        require(a>=t&&seen.insert({id,a}).second);if(live){positive(p);positive(q);}else require(p==0&&q==0);r[id].push_back({a,t,p,q,bool(live)});}
    Vec out;
    for(int i=0;i<m;++i){size_t j=2+6*n+3*i;double now=x[j],l=x[j+1],u=x[j+2];require(l<u&&u<=now);
        double vol=0,nt=0,count=0;
        for(const auto& z:r){const Rev* best=nullptr;for(const auto& v:z.second)if(v.avail<=now&&(!best||v.avail>best->avail))best=&v;
            if(best&&best->live&&best->event>=l&&best->event<u){vol+=best->q;nt+=best->p*best->q;++count;}}
        out.insert(out.end(),{count,vol,nt,vol?nt/vol:0});}
    return out;
}
inline Vec fractional_rank(const Vec& x){
    require(x.size()>=6&&(x.size()-2)%2==0);int n=int((x.size()-2)/2),k=integer(x[0],1,n);double gross=x[1];positive(gross);
    std::vector<std::pair<double,int>> eligible;Vec w(n),longw(n),shortw(n);
    for(int i=0;i<n;++i)if(integer(x[2+2*i],0,1))eligible.push_back({x[3+2*i],i});
    require(int(eligible.size())>=2*k);std::sort(eligible.begin(),eligible.end());
    double lower=eligible[k-1].first,upper=eligible[eligible.size()-k].first;require(lower<upper);
    int below=0,above=0,ltie=0,utie=0;
    for(auto z:eligible){below+=z.first<lower;above+=z.first>upper;ltie+=z.first==lower;utie+=z.first==upper;}
    double ls=0,ss=0;
    for(auto z:eligible){double a=z.first>upper?1:(z.first==upper?double(k-above)/utie:0);
        double b=z.first<lower?1:(z.first==lower?double(k-below)/ltie:0);longw[z.second]=gross/2*a/k;shortw[z.second]=gross/2*b/k;w[z.second]=longw[z.second]-shortw[z.second];ls+=longw[z.second]*z.first;ss+=shortw[z.second]*z.first;}
    double absw=0;for(double z:w)absw+=std::abs(z);Vec out{sum(w),absw,lower,upper,ls-ss};out.insert(out.end(),w.begin(),w.end());return out;
}
}
