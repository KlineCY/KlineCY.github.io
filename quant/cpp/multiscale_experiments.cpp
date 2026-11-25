#include "multiscale_simulation.hpp"
#include "multiscale_microstructure.hpp"
#include "multiscale_engineering.hpp"
#include "multiscale_empirical.hpp"
using namespace msq;

struct Random {
    long long state;
    explicit Random(long long seed):state(seed){require(seed>0&&seed<2147483647);}
    double uniform(){state=48271*state%2147483647;return double(state)/2147483647.;}
    double normal(){double u=uniform(),v=uniform();return std::sqrt(-2*std::log(u))*std::cos(2*std::acos(-1.)*v);}
};
void header(const std::string& s){std::cout<<s<<'\n';}
void row(const Vec& a){for(size_t i=0;i<a.size();++i){require(std::isfinite(a[i]));if(i)std::cout<<',';std::cout<<a[i];}std::cout<<'\n';}
Vec paired(Random& rng,int n,double strike,double sigma){
    Vec x{100,strike,.02,sigma,1};for(int j=0;j<n;++j)x.push_back(rng.normal());return asian_coupling(x);
}

void mlmc_project(){
    header("Strike,Sigma,SEBudget,Estimate,EstimatedSE,ProductionCost,PilotCost,PlannedVariance,FinestCorrection,DirectFineEstimate,DirectFineSE,N0,N1,N2,N3");
    int id=0;
    for(double strike:{90.,100.,110.})for(double sigma:{.15,.3,.45})for(double eps:{1.,2.,3.}){
        int seed=100000+1000*++id;Vec design{eps};std::array<int,4> cost{2,2,4,8};
        for(int l=0;l<4;++l){Random rng(seed+10*l+1);Vec d;
            for(int j=0;j<96;++j){Vec z=paired(rng,cost[l],strike,sigma);d.push_back(l==0?z[3]:z[4]);}
            design.insert(design.end(),{var(d),double(cost[l])});}
        Vec plan=mlmc_plan(design),data{4};
        for(int l=0;l<4;++l){int n=integer(plan[4+l],2,100000);Random rng(seed+10*l+2);
            data.insert(data.end(),{double(n),double(cost[l])});
            for(int j=0;j<n;++j){Vec z=paired(rng,cost[l],strike,sigma);data.insert(data.end(),l==0?Vec{z[3],0}:Vec{z[2],z[3]});}}
        Vec a=mlmc_estimate(data),direct;Random rng(seed+99);
        int n=std::max(2,int(a[3]/8));
        for(int j=0;j<n;++j)direct.push_back(paired(rng,8,strike,sigma)[2]);
        Vec out{strike,sigma,eps,a[0],a[2],a[3],1536,plan[0],a[7],avg(direct),std::sqrt(var(direct)/n)};
        out.insert(out.end(),plan.begin()+4,plan.end());row(out);
    }
}

