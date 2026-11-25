#include "predictive_online.hpp"
#include "predictive_portfolio.hpp"
using namespace pdq;
using Method=Vec(*)(const Vec&);
const std::map<std::string,Method> methods={
    {"kernel_center",pdq::kernel_center},
    {"fourier_gram",pdq::fourier_gram},
    {"nystrom_ridge",pdq::nystrom_ridge},
    {"ridge_loo",pdq::ridge_loo},
    {"gp_derivative",pdq::gp_derivative},
    {"harmonic_graph",pdq::harmonic_graph},
    {"delayed_ogd",pdq::delayed_ogd},
    {"fixed_share",pdq::fixed_share},
    {"recursive_ridge",pdq::recursive_ridge},
    {"passive_aggressive",pdq::passive_aggressive},
    {"adaptive_miscoverage",pdq::adaptive_miscoverage},
    {"exp3_observed",pdq::exp3_observed},
    {"sparsemax_vjp",pdq::sparsemax_vjp},
    {"layernorm_vjp",pdq::layernorm_vjp},
    {"rmsnorm_vjp",pdq::rmsnorm_vjp},
    {"gelu_vjp",pdq::gelu_vjp},
    {"causal_attention_vjp",pdq::causal_attention_vjp},
    {"infonce_vjp",pdq::infonce_vjp},
    {"stacking_simplex",pdq::stacking_simplex},
    {"neutral_layer_vjp",pdq::neutral_layer_vjp},
    {"spo_plus",pdq::spo_plus},
    {"policy_dr",pdq::policy_dr},
    {"soft_portfolio_vjp",pdq::soft_portfolio_vjp},
    {"ensemble_risk",pdq::ensemble_risk}
};
double number(const std::string& s){size_t pos=0;double v=std::stod(s,&pos);require(pos==s.size()&&std::isfinite(v));return v;}
void run(const std::string& id,const Vec& x){
    auto it=methods.find(id);require(it!=methods.end()&&x.size()<=300000);
    Vec y=it->second(x);require(!y.empty());
    for(double v:y)require(std::isfinite(v));
    for(size_t i=0;i<y.size();++i){if(i)std::cout<<',';std::cout<<y[i];}
    std::cout<<'\n';
}
int main(int argc,char** argv){
    std::cout<<std::setprecision(17);
    try{
        require(argc>=2);std::string id=argv[1];
        if(id=="--list"){require(argc==2);for(const auto& a:methods)std::cout<<a.first<<'\n';return 0;}
        if(id=="--batch"){
            require(argc==2);std::string line;
            while(std::getline(std::cin,line)){
                try{std::istringstream s(line);std::string m,v;require(bool(s>>m));Vec x;while(s>>v)x.push_back(number(v));run(m,x);}
                catch(const std::exception&){std::cout<<"ERROR\n";}
            }return 0;
        }
        Vec x;for(int i=2;i<argc;++i)x.push_back(number(argv[i]));run(id,x);
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
