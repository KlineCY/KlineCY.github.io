#pragma once
#include "multiscale_simulation.hpp"
#include "multiscale_microstructure.hpp"
#include <queue>
namespace msq {
inline Vec mlmc_bias_budget(const Vec& x){
    require(x.size()==9);double eps=x[0],theta=x[1],B=x[2],alpha=x[3],V=x[4],beta=x[5],C=x[6],gamma=x[7];int cap=integer(x[8],0,30);
    positive(eps);require(theta>0&&theta<1&&B>=0&&alpha>0&&V>=0&&beta>0&&C>0&&gamma>=0);
    int L=0;while(B*std::exp2(-alpha*L)>eps*std::sqrt(1-theta)&&L<cap)++L;
    double bias=B*std::exp2(-alpha*L);require(bias<=eps*std::sqrt(1-theta));
    Vec design{eps*std::sqrt(theta)};
    for(int l=0;l<=L;++l)design.insert(design.end(),{V*std::exp2(-beta*l),C*std::exp2(gamma*l)});
    Vec p=mlmc_plan(design),out{double(L),bias,p[0],bias*bias+p[0],p[1],p[2]};
    out.insert(out.end(),p.begin()+4,p.end());return out;
}
inline Vec neyman_integer(const Vec& x){
    require(x.size()>=5&&(x.size()-1)%4==0);int N=integer(x[0],1,1000000),k=(x.size()-1)/4;require(k<=1000);
    Vec a(k),p(k);std::vector<int> n(k),upper(k);int used=0,maxn=0;double h=0;
    using Entry=std::pair<double,int>;std::priority_queue<Entry> heap;
    auto marginal=[&](int j){return a[j]/(double(n[j])*(n[j]+1));};
    for(int i=0;i<k;++i){
        size_t j=1+4*i;p[i]=x[j];prob(p[i]);double v=x[j+1];require(v>=0);n[i]=integer(x[j+2],1,N);upper[i]=integer(x[j+3],n[i],N);
        used+=n[i];maxn+=upper[i];a[i]=sq(p[i])*v;h+=std::sqrt(a[i]);if(n[i]<upper[i])heap.push({marginal(i),-i});}
    require(std::abs(sum(p)-1)<1e-10&&used<=N&&N<=maxn);
    while(used<N){require(!heap.empty());int j=-heap.top().second;heap.pop();++n[j];++used;if(n[j]<upper[j])heap.push({marginal(j),-j});}
    double v=0,largest=heap.empty()?0:heap.top().first;for(int j=0;j<k;++j)v+=a[j]/n[j];
    Vec out{v,h*h/N,double(used),largest};for(int z:n)out.push_back(z);return out;
}
inline Vec preavg_optimal(const Vec& x){
    require(x.size()==1);int k=integer(x[0],2,10000);double pi=std::acos(-1.),norm=std::sqrt(2./(k+1));Vec w(k);
    for(int i=0;i<k;++i)w[i]=norm*std::sin(pi*(i+1)/(k+1));
    double a=dot(w,w),b=sq(w.front())+sq(w.back()),eig=2-2*std::cos(pi/(k+1)),res=0;
    for(int i=1;i<k;++i)b+=sq(w[i]-w[i-1]);
    for(int i=0;i<k;++i){double z=2*w[i]-(i?w[i-1]:0)-(i+1<k?w[i+1]:0);res=std::max(res,std::abs(z-eig*w[i]));}
    Vec out{a,b/a,eig,res};out.insert(out.end(),w.begin(),w.end());return out;
}
inline Vec two_scale_unknown(const Vec& x){
    require(x.size()>=7&&(x.size()-1)%2==0);int n=(x.size()-1)/2,k=integer(x[0],2,n-1);
    Vec a{1,0,0,0},b{double(k),0,0,0};a.insert(a.end(),x.begin()+1,x.end());b.insert(b.end(),x.begin()+1,x.end());
    Vec r=lag_cov_noise(a),s=lag_cov_noise(b),latent(3),noise(3);
    for(int i=0;i<3;++i){latent[i]=(k*s[i]-r[i])/(k-1);noise[i]=(r[i]-s[i])*k/(2.*(n-1)*(k-1));}
    Vec out{latent[0],latent[1],latent[2],noise[0],noise[1],noise[2],latent[0]*latent[1]-sq(latent[2]),noise[0]*noise[1]-sq(noise[2])};
    out.insert(out.end(),r.begin(),r.begin()+3);out.insert(out.end(),s.begin(),s.begin()+3);return out;
}
inline Vec cpcv_intervals(const Vec& x){
    require(x.size()>=6&&(x.size()-3)%3==0);int N=integer(x[0],2,12),K=integer(x[1],1,N-1),n=(x.size()-3)/3;double gap=x[2];require(gap>=0&&n>=N&&n<=500);
    std::vector<int> group(n),seen(N);Vec a(n),b(n);
    for(int i=0;i<n;++i){group[i]=integer(x[3+3*i],0,N-1);a[i]=x[4+3*i];b[i]=x[5+3*i];require(a[i]<b[i]&&(i==0||(group[i]>=group[i-1]&&a[i]>=a[i-1])));seen[group[i]]++;}
    for(int v:seen)require(v>0);
    std::vector<std::vector<int>> folds;
    std::vector<int> chosen;
    auto visit=[&](auto&& self,int j,int remain)->void{
        if(!remain){folds.push_back(chosen);return;}
        for(int i=j;i<=N-remain;++i){chosen.push_back(i);self(self,i+1,remain-1);chosen.pop_back();}};
    visit(visit,0,K);int splits=folds.size(),paths=splits*K/N;
    require(splits<=1000&&splits*K==paths*N&&double(splits)*n*n<=5e7);
    std::vector<std::vector<int>> slot(N);
    Vec out{double(splits),double(paths),double(N),double(K)};
    for(int f=0;f<splits;++f){std::vector<bool> hold(N);for(int g:folds[f]){hold[g]=true;slot[g].push_back(f);}
        int test=0,train=0,purge=0,embargo=0;
        for(int i=0;i<n;++i){
            if(hold[group[i]]){++test;continue;}
            bool overlap=false,blocked=false;
            for(int j=0;j<n;++j)if(hold[group[j]]){overlap|=a[i]<b[j]&&a[j]<b[i];blocked|=a[i]>=b[j]&&a[i]<b[j]+gap;}
            if(overlap)++purge;else if(blocked)++embargo;else ++train;
        }
        out.insert(out.end(),{double(test),double(train),double(purge),double(embargo)});
    }
    for(const auto& z:slot)require(int(z.size())==paths);
    for(int j=0;j<paths;++j)for(int g=0;g<N;++g)out.push_back(slot[g][j]);
    return out;
}
inline Vec two_way_cluster(const Vec& x){
    require(x.size()>=16&&x.size()%4==0);int n=x.size()/4;require(n<=100000);
    Vec a(n),b(n);std::vector<int> id(n),time(n);
    for(int i=0;i<n;++i){id[i]=integer(x[4*i],0,1000000);time[i]=integer(x[4*i+1],0,1000000);a[i]=x[4*i+2];b[i]=x[4*i+3];}
    double am=avg(a),bm=avg(b),sxx=0,sxy=0;
    for(int i=0;i<n;++i){sxx+=sq(a[i]-am);sxy+=(a[i]-am)*(b[i]-bm);}positive(sxx);
    double beta=sxy/sxx,alpha=bm-beta*am,rss=0;std::map<int,double> g1,g2;std::map<std::pair<int,int>,double> g12;
    for(int i=0;i<n;++i){double e=b[i]-alpha-beta*a[i],z=(a[i]-am)*e;rss+=e*e;g1[id[i]]+=z;g2[time[i]]+=z;g12[{id[i],time[i]}]+=z;}
    require(g1.size()>=2&&g2.size()>=2);
    auto meat=[&](const auto& g){double z=0;for(auto p:g)z+=sq(p.second);return z/sq(sxx);};
    double v1=meat(g1),v2=meat(g2),v12=meat(g12),v=v1+v2-v12;
    return {alpha,beta,rss,v1,v2,v12,v,std::sqrt(std::max(0.,v)),double(g1.size()),double(g2.size()),double(g12.size())};
}
inline long long block_paths(int n,int blocks){
    long long count=1;for(int i=0;i<blocks;++i){require(count<=1000000/n);count*=n;}return count;
}
inline Vec circular_sharpe_distribution(const Vec& x){
    require(x.size()>=4);int n=x.size()-1,k=integer(x[0],1,n);require(n%k==0&&n<=100);int m=n/k;
    long long paths=block_paths(n,m);Vec a(x.begin()+1,x.end()),values;
    Vec s(n),q(n),lo(n),hi(n);
    for(int i=0;i<n;++i){lo[i]=hi[i]=a[i];for(int j=0;j<k;++j){double z=a[(i+j)%n];s[i]+=z;q[i]+=z*z;lo[i]=std::min(lo[i],z);hi[i]=std::max(hi[i],z);}}
    for(long long mask=0;mask<paths;++mask){long long z=mask;long double u=0,v=0;double l=std::numeric_limits<double>::infinity(),h=-l;
        for(int j=0;j<m;++j){int t=z%n;z/=n;u+=s[t];v+=q[t];l=std::min(l,lo[t]);h=std::max(h,hi[t]);}
        if(l==h)continue;
        long double mean=u/n,variance=v/n-mean*mean;require(variance>0);values.push_back(double(mean/std::sqrt(variance)));}
    require(!values.empty());std::sort(values.begin(),values.end());double mu=avg(values),sd=values.size()>1?std::sqrt(var(values)*(values.size()-1)/values.size()):0;
    auto quantile=[&](double p){size_t j=std::max(1.,std::ceil(p*values.size()));return values[j-1];};
    return {double(paths),double(values.size()),1-double(values.size())/paths,mu,sd,quantile(.025),quantile(.5),quantile(.975)};
}
inline Vec reality_check_circular(const Vec& x){
    require(x.size()>=8);int n=integer(x[0],3,100),k=integer(x[1],2,30),b=integer(x[2],1,n);require(n%b==0&&x.size()==size_t(3+n*k));
    int m=n/b;long long paths=block_paths(n,m);require(double(paths)*m*k<=5e7);
    Vec mu(k);Mat a(n,Vec(k)),s(n,Vec(k));size_t at=3;
    for(auto& r:a)for(int j=0;j<k;++j){r[j]=x[at++];mu[j]+=r[j]/n;}
    for(int i=0;i<n;++i)for(int t=0;t<b;++t)for(int j=0;j<k;++j)s[i][j]+=a[(i+t)%n][j]-mu[j];
    double observed=*std::max_element(mu.begin(),mu.end()),exceed=0;Vec nulls;nulls.reserve(paths);
    for(long long mask=0;mask<paths;++mask){long long z=mask;Vec c(k);
        for(int t=0;t<m;++t){int i=z%n;z/=n;for(int j=0;j<k;++j)c[j]+=s[i][j]/n;}
        double q=*std::max_element(c.begin(),c.end());nulls.push_back(q);exceed+=q+1e-12>=observed;}
    double sd=nulls.size()>1?std::sqrt(var(nulls)*(nulls.size()-1)/nulls.size()):0;
    Vec out{observed,exceed/paths,avg(nulls),sd,double(paths)};out.insert(out.end(),mu.begin(),mu.end());return out;
}
}
