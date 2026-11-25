#include "multiscale_extensions.hpp"
using namespace msq;
void row(const Vec& a){for(size_t i=0;i<a.size();++i){require(std::isfinite(a[i]));if(i)std::cout<<',';std::cout<<a[i];}std::cout<<'\n';}
void budget_project(){
    std::cout<<"RMSE,Theta,Alpha,Beta,Gamma,Level,Bias,Variance,MSE,Cost,ContinuousCost,SampleCount\n";
    for(double eps:{.25,.125,.0625})for(double theta:{.25,.5,.75})
    for(double alpha:{1.,2.})for(double beta:{1.,2.,3.})for(double gamma:{1.,2.}){
        Vec z=mlmc_bias_budget({eps,theta,1,alpha,1,beta,1,gamma,24});
        row({eps,theta,alpha,beta,gamma,z[0],z[1],z[2],z[3],z[4],z[5],std::accumulate(z.begin()+6,z.end(),0.)});
    }
}
void allocation_project(){
    std::cout<<"Budget,VarianceMultiplier,Upper0,Variance,ContinuousBound,Gap,TotalSamples\n";
    for(int N:{8,12,16,24,32})for(double a:{.25,1.,4.,16.})for(int upper:{3,5,8}){
        Vec z=neyman_integer({double(N),.2,a,1,double(std::min(N,upper)),.3,2,1,double(N),.5,.5,1,double(N)});
        require(z[4]<=upper&&z[4]>=1&&z[5]>=1&&z[6]>=1&&z[4]+z[5]+z[6]==N);
        row({double(N),a,double(upper),z[0],z[1],z[0]-z[1],z[2]});
    }
}
void noise_project(){
    std::cout<<"NoiseSD,Lag,Window,LatentXX,LatentYY,LatentXY,NoiseXX,NoiseYY,NoiseXY,UniformRatio,SineRatio,MeanUniformNoise,MeanSineNoise,Paths\n";
    for(double eta:{0.,.005,.01,.02})for(int k:{2,3,4})for(int w:{2,3,4}){
        Vec a(6);double u2=0,v2=0;
        Vec best=preavg_optimal({double(w)}),sine(best.begin()+4,best.end());
        for(int bits=0;bits<512;++bits){
            Vec x(5),y(5),ex(5);double p=0;
            for(int j=0;j<5;++j){
                if(j)p+=.01*((bits&(1<<(j-1)))?1:-1);
                ex[j]=eta*((bits&(1<<(4+j)))?1:-1);
                x[j]=p+ex[j];y[j]=1.5*p-.5*ex[j];
            }
            Vec raw{double(k)};for(int j=0;j<5;++j)raw.insert(raw.end(),{x[j],y[j]});
            Vec r=two_scale_unknown(raw);for(int j=0;j<6;++j)a[j]+=r[j]/512;
            for(int t=0;t<=4-w;++t){
                double u=0,v=0;for(int j=0;j<w;++j){double e=ex[t+j+1]-ex[t+j];u+=e/std::sqrt(double(w));v+=sine[j]*e;}
                u2+=u*u/(512*(5-w));v2+=v*v/(512*(5-w));
            }
        }
        Vec out{eta,double(k),double(w)};out.insert(out.end(),a.begin(),a.end());
        out.insert(out.end(),{2./w,best[1],u2,v2,512});row(out);
    }
}
void validation_project(){
    std::cout<<"Groups,TestGroups,LabelLength,Embargo,Split,Paths,Test,Train,Purged,Embargoed\n";
    for(int N:{3,4,5,6})for(int K=1;K<N;++K)for(int length:{1,3,5})for(int gap:{0,2}){
        Vec x{double(N),double(K),double(gap)};
        for(int g=0;g<N;++g)for(int j=0;j<3;++j){double a=3*g+j;x.insert(x.end(),{double(g),a,a+length});}
        Vec z=cpcv_intervals(x);for(int f=0;f<int(z[0]);++f)
            row({double(N),double(K),double(length),double(gap),double(f),z[1],z[4+4*f],z[5+4*f],z[6+4*f],z[7+4*f]});
    }
}
void cluster_project(){
    std::cout<<"AssetScale,DateScale,IdioScale,MissingCell,N,Alpha,Beta,RSS,AssetVariance,DateVariance,IntersectionVariance,TwoWayVariance,TruncatedSE\n";
    for(double a:{0.,.2,.5})for(double b:{0.,.2,.5})for(double e:{.1,.3,.6})for(int missing:{0,1}){
        Vec x;
        for(int i=0;i<5;++i)for(int t=0;t<6;++t){
            if(missing&&i==1&&t==2)continue;
            double z=(i-2)*.4+(t-2.5)*.2+.1*((i+t)%3);
            double y=.05+.7*z+a*(i-2)+b*(t-2.5)+e*((3*i+2*t)%5-2);
            x.insert(x.end(),{double(i),double(t),z,y});
        }
        Vec r=two_way_cluster(x),out{a,b,e,double(missing),double(x.size()/4)};out.insert(out.end(),r.begin(),r.begin()+8);row(out);
    }
}
void block_project(){
    std::cout<<"Dates,Block,Effect,Persistence,SharpePaths,ValidPaths,UndefinedMass,SharpeMean,SharpeSD,Sharpe025,SharpeMedian,Sharpe975,ObservedMax,RealityP,NullMean,NullSD\n";
    for(int n:{6,8,12})for(int k:{2,3,4,6})if(n%k==0&&n/k<=3)for(double effect:{0.,.2,.5})for(int persist:{0,1}){
        Vec a{double(k)},b{double(n),3,double(k)};double state=0;
        for(int t=0;t<n;++t){
            double u=((5*t+1)%7-3)*.1;state=(persist?.7:0)*state+u;
            double r=state+effect;
            a.push_back(r);b.insert(b.end(),{r,state+.5*effect+.1*((t+2)%3-1),-.5*state+effect*.25+.05*((2*t)%3-1)});
        }
        Vec s=circular_sharpe_distribution(a),z=reality_check_circular(b),out{double(n),double(k),effect,double(persist)};
        out.insert(out.end(),s.begin(),s.end());out.insert(out.end(),z.begin(),z.begin()+4);row(out);
    }
}
int main(int argc,char** argv){
    std::cout<<std::setprecision(17);
    try{
        require(argc==2);std::map<std::string,void(*)()> p={{"budget",budget_project},{"allocation",allocation_project},{"noise",noise_project},{"validation",validation_project},{"cluster",cluster_project},{"blocks",block_project}};
        auto it=p.find(argv[1]);require(it!=p.end());it->second();
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
