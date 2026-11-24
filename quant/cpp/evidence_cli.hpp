#pragma once
#include "evidence_pricing.hpp"
#include "evidence_selection.hpp"
#include "evidence_density.hpp"
#include "evidence_stateprice.hpp"
#include "evidence_ledger.hpp"
#include "evidence_shift.hpp"
#include "evidence_marketdesign.hpp"
#include "evidence_causal.hpp"
#include "inference_monitoring.hpp"
#include "inference_identification.hpp"
#include "forecast_comparison.hpp"
#include "applied_linear.hpp"
#include "applied_panel.hpp"
#include "applied_learning.hpp"
#include "applied_pricing.hpp"
#include "applied_allocation.hpp"
#include "applied_credit.hpp"
#include "applied_execution.hpp"
#include "applied_stochastic.hpp"
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
    if(command=="inference-bernoulli-mixture")fn=monitoring::bernoulli_mixture;
    if(command=="inference-bounded-betting")fn=monitoring::bounded_betting;
    if(command=="inference-alpha-spending")fn=monitoring::alpha_spending;
    if(command=="inference-likelihood-stopping")fn=monitoring::likelihood_stopping;
    if(command=="inference-fieller-grid")fn=identification::fieller_grid;
    if(command=="inference-anderson-rubin-grid")fn=identification::anderson_rubin_grid;
    if(command=="inference-jackknife-ratio")fn=identification::jackknife_ratio;
    if(command=="inference-exact-sign")fn=identification::exact_sign;
    if(command=="inference-brier-groups")fn=comparison::brier_groups;
    if(command=="inference-forecast-revisions")fn=comparison::forecast_revisions;
    if(command=="inference-encompassing")fn=comparison::encompassing;
    if(command=="inference-murphy-quantile")fn=comparison::murphy_quantile;
    if(command=="applied-hc3")fn=applied_linear::hc3;
    if(command=="applied-press")fn=applied_linear::press;
    if(command=="applied-partial-r2")fn=applied_linear::partial_r2;
    if(command=="applied-vif")fn=applied_linear::vif;
    if(command=="applied-within-between")fn=applied_panel::within_between;
    if(command=="applied-cluster-mean")fn=applied_panel::cluster_mean;
    if(command=="applied-first-stage")fn=applied_panel::first_stage;
    if(command=="applied-omitted-bias")fn=applied_panel::omitted_bias;
    if(command=="applied-kernel-alignment")fn=applied_learning::kernel_alignment;
    if(command=="applied-gram-centering")fn=applied_learning::gram_centering;
    if(command=="applied-soft-label")fn=applied_learning::soft_label;
    if(command=="applied-calibration-slope")fn=applied_learning::calibration_slope;
    if(command=="applied-dividend-forward")fn=applied_pricing::dividend_forward;
    if(command=="applied-option-box")fn=applied_pricing::option_box;
    if(command=="applied-implied-dividend")fn=applied_pricing::implied_dividend;
    if(command=="applied-geometric-asian")fn=applied_pricing::geometric_asian;
    if(command=="applied-brinson-fachler")fn=applied_allocation::brinson_fachler;
    if(command=="applied-carino")fn=applied_allocation::carino;
    if(command=="applied-daily-leverage")fn=applied_allocation::daily_leverage;
    if(command=="applied-active-wealth")fn=applied_allocation::active_wealth;
    if(command=="applied-competing-incidence")fn=applied_credit::competing_incidence;
    if(command=="applied-exposure-profile")fn=applied_credit::exposure_profile;
    if(command=="applied-wrong-way")fn=applied_credit::wrong_way;
    if(command=="applied-netting")fn=applied_credit::netting;
    if(command=="applied-markout")fn=applied_execution::markout;
    if(command=="applied-participation-cap")fn=applied_execution::participation_cap;
    if(command=="applied-recovery")fn=applied_execution::recovery;
    if(command=="applied-maker-taker")fn=applied_execution::maker_taker;
    if(command=="applied-semivariance")fn=applied_stochastic::semivariance;
    if(command=="applied-bridge")fn=applied_stochastic::bridge;
    if(command=="applied-covariation")fn=applied_stochastic::covariation;
    if(command=="applied-ar-aggregation")fn=applied_stochastic::ar_aggregation;
    if(!fn)return false;
    require(argc>=3,"Evidence laboratory requires a numeric CSV");
    researchflow::Vec p;
    for(int i=3;i<argc;++i)p.push_back(parse_number(argv[i]));
    researchflow::write(out,fn(researchflow::read_matrix(argv[2]),p));return true;
}
inline void evidence_usage(std::ostream& out){
    out<<"  inference-bernoulli-mixture CSV PARAMETERS...\n";
    out<<"  inference-bounded-betting CSV PARAMETERS...\n";
    out<<"  inference-alpha-spending CSV PARAMETERS...\n";
    out<<"  inference-likelihood-stopping CSV PARAMETERS...\n";
    out<<"  inference-fieller-grid CSV PARAMETERS...\n";
    out<<"  inference-anderson-rubin-grid CSV PARAMETERS...\n";
    out<<"  inference-jackknife-ratio CSV\n";
    out<<"  inference-exact-sign CSV PARAMETERS...\n";
    out<<"  inference-brier-groups CSV\n";
    out<<"  inference-forecast-revisions CSV\n";
    out<<"  inference-encompassing CSV\n";
    out<<"  inference-murphy-quantile CSV PARAMETERS...\n";
    out<<"  applied-hc3 CSV\n";
    out<<"  applied-press CSV\n";
    out<<"  applied-partial-r2 CSV PARAMETERS...\n";
    out<<"  applied-vif CSV\n";
    out<<"  applied-within-between CSV\n";
    out<<"  applied-cluster-mean CSV\n";
    out<<"  applied-first-stage CSV\n";
    out<<"  applied-omitted-bias CSV\n";
    out<<"  applied-kernel-alignment CSV\n";
    out<<"  applied-gram-centering CSV\n";
    out<<"  applied-soft-label CSV\n";
    out<<"  applied-calibration-slope CSV\n";
    out<<"  applied-dividend-forward CSV PARAMETERS...\n";
    out<<"  applied-option-box CSV\n";
    out<<"  applied-implied-dividend CSV PARAMETERS...\n";
    out<<"  applied-geometric-asian CSV PARAMETERS...\n";
    out<<"  applied-brinson-fachler CSV\n";
    out<<"  applied-carino CSV\n";
    out<<"  applied-daily-leverage CSV PARAMETERS...\n";
    out<<"  applied-active-wealth CSV\n";
    out<<"  applied-competing-incidence CSV\n";
    out<<"  applied-exposure-profile CSV PARAMETERS...\n";
    out<<"  applied-wrong-way CSV PARAMETERS...\n";
    out<<"  applied-netting CSV\n";
    out<<"  applied-markout CSV\n";
    out<<"  applied-participation-cap CSV PARAMETERS...\n";
    out<<"  applied-recovery CSV PARAMETERS...\n";
    out<<"  applied-maker-taker CSV\n";
    out<<"  applied-semivariance CSV\n";
    out<<"  applied-bridge CSV PARAMETERS...\n";
    out<<"  applied-covariation CSV PARAMETERS...\n";
    out<<"  applied-ar-aggregation CSV PARAMETERS...\n";
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
