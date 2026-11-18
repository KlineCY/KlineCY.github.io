#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <utility>
#include <vector>

namespace kline {
struct LocalLinearEffect { double treatmentEffect=0,leftIntercept=0,rightIntercept=0; };
inline LocalLinearEffect regression_discontinuity_triangular(
    const std::vector<double>& running,const std::vector<double>& outcome,
    double cutoff,double bandwidth){
    check_finite(running);check_finite(outcome);
    require(running.size()==outcome.size()&&std::isfinite(cutoff)&&
            std::isfinite(bandwidth)&&bandwidth>0,"Invalid RD observations");
    struct Moments{double sw=0,sx=0,sy=0,sxx=0,sxy=0;int n=0;};
    Moments sides[2];
    for(std::size_t i=0;i<running.size();++i){
        const double x=running[i]-cutoff,weight=1-std::abs(x)/bandwidth;
        if(weight<=0)continue;
        Moments& m=sides[x<0?0:1];m.n++;m.sw+=weight;m.sx+=weight*x;
        m.sy+=weight*outcome[i];m.sxx+=weight*x*x;
        m.sxy+=weight*x*outcome[i];
    }
    double intercepts[2];
    for(int i=0;i<2;++i){
        const auto& m=sides[i];require(m.n>=3&&m.sw>0,"RD needs three observations per side");
        const double determinant=m.sw*m.sxx-m.sx*m.sx;
        require(determinant>1e-12*m.sw*m.sw*bandwidth*bandwidth,
                "RD local regression is rank deficient");
        intercepts[i]=(m.sy*m.sxx-m.sx*m.sxy)/determinant;
    }
    return {intercepts[1]-intercepts[0],intercepts[0],intercepts[1]};
}
inline double wald_instrumental_variable(const std::vector<double>& instrument,
                                         const std::vector<double>& treatment,
                                         const std::vector<double>& outcome){
    check_finite(instrument);check_finite(treatment);check_finite(outcome);
    require(instrument.size()>=4&&instrument.size()==treatment.size()&&
            treatment.size()==outcome.size(),"IV columns must align");
    double z=mean(instrument),x=mean(treatment),y=mean(outcome),zx=0,zy=0,zvar=0,xvar=0;
    for(std::size_t i=0;i<instrument.size();++i){
        const double a=instrument[i]-z,b=treatment[i]-x;
        zx+=a*b;zy+=a*(outcome[i]-y);zvar+=a*a;xvar+=b*b;
    }
    require(zvar>0&&xvar>0&&std::abs(zx)>1e-10*std::sqrt(zvar*xvar),
            "Instrument has insufficient first-stage relevance");
    return zy/zx;
}
struct FixedEffectMeta{double effect=0,standardError=0,heterogeneityQ=0,iSquared=0;};
inline FixedEffectMeta inverse_variance_meta(
    const std::vector<double>& estimates,const std::vector<double>& standardErrors){
    check_finite(estimates);check_finite(standardErrors);
    require(estimates.size()==standardErrors.size()&&estimates.size()>=2,
            "Meta-analysis needs at least two estimates");
    double sumW=0,sumWY=0;
    for(std::size_t i=0;i<estimates.size();++i){
        require(standardErrors[i]>0,"Meta-analysis standard errors must be positive");
        const double w=1/(standardErrors[i]*standardErrors[i]);
        require(std::isfinite(w),"Meta-analysis weights overflow");
        sumW+=w;sumWY+=w*estimates[i];
    }
    require(sumW>0&&std::isfinite(sumW),"Invalid meta-analysis weights");
    const double pooled=sumWY/sumW;double q=0;
    for(std::size_t i=0;i<estimates.size();++i)
        q+=std::pow((estimates[i]-pooled)/standardErrors[i],2);
    return {pooled,1/std::sqrt(sumW),q,q>0?
        std::max(0.0,(q-(estimates.size()-1))/q):0};
}
struct SyntheticControlFit{std::vector<double>weights;double inSampleRmse=0;};
inline std::vector<double> simplex_projection_research(std::vector<double> value){
    std::vector<double> sorted=value;
    std::sort(sorted.begin(),sorted.end(),std::greater<double>());
    double partial=0,theta=0;int rho=0;
    for(int j=0;j<int(sorted.size());++j){
        partial+=sorted[j];
        const double candidate=(partial-1)/(j+1);
        if(sorted[j]>candidate){rho=j+1;theta=candidate;}
    }
    require(rho>0,"Simplex projection failed");
    for(double& v:value)v=std::max(0.0,v-theta);
    return value;
}
inline SyntheticControlFit synthetic_control_simplex(
    const std::vector<double>& treated,
    const std::vector<std::vector<double>>& donors,int iterations=4000){
    check_finite(treated);
    require(treated.size()>=3&&donors.size()==treated.size()&&
            iterations>=10&&iterations<=100000,"Invalid synthetic-control panel");
    const std::size_t n=treated.size(),k=donors.front().size();
    require(k>=2&&k<=100,"Synthetic control needs two to 100 donors");
    std::vector<std::vector<double>> gram(k,std::vector<double>(k,0));
    std::vector<double> target(k,0);
    for(std::size_t t=0;t<n;++t){
        check_finite(donors[t]);
        require(donors[t].size()==k,"Donor matrix must be rectangular");
        for(std::size_t i=0;i<k;++i){
            target[i]+=donors[t][i]*treated[t]/n;
            for(std::size_t j=0;j<k;++j)
                gram[i][j]+=donors[t][i]*donors[t][j]/n;
        }
    }
    double lip=0;
    for(std::size_t i=0;i<k;++i){
        double row=0;for(double v:gram[i])row+=std::abs(v);
        lip=std::max(lip,2*row);
    }
    require(lip>0&&std::isfinite(lip),"Synthetic control has no donor variation");
    std::vector<double> weights(k,1.0/k);
    for(int step=0;step<iterations;++step){
        std::vector<double> updated(k);
        for(std::size_t i=0;i<k;++i){
            double fitted=0;
            for(std::size_t j=0;j<k;++j)fitted+=gram[i][j]*weights[j];
            updated[i]=weights[i]-2*(fitted-target[i])/lip;
        }
        updated=simplex_projection_research(updated);
        double delta=0;
        for(std::size_t i=0;i<k;++i)
            delta=std::max(delta,std::abs(updated[i]-weights[i]));
        weights=std::move(updated);
        if(delta<1e-12)break;
    }
    double loss=0;
    for(std::size_t t=0;t<n;++t){
        double fitted=0;
        for(std::size_t j=0;j<k;++j)fitted+=donors[t][j]*weights[j];
        loss+=std::pow(fitted-treated[t],2);
    }
    return {weights,std::sqrt(loss/n)};
}
} // namespace kline
