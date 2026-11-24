#pragma once
#include "lab24_linear.hpp"
#include "lab24_optim.hpp"
#include "lab24_network.hpp"
#include "lab24_state.hpp"
#include "lab24_panel.hpp"
#include "lab24_learning.hpp"
#include "lab24_rates.hpp"
#include "lab24_derivative.hpp"
#include "lab24_credit.hpp"
#include "lab24_portfolio.hpp"
#include "lab24_execution.hpp"
#include "lab24_data.hpp"
#include "io.hpp"
#include <iomanip>
#include <ostream>
#include <string>
namespace kline {
inline bool run_lab24_cli(const std::string& command,int argc,char** argv,std::ostream& out){
    using lab24::Mat;using lab24::Vec;
    using Function=Vec(*)(const Mat&,const Vec&);
    Function fn=nullptr;
    if(command=="lab24-linear-qr-solve")fn=lab24::qr_solve;
    if(command=="lab24-linear-spectral2")fn=lab24::spectral2;
    if(command=="lab24-linear-ridge2")fn=lab24::ridge2;
    if(command=="lab24-linear-woodbury-rank1")fn=lab24::woodbury_rank1;
    if(command=="lab24-linear-power-iteration")fn=lab24::power_iteration;
    if(command=="lab24-optim-simplex-projection")fn=lab24::simplex_projection;
    if(command=="lab24-optim-l1-ball")fn=lab24::l1_ball;
    if(command=="lab24-optim-box-budget")fn=lab24::box_budget;
    if(command=="lab24-optim-elastic-coordinate")fn=lab24::elastic_coordinate;
    if(command=="lab24-optim-entropy-tilt")fn=lab24::entropy_tilt;
    if(command=="lab24-network-laplacian-energy")fn=lab24::laplacian_energy;
    if(command=="lab24-network-pagerank")fn=lab24::pagerank;
    if(command=="lab24-network-diffusion")fn=lab24::diffusion;
    if(command=="lab24-network-minimax-path")fn=lab24::minimax_path;
    if(command=="lab24-network-minimum-spanning-tree")fn=lab24::minimum_spanning_tree;
    if(command=="lab24-state-scalar-kalman")fn=lab24::scalar_kalman;
    if(command=="lab24-state-hmm-forward")fn=lab24::hmm_forward;
    if(command=="lab24-state-viterbi")fn=lab24::viterbi;
    if(command=="lab24-state-ou-transition-fit")fn=lab24::ou_transition_fit;
    if(command=="lab24-state-markov-hitting")fn=lab24::markov_hitting;
    if(command=="lab24-panel-within-panel")fn=lab24::within_panel;
    if(command=="lab24-panel-cluster-ols")fn=lab24::cluster_ols;
    if(command=="lab24-panel-paired-t")fn=lab24::paired_t;
    if(command=="lab24-panel-randomization-exact")fn=lab24::randomization_exact;
    if(command=="lab24-panel-iv-first-stage")fn=lab24::iv_first_stage;
    if(command=="lab24-learning-online-logistic")fn=lab24::online_logistic;
    if(command=="lab24-learning-adagrad-ridge")fn=lab24::adagrad_ridge;
    if(command=="lab24-learning-brier-decomposition")fn=lab24::brier_decomposition;
    if(command=="lab24-learning-isotonic-pav")fn=lab24::isotonic_pav;
    if(command=="lab24-learning-stacking-two")fn=lab24::stacking_two;
    if(command=="lab24-rates-discount-bootstrap")fn=lab24::discount_bootstrap;
    if(command=="lab24-rates-forward-curve")fn=lab24::forward_curve;
    if(command=="lab24-rates-curve-keyrate")fn=lab24::curve_keyrate;
    if(command=="lab24-rates-duration-immunize")fn=lab24::duration_immunize;
    if(command=="lab24-rates-yield-newton")fn=lab24::yield_newton;
    if(command=="lab24-derivative-crr-american")fn=lab24::crr_american;
    if(command=="lab24-derivative-asian-path")fn=lab24::asian_path;
    if(command=="lab24-derivative-butterfly-density")fn=lab24::butterfly_density;
    if(command=="lab24-derivative-variance-strip")fn=lab24::variance_strip;
    if(command=="lab24-derivative-discrete-delta-pnl")fn=lab24::discrete_delta_pnl;
    if(command=="lab24-credit-migration-two")fn=lab24::migration_two;
    if(command=="lab24-credit-poisson-binomial")fn=lab24::poisson_binomial;
    if(command=="lab24-credit-loss-convolution")fn=lab24::loss_convolution;
    if(command=="lab24-credit-survival-hazard")fn=lab24::survival_hazard;
    if(command=="lab24-credit-discrete-cds-spread")fn=lab24::discrete_cds_spread;
    if(command=="lab24-portfolio-factor-neutral-two")fn=lab24::factor_neutral_two;
    if(command=="lab24-portfolio-risk-budget-diag")fn=lab24::risk_budget_diag;
    if(command=="lab24-portfolio-factor-cov-risk")fn=lab24::factor_cov_risk;
    if(command=="lab24-portfolio-turnover-prox")fn=lab24::turnover_prox;
    if(command=="lab24-portfolio-rebalance-drift")fn=lab24::rebalance_drift;
    if(command=="lab24-execution-ac-schedule")fn=lab24::ac_schedule;
    if(command=="lab24-execution-participation-schedule")fn=lab24::participation_schedule;
    if(command=="lab24-execution-queue-fifo")fn=lab24::queue_fifo;
    if(command=="lab24-execution-multi-venue-sweep")fn=lab24::multi_venue_sweep;
    if(command=="lab24-execution-impact-decay-fit")fn=lab24::impact_decay_fit;
    if(command=="lab24-data-asof-age")fn=lab24::asof_age;
    if(command=="lab24-data-purged-mask")fn=lab24::purged_mask;
    if(command=="lab24-data-label-uniqueness")fn=lab24::label_uniqueness;
    if(command=="lab24-data-revision-latest")fn=lab24::revision_latest;
    if(command=="lab24-data-event-time-bars")fn=lab24::event_time_bars;
    if(!fn)return false;
    require(argc>=3,"A numeric matrix CSV is required");
    const auto data=read_csv(argv[2]);
    Mat x;
    x.reserve(data.rows.size());
    for(const auto& row:data.rows){
        Vec values;
        for(const auto& field:row)values.push_back(parse_number(field));
        x.push_back(std::move(values));
    }
    Vec p;
    for(int j=3;j<argc;++j)p.push_back(parse_number(argv[j]));
    const Vec result=lab24::checked(fn(x,p));
    out<<std::setprecision(16);
    for(std::size_t i=0;i<result.size();++i)out<<i<<","<<result[i]<<"\n";
    return true;
}
inline void lab24_usage(std::ostream& out){
    out<<"Advanced numerical matrix workflows: lab24-AREA-METHOD CSV [PARAMETERS...]\n";
    out<<"  lab24-linear-qr-solve CSV\n";
    out<<"  lab24-linear-spectral2 CSV\n";
    out<<"  lab24-linear-ridge2 CSV PARAMETER...\n";
    out<<"  lab24-linear-woodbury-rank1 CSV PARAMETER...\n";
    out<<"  lab24-linear-power-iteration CSV PARAMETER...\n";
    out<<"  lab24-optim-simplex-projection CSV\n";
    out<<"  lab24-optim-l1-ball CSV PARAMETER...\n";
    out<<"  lab24-optim-box-budget CSV PARAMETER...\n";
    out<<"  lab24-optim-elastic-coordinate CSV PARAMETER...\n";
    out<<"  lab24-optim-entropy-tilt CSV PARAMETER...\n";
    out<<"  lab24-network-laplacian-energy CSV\n";
    out<<"  lab24-network-pagerank CSV PARAMETER...\n";
    out<<"  lab24-network-diffusion CSV PARAMETER...\n";
    out<<"  lab24-network-minimax-path CSV PARAMETER...\n";
    out<<"  lab24-network-minimum-spanning-tree CSV PARAMETER...\n";
    out<<"  lab24-state-scalar-kalman CSV PARAMETER...\n";
    out<<"  lab24-state-hmm-forward CSV PARAMETER...\n";
    out<<"  lab24-state-viterbi CSV PARAMETER...\n";
    out<<"  lab24-state-ou-transition-fit CSV PARAMETER...\n";
    out<<"  lab24-state-markov-hitting CSV PARAMETER...\n";
    out<<"  lab24-panel-within-panel CSV\n";
    out<<"  lab24-panel-cluster-ols CSV\n";
    out<<"  lab24-panel-paired-t CSV\n";
    out<<"  lab24-panel-randomization-exact CSV\n";
    out<<"  lab24-panel-iv-first-stage CSV\n";
    out<<"  lab24-learning-online-logistic CSV PARAMETER...\n";
    out<<"  lab24-learning-adagrad-ridge CSV PARAMETER...\n";
    out<<"  lab24-learning-brier-decomposition CSV\n";
    out<<"  lab24-learning-isotonic-pav CSV\n";
    out<<"  lab24-learning-stacking-two CSV\n";
    out<<"  lab24-rates-discount-bootstrap CSV\n";
    out<<"  lab24-rates-forward-curve CSV\n";
    out<<"  lab24-rates-curve-keyrate CSV PARAMETER...\n";
    out<<"  lab24-rates-duration-immunize CSV PARAMETER...\n";
    out<<"  lab24-rates-yield-newton CSV PARAMETER...\n";
    out<<"  lab24-derivative-crr-american CSV PARAMETER...\n";
    out<<"  lab24-derivative-asian-path CSV PARAMETER...\n";
    out<<"  lab24-derivative-butterfly-density CSV PARAMETER...\n";
    out<<"  lab24-derivative-variance-strip CSV PARAMETER...\n";
    out<<"  lab24-derivative-discrete-delta-pnl CSV PARAMETER...\n";
    out<<"  lab24-credit-migration-two CSV PARAMETER...\n";
    out<<"  lab24-credit-poisson-binomial CSV\n";
    out<<"  lab24-credit-loss-convolution CSV PARAMETER...\n";
    out<<"  lab24-credit-survival-hazard CSV\n";
    out<<"  lab24-credit-discrete-cds-spread CSV PARAMETER...\n";
    out<<"  lab24-portfolio-factor-neutral-two CSV PARAMETER...\n";
    out<<"  lab24-portfolio-risk-budget-diag CSV\n";
    out<<"  lab24-portfolio-factor-cov-risk CSV PARAMETER...\n";
    out<<"  lab24-portfolio-turnover-prox CSV\n";
    out<<"  lab24-portfolio-rebalance-drift CSV\n";
    out<<"  lab24-execution-ac-schedule CSV PARAMETER...\n";
    out<<"  lab24-execution-participation-schedule CSV PARAMETER...\n";
    out<<"  lab24-execution-queue-fifo CSV PARAMETER...\n";
    out<<"  lab24-execution-multi-venue-sweep CSV PARAMETER...\n";
    out<<"  lab24-execution-impact-decay-fit CSV\n";
    out<<"  lab24-data-asof-age CSV PARAMETER...\n";
    out<<"  lab24-data-purged-mask CSV PARAMETER...\n";
    out<<"  lab24-data-label-uniqueness CSV\n";
    out<<"  lab24-data-revision-latest CSV PARAMETER...\n";
    out<<"  lab24-data-event-time-bars CSV PARAMETER...\n";
}
}
