#pragma once
#include "evidence_pricing.hpp"
#include "evidence_selection.hpp"
#include "evidence_density.hpp"
#include "evidence_stateprice.hpp"
#include "evidence_ledger.hpp"
#include "evidence_shift.hpp"
#include "evidence_marketdesign.hpp"
#include "evidence_causal.hpp"
namespace kline {
inline bool run_evidence_cli(const std::string& command,int argc,char** argv,std::ostream& out){
    using Function=researchflow::Report(*)(const researchflow::Mat&,const researchflow::Vec&);
    Function fn=nullptr;
    if(command=="evidence-hj-bound")fn=evidence::hj_bound;
    if(command=="evidence-minimum-norm-sdf")fn=evidence::minimum_norm_sdf;
    if(command=="evidence-sdf-pricing-errors")fn=evidence::sdf_pricing_errors;
    if(command=="evidence-grs-joint-alpha")fn=evidence::grs_joint_alpha;
    if(command=="evidence-fdr-adjustment")fn=evidence::fdr_adjustment;
    if(command=="evidence-holm-adjustment")fn=evidence::holm_adjustment;
    if(command=="evidence-cscv-mean-pbo")fn=evidence::cscv_mean_pbo;
    if(command=="evidence-max-signflip")fn=evidence::max_signflip;
    if(command=="evidence-energy-score")fn=evidence::energy_score;
    if(command=="evidence-variogram-score")fn=evidence::variogram_score;
    if(command=="evidence-quantile-rearrangement")fn=evidence::quantile_rearrangement;
    if(command=="evidence-ensemble-rank")fn=evidence::ensemble_rank;
    if(command=="evidence-call-implied-atoms")fn=evidence::call_implied_atoms;
    if(command=="evidence-complete-replication")fn=evidence::complete_replication;
    if(command=="evidence-incomplete-claim-bounds")fn=evidence::incomplete_claim_bounds;
    if(command=="evidence-parity-quote-intersection")fn=evidence::parity_quote_intersection;
    if(command=="evidence-heterogeneous-cost-rebalance")fn=evidence::heterogeneous_cost_rebalance;
    if(command=="evidence-corporate-action-cashbook")fn=evidence::corporate_action_cashbook;
    if(command=="evidence-futures-margin-cashbook")fn=evidence::futures_margin_cashbook;
    if(command=="evidence-currency-return-attribution")fn=evidence::currency_return_attribution;
    if(command=="evidence-discrete-shift-weight")fn=evidence::discrete_shift_weight;
    if(command=="evidence-empirical-wasserstein")fn=evidence::empirical_wasserstein;
    if(command=="evidence-adaptive-prediction-set")fn=evidence::adaptive_prediction_set;
    if(command=="evidence-risk-coverage-curve")fn=evidence::risk_coverage_curve;
    if(command=="evidence-call-auction-grid")fn=evidence::call_auction_grid;
    if(command=="evidence-auction-pro-rata")fn=evidence::auction_pro_rata;
    if(command=="evidence-state-event-forecast")fn=evidence::state_event_forecast;
    if(command=="evidence-delayed-entry-survival")fn=evidence::delayed_entry_survival;
    if(command=="evidence-twfe-balanced")fn=evidence::twfe_balanced;
    if(command=="evidence-cohort-att")fn=evidence::cohort_att;
    if(command=="evidence-local-linear-rd")fn=evidence::local_linear_rd;
    if(command=="evidence-oaxaca-reference")fn=evidence::oaxaca_reference;
    if(!fn)return false;
    require(argc>=3,"Evidence laboratory requires a numeric CSV");
    researchflow::Vec p;
    for(int i=3;i<argc;++i)p.push_back(parse_number(argv[i]));
    researchflow::write(out,fn(researchflow::read_matrix(argv[2]),p));return true;
}
inline void evidence_usage(std::ostream& out){
    out<<"Evidence laboratories: evidence-METHOD CSV [PARAMETERS...]\n";
    out<<"  evidence-hj-bound CSV PARAMETERS...\n";
    out<<"  evidence-minimum-norm-sdf CSV PARAMETERS...\n";
    out<<"  evidence-sdf-pricing-errors CSV PARAMETERS...\n";
    out<<"  evidence-grs-joint-alpha CSV PARAMETERS...\n";
    out<<"  evidence-fdr-adjustment CSV PARAMETERS...\n";
    out<<"  evidence-holm-adjustment CSV PARAMETERS...\n";
    out<<"  evidence-cscv-mean-pbo CSV PARAMETERS...\n";
    out<<"  evidence-max-signflip CSV\n";
    out<<"  evidence-energy-score CSV\n";
    out<<"  evidence-variogram-score CSV PARAMETERS...\n";
    out<<"  evidence-quantile-rearrangement CSV PARAMETERS...\n";
    out<<"  evidence-ensemble-rank CSV\n";
    out<<"  evidence-call-implied-atoms CSV PARAMETERS...\n";
    out<<"  evidence-complete-replication CSV PARAMETERS...\n";
    out<<"  evidence-incomplete-claim-bounds CSV PARAMETERS...\n";
    out<<"  evidence-parity-quote-intersection CSV PARAMETERS...\n";
    out<<"  evidence-heterogeneous-cost-rebalance CSV\n";
    out<<"  evidence-corporate-action-cashbook CSV PARAMETERS...\n";
    out<<"  evidence-futures-margin-cashbook CSV PARAMETERS...\n";
    out<<"  evidence-currency-return-attribution CSV\n";
    out<<"  evidence-discrete-shift-weight CSV\n";
    out<<"  evidence-empirical-wasserstein CSV\n";
    out<<"  evidence-adaptive-prediction-set CSV PARAMETERS...\n";
    out<<"  evidence-risk-coverage-curve CSV\n";
    out<<"  evidence-call-auction-grid CSV PARAMETERS...\n";
    out<<"  evidence-auction-pro-rata CSV PARAMETERS...\n";
    out<<"  evidence-state-event-forecast CSV PARAMETERS...\n";
    out<<"  evidence-delayed-entry-survival CSV\n";
    out<<"  evidence-twfe-balanced CSV\n";
    out<<"  evidence-cohort-att CSV\n";
    out<<"  evidence-local-linear-rd CSV PARAMETERS...\n";
    out<<"  evidence-oaxaca-reference CSV\n";
}
}
