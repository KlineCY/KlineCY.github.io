#include "integrated_projects.hpp"
#include <iostream>
#include <string>

int main(int argc,char** argv){
    try{
        if(argc<3){
            std::cerr<<"Usage: kline_integrated PROJECT CSV PARAMETERS...\n"
                     <<"  nested-panel-risk-strategy\n  repaired-option-strip\n"
                     <<"  factor-filter-market-neutral\n  order-replay-cost-report\n";
            return 2;
        }
        std::string name=argv[1];
        auto data=kline::researchflow::read_matrix(argv[2]);
        kline::researchflow::Vec p;
        for(int i=3;i<argc;++i)p.push_back(kline::parse_number(argv[i]));
        kline::researchflow::Report report;
        if(name=="nested-panel-risk-strategy")report=kline::integrated::nested_panel_risk_strategy(data,p);
        else if(name=="repaired-option-strip")report=kline::integrated::repaired_option_strip(data,p);
        else if(name=="factor-filter-market-neutral")report=kline::integrated::factor_filter_market_neutral(data,p);
        else if(name=="order-replay-cost-report")report=kline::integrated::order_replay_cost_report(data,p);
        else throw std::runtime_error("Unknown project");
        kline::researchflow::write(std::cout,report);
        return 0;
    }catch(const std::exception& e){
        std::cerr<<e.what()<<'\n';return 1;
    }
}
