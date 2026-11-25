#include "finite_state_inference.hpp"
#include "finite_state_credit.hpp"
#include "finite_state_derivatives.hpp"
#include "finite_state_policy.hpp"
#include "finite_state_exact.hpp"
#include "finite_state_queue.hpp"
#include "finite_state_optimization.hpp"
#include "finite_state_control.hpp"
using namespace fsq;
using Method=Vec(*)(const Vec&);
const std::map<std::string,Method> methods={
    {"duration_study",fsq::duration_study},
    {"segmentation",fsq::segmentation},
    {"bayes_break",fsq::bayes_break},
    {"two_way_within",fsq::two_way_within},
    {"count_credit",fsq::count_credit},
    {"loss_distribution",fsq::loss_distribution},
    {"tranche_sensitivity",fsq::tranche_sensitivity},
    {"mixture_count",fsq::mixture_count},
    {"barrier_rebate",fsq::barrier_rebate},
    {"swing_rights",fsq::swing_rights},
    {"lookback_max",fsq::lookback_max},
    {"state_price_chain",fsq::state_price_chain},
    {"inventory_schedule",fsq::inventory_schedule},
    {"lot_rebalance",fsq::lot_rebalance},
    {"regime_policy",fsq::regime_policy},
    {"drawdown_chain",fsq::drawdown_chain},
    {"stratified_exact",fsq::stratified_exact},
    {"rank_sum_exact",fsq::rank_sum_exact},
    {"signed_rank_exact",fsq::signed_rank_exact},
    {"sequential_test",fsq::sequential_test},
    {"tagged_queue",fsq::tagged_queue},
    {"queue_regime",fsq::queue_regime},
    {"queue_execution_choice",fsq::queue_execution_choice},
    {"queue_batch_fill",fsq::queue_batch_fill},
    {"transport_flow",fsq::transport_flow},
    {"band_dtw",fsq::band_dtw},
    {"assignment",fsq::assignment},
    {"wasserstein",fsq::wasserstein},
    {"mdp_optimal",fsq::mdp_optimal},
    {"policy_iteration",fsq::policy_iteration},
    {"worst_tv",fsq::worst_tv},
    {"stopping_policy",fsq::stopping_policy}
};
double number(const std::string& s){
    size_t pos=0;double v=std::stod(s,&pos);require(pos==s.size()&&std::isfinite(v));return v;
}
void run(const std::string& id,const Vec& x){
    auto it=methods.find(id);require(it!=methods.end()&&x.size()<=100000);
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
            }
            return 0;
        }
        Vec x;for(int i=2;i<argc;++i)x.push_back(number(argv[i]));run(id,x);
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
