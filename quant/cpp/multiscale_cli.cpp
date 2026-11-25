#include "multiscale_simulation.hpp"
#include "multiscale_microstructure.hpp"
#include "multiscale_engineering.hpp"
#include "multiscale_empirical.hpp"
using namespace msq;
using Method=Vec(*)(const Vec&);
const std::map<std::string,Method> methods={
    {"mlmc_plan",msq::mlmc_plan},
    {"mlmc_estimate",msq::mlmc_estimate},
    {"asian_coupling",msq::asian_coupling},
    {"asian_moments",msq::asian_moments},
    {"stratified_call",msq::stratified_call},
    {"greek_estimators",msq::greek_estimators},
    {"bridge_path",msq::bridge_path},
    {"frozen_control",msq::frozen_control},
    {"hy_covariance",msq::hy_covariance},
    {"refresh_covariance",msq::refresh_covariance},
    {"hy_shift_grid",msq::hy_shift_grid},
    {"bipower_variation",msq::bipower_variation},
    {"noise_moments",msq::noise_moments},
    {"lag_cov_noise",msq::lag_cov_noise},
    {"weighted_preaverage",msq::weighted_preaverage},
    {"volume_bars",msq::volume_bars},
    {"membership_vintage",msq::membership_vintage},
    {"delist_wealth",msq::delist_wealth},
    {"delayed_orders",msq::delayed_orders},
    {"fee_rebalance",msq::fee_rebalance},
    {"matured_ridge",msq::matured_ridge},
    {"settlement_cash",msq::settlement_cash},
    {"trade_revisions",msq::trade_revisions},
    {"fractional_rank",msq::fractional_rank},
    {"hj_bound",msq::hj_bound},
    {"factor_spanning",msq::factor_spanning},
    {"timing_regression",msq::timing_regression},
    {"sdf_distance",msq::sdf_distance},
    {"joint_sign_stepdown",msq::joint_sign_stepdown},
    {"fdr_stepup",msq::fdr_stepup},
    {"sharpe_hac",msq::sharpe_hac},
    {"rank_ic_panel",msq::rank_ic_panel}
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
