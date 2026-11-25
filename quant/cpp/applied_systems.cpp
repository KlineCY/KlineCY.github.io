#include "applied_synthesis.hpp"
using Model=Vec(*)(const Vec&);
const std::map<std::string,Model> models={
    {"forecast_brier_groups",applied_forecast::brier_groups},
    {"forecast_pinball",applied_forecast::pinball},
    {"forecast_ensemble_crps",applied_forecast::ensemble_crps},
    {"forecast_split_conformal",applied_forecast::split_conformal},
    {"forecast_isotonic_fit",applied_forecast::isotonic_fit},
    {"forecast_interval_score",applied_forecast::interval_score},
    {"forecast_frozen_calibration",applied_forecast::frozen_calibration},
    {"inference_pit_ks",applied_inference::pit_ks},
    {"inference_pit_cvm",applied_inference::pit_cvm},
    {"inference_pit_ad",applied_inference::pit_ad},
    {"inference_runs_exact",applied_inference::runs_exact},
    {"inference_circular_boot_var",applied_inference::circular_boot_var},
    {"inference_ljung_box",applied_inference::ljung_box},
    {"inference_pit_joint",applied_inference::pit_joint},
    {"liquidity_microprice",applied_liquidity::microprice},
    {"liquidity_sweep_asks",applied_liquidity::sweep_asks},
    {"liquidity_pro_rata_lots",applied_liquidity::pro_rata_lots},
    {"liquidity_auction_cross",applied_liquidity::auction_cross},
    {"liquidity_ofi_event",applied_liquidity::ofi_event},
    {"liquidity_passive_markout",applied_liquidity::passive_markout},
    {"liquidity_sweep_shortfall",applied_liquidity::sweep_shortfall},
    {"rates_continuous_duration",applied_rates::continuous_duration},
    {"rates_par_coupon",applied_rates::par_coupon},
    {"rates_key_rate_pv",applied_rates::key_rate_pv},
    {"rates_yield_bisect",applied_rates::yield_bisect},
    {"rates_forward_discount",applied_rates::forward_discount},
    {"rates_two_zero_immunize",applied_rates::two_zero_immunize},
    {"rates_curve_shock",applied_rates::curve_shock},
    {"options_one_step_claim",applied_options::one_step_claim},
    {"options_american_tree",applied_options::american_tree},
    {"options_digital_vertical",applied_options::digital_vertical},
    {"options_uneven_butterfly",applied_options::uneven_butterfly},
    {"options_discrete_hedge",applied_options::discrete_hedge},
    {"options_delta_vega_neutral",applied_options::delta_vega_neutral},
    {"options_two_step_replication",applied_options::two_step_replication},
    {"credit_hazard_curve",applied_credit::hazard_curve},
    {"credit_risky_zero",applied_credit::risky_zero},
    {"credit_cds_discrete",applied_credit::cds_discrete},
    {"credit_net_collateral",applied_credit::net_collateral},
    {"credit_cva_independent",applied_credit::cva_independent},
    {"credit_wrong_way",applied_credit::wrong_way},
    {"credit_credit_curve_pipeline",applied_credit::credit_curve_pipeline},
    {"accounts_twr_exact",applied_accounts::twr_exact},
    {"accounts_modified_dietz",applied_accounts::modified_dietz},
    {"accounts_mwr_continuous",applied_accounts::mwr_continuous},
    {"accounts_high_water_fee",applied_accounts::high_water_fee},
    {"accounts_futures_margin",applied_accounts::futures_margin},
    {"accounts_borrow_accrual",applied_accounts::borrow_accrual},
    {"accounts_flow_performance_pipeline",applied_accounts::flow_performance_pipeline},
    {"allocation_neutral_projection",applied_allocation::neutral_projection},
    {"allocation_l1_box_trade",applied_allocation::l1_box_trade},
    {"allocation_kl_reverse_stress",applied_allocation::kl_reverse_stress},
    {"allocation_tail_euler",applied_allocation::tail_euler},
    {"allocation_two_asset_risk_budget",applied_allocation::two_asset_risk_budget},
    {"allocation_robust_spread",applied_allocation::robust_spread},
    {"allocation_neutral_risk_pipeline",applied_allocation::neutral_risk_pipeline},
    {"synthesis_forecast_serial",applied_synthesis::forecast_serial},
    {"synthesis_conformal_external",applied_synthesis::conformal_external},
    {"synthesis_book_markout",applied_synthesis::book_markout},
    {"synthesis_auction_arrival",applied_synthesis::auction_arrival},
    {"synthesis_rate_credit_shock",applied_synthesis::rate_credit_shock},
    {"synthesis_immunization_shock",applied_synthesis::immunization_shock},
    {"synthesis_replication_path",applied_synthesis::replication_path},
    {"synthesis_surface_probabilities",applied_synthesis::surface_probabilities},
    {"synthesis_netting_cva",applied_synthesis::netting_cva},
    {"synthesis_dependent_cva",applied_synthesis::dependent_cva},
    {"synthesis_investor_manager_return",applied_synthesis::investor_manager_return},
    {"synthesis_futures_borrow_funding",applied_synthesis::futures_borrow_funding},
    {"synthesis_budget_tail",applied_synthesis::budget_tail},
    {"synthesis_kl_robust",applied_synthesis::kl_robust},
    {"synthesis_cost_tail",applied_synthesis::cost_tail},
    {"synthesis_execution_wealth",applied_synthesis::execution_wealth}
};
double number(const std::string& s){
    size_t n=0;double v=std::stod(s,&n);
    check(n==s.size()&&std::isfinite(v));
    return v;
}
void calculate(const std::string& id,const Vec& x){
    auto it=models.find(id);check(it!=models.end());
    check(x.size()<=100000);
    Vec out=it->second(x);
    check(!out.empty());
    for(double v:out)check(std::isfinite(v));
    for(size_t i=0;i<out.size();++i){if(i)std::cout<<',';std::cout<<out[i];}
    std::cout<<'\n';
}
int main(int argc,char** argv){
    std::cout<<std::setprecision(17);
    try{
        check(argc>=2);
        std::string id=argv[1];
        if(id=="--list"){
            check(argc==2);
            for(const auto& p:models)std::cout<<p.first<<'\n';
            return 0;
        }
        if(id=="--batch"){
            check(argc==2);std::string line;
            while(std::getline(std::cin,line)){
                try{std::istringstream s(line);std::string token,method;check(bool(s>>method));Vec x;while(s>>token)x.push_back(number(token));calculate(method,x);}
                catch(const std::exception&){std::cout<<"ERROR\n";}
            }
            return 0;
        }
        Vec x;
        for(int i=2;i<argc;++i)x.push_back(number(argv[i]));
        calculate(id,x);return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
