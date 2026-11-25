#pragma once
#include "finite_state_common.hpp"
namespace fsq {
// Discrete event queue: cancellation ahead, market execution, insertion ahead, idle.
// A tagged order fills only when at the front and a market execution occurs.
inline Vec tagged_queue(const Vec& x){
    require(x.size()==7);int n=count(x[0],1,2000),cap=count(x[1],1,300),q=count(x[2],0,cap);
    double exec=x[3],cancel=x[4],add=x[5],own=x[6];
    Vec prob{exec,cancel,add,own,1-exec-cancel-add-own};
    distribution(prob);
    Vec f(cap+1);f[q]=1;double fill=0,withdraw=0,tf=0,tw=0,occupancy=0;
    for(int t=1;t<=n;++t){
        Vec g(cap+1);occupancy+=sum(f);
        for(int i=0;i<=cap;++i){
            double p=f[i];
            if(i==0){fill+=p*exec;tf+=t*p*exec;}else g[i-1]+=p*exec;
            g[std::max(i-1,0)]+=p*cancel;
            g[std::min(i+1,cap)]+=p*add;
            withdraw+=p*own;tw+=t*p*own;
            g[i]+=p*prob[4];
        }
        f.swap(g);
    }
    return {fill,withdraw,sum(f),tf,tw,occupancy,fill+withdraw+sum(f)};
}
// Exact queue fill probability before an adverse state exit; regime observed each event.
inline Vec queue_regime(const Vec& x){
    require(x.size()==12);
    int n=count(x[0],1,1000),q=count(x[1],0,100),initial=count(x[2],0,1);
    Vec exec{x[3],x[4]},exit{x[5],x[6]},change{x[7],x[8]},cancel{x[9],x[10]};
    double reward=x[11];require(reward>=0);
    Mat p(2,Vec(5));
    for(int i=0;i<2;++i){p[i]={exec[i],exit[i],change[i],cancel[i],1-exec[i]-exit[i]-change[i]-cancel[i]};distribution(p[i]);}
    Mat f(2,Vec(q+1));f[initial][q]=1;
    double fill=0,adverse=0,time=0;
    for(int t=0;t<n;++t){Mat g(2,Vec(q+1));
        for(int i=0;i<2;++i)for(int j=0;j<=q;++j){double a=f[i][j];time+=a;
            if(j==0)fill+=a*p[i][0];else g[i][j-1]+=a*p[i][0];
            adverse+=a*p[i][1];g[1-i][j]+=a*p[i][2];g[i][std::max(0,j-1)]+=a*p[i][3];g[i][j]+=a*p[i][4];
        }
        f.swap(g);
    }
    double survive=sum(f[0])+sum(f[1]);
    return {fill,adverse,survive,reward*fill,time,fill+adverse+survive};
}
// A discrete-time maker quote decision, observed queue position, fill reward, exit cost.
// A market order of the remaining unit is available immediately; the terminal unit must execute.
inline Vec queue_execution_choice(const Vec& x){
    require(x.size()==9);
    int n=count(x[0],1,1000),q=count(x[1],0,100);
    double exec=x[2],cancel=x[3],adverse=x[4],waitcost=x[5],fillcost=x[6],market=x[7],shock=x[8];
    Vec p{exec,cancel,adverse,1-exec-cancel-adverse};distribution(p);require(waitcost>=0&&market>=0&&shock>=0);
    Vec v(q+1,market);
    std::vector<std::vector<int>> pol(n,std::vector<int>(q+1));
    for(int t=n-1;t>=0;--t){Vec a(q+1);
        for(int j=0;j<=q;++j){
            double hold=waitcost+exec*(j==0?fillcost:v[j-1])+cancel*v[std::max(0,j-1)]+adverse*(market+shock)+p[3]*v[j];
            a[j]=std::min(market,hold);pol[t][j]=hold<market-1e-12;
        }
        v.swap(a);
    }
    Vec mass(q+1);mass[q]=1;double actual=0,fills=0,takes=0,exits=0,waiting=0;
    for(int t=0;t<n;++t){Vec next(q+1);
        for(int j=0;j<=q;++j){double z=mass[j];if(!pol[t][j]){takes+=z;actual+=z*market;continue;}
            waiting+=z;actual+=z*waitcost;exits+=z*adverse;actual+=z*adverse*(market+shock);
            if(j==0){fills+=z*exec;actual+=z*exec*fillcost;}else next[j-1]+=z*exec;
            next[std::max(0,j-1)]+=z*cancel;next[j]+=z*p[3];
        }
        mass.swap(next);
    }
    takes+=sum(mass);actual+=sum(mass)*market;
    return {v[q],double(pol[0][q]),fills,takes,exits,waiting,actual,v[q]-actual};
}
// Aggregate liquidity with multi-unit bid queue and fixed market order size distribution.
inline Vec queue_batch_fill(const Vec& x){
    require(x.size()>=5);
    int n=count(x[0],1,500),ahead=count(x[1],0,100),own=count(x[2],1,100),m=count(x[3],1,100);
    require(x.size()==size_t(4+m+1));
    Vec p(x.begin()+4,x.end());distribution(p);
    int total=ahead+own;Vec f(total+1);f[0]=1;
    for(int t=0;t<n;++t){Vec g(total+1);for(int j=0;j<=total;++j)for(int k=0;k<=m;++k)g[std::min(total,j+k)]+=f[j]*p[k];f.swap(g);}
    Vec fill(own+1);
    for(int j=0;j<=total;++j)fill[std::max(0,j-ahead)]+=f[j];
    double mean=0;for(int j=0;j<=own;++j)mean+=j*fill[j];
    Vec out{mean,fill[0],fill.back(),sum(fill)};out.insert(out.end(),fill.begin(),fill.end());return out;
}
}