Vec averages(Random& rng,int n,double sigma){
    double logp=std::log(100.),a=0,g=0,dt=1./n;
    for(int j=0;j<n;++j){logp+=(.02-.5*sq(sigma))*dt+sigma*std::sqrt(dt)*rng.normal();a+=std::exp(logp);g+=logp;}
    return {a/n,std::exp(g/n)};
}
void control_project(){
    header("Monitoring,Sigma,EA,VarA,EG,VarG,AnalyticBeta,AnalyticResidualVariance,PilotBeta,RawExternalMean,AdjustedExternalMean,RawExternalVariance,AdjustedExternalVariance");
    int id=0;
    for(int n:{4,8,16,32})for(double sigma:{.1,.2,.4}){
        int seed=200000+1000*++id;Vec design{100,.02,sigma};
        for(int j=1;j<=n;++j)design.insert(design.end(),{double(j)/n,1./n});
        Vec exact=asian_moments(design),data{80,exact[2]};Random pilot(seed+1),external(seed+2);
        for(int j=0;j<80;++j){Vec a=averages(pilot,n,sigma);data.insert(data.end(),a.begin(),a.end());}
        for(int j=0;j<400;++j){Vec a=averages(external,n,sigma);data.insert(data.end(),a.begin(),a.end());}
        Vec z=frozen_control(data);
        row({double(n),sigma,exact[0],exact[1],exact[2],exact[3],exact[5],exact[6],z[0],z[1],z[2],z[3],z[4]});
    }
}
void async_project(){
    header("SpacingX,SpacingY,TimestampDelay,Noise,LatentPathCov,HY,Refresh,ShiftAlignedHY,PeakAbsHY,PeakShift,Overlap,RefreshNodes,RefreshPSDDet");
    int id=0;
    for(int a:{2,3,5})for(int b:{3,4,6})for(int delay:{0,2,4})for(double noise:{0.,.002}){
        int seed=300000+1000*++id;Random latent(seed+1),eps(seed+2);Vec x(121),y(121);double truth=0;
        for(int j=1;j<=120;++j){double z=latent.normal(),w=latent.normal(),u=.006*z,v=.004*(.6*z+.8*w);x[j]=x[j-1]+u;y[j]=y[j-1]+v;truth+=u*v;}
        Vec t1,t2;
        for(int j=0;j<=120;j+=a)t1.insert(t1.end(),{double(j),std::exp(x[j]+noise*eps.normal())});
        for(int j=0;j<=120;j+=b)t2.insert(t2.end(),{double(j+delay),std::exp(y[j]+noise*eps.normal())});
        Vec raw{double(t1.size()/2),double(t2.size()/2)};raw.insert(raw.end(),t1.begin(),t1.end());raw.insert(raw.end(),t2.begin(),t2.end());
        Vec h=hy_covariance(raw),r=refresh_covariance(raw),grid{5,-4,-2,0,2,4};grid.insert(grid.end(),raw.begin(),raw.end());
        Vec g=hy_shift_grid(grid);auto ab=two_ticks(raw);Vec aligned=hy(ab.first,ab.second,-delay);
        row({double(a),double(b),double(delay),noise,truth,h[0],r[0],aligned[0],g[2],g[0],h[4],r[3],r[1]*r[2]-sq(r[0])});
    }
}
void noise_project(){
    header("NoiseSD,Lag,MeanRawVarX,MeanRawVarY,MeanRawCov,MeanCorrectedVarX,MeanCorrectedVarY,MeanCorrectedCov,MeanPreavgRaw,MeanPreavgCorrected,EnumeratedPaths");
    for(double eta:{0.,.005,.01,.02})for(int k:{1,2,3,4}){
        Vec a(8);
        for(int bits=0;bits<(1<<9);++bits){
            Vec p(5),q(5);double s=0;
            for(int j=0;j<5;++j){if(j)s+=.01*((bits&(1<<(j-1)))?1:-1);double e=eta*((bits&(1<<(4+j)))?1:-1);p[j]=s+e;q[j]=1.5*s-.5*e;}
            Vec raw{double(k),eta*eta,.25*eta*eta,-.5*eta*eta};
            for(int j=0;j<5;++j)raw.insert(raw.end(),{p[j],q[j]});
            Vec z=lag_cov_noise(raw);for(int j=0;j<6;++j)a[j]+=z[j]/512;
            Vec u{2,eta*eta,1,.5};for(int j=1;j<5;++j)u.push_back(p[j]-p[j-1]);Vec v=weighted_preaverage(u);
            a[6]+=v[4]/512;a[7]+=v[5]/512;
        }
        Vec out{eta,double(k)};out.insert(out.end(),a.begin(),a.end());out.push_back(512);row(out);
    }
}
void universe_project(){
    header("AvailabilityDelay,Fee,Date,EligibleCount,LongScore,PretradeWealth,PostFeeWealth,Fees,TradeNotional,EndWealth,Cash,RankGross,RankNet");
    for(int delay:{0,1,2})for(double fee:{0.,.001,.005}){
        Vec data{6,8,9};for(int t=0;t<8;++t)data.push_back(t);
        for(int i=0;i<6;++i)data.insert(data.end(),{double(i),0,0,1});
        data.insert(data.end(),{0,2,double(2+delay),0,1,4,double(4+delay),0,0,6,double(6+delay),1});
        Vec mem=membership_vintage(data),q(6),p{10,12,14,16,18,20};double cash=1000;
        for(int t=0;t<8;++t){
            Vec ranks{1,2},scores;
            for(int i=0;i<6;++i){double s=(3*i+2*t)%7;scores.push_back(s);ranks.insert(ranks.end(),{mem[7*t+1+i],s});}
            Vec rank=fractional_rank(ranks),next(6),rebal{cash,fee};double score=0;
            for(int i=0;i<6;++i){double w=std::max(0.,rank[5+i]);next[i]=p[i]*(1+.01*((5*i+3*t)%7-3));score+=w*scores[i];rebal.insert(rebal.end(),{q[i],p[i],w,next[i]});}
            Vec a=fee_rebalance(rebal);for(int i=0;i<6;++i)q[i]=a[7+i];cash=a[4];p=next;
            row({double(delay),fee,double(t),mem[7*t],score,a[0],a[1],a[2],a[3],a[5],cash,rank[1],rank[0]});
        }
    }
}
void maturity_project(){
    header("Ridge,Embargo,Decision,TrainingCount,Forecast,NaiveUnavailableForecast,ActualLabel,ProperLoss,NaiveLoss,UnavailableLabelPerturbation");
    Vec records;
    for(int t=0;t<=44;++t){double x=((7*t)%13-6)/5.,y=.04+.3*x+.01*((t*3)%7-3);records.insert(records.end(),{double(t),double(t),double(t+3),double(t+3+t%2),x,y});}
    for(double lam:{0.,1.,5.})for(int embargo:{0,2})for(int t=8;t<=40;++t){
        double x=records[6*t+4],y=records[6*t+5];Vec raw{double(t),x,lam,double(embargo),45},naive=raw,poison=raw;raw.insert(raw.end(),records.begin(),records.end());
        for(int j=0;j<45;++j){double s=records[6*j],e=records[6*j+2],a=records[6*j+3],z=records[6*j+4],label=records[6*j+5];
            naive.insert(naive.end(),{s,s,s,s,z,label});
            poison.insert(poison.end(),{s,s,e,a,z,label+(a>t-embargo?10000:0)});}
        Vec proper=matured_ridge(raw),bad=matured_ridge(naive),alt=matured_ridge(poison);
        row({lam,double(embargo),double(t),proper[1],proper[0],bad[0],y,sq(proper[0]-y),sq(bad[0]-y),alt[0]-proper[0]});
    }
}
void selection_project(){
    header("Dates,Effect,Strategy,Statistic,RawP,MaxTP,StepdownP,BH,BY,RawReject,StepdownReject");
    for(int n:{6,8,10,12})for(double effect:{0.,.2,.5,1.}){
        Vec data{double(n),4};
        for(int t=0;t<n;++t){double common=((5*t+1)%7-3)*.3;
            for(int j=0;j<4;++j)data.push_back(common+((3*t+2*j+1)%5-2)*.2+effect*(j+1)/4);}
        Vec a=joint_sign_stepdown(data),p{.1};
        for(int j=0;j<4;++j)p.push_back(a[5+j]);
        Vec f=fdr_stepup(p);
        for(int j=0;j<4;++j)row({double(n),effect,double(j),a[1+j],a[5+j],a[9+j],a[13+j],f[3+j],f[7+j],a[5+j]<=.1?1.:0.,a[13+j]<=.1?1.:0.});
    }
}
void spanning_project(){
    header("Alpha,ResidualSD,Beta,BaseSR2,ExpandedSR2,Increment,SpanningClosure,HJLowerBound,SDFDistance,NegativeSDFShare,MaxPricingError");
    for(double alpha:{0.,.01,.03,.05})for(double residual:{.02,.05,.1,.2})for(double beta:{0.,.5,1.5}){
        Vec factors{1,4},h{2,4,1},sdf{3,4,1,0,0};
        for(int a:{-1,1})for(int b:{-1,1}){double f=.02+.1*a,r=alpha+beta*f+residual*b;
            factors.insert(factors.end(),{f,r});h.insert(h.end(),{f,r});sdf.insert(sdf.end(),{1,1,f,r});}
        Vec x=factor_spanning(factors),y=hj_bound(h),z=sdf_distance(sdf);
        row({alpha,residual,beta,x[0],x[1],x[2],x[5],y[1],z[0],y[4],y[5]});
    }
}
int main(int argc,char** argv){
    std::cout<<std::setprecision(17);
    try{
        require(argc==2);const std::map<std::string,void(*)()> projects={
            {"mlmc",mlmc_project},{"control",control_project},{"async",async_project},{"noise",noise_project},
            {"universe",universe_project},{"maturity",maturity_project},{"selection",selection_project},{"spanning",spanning_project}
        };
        auto it=projects.find(argv[1]);require(it!=projects.end());it->second();
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
