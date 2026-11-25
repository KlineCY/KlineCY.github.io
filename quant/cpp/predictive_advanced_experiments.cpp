#include "predictive_optimization.hpp"
#include "predictive_distribution.hpp"
#include "predictive_interpretation.hpp"
#include "predictive_sequence.hpp"
#include "predictive_portfolio.hpp"
using namespace pdq;
void row(const Vec& x){for(size_t i=0;i<x.size();++i){require(std::isfinite(x[i]));if(i)std::cout<<',';std::cout<<x[i];}std::cout<<'\n';}
Vec data(int a,int b,double noise,double drift){
    Vec x;for(int t=a;t<b;++t){double z=std::sin(.23*t),w=std::cos(.31*t),v=.8*z+.2*w;
        x.insert(x.end(),{1.,z,w,v,.4*z-.3*w+drift*(t>=24?z*z:0)+noise*((t*7)%9-4)/4.});}return x;
}
double mse(const Vec& b,const Vec& data0,int d){double e=0;int n=data0.size()/(d+1);for(int i=0;i<n;++i){Vec x(data0.begin()+i*(d+1),data0.begin()+i*(d+1)+d);e+=sq(dot(x,b)-data0[i*(d+1)+d])/n;}return e;}
void hypergrad_project(){
    std::cout<<"InitialLambda,Step,Noise,Drift,FinalLambda,InitialValidation,FinalValidation,TestMSE,GridValidation,NormalResidual\n";
    for(double l:{.01,.2,2.})for(double step:{.5,2.,8.})for(double noise:{.05,.3})for(double drift:{0.,.4}){
        Vec train=data(0,24,noise,drift),val=data(24,36,noise,drift),test=data(36,48,noise,drift);
        auto f=[&](double lambda){Vec a{24,4,12,lambda};a.insert(a.end(),train.begin(),train.end());a.insert(a.end(),val.begin(),val.end());return ridge_hypergrad(a);};
        double logl=std::log(l),initial=f(l)[0],best=1e10;
        for(int t=0;t<30;++t){Vec a=f(std::exp(logl));logl=std::clamp(logl-step*a[2],-8.,5.);}
        double final=std::exp(logl);for(double g:{.001,.01,.1,1.,10.,100.})best=std::min(best,f(g)[0]);
        Vec a=f(final),b(a.begin()+3,a.begin()+7),c{24,4,final};c.insert(c.end(),train.begin(),train.end());Vec r=cg_ridge(c);
        row({l,step,noise,drift,final,initial,a[0],mse(b,test,4),best,r[1]});
    }
}
void influence_project(){
    std::cout<<"Lambda,Outlier,Position,Query,Prediction,LocalInfluence,DeletionChange,ApproximationError,LargestDeletionIndex\n";
    const int n=12;for(double l:{.03,.3,3.})for(double shock:{0.,.5,2.,8.})for(int pos:{0,5,11})for(double q:{-.6,.9}){
        Vec a{n,3,l};for(int t=0;t<n;++t){double z=(t-5.5)/5.5;a.insert(a.end(),{1.,z,z*z,.3+.5*z+.1*std::sin(t)+(t==pos?shock:0)});}a.insert(a.end(),{1.,q,q*q});Vec r=ridge_influence(a);
        double change=r[1+n+pos]-r[0],largest=-1;int ix=0;
        for(int t=0;t<n;++t){double e=std::abs(r[1+n+t]-r[0]);if(e>largest){largest=e;ix=t;}}
        row({l,shock,double(pos),q,r[0],r[1+pos],change,std::abs(change+r[1+pos]),double(ix)});
    }
}
void shift_project(){
    std::cout<<"SourceOnes,TargetOnes,Alpha,Gamma,Regularization,Query,OracleRatio,EstimatedRatio,OracleFinite,OracleRadius,EstimatedFinite,EstimatedRadius,UnweightedRadius,EffectiveSize\n";
    const int n=24;for(int s:{6,12,18})for(int t:{6,12,18})for(double alpha:{.11,.21})for(double g:{.5,2.})for(double l:{.01,.2})for(int q:{0,1}){
        Vec u{n,n,2,2,g,l};for(int i=0;i<n;++i)u.push_back(i<s?1:0);for(int i=0;i<n;++i)u.push_back(i<t?1:0);u.insert(u.end(),{0,1,0,1});
        Vec ratio=ulsif_ratio(u);double w0=double(n-t)/(n-s),w1=double(t)/s,e0=ratio[8],e1=ratio[9];
        Vec a{alpha,q?w1:w0},b{alpha,q?e1:e0},c{alpha,1};for(int i=0;i<n;++i){bool z=i<s;double score=.05*(1+(i*7)%11)+(z?.6:0);a.insert(a.end(),{score,z?w1:w0});b.insert(b.end(),{score,z?e1:e0});c.insert(c.end(),{score,1});}
        Vec A=weighted_conformal(a),B=weighted_conformal(b),C=weighted_conformal(c);
        row({double(s),double(t),alpha,g,l,double(q),q?w1:w0,q?e1:e0,A[0],A[1],B[0],B[1],C[1],ratio[3]});
    }
}
void scores_project(){
    std::cout<<"TrueCorrelation,ForecastCorrelation,Power,Weight,ExpectedEnergy,ExpectedVariogram,TruePortfolioVariance,ForecastPortfolioVariance,VarianceError\n";
    Mat states{{-1,-1},{-1,1},{1,-1},{1,1}};for(double truth:{-.8,0.,.8})for(double pred:{-.8,-.4,0.,.4,.8})for(double power:{.5,1.,1.5})for(double w:{.25,.5,.75}){
        Vec p{(1+pred)/4,(1-pred)/4,(1-pred)/4,(1+pred)/4},q{(1+truth)/4,(1-truth)/4,(1-truth)/4,(1+truth)/4};double en=0,va=0,risk=0,actual=0;
        for(int i=0;i<4;++i){Vec a{power,4,2,states[i][0],states[i][1]};for(int j=0;j<4;++j)a.insert(a.end(),{p[j],states[j][0],states[j][1]});en+=q[i]*energy_score(a)[0];va+=q[i]*variogram_score(a)[0];double ret=w*states[i][0]+(1-w)*states[i][1];risk+=p[i]*ret*ret;actual+=q[i]*ret*ret;}
        row({truth,pred,power,w,en,va,actual,risk,risk-actual});
    }
}
void explanation_project(){
    std::cout<<"Interaction,Dependence,Query,Noise,Prediction,Base,Shapley0,Shapley1,Closure,ALELast,MarginalImportance,ConditionalImportance\n";
    const int n=24;for(double c:{0.,.5,2.})for(double dep:{0.,.5,1.})for(double q:{-.7,.3,.8})for(double noise:{0.,.2}){
        Vec a{2,n,0,.5,.2,q,.4,c},b{4,n,.5,0,c,-1,-.5,0,.5,1},d{.5};
        for(int i=0;i<n;++i){double z=-.96+1.92*i/(n-1),w=dep*z+.3*std::sin(i),base=.2*w+c*z*w,y=.5*z+base+noise*((i*5)%7-3)/3.;a.insert(a.end(),{z,w});b.insert(b.end(),{z,w});d.insert(d.end(),{double(i/6),z,y,base});}
        Vec A=shapley_interaction(a),B=ale_bilinear(b),D=conditional_permutation(d);
        row({c,dep,q,noise,A[1],A[0],A[4],A[5],A[3],B.back(),D[3],D[4]});
    }
}
void calibration_project(){
    std::cout<<"Size,Bias,Tolerance,RiskTarget,BrierBefore,BrierAfter,WorstGroup,Updates,RiskFeasible,Threshold,EmpiricalRisk,CorrectedRisk\n";
    for(int n:{20,40,80})for(double bias:{-.2,0.,.2})for(double eps:{.02,.05})for(double alpha:{.1,.2,.3}){
        Vec x{double(n),3,eps},g{double(n),11,alpha,1};for(int j=0;j<11;++j)g.push_back(j/10.);
        for(int i=0;i<n;++i){double p=std::clamp(.15+.7*((i*7)%19)/18.+bias,0.,1.),y=((i*11)%23)/22.<.15+.7*((i*7)%19)/18.?1.:0.;x.insert(x.end(),{p,y,1.,double(i%2==0),double(i%3!=0)});double score=std::abs(y-p);for(int j=0;j<11;++j)g.push_back(score>j/10.?1.:0.);}
        Vec a=group_calibration(x),b=conformal_risk_grid(g);int j=int(b[2]);row({double(n),bias,eps,alpha,a[0],a[1],a[2],a[3],b[0],b[1],j<0?0:b[3+j],j<0?0:b[14+j]});
    }
}
Vec price_window(int end,int n,int h,double scale,double shift){
    Vec x{double(n),double(h)};for(int t=end-n+1;t<=end;++t){double o=100+.3*t+2*std::sin(.21*t),c=o+.8*std::sin(.5*t),hi=std::max(o,c)+.35,lo=std::min(o,c)-.25;for(double z:{o,hi,lo,c})x.push_back(scale*z+shift);x.push_back(100+10*((t*7)%13));}return x;
}
Vec image_features(const Vec& a){int h=int(a[5]),w=int(a[6]);double left=0,right=0;for(int i=0;i<h;++i){left+=i*a[7+i*w];right+=i*a[7+i*w+w-1];}return {1.,a[3]/(h*w),a[4]/(h*w),(right-left)/(h-1)};}
void image_project(){
    std::cout<<"Window,Height,Lambda,TrainMSE,TestMSE,ZeroBaselineMSE,MaxPricePixelDifference,MaxVolumePixelDifference,Residual\n";
    for(int n:{5,10,20})for(int h:{8,16,32})for(double l:{.01,.1,1.,10.}){
        Vec train,test;double pd=0,vd=0,base=0;
        for(int t=20;t<80;++t){Vec a=ohlc_raster(price_window(t,n,h,1,0)),b=ohlc_raster(price_window(t,n,h,2,17));size_t size=size_t(a[5]*a[6]);for(size_t j=0;j<size;++j){pd=std::max(pd,std::abs(a[7+j]-b[7+j]));vd=std::max(vd,std::abs(a[7+size+j]-b[7+size+j]));}
            Vec f=image_features(a);double y=(.3+2*(std::sin(.21*(t+1))-std::sin(.21*t))+.8*std::sin(.5*(t+1)))/(100+.3*t+2*std::sin(.21*t));f.push_back(y);Vec& v=t<60?train:test;v.insert(v.end(),f.begin(),f.end());if(t>=60)base+=y*y/20;}
        Vec x{40,4,l};x.insert(x.end(),train.begin(),train.end());Vec a=cg_ridge(x),b(a.begin()+2,a.begin()+6);row({double(n),double(h),l,mse(b,train,4),mse(b,test,4),base,pd,vd,a[1]});
    }
}
void maturity_project(){
    std::cout<<"AsOf,Horizon,Delay,Poison,ValidLabels,ProperLoss,NaiveLoss,PoisonLossDifference,MaskedGradient,GradientDifference\n";
    const int n=30;for(int now:{12,18,24,29})for(int h:{1,3,5})for(int delay:{0,2,5})for(double poison:{1.,10.,100.}){
        Vec a{double(now),n},b=a,c=a;for(int i=0;i<n;++i){double y=.5*std::sin(.3*i),mu=.2*std::cos(.2*i),lv=-.7+.1*(i%4),w=1.+.1*(i%3);int available=i+h+delay;a.insert(a.end(),{double(i),double(available),y,mu,lv,w});b.insert(b.end(),{double(i),double(available),y+(available>now?poison:0),mu,lv,w});c.insert(c.end(),{double(i),double(i),y,mu,lv,w});}
        Vec A=matured_gaussian_loss(a),B=matured_gaussian_loss(b),C=matured_gaussian_loss(c);double masked=0,gap=0;for(int i=0;i<n;++i){if(!A[3+i])masked=std::max(masked,std::max(std::abs(A[3+n+i]),std::abs(A[3+2*n+i])));gap=std::max(gap,std::abs(A[3+n+i]-B[3+n+i]));gap=std::max(gap,std::abs(A[3+2*n+i]-B[3+2*n+i]));}
        row({double(now),double(h),double(delay),poison,A[2],A[0],C[0],std::abs(A[0]-B[0]),masked,gap});
    }
}
int main(int argc,char** argv){std::cout<<std::setprecision(17);try{require(argc==2);std::map<std::string,void(*)()> p={{"hypergrad",hypergrad_project},{"influence",influence_project},{"shift",shift_project},{"scores",scores_project},{"explanation",explanation_project},{"calibration",calibration_project},{"image",image_project},{"maturity",maturity_project}};auto it=p.find(argv[1]);require(it!=p.end());it->second();}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
