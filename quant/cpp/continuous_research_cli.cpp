#include "continuous_research_filter.hpp"
#include "continuous_research_pricing.hpp"
using namespace crq;
using Method=Vec(*)(const Vec&);
const std::map<std::string,Method> methods={
    {"kalman_missing",crq::kalman_missing},
    {"nonlinear_gaussian_moments",crq::nonlinear_gaussian_moments},
    {"gaussian_density_filter",crq::gaussian_density_filter},
    {"ensemble_analysis",crq::ensemble_analysis},
    {"bs_rannacher",crq::bs_rannacher},
    {"tridiagonal_adjoint",crq::tridiagonal_adjoint},
    {"spread_conditional",crq::spread_conditional},
    {"ou_coupling",crq::ou_coupling}
};
double number(const std::string& s){size_t p=0;double z=std::stod(s,&p);check(p==s.size()&&std::isfinite(z));return z;}
void run(const std::string& id,const Vec& x){auto it=methods.find(id);check(it!=methods.end()&&x.size()<=100000);Vec y=it->second(x);for(double z:y)check(std::isfinite(z));for(size_t j=0;j<y.size();++j){if(j)std::cout<<',';std::cout<<y[j];}std::cout<<'\n';}
int main(int argc,char** argv){
    std::cout<<std::setprecision(17);
    try{
        check(argc>=2);std::string id=argv[1];
        if(id=="--list"){check(argc==2);for(const auto& a:methods)std::cout<<a.first<<'\n';return 0;}
        if(id=="--batch"){
            check(argc==2);std::string line;
            while(std::getline(std::cin,line)){
                try{std::istringstream s(line);std::string m,z;check(bool(s>>m));Vec x;while(s>>z)x.push_back(number(z));run(m,x);}
                catch(const std::exception&){std::cout<<"ERROR\n";}
            }return 0;
        }
        Vec x;for(int j=2;j<argc;++j)x.push_back(number(argv[j]));run(id,x);return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
