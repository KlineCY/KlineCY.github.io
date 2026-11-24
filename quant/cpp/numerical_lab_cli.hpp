#pragma once
#include "numerical_lab_options.hpp"
#include "numerical_lab_rates.hpp"
#include "numerical_lab_optim.hpp"
#include "numerical_lab_inference.hpp"
#include "numerical_lab_learning.hpp"
#include "numerical_lab_simulation.hpp"
#include "numerical_lab_factors.hpp"
#include "numerical_lab_market.hpp"
namespace kline {
inline bool run_numerical_lab_cli(const std::string& command,int argc,char** argv,std::ostream& out){
    using Function=researchflow::Report(*)(const researchflow::Mat&,const researchflow::Vec&);
    Function fn=nullptr;
    if(command=="num-pde-european")fn=numerical_lab::pde_european;
    if(command=="num-pde-barrier")fn=numerical_lab::pde_barrier;
    if(command=="num-american-psor")fn=numerical_lab::american_psor;
    if(command=="num-svi-slice")fn=numerical_lab::svi_slice;
    if(command=="num-calendar-grid")fn=numerical_lab::calendar_grid;
    if(command=="num-nelson-siegel")fn=numerical_lab::nelson_siegel;
    if(command=="num-keyrate-curve")fn=numerical_lab::keyrate_curve;
    if(command=="num-bond-cashflow-risk")fn=numerical_lab::bond_cashflow_risk;
    if(command=="num-cashflow-hedge")fn=numerical_lab::cashflow_hedge;
    if(command=="num-inflation-indexed")fn=numerical_lab::inflation_indexed;
    if(command=="num-box-markowitz")fn=numerical_lab::box_markowitz;
    if(command=="num-turnover-admm")fn=numerical_lab::turnover_admm;
    if(command=="num-equality-quadratic")fn=numerical_lab::equality_quadratic;
    if(command=="num-robust-mean-ellipsoid")fn=numerical_lab::robust_mean_ellipsoid;
    if(command=="num-exposure-frontier")fn=numerical_lab::exposure_frontier;
    if(command=="num-multivariate-hc3")fn=numerical_lab::multivariate_hc3;
    if(command=="num-multivariate-hac")fn=numerical_lab::multivariate_hac;
    if(command=="num-multivariate-cluster")fn=numerical_lab::multivariate_cluster;
    if(command=="num-wild-coefficient")fn=numerical_lab::wild_coefficient;
    if(command=="num-joint-wald-hc0")fn=numerical_lab::joint_wald_hc0;
    if(command=="num-damped-logistic")fn=numerical_lab::damped_logistic;
    if(command=="num-ridge-press")fn=numerical_lab::ridge_press;
    if(command=="num-recursive-least-squares")fn=numerical_lab::recursive_least_squares;
    if(command=="num-gaussian-discriminant")fn=numerical_lab::gaussian_discriminant;
    if(command=="num-pca-jacobi")fn=numerical_lab::pca_jacobi;
    if(command=="num-antithetic-gbm")fn=numerical_lab::antithetic_gbm;
    if(command=="num-geometric-control")fn=numerical_lab::geometric_control;
    if(command=="num-gbm-path-audit")fn=numerical_lab::gbm_path_audit;
    if(command=="num-bridge-survival")fn=numerical_lab::bridge_survival;
    if(command=="num-heston-paths")fn=numerical_lab::heston_paths;
    if(command=="num-fama-macbeth-multi")fn=numerical_lab::fama_macbeth_multi;
    if(command=="num-weighted-neutralization")fn=numerical_lab::weighted_neutralization;
    if(command=="num-event-market-model")fn=numerical_lab::event_market_model;
    if(command=="num-independent-double-sort")fn=numerical_lab::independent_double_sort;
    if(command=="num-horizon-rank-ic")fn=numerical_lab::horizon_rank_ic;
    if(command=="num-best-quote-ofi")fn=numerical_lab::best_quote_ofi;
    if(command=="num-trade-sign-quote-tick")fn=numerical_lab::trade_sign_quote_tick;
    if(command=="num-preaveraging-variation")fn=numerical_lab::preaveraging_variation;
    if(command=="num-time-markout")fn=numerical_lab::time_markout;
    if(command=="num-depth-liquidity-sweep")fn=numerical_lab::depth_liquidity_sweep;
    if(!fn)return false;
    require(argc>=3,"Numerical laboratory requires a numeric CSV");
    researchflow::Vec p;
    for(int i=3;i<argc;++i)p.push_back(parse_number(argv[i]));
    researchflow::write(out,fn(researchflow::read_matrix(argv[2]),p));
    return true;
}
inline void numerical_lab_usage(std::ostream& out){
    out<<"Numerical research laboratories: num-METHOD CSV [PARAMETERS...]\n";
    out<<"  num-pde-european CSV PARAMETERS...\n";
    out<<"  num-pde-barrier CSV PARAMETERS...\n";
    out<<"  num-american-psor CSV PARAMETERS...\n";
    out<<"  num-svi-slice CSV PARAMETERS...\n";
    out<<"  num-calendar-grid CSV PARAMETERS...\n";
    out<<"  num-nelson-siegel CSV PARAMETERS...\n";
    out<<"  num-keyrate-curve CSV PARAMETERS...\n";
    out<<"  num-bond-cashflow-risk CSV PARAMETERS...\n";
    out<<"  num-cashflow-hedge CSV PARAMETERS...\n";
    out<<"  num-inflation-indexed CSV PARAMETERS...\n";
    out<<"  num-box-markowitz CSV PARAMETERS...\n";
    out<<"  num-turnover-admm CSV PARAMETERS...\n";
    out<<"  num-equality-quadratic CSV PARAMETERS...\n";
    out<<"  num-robust-mean-ellipsoid CSV PARAMETERS...\n";
    out<<"  num-exposure-frontier CSV PARAMETERS...\n";
    out<<"  num-multivariate-hc3 CSV\n";
    out<<"  num-multivariate-hac CSV PARAMETERS...\n";
    out<<"  num-multivariate-cluster CSV\n";
    out<<"  num-wild-coefficient CSV PARAMETERS...\n";
    out<<"  num-joint-wald-hc0 CSV PARAMETERS...\n";
    out<<"  num-damped-logistic CSV PARAMETERS...\n";
    out<<"  num-ridge-press CSV PARAMETERS...\n";
    out<<"  num-recursive-least-squares CSV PARAMETERS...\n";
    out<<"  num-gaussian-discriminant CSV PARAMETERS...\n";
    out<<"  num-pca-jacobi CSV PARAMETERS...\n";
    out<<"  num-antithetic-gbm CSV PARAMETERS...\n";
    out<<"  num-geometric-control CSV PARAMETERS...\n";
    out<<"  num-gbm-path-audit CSV PARAMETERS...\n";
    out<<"  num-bridge-survival CSV PARAMETERS...\n";
    out<<"  num-heston-paths CSV PARAMETERS...\n";
    out<<"  num-fama-macbeth-multi CSV PARAMETERS...\n";
    out<<"  num-weighted-neutralization CSV\n";
    out<<"  num-event-market-model CSV PARAMETERS...\n";
    out<<"  num-independent-double-sort CSV\n";
    out<<"  num-horizon-rank-ic CSV\n";
    out<<"  num-best-quote-ofi CSV\n";
    out<<"  num-trade-sign-quote-tick CSV\n";
    out<<"  num-preaveraging-variation CSV PARAMETERS...\n";
    out<<"  num-time-markout CSV PARAMETERS...\n";
    out<<"  num-depth-liquidity-sweep CSV PARAMETERS...\n";
}
}
