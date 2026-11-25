#include "predictive_online.hpp"
#include "predictive_portfolio.hpp"
#include "predictive_sequence.hpp"
#include "predictive_interpretation.hpp"
using namespace pdq;
void row(const Vec& x){for(size_t i=0;i<x.size();++i){require(std::isfinite(x[i]));if(i)std::cout<<',';std::cout<<x[i];}std::cout<<'\n';}
void kernel_project(){
    std::cout<<"Gamma,Landmarks,Lambda,Shift,LOOMSE,FullTestMSE,NystromTestMSE,PredictionGap,MeanDerivativeVariance,RFFGramRMSE\n";
    const int n=12,m=6;Vec p(n),y(n);
    for(int i=0;i<n;++i){p[i]=-.9+.18*i;y[i]=std::sin(2*p[i])+.1*((3*i)%5-2);}
    for(double g:{.2,1.,3.})for(int k:{2,4,8,12})for(double l:{.05,.2,.8})for(double shift:{0.,.4}){
        Vec loo{n,g,l},gram{n,double(k),g};
        for(int i=0;i<n;++i){loo.insert(loo.end(),{p[i],y[i]});gram.push_back(p[i]);}
        for(int j=0;j<k;++j)gram.push_back(std::sqrt(2*g)*(2.*j+1-k)/k*2);
        Vec a=ridge_loo(loo),R=fourier_gram(gram);double full=0,low=0,gap=0,dvar=0;
        for(int j=0;j<m;++j){
            double q=-.8+.34*j+shift,z=std::sin(2*q)+.1*((2*j)%5-2);
            Vec f{n,g,l,q},v{n,double(k),g,.001,l,q};
            for(int i=0;i<n;++i){f.insert(f.end(),{p[i],y[i]});v.insert(v.end(),{p[i],y[i]});}
            for(int i=0;i<k;++i)v.push_back((i*(n-1))/(k-1));
            Vec b=gp_derivative(f),c=nystrom_ridge(v);full+=sq(b[0]-z)/m;low+=sq(c[0]-z)/m;gap+=sq(b[0]-c[0])/m;dvar+=b[3]/m;
        }
        row({g,double(k),l,shift,a[0],full,low,gap,dvar,R[0]});
    }
}
void graph_project(){
    std::cout<<"Labels,Gamma,Ridge,Noise,GraphMSE,LocalMSE,MeanMSE,Energy,Stationarity\n";
    int n=15;Vec p(n),y(n);for(int i=0;i<n;++i){p[i]=i/7.-1;y[i]=std::sin(2*p[i]);}
    for(int k:{3,5,8})for(double g:{.5,2.,8.})for(double l:{.02,.2})for(double noise:{0.,.2}){
        std::set<int> ids;Vec x{double(n),double(k),l};for(int i=0;i<n;++i)for(int j=i+1;j<n;++j)x.push_back(rbf(p[i],p[j],g));
        Vec train,labels;for(int j=0;j<k;++j){int i=j*(n-1)/(k-1);ids.insert(i);double z=y[i]+noise*((j*3)%5-2);x.insert(x.end(),{double(i),z});train.insert(train.end(),{p[i],z});labels.push_back(z);}
        Vec f=harmonic_graph(x);double gm=0,lm=0,mm=0;
        for(int i=0;i<n;++i)if(!ids.count(i)){Vec a{p[i],.5};a.insert(a.end(),train.begin(),train.end());Vec b=local_linear_smoother(a);gm+=sq(f[2+i]-y[i])/(n-k);lm+=sq(b[0]-y[i])/(n-k);mm+=sq(avg(labels)-y[i])/(n-k);}
        row({double(k),g,l,noise,gm,lm,mm,f[0],f[1]});
    }
}
void delayed_project(){
    std::cout<<"Length,Switch,Delay,Eta,Loss,ImmediateLoss,FirstRegimeLoss,SecondRegimeLoss,Pending,FinalWeight,PoisonPredictionGap\n";
    for(int n:{30,60,90})for(int sw:{10,20})for(int d:{1,3,6})for(double eta:{.05,.2,.5}){
        Vec x{eta,-2,2,0},z=x,p=x;for(int t=0;t<n;++t){double a=((t*5)%13-6)/6.,y=(t<sw?.8:-.8)*a+.04*((t*3)%7-3);int delay=d+t%2;
            x.insert(x.end(),{a,y,double(delay),1});z.insert(z.end(),{a,y,1,1});p.insert(p.end(),{a,y+(t+delay>=n?10000:0),double(delay),1});}
        Vec a=delayed_ogd(x),b=delayed_ogd(z),c=delayed_ogd(p);double first=0,second=0,gap=0;
        for(int t=0;t<n;++t){double l=a[3+4*t+2];if(t<sw)first+=l;else second+=l;gap=std::max(gap,std::abs(a[3+4*t+1]-c[3+4*t+1]));}
        row({double(n),double(sw),double(d),eta,a[0],b[0],first,second,a[2],a[1],gap});
    }
}
void experts_project(){
    std::cout<<"Length,Switch,Eta,Share,CumulativeLoss,BestFixed,FixedRegret,DynamicOracle,SecondRegimeMass,FinalMass\n";
    for(int n:{30,60,90})for(int sw:{10,20})for(double eta:{.5,2.,4.})for(double share:{0.,.05,.2}){
        Vec x{3,eta,share};double oracle=0;
        for(int t=0;t<n;++t){Vec l{(t<sw?.1:.7)+.02*(t%4),.35+.02*((t+1)%4),(t<sw?.7:.1)+.02*((t+2)%4)};x.insert(x.end(),l.begin(),l.end());oracle+=*std::min_element(l.begin(),l.end());}
        Vec a=fixed_share(x);row({double(n),double(sw),eta,share,a[0],a[1],a[2],oracle,a[3+4*sw+3],a.back()});
    }
}
Vec attention_chain(const Vec& x,int dilation){
    int n=x.size();Vec a{double(n),3.,double(dilation),.1,.5,-.2,.3};a.insert(a.end(),x.begin(),x.end());a.insert(a.end(),n,1.);Vec c=causal_conv_vjp(a),q,k,v,g;
    for(int t=0;t<n;++t){double y=c[2+t];q.insert(q.end(),{y,x[t]});k.insert(k.end(),{.7*y,.2*x[t]});v.insert(v.end(),{y,.5*x[t]});g.insert(g.end(),{1.,-.3});}
    Vec p{double(n),2};for(const auto& z:{q,k,v,g})p.insert(p.end(),z.begin(),z.end());return causal_attention_vjp(p);
}
void attention_project(){
    std::cout<<"Length,Dilation,Cut,Shock,PrefixDifference,FutureDifference,AttentionLoss,MaskedMass,GradientNorm\n";
    for(int n:{6,8,12})for(int d:{1,2,3})for(int cut:{2,4})for(double shock:{1.,4.}){
        Vec x(n),z(n);for(int t=0;t<n;++t){x[t]=std::sin(.3*t)+.1*((t*3)%5-2);z[t]=x[t]+(t>=cut?shock:0);}
        Vec a=attention_chain(x,d),b=attention_chain(z,d);size_t off=1+n*n;double pre=0,post=0,masked=0,gn=0;
        for(int t=0;t<n;++t){for(int j=0;j<2;++j){double e=std::abs(a[off+2*t+j]-b[off+2*t+j]);if(t<cut)pre=std::max(pre,e);else post=std::max(post,e);}for(int j=t+1;j<n;++j)masked+=a[1+t*n+j];}
        for(size_t j=off+2*n;j<a.size();++j)gn+=sq(a[j]);
        row({double(n),double(d),double(cut),shock,pre,post,a[0],masked,std::sqrt(gn)});
    }
}
void recurrence_project(){
    std::cout<<"Length,Decay,GateBias,ScanFinal,ScanFirstInputGradient,ScanInitialGradient,GRUFinal,GRUFirstInputGradient,GRUInitialGradient\n";
    for(int n:{6,12,24})for(double decay:{.2,.6,.95})for(double gate:{-1.,0.,2.}){
        Vec x(n),g(n);g.back()=1;for(int t=0;t<n;++t)x[t]=.2*std::sin(.3*t)+.1*((2*t)%5-2);
        Vec a{double(n),1,decay,.7,1,.1};a.insert(a.end(),x.begin(),x.end());a.insert(a.end(),g.begin(),g.end());
        Vec b{.2,.1,gate,.2,.1,0,.7,.4,0,.1};for(int t=0;t<n;++t)b.insert(b.end(),{x[t],g[t]});
        Vec u=diagonal_scan_vjp(a),v=gru_scalar_vjp(b);
        row({double(n),decay,gate,u[0],u[1+2*n+4],u[1+2*n+3],v[0],v[2+4*n+9],v[1]});
    }
}
Vec costs(int t,double regime){
    double z=((t*5)%17-8)/8.;return {.02+.35*z+regime*.1*((t+1)%4-1.5),-.03-.25*z+.1*std::sin(.4*t),.08+.12*z-.15*std::sin(.2*t)};
}
Vec pred_cost(int t,double beta){double z=((t*5)%17-8)/8.;return {.02+beta*z,-.03-beta*z,.08+.2*beta*z};}
void decision_project(){
    std::cout<<"Regime,Eta,Epochs,Model,Parameter,TrainMSE,TrainSPO,TestMSE,TestSPO,TestSPOPlus\n";
    for(double regime:{0.,.5,1.})for(double eta:{.01,.03,.08})for(int epochs:{5,15,40})for(int method:{0,1}){
        double beta=.15;Vec cap{.55,.65,.6};
        for(int ep=0;ep<epochs;++ep){double grad=0;for(int t=0;t<24;++t){Vec c=costs(t,regime),p=pred_cost(t,beta),a=c;a.insert(a.end(),p.begin(),p.end());a.insert(a.end(),cap.begin(),cap.end());Vec d=spo_plus(a);double z=((t*5)%17-8)/8.;Vec f{z,-z,.2*z};if(method)grad+=dot(Vec(d.begin()+13,d.end()),f)/24;else for(int j=0;j<3;++j)grad+=2*(p[j]-c[j])*f[j]/(24*3);}
            beta=std::clamp(beta-eta*grad,-1.,1.);
        }
        Vec out{regime,eta,double(epochs),double(method),beta};
        for(int part:{0,1}){int begin=part?24:0,end=part?40:24;double mse=0,spo=0,plus=0;for(int t=begin;t<end;++t){Vec c=costs(t,regime),p=pred_cost(t,beta),a=c;a.insert(a.end(),p.begin(),p.end());a.insert(a.end(),cap.begin(),cap.end());Vec d=spo_plus(a);for(int j=0;j<3;++j)mse+=sq(p[j]-c[j])/(3*(end-begin));spo+=d[0]/(end-begin);plus+=d[1]/(end-begin);}out.insert(out.end(),{mse,spo});if(part)out.push_back(plus);}
        row(out);
    }
}
Vec portfolio_data(int start,int m,double shift){
    Vec r;for(int t=start;t<start+m;++t){double common=.015*((5*t)%7-3);r.insert(r.end(),{.004+common+.01*std::sin(.2*t),.002+shift+common*.3+.015*std::cos(.3*t),-.001-shift*.4-common*.5+.012*std::sin(.5*t)});}return r;
}
Vec utility(const Vec& s,double T,double gamma,double cost,const Vec& R){Vec x{3.,double(R.size()/3),T,gamma,cost,.001};x.insert(x.end(),s.begin(),s.end());x.insert(x.end(),3,1./3);x.insert(x.end(),R.begin(),R.end());return soft_portfolio_vjp(x);}
void utility_project(){
    std::cout<<"Temperature,RiskPenalty,Fee,Shift,TrainStartUtility,TrainFinalUtility,TestUtility,TestMean,TestVariance,Turnover,W0,W1,W2,GradientSum\n";
    for(double T:{.3,.7,1.})for(double gamma:{0.,5.,20.})for(double fee:{0.,.001,.005})for(double shift:{0.,.01}){
        Vec train=portfolio_data(0,24,0),test=portfolio_data(24,16,shift),s{0.,0.,0.};Vec init=utility(s,T,gamma,fee,train);
        for(int j=0;j<60;++j){Vec a=utility(s,T,gamma,fee,train);for(int i=0;i<3;++i)s[i]+=5*a[8+i];}
        Vec a=utility(s,T,gamma,fee,train),b=utility(s,T,gamma,fee,test);
        row({T,gamma,fee,shift,init[0],a[0],b[0],b[1],b[2],b[3],b[5],b[6],b[7],b[4]});
    }
}
int main(int argc,char** argv){
    std::cout<<std::setprecision(17);
    try{require(argc==2);std::map<std::string,void(*)()> p={{"kernel",kernel_project},{"graph",graph_project},{"delayed",delayed_project},{"experts",experts_project},{"attention",attention_project},{"recurrence",recurrence_project},{"decision",decision_project},{"utility",utility_project}};
        auto it=p.find(argv[1]);require(it!=p.end());it->second();
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
