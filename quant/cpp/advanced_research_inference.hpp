#pragma once
#include "advanced_research_common.hpp"
#include "model_research_likelihood.hpp"
#include <map>
#include <set>
namespace kline { namespace advanced_research {
inline Report local_projection_hac(const Mat& x,const Vec& p){
    auto d=validate(x,4,2,18);parameters(p,2);require(x.size()>=d+2,"Insufficient projection sample");auto H=count(p[0],0,x.size()-d-2),L=count(p[1],0,x.size()-H-1);
    require(double(H+1)*x.size()*d*d*(L+1)<=5e7,"Local projection work limit");
    Report o{{"Horizon","Observations","Response","HACVariance","HACStandardError","Intercept","ResidualSSE"},{}};
    for(std::size_t h=0;h<=H;++h){
        auto n=x.size()-h;Mat D;Vec y;
        for(std::size_t t=0;t<n;++t){Vec a{1,x[t][0]};a.insert(a.end(),x[t].begin()+2,x[t].end());D.push_back(a);y.push_back(x[t+h][1]);}
        Vec b=fit_ridge(D,y,0,false);Mat gram(d,Vec(d,0)),scores;double ss=0;
        for(std::size_t t=0;t<n;++t){add_outer(gram,D[t],1);double e=y[t]-dot(D[t],b);ss+=e*e;Vec s=D[t];for(double&v:s)v*=e;scores.push_back(s);}
        Vec unit(d,0);unit[1]=1;Vec v=solve_spd(gram,unit),z;for(auto&s:scores)z.push_back(dot(v,s));
        double var=squared(z);
        for(std::size_t l=1;l<=L;++l){double cov=0;for(std::size_t t=l;t<n;++t)cov+=z[t]*z[t-l];var+=2*(1-double(l)/(L+1))*cov;}
        require(var>=-1e-12*std::max(1.,squared(z)),"Negative HAC variance");
        o.rows.push_back({double(h),double(n),b[1],std::max(0.,var),std::sqrt(std::max(0.,var)),b[0],ss});
    }return o;
}

inline Report twfe_two_way_cluster(const Mat& x,const Vec& p){
    validate(x,4,4,4);parameters(p,0);
    struct Agg{double x=0,y=0,n=0;};std::map<double,Agg> entity,time;std::set<std::pair<double,double>> seen;double mx=0,my=0,n=x.size();
    for(auto&r:x){require(r[0]==std::floor(r[0])&&r[1]==std::floor(r[1]),"Integer panel IDs required");require(seen.insert({r[0],r[1]}).second,"Duplicate panel cell");for(Agg* a:{&entity[r[0]],&time[r[1]]}){a->x+=r[2];a->y+=r[3];++a->n;}mx+=r[2]/n;my+=r[3]/n;}
    require(entity.size()>=2&&time.size()>=2&&entity.size()*time.size()==x.size(),"Balanced panel required");
    Vec a,b;
    for(auto&r:x){auto e=entity.at(r[0]),t=time.at(r[1]);a.push_back(r[2]-e.x/e.n-t.x/t.n+mx);b.push_back(r[3]-e.y/e.n-t.y/t.n+my);}
    double den=positive(squared(a)),beta=dot(a,b)/den;std::map<double,double> e,t;double cell=0;
    for(std::size_t i=0;i<x.size();++i){double s=a[i]*(b[i]-beta*a[i]);e[x[i][0]]+=s;t[x[i][1]]+=s;cell+=s*s;}
    double ve=0,vt=0;for(auto&v:e)ve+=v.second*v.second;for(auto&v:t)vt+=v.second*v.second;
    ve/=den*den;vt/=den*den;cell/=den*den;double var=ve+vt-cell;
    return scalar({"Coefficient","VarianceEntity","VarianceTime","VarianceCell","TwoWayVariance","StandardError","NegativeVarianceFlag","Observations"},{beta,ve,vt,cell,var,std::sqrt(std::max(0.,var)),double(var<0),n});
}

inline Report crossfit_aipw(const Mat& x,const Vec& p){
    auto d=validate(x,4,4,19)-2;parameters(p,5);double lp=positive(p[0]),ly=positive(p[1]),clip=p[2],tol=positive(p[3]);auto K=count(p[4],1,2000);
    require(clip>0&&clip<.5,"Invalid propensity clipping");
    std::map<double,std::vector<std::size_t>> fold;Mat Z;
    for(std::size_t i=0;i<x.size();++i){auto&r=x[i];require(r[0]==std::floor(r[0])&&(r[1]==0||r[1]==1),"Invalid fold or treatment");fold[r[0]].push_back(i);Vec z{1};z.insert(z.end(),r.begin()+3,r.end());Z.push_back(z);}
    require(fold.size()>=2&&fold.size()<=100&&double(fold.size())*K*x.size()*d*d<=1e8,"Cross-fit work limit");
    Mat rows(x.size());Vec score(x.size());
    for(auto&kv:fold){
        Mat train,X0,X1;Vec label,y0,y1;
        for(std::size_t i=0;i<x.size();++i)if(x[i][0]!=kv.first){train.push_back(Z[i]);label.push_back(x[i][1]);if(x[i][1]){X1.push_back(Z[i]);y1.push_back(x[i][2]);}else{X0.push_back(Z[i]);y0.push_back(x[i][2]);}}
        require(!X0.empty()&&!X1.empty(),"Both treatment arms required outside each fold");
        auto eval=[&](const Vec&w){auto s=model_research::regularized_state(w,lp);s.value-=.5*lp*w[0]*w[0];s.gradient[0]=0;s.hessian[0][0]=0;
            for(std::size_t i=0;i<train.size();++i){double z=dot(train[i],w),v=logistic(z);s.value+=softplus(z)-label[i]*z;for(std::size_t j=0;j<d;++j)s.gradient[j]+=(v-label[i])*train[i][j];add_outer(s.hessian,train[i],v*(1-v));}return s;};
        Vec w=model_research::damped_newton(d,tol,K,eval).beta,b0=fit_ridge(X0,y0,ly,true),b1=fit_ridge(X1,y1,ly,true);
        for(auto i:kv.second){double raw=logistic(dot(Z[i],w)),e=std::clamp(raw,clip,1-clip),a=dot(Z[i],b0),b=dot(Z[i],b1),t=x[i][1],y=x[i][2];score[i]=b-a+t*(y-b)/e-(1-t)*(y-a)/(1-e);rows[i]={double(i+1),x[i][0],t,y,e,a,b,score[i],0,0,double(e!=raw)};}
    }
    double mean=sum(score)/score.size(),var=0;for(double v:score)var+=(v-mean)*(v-mean)/(score.size()*(score.size()-1.));
    Report o{{"Row","Fold","Treatment","Outcome","Propensity","Outcome0","Outcome1","Score","ATE","IIDSE","ClippedFlag"},{}};
    for(auto&r:rows){r[8]=mean;r[9]=std::sqrt(var);o.rows.push_back(r);}return o;
}

inline Report weibull_survival_profile(const Mat& x,const Vec& p){
    validate(x,2,2,2);parameters(p,3);double lo=positive(p[0]),hi=positive(p[1]);auto g=count(p[2],2,10000);
    require(hi>=lo&&double(g)*x.size()<=5e7,"Invalid Weibull grid");Vec lt;double events=0,evlog=0;
    for(auto&r:x){double z=std::log(positive(r[0]));require(r[1]==0||r[1]==1,"Binary event indicator");lt.push_back(z);events+=r[1];evlog+=r[1]*z;}
    positive(events);double best=std::numeric_limits<double>::infinity(),shape=lo,logscale=0;
    for(std::size_t i=0;i<g;++i){double k=std::exp(std::log(lo)+(std::log(hi)-std::log(lo))*i/(g-1)),mx=k*(*std::max_element(lt.begin(),lt.end())),s=0;for(double v:lt)s+=std::exp(k*v-mx);double eta=(mx+std::log(s)-std::log(events))/k,ll=events*(std::log(k)-k*eta-1)+(k-1)*evlog;if(-ll<best){best=-ll;shape=k;logscale=eta;}}
    Report o{{"Time","Event","Shape","Scale","Survival","Hazard","NegativeLogLikelihood"},{}};
    for(std::size_t i=0;i<x.size();++i){double z=shape*(lt[i]-logscale);o.rows.push_back({x[i][0],x[i][1],shape,std::exp(logscale),std::exp(-std::exp(z)),std::exp(std::log(shape)+z-lt[i]),best});}return o;
}

inline Report dynamic_panel_diff_iv(const Mat& x,const Vec& p){
    validate(x,6,3,3);parameters(p,0);Mat rows=x;std::sort(rows.begin(),rows.end(),[](const Vec&a,const Vec&b){return a[0]<b[0]||(a[0]==b[0]&&a[1]<b[1]);});
    Vec z,a,b;std::size_t entities=0;
    for(std::size_t i=0;i<rows.size();){std::size_t j=i;while(j<rows.size()&&rows[j][0]==rows[i][0]){require(rows[j][0]==std::floor(rows[j][0])&&rows[j][1]==std::floor(rows[j][1]),"Integer IDs required");if(j>i)require(rows[j][1]==rows[j-1][1]+1,"Consecutive entity times required");if(j>=i+2){z.push_back(rows[j-2][2]);a.push_back(rows[j-1][2]-rows[j-2][2]);b.push_back(rows[j][2]-rows[j-1][2]);}++j;}require(j-i>=3,"Each entity needs three times");++entities;i=j;}
    require(entities>=2,"At least two entities required");double den=dot(z,a);require(std::abs(den)>1e-14*std::max(1.,std::sqrt(squared(z)*squared(a))),"Weak or unidentified moment");
    double beta=dot(z,b)/den,first=den/positive(squared(z)),r2=den*den/(positive(squared(z))*positive(squared(a))),mom=0;for(std::size_t i=0;i<z.size();++i)mom+=z[i]*(b[i]-beta*a[i])/z.size();
    return scalar({"Coefficient","InstrumentMoment","FirstStageCoefficient","FirstStageR2","Observations","Entities"},{beta,mom,first,r2,double(z.size()),double(entities)});
}

inline Report supf_break_grid(const Mat& x,const Vec& p){
    auto d=validate(x,4,2,17)-1;parameters(p,1);auto n=x.size(),m=count(p[0],d+1,n/2);
    require(n>2*d&&double(n-2*m+1)*n*d*d<=5e7,"Break scan work limit");
    Mat X;Vec y;for(auto&r:x){y.push_back(r[0]);X.emplace_back(r.begin()+1,r.end());}
    auto fit=[&](std::size_t a,std::size_t b){Mat z(X.begin()+a,X.begin()+b);Vec v(y.begin()+a,y.begin()+b),w=fit_ridge(z,v,0,false);double ss=0;for(std::size_t i=0;i<v.size();++i)ss+=std::pow(v[i]-dot(z[i],w),2);return std::make_pair(w,ss);};
    double pooled=fit(0,n).second,best=-1;std::size_t idx=0;Report o{{"BreakAfter","PooledSSE","SegmentSSE","FStatistic","Selected"},{}};
    for(const auto&prefix:{"LeftCoefficient","RightCoefficient"})for(std::size_t j=0;j<d;++j)o.header.push_back(std::string(prefix)+std::to_string(j+1));
    for(std::size_t k=m;k<=n-m;++k){auto a=fit(0,k),b=fit(k,n);double ss=positive(a.second+b.second),f=std::max(0.,pooled-ss)/d/(ss/(n-2*d));if(f>best){best=f;idx=o.rows.size();}Vec r{double(k),pooled,ss,f,0};r.insert(r.end(),a.first.begin(),a.first.end());r.insert(r.end(),b.first.begin(),b.first.end());o.rows.push_back(r);}
    o.rows[idx][4]=1;return o;
}
} }
