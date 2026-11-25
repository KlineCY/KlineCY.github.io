#include "applied_synthesis.hpp"
void header(const std::string& s){std::cout<<s<<'\n';}
void row(const Vec& prefix,const Vec& values){
    Vec x=prefix;x.insert(x.end(),values.begin(),values.end());
    for(size_t i=0;i<x.size();++i){
        check(std::isfinite(x[i]));
        if(i)std::cout<<',';
        std::cout<<x[i];
    }
    std::cout<<'\n';
}
void forecast(){
    header("ShiftCount,RawBrier,CalibratedBrier,Improvement,TrainBlocks");
    for(int d=0;d<=8;++d){
        Vec x{5,.1,0,.3,1,.5,0,.7,1,.9,1};
        Vec p{.2,.4,.6,.8};
        std::array<int,4> counts{1+d,4,6,9-d};
        for(size_t j=0;j<p.size();++j)
            for(int i=0;i<10;++i)x.insert(x.end(),{p[j],i<counts[j]?1.:0.});
        row({double(d)},applied_forecast::frozen_calibration(x));
    }
}

void inference(){
    header("OrderId,U1,U2,U3,U4,U5,U6,KS,CvM,AD,LjungBox,FirstACF");
    Vec u{1./12,3./12,5./12,7./12,9./12,11./12};
    int id=0;
    do{
        Vec x{double(++id)};x.insert(x.end(),u.begin(),u.end());
        Vec q=applied_inference::pit_joint(u),z{1};
        z.insert(z.end(),u.begin(),u.end());
        q.push_back(applied_inference::ljung_box(z)[1]);
        row(x,q);
    }while(std::next_permutation(u.begin(),u.end()));
}

void liquidity(){
    header("Target,Limit,Filled,VWAP,Execution,Opportunity,Fees,Shortfall,ActualPnL,IdealPnL,Closure");
    for(int q=2;q<=32;q+=2)for(int limit:{100,101,103,105}){
        Vec x{double(q),double(limit),99,104,.1,100,4,101,6,103,10,105,10};
        Vec a=applied_liquidity::sweep_shortfall(x),y{10000};
        y.insert(y.end(),x.begin(),x.end());
        Vec w=applied_synthesis::execution_wealth(y);
        a.insert(a.end(),{w[0],w[1],w[4]});
        row({double(q),double(limit)},a);
    }
}

void rates(){
    header("ParallelBp,SlopeBpPerYear,AssetPV,LiabilityPV,Surplus,ParallelQuadratic,SlopeLinear,Remainder");
    Vec w=applied_rates::two_zero_immunize({1,5,3,100,.03});
    for(int bp=-100;bp<=100;bp+=10)for(int slope:{-50,0,50}){
        double h=bp/10000.,s=slope/10000.;
        double a=w[0]*std::exp(-(.03+h-2*s))+w[1]*std::exp(-5*(.03+h+2*s));
        double l=100*std::exp(-3*(.03+h));
        double quad=.5*w[4]*h*h,linear=-w[4]*s;
        row({double(bp),double(slope)},{a,l,a-l,quad,linear,a-l-quad-linear});
    }
}

void options(){
    header("CallPut,Steps,Growth,Strike,RiskNeutralP,American,European,Premium,ExerciseNodes");
    for(int cp:{-1,1})for(int n:{1,2,4,8})
        for(double r:{.99,1.,1.02,1.05})for(int k:{90,100,110}){
            Vec v=applied_options::american_tree({100.,double(k),1.15,.9,r,double(n),double(cp)});
            row({double(cp),double(n),r,double(k),(r-.9)/(.25)},v);
        }
}

void credit(){
    header("PDShiftBp,Threshold,CollateralizedEPE,IndependentCVA,JointCVA,WWRAdjustment,ExpectedCollateral");
    for(int bp=-500;bp<=500;bp+=100)for(int h:{0,10,20,40,60}){
        double d=bp/10000.,a=std::min(10.,double(h)),b=std::min(50.,double(h));
        Vec e=applied_credit::net_collateral({2,1,double(h),10,50});
        Vec v=applied_synthesis::dependent_cva({.4,.95,.5,a,.05-d,.5,b,.05+d});
        row({double(bp),double(h)},{e[2],v[0],v[1],v[2],e[4]});
    }
}

void accounts(){
    header("Order,Deposit,PreFlowValue,TerminalValue,TWR,AnnualMWR,Dietz,Gain");
    for(int order:{0,1})for(int f=0;f<=200;f+=25){
        double r1=order==0?.2:-.1,r2=order==0?-.1:.2;
        double v1=100*(1+r1),v2=(v1+f)*(1+r2);
        Vec a=applied_synthesis::investor_manager_return({100,v1,double(f),v2});
        row({double(order),double(f),v1,v2},a);
    }
}

void allocation(){
    header("Cost,Alpha,Weight1,Weight2,Objective,ES,TailRC1,TailRC2,Turnover");
    for(int j=0;j<=30;j+=2)for(double alpha:{.5,.75,.9}){
        double c=j/100.;
        Vec x{.7,.2,2,c,0,.6,-.5,-.2,1,c,-.6,0,alpha,
              .25,-.02,.04,.25,.1,-.02,.25,.2,.2,.25,-.05,.3};
        Vec a=applied_synthesis::cost_tail(x);
        double t=std::abs(a[0]-.2)+std::abs(a[1]+.2);
        a.push_back(t);row({c,alpha},a);
    }
}
int main(int argc,char** argv){
    std::cout<<std::setprecision(17);
    try{
        check(argc==2);
        const std::map<std::string,void(*)()> projects={
            {"forecast",forecast},
            {"inference",inference},
            {"liquidity",liquidity},
            {"rates",rates},
            {"options",options},
            {"credit",credit},
            {"accounts",accounts},
            {"allocation",allocation}
        };
        auto it=projects.find(argv[1]);
        check(it!=projects.end());
        it->second();
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
