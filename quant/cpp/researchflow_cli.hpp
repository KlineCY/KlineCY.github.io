#pragma once
#include "researchflow_portfolio.hpp"
#include "researchflow_ml.hpp"
#include "researchflow_timeseries.hpp"
#include "researchflow_risk.hpp"
#include "researchflow_execution.hpp"
#include <ostream>
#include <string>
namespace kline {
inline bool run_researchflow_cli(const std::string& command,int argc,char** argv,std::ostream& out){
    using researchflow::Mat;
    using researchflow::Vec;
    using Func=researchflow::Report(*)(const Mat&,const Vec&);
    Func fn=nullptr;
    if(command=="flow-cta")fn=researchflow::cta;
    if(command=="flow-cross-section")fn=researchflow::cross_section;
    if(command=="flow-minimum-variance")fn=researchflow::minimum_variance;
    if(command=="flow-equal-risk")fn=researchflow::equal_risk;
    if(command=="flow-performance")fn=researchflow::performance;
    if(command=="flow-rolling-ridge")fn=researchflow::rolling_ridge;
    if(command=="flow-nested-ridge")fn=researchflow::nested_ridge;
    if(command=="flow-conformal-rolling")fn=researchflow::conformal_rolling;
    if(command=="flow-rolling-calibration")fn=researchflow::rolling_calibration;
    if(command=="flow-forecast-comparison")fn=researchflow::forecast_comparison;
    if(command=="flow-rolling-ar")fn=researchflow::rolling_ar;
    if(command=="flow-rolling-var")fn=researchflow::rolling_var;
    if(command=="flow-volatility-forecast")fn=researchflow::volatility_forecast;
    if(command=="flow-regime-forecast")fn=researchflow::regime_forecast;
    if(command=="flow-filter-forecast")fn=researchflow::filter_forecast;
    if(command=="flow-rolling-tail")fn=researchflow::rolling_tail;
    if(command=="flow-coverage")fn=researchflow::coverage;
    if(command=="flow-moving-block-paths")fn=researchflow::moving_block_paths;
    if(command=="flow-factor-attribution")fn=researchflow::factor_attribution;
    if(command=="flow-risk-decomposition")fn=researchflow::risk_decomposition;
    if(command=="flow-order-tca")fn=researchflow::order_tca;
    if(command=="flow-participation-audit")fn=researchflow::participation_audit;
    if(command=="flow-spread-decomposition")fn=researchflow::spread_decomposition;
    if(command=="flow-optimal-participation")fn=researchflow::optimal_participation;
    if(command=="flow-competing-fill")fn=researchflow::competing_fill;
    if(!fn)return false;
    require(argc>=3,"Research workflow requires a numeric CSV");
    Vec p;for(int i=3;i<argc;++i)p.push_back(parse_number(argv[i]));
    researchflow::write(out,fn(researchflow::read_matrix(argv[2]),p));
    return true;
}
inline void researchflow_usage(std::ostream& out){
    out<<"Reproducible research flows: flow-METHOD CSV [PARAMETERS...]\n";
    out<<"  flow-cta CSV PARAMETERS...\n";
    out<<"  flow-cross-section CSV PARAMETERS...\n";
    out<<"  flow-minimum-variance CSV PARAMETERS...\n";
    out<<"  flow-equal-risk CSV PARAMETERS...\n";
    out<<"  flow-performance CSV PARAMETERS...\n";
    out<<"  flow-rolling-ridge CSV PARAMETERS...\n";
    out<<"  flow-nested-ridge CSV PARAMETERS...\n";
    out<<"  flow-conformal-rolling CSV PARAMETERS...\n";
    out<<"  flow-rolling-calibration CSV PARAMETERS...\n";
    out<<"  flow-forecast-comparison CSV PARAMETERS...\n";
    out<<"  flow-rolling-ar CSV PARAMETERS...\n";
    out<<"  flow-rolling-var CSV PARAMETERS...\n";
    out<<"  flow-volatility-forecast CSV PARAMETERS...\n";
    out<<"  flow-regime-forecast CSV PARAMETERS...\n";
    out<<"  flow-filter-forecast CSV PARAMETERS...\n";
    out<<"  flow-rolling-tail CSV PARAMETERS...\n";
    out<<"  flow-coverage CSV PARAMETERS...\n";
    out<<"  flow-moving-block-paths CSV PARAMETERS...\n";
    out<<"  flow-factor-attribution CSV PARAMETERS...\n";
    out<<"  flow-risk-decomposition CSV PARAMETERS...\n";
    out<<"  flow-order-tca CSV\n";
    out<<"  flow-participation-audit CSV PARAMETERS...\n";
    out<<"  flow-spread-decomposition CSV\n";
    out<<"  flow-optimal-participation CSV PARAMETERS...\n";
    out<<"  flow-competing-fill CSV\n";
}
}
