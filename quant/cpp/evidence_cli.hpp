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
#include "methods_geometry.hpp"
#include "methods_regularization.hpp"
#include "methods_optimization.hpp"
#include "methods_dynamics.hpp"
#include "methods_series.hpp"
#include "methods_sampling.hpp"
#include "methods_classification.hpp"
#include "methods_decision.hpp"
#include "methods_derivatives.hpp"
#include "methods_rates.hpp"
#include "methods_counterparty.hpp"
#include "methods_microstructure.hpp"
#include "methods_portfolio.hpp"
#include "methods_empirical.hpp"
#include "methods_macroeconomics.hpp"
#include "methods_dataengineering.hpp"
#include "depth_numerics.hpp"
#include "depth_proximal.hpp"
#include "depth_distribution.hpp"
#include "depth_sampling.hpp"
#include "depth_temporal.hpp"
#include "depth_inference.hpp"
#include "depth_kernel.hpp"
#include "depth_loss.hpp"
#include "depth_differentiation.hpp"
#include "depth_validation.hpp"
#include "depth_uncertainty.hpp"
#include "depth_features.hpp"
#include "depth_optiongreeks.hpp"
#include "depth_termstructure.hpp"
#include "depth_dynamicpricing.hpp"
#include "depth_creditmodels.hpp"
#include "depth_robustallocation.hpp"
#include "depth_volatility.hpp"
#include "depth_executiontheory.hpp"
#include "depth_engineering.hpp"
#include "depth_identification.hpp"
#include "depth_economics.hpp"
#include "depth_games.hpp"
#include "depth_factorconstruction.hpp"
#include "model_research_optimization.hpp"
#include "model_research_likelihood.hpp"
#include "model_research_dynamics.hpp"
#include "model_research_pricing.hpp"
#include "model_research_portfolio.hpp"
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
    if(command=="methods-gram-schmidt")fn=methods_geometry::gram_schmidt;
    if(command=="methods-rank-one-inverse")fn=methods_geometry::rank_one_inverse;
    if(command=="methods-gershgorin")fn=methods_geometry::gershgorin;
    if(command=="methods-spectral-participation")fn=methods_geometry::spectral_participation;
    if(command=="methods-orthogonal-lasso")fn=methods_regularization::orthogonal_lasso;
    if(command=="methods-ridge-effective-df")fn=methods_regularization::ridge_effective_df;
    if(command=="methods-sure-linear")fn=methods_regularization::sure_linear;
    if(command=="methods-stability-frequency")fn=methods_regularization::stability_frequency;
    if(command=="methods-simplex-projection")fn=methods_optimization::simplex_projection;
    if(command=="methods-diagonal-kkt")fn=methods_optimization::diagonal_kkt;
    if(command=="methods-entropic-prox")fn=methods_optimization::entropic_prox;
    if(command=="methods-quadratic-line-search")fn=methods_optimization::quadratic_line_search;
    if(command=="methods-scalar-riccati")fn=methods_dynamics::scalar_riccati;
    if(command=="methods-ou-transition")fn=methods_dynamics::ou_transition;
    if(command=="methods-markov-occupation")fn=methods_dynamics::markov_occupation;
    if(command=="methods-innovation-likelihood")fn=methods_dynamics::innovation_likelihood;
    if(command=="methods-ma-autocov")fn=methods_series::ma_autocov;
    if(command=="methods-harmonic-power")fn=methods_series::harmonic_power;
    if(command=="methods-overlap-mean")fn=methods_series::overlap_mean;
    if(command=="methods-seasonal-difference")fn=methods_series::seasonal_difference;
    if(command=="methods-weighted-effective-n")fn=methods_sampling::weighted_effective_n;
    if(command=="methods-stratified-mean")fn=methods_sampling::stratified_mean;
    if(command=="methods-median-of-means")fn=methods_sampling::median_of_means;
    if(command=="methods-self-normalized-is")fn=methods_sampling::self_normalized_is;
    if(command=="methods-prior-adjustment")fn=methods_classification::prior_adjustment;
    if(command=="methods-pairwise-auc")fn=methods_classification::pairwise_auc;
    if(command=="methods-multiclass-brier")fn=methods_classification::multiclass_brier;
    if(command=="methods-binary-cost-frontier")fn=methods_classification::binary_cost_frontier;
    if(command=="methods-log-utility-grid")fn=methods_decision::log_utility_grid;
    if(command=="methods-cara-ce")fn=methods_decision::cara_ce;
    if(command=="methods-regret-matrix")fn=methods_decision::regret_matrix;
    if(command=="methods-value-information")fn=methods_decision::value_information;
    if(command=="methods-margrabe-exchange")fn=methods_derivatives::margrabe_exchange;
    if(command=="methods-forward-start")fn=methods_derivatives::forward_start;
    if(command=="methods-quanto-expectation")fn=methods_derivatives::quanto_expectation;
    if(command=="methods-power-payoff")fn=methods_derivatives::power_payoff;
    if(command=="methods-duration-immunization")fn=methods_rates::duration_immunization;
    if(command=="methods-bond-holding-return")fn=methods_rates::bond_holding_return;
    if(command=="methods-fisher-breakeven")fn=methods_rates::fisher_breakeven;
    if(command=="methods-discount-forward-roll")fn=methods_rates::discount_forward_roll;
    if(command=="methods-discrete-cva")fn=methods_counterparty::discrete_cva;
    if(command=="methods-joint-default")fn=methods_counterparty::joint_default;
    if(command=="methods-tranche-loss")fn=methods_counterparty::tranche_loss;
    if(command=="methods-constant-hazard-bond")fn=methods_counterparty::constant_hazard_bond;
    if(command=="methods-quote-time-weighted")fn=methods_microstructure::quote_time_weighted;
    if(command=="methods-signed-response")fn=methods_microstructure::signed_response;
    if(command=="methods-depth-walk")fn=methods_microstructure::depth_walk;
    if(command=="methods-depth-concentration")fn=methods_microstructure::depth_concentration;
    if(command=="methods-active-factor-risk")fn=methods_portfolio::active_factor_risk;
    if(command=="methods-variance-shapley")fn=methods_portfolio::variance_shapley;
    if(command=="methods-binary-kelly")fn=methods_portfolio::binary_kelly;
    if(command=="methods-liability-surplus")fn=methods_portfolio::liability_surplus;
    if(command=="methods-event-car")fn=methods_empirical::event_car;
    if(command=="methods-paired-randomization")fn=methods_empirical::paired_randomization;
    if(command=="methods-ipw-balance")fn=methods_empirical::ipw_balance;
    if(command=="methods-placebo-rank")fn=methods_empirical::placebo_rank;
    if(command=="methods-debt-dynamics")fn=methods_macroeconomics::debt_dynamics;
    if(command=="methods-gdp-growth-contributions")fn=methods_macroeconomics::gdp_growth_contributions;
    if(command=="methods-real-fx-decomposition")fn=methods_macroeconomics::real_fx_decomposition;
    if(command=="methods-uip-carry")fn=methods_macroeconomics::uip_carry;
    if(command=="methods-sequence-gap")fn=methods_dataengineering::sequence_gap;
    if(command=="methods-asof-staleness")fn=methods_dataengineering::asof_staleness;
    if(command=="methods-latency-tail")fn=methods_dataengineering::latency_tail;
    if(command=="methods-position-cash-reconcile")fn=methods_dataengineering::position_cash_reconcile;
    if(command=="depth-backward-error")fn=depth_numerics::backward_error;
    if(command=="depth-row-equilibration")fn=depth_numerics::row_equilibration;
    if(command=="depth-two-by-two-spectrum")fn=depth_numerics::two_by_two_spectrum;
    if(command=="depth-precision-conditional")fn=depth_numerics::precision_conditional;
    if(command=="depth-group-prox")fn=depth_proximal::group_prox;
    if(command=="depth-affine-projection")fn=depth_proximal::affine_projection;
    if(command=="depth-bounded-budget")fn=depth_proximal::bounded_budget;
    if(command=="depth-huber-prox")fn=depth_proximal::huber_prox;
    if(command=="depth-discrete-convolution")fn=depth_distribution::discrete_convolution;
    if(command=="depth-latent-binomial")fn=depth_distribution::latent_binomial;
    if(command=="depth-exponential-tilt")fn=depth_distribution::exponential_tilt;
    if(command=="depth-randomized-pit")fn=depth_distribution::randomized_pit;
    if(command=="depth-reliability-variance")fn=depth_sampling::reliability_variance;
    if(command=="depth-pairwise-missing")fn=depth_sampling::pairwise_missing;
    if(command=="depth-group-jackknife")fn=depth_sampling::group_jackknife;
    if(command=="depth-trim-winsor")fn=depth_sampling::trim_winsor;
    if(command=="depth-ar-impulse")fn=depth_temporal::ar_impulse;
    if(command=="depth-ar-forecast-variance")fn=depth_temporal::ar_forecast_variance;
    if(command=="depth-filter-frequency")fn=depth_temporal::filter_frequency;
    if(command=="depth-lag-aligned-cov")fn=depth_temporal::lag_aligned_cov;
    if(command=="depth-paired-student")fn=depth_inference::paired_student;
    if(command=="depth-equivalence-tost")fn=depth_inference::equivalence_tost;
    if(command=="depth-survey-hajek")fn=depth_inference::survey_hajek;
    if(command=="depth-fisher-exact")fn=depth_inference::fisher_exact;
    if(command=="depth-nystrom-schur")fn=depth_kernel::nystrom_schur;
    if(command=="depth-rff-pair")fn=depth_kernel::rff_pair;
    if(command=="depth-mahalanobis-residual")fn=depth_kernel::mahalanobis_residual;
    if(command=="depth-outofsample-kernel-center")fn=depth_kernel::outofsample_kernel_center;
    if(command=="depth-binary-focal")fn=depth_loss::binary_focal;
    if(command=="depth-huberized-pinball")fn=depth_loss::huberized_pinball;
    if(command=="depth-logit-intercept-score")fn=depth_loss::logit_intercept_score;
    if(command=="depth-temperature-curvature")fn=depth_loss::temperature_curvature;
    if(command=="depth-logistic-hvp")fn=depth_differentiation::logistic_hvp;
    if(command=="depth-softmax-jvp")fn=depth_differentiation::softmax_jvp;
    if(command=="depth-gradient-noise")fn=depth_differentiation::gradient_noise;
    if(command=="depth-global-gradient-clip")fn=depth_differentiation::global_gradient_clip;
    if(command=="depth-purge-interval")fn=depth_validation::purge_interval;
    if(command=="depth-train-only-standardization")fn=depth_validation::train_only_standardization;
    if(command=="depth-forecast-oos-r2")fn=depth_validation::forecast_oos_r2;
    if(command=="depth-block-loss-decomposition")fn=depth_validation::block_loss_decomposition;
    if(command=="depth-mixture-predictive-variance")fn=depth_uncertainty::mixture_predictive_variance;
    if(command=="depth-ensemble-entropy-mi")fn=depth_uncertainty::ensemble_entropy_mi;
    if(command=="depth-mondrian-calibration")fn=depth_uncertainty::mondrian_calibration;
    if(command=="depth-wilson-coverage")fn=depth_uncertainty::wilson_coverage;
    if(command=="depth-weighted-group-demean")fn=depth_features::weighted_group_demean;
    if(command=="depth-train-missing-indicator")fn=depth_features::train_missing_indicator;
    if(command=="depth-ew-prior-zscore")fn=depth_features::ew_prior_zscore;
    if(command=="depth-regime-covariance")fn=depth_features::regime_covariance;
    if(command=="depth-digital-decomposition")fn=depth_optiongreeks::digital_decomposition;
    if(command=="depth-vanna-volga")fn=depth_optiongreeks::vanna_volga;
    if(command=="depth-delta-hedge-variance")fn=depth_optiongreeks::delta_hedge_variance;
    if(command=="depth-spot-vol-taylor")fn=depth_optiongreeks::spot_vol_taylor;
    if(command=="depth-log-discount-forward")fn=depth_termstructure::log_discount_forward;
    if(command=="depth-forward-swap-annuity")fn=depth_termstructure::forward_swap_annuity;
    if(command=="depth-fra-settlement")fn=depth_termstructure::fra_settlement;
    if(command=="depth-overnight-accrual-bridge")fn=depth_termstructure::overnight_accrual_bridge;
    if(command=="depth-node-martingale")fn=depth_dynamicpricing::node_martingale;
    if(command=="depth-numeraire-measure")fn=depth_dynamicpricing::numeraire_measure;
    if(command=="depth-stopping-policy-value")fn=depth_dynamicpricing::stopping_policy_value;
    if(command=="depth-dual-stopping-bound")fn=depth_dynamicpricing::dual_stopping_bound;
    if(command=="depth-forward-default")fn=depth_creditmodels::forward_default;
    if(command=="depth-margin-period-risk")fn=depth_creditmodels::margin_period_risk;
    if(command=="depth-heterogeneous-credit-mixture")fn=depth_creditmodels::heterogeneous_credit_mixture;
    if(command=="depth-exponential-hazard-mle")fn=depth_creditmodels::exponential_hazard_mle;
    if(command=="depth-ellipsoid-worst-mean")fn=depth_robustallocation::ellipsoid_worst_mean;
    if(command=="depth-stochastic-dominance")fn=depth_robustallocation::stochastic_dominance;
    if(command=="depth-single-asset-no-trade")fn=depth_robustallocation::single_asset_no_trade;
    if(command=="depth-mean-estimation-risk")fn=depth_robustallocation::mean_estimation_risk;
    if(command=="depth-forward-variance")fn=depth_volatility::forward_variance;
    if(command=="depth-noise-signature")fn=depth_volatility::noise_signature;
    if(command=="depth-lognormal-vol-convexity")fn=depth_volatility::lognormal_vol_convexity;
    if(command=="depth-gjr-news-impact")fn=depth_volatility::gjr_news_impact;
    if(command=="depth-execution-kkt")fn=depth_executiontheory::execution_kkt;
    if(command=="depth-competing-execution-hazards")fn=depth_executiontheory::competing_execution_hazards;
    if(command=="depth-queue-cancel-bounds")fn=depth_executiontheory::queue_cancel_bounds;
    if(command=="depth-quote-bayesian-selection")fn=depth_executiontheory::quote_bayesian_selection;
    if(command=="depth-bitemporal-asof")fn=depth_engineering::bitemporal_asof;
    if(command=="depth-event-watermark")fn=depth_engineering::event_watermark;
    if(command=="depth-event-idempotency")fn=depth_engineering::event_idempotency;
    if(command=="depth-tick-grid")fn=depth_engineering::tick_grid;
    if(command=="depth-unbalanced-two-way-projection")fn=depth_identification::unbalanced_two_way_projection;
    if(command=="depth-manski-missing-bounds")fn=depth_identification::manski_missing_bounds;
    if(command=="depth-lee-trimming-bounds")fn=depth_identification::lee_trimming_bounds;
    if(command=="depth-synthetic-control-gap")fn=depth_identification::synthetic_control_gap;
    if(command=="depth-leontief-demand")fn=depth_economics::leontief_demand;
    if(command=="depth-fisher-price-index")fn=depth_economics::fisher_price_index;
    if(command=="depth-shift-share-exposure")fn=depth_economics::shift_share_exposure;
    if(command=="depth-taylor-inertia")fn=depth_economics::taylor_inertia;
    if(command=="depth-mixed-nash-interior")fn=depth_games::mixed_nash_interior;
    if(command=="depth-cournot-active-set")fn=depth_games::cournot_active_set;
    if(command=="depth-auction-uniform-bid")fn=depth_games::auction_uniform_bid;
    if(command=="depth-kyle-linear-equilibrium")fn=depth_games::kyle_linear_equilibrium;
    if(command=="depth-tie-aware-tail-weights")fn=depth_factorconstruction::tie_aware_tail_weights;
    if(command=="depth-size-value-six-cells")fn=depth_factorconstruction::size_value_six_cells;
    if(command=="depth-beta-posterior-shrink")fn=depth_factorconstruction::beta_posterior_shrink;
    if(command=="depth-constant-mix-diversification")fn=depth_factorconstruction::constant_mix_diversification;
    if(command=="model-fista-elastic")fn=model_research::fista_elastic;
    if(command=="model-fused-admm")fn=model_research::fused_admm;
    if(command=="model-box-covariance-qp")fn=model_research::box_covariance_qp;
    if(command=="model-entropy-balance-fit")fn=model_research::entropy_balance_fit;
    if(command=="model-nnls-coordinate")fn=model_research::nnls_coordinate;
    if(command=="model-graph-smoother")fn=model_research::graph_smoother;
    if(command=="model-nb2-exposure")fn=model_research::nb2_exposure;
    if(command=="model-zip-em")fn=model_research::zip_em;
    if(command=="model-cox-breslow-ridge")fn=model_research::cox_breslow_ridge;
    if(command=="model-tobit-known-scale")fn=model_research::tobit_known_scale;
    if(command=="model-shrinkage-lda")fn=model_research::shrinkage_lda;
    if(command=="model-kernel-logistic-fit")fn=model_research::kernel_logistic_fit;
    if(command=="model-var-ridge-forecast")fn=model_research::var_ridge_forecast;
    if(command=="model-kalman-grid-fit")fn=model_research::kalman_grid_fit;
    if(command=="model-hawkes-fixed-decay-fit")fn=model_research::hawkes_fixed_decay_fit;
    if(command=="model-har-rv-forecast")fn=model_research::har_rv_forecast;
    if(command=="model-ou-conditional-fit")fn=model_research::ou_conditional_fit;
    if(command=="model-dcc-twoasset-grid")fn=model_research::dcc_twoasset_grid;
    if(command=="model-american-put-pde")fn=model_research::american_put_pde;
    if(command=="model-localvol-call-pde")fn=model_research::localvol_call_pde;
    if(command=="model-asian-independent-control")fn=model_research::asian_independent_control;
    if(command=="model-cds-hazard-bootstrap")fn=model_research::cds_hazard_bootstrap;
    if(command=="model-nelson-siegel-calibration")fn=model_research::nelson_siegel_calibration;
    if(command=="model-risk-neutral-entropy-fit")fn=model_research::risk_neutral_entropy_fit;
    if(command=="model-rolling-elastic-portfolio")fn=model_research::rolling_elastic_portfolio;
    if(command=="model-rolling-covariance-qp")fn=model_research::rolling_covariance_qp;
    if(command=="model-risk-budget-newton")fn=model_research::risk_budget_newton;
    if(command=="model-synthetic-control-fit")fn=model_research::synthetic_control_fit;
    if(command=="model-crossfit-dml-ridge")fn=model_research::crossfit_dml_ridge;
    if(command=="model-rolling-conformal-ridge")fn=model_research::rolling_conformal_ridge;
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
    out<<"  methods-gram-schmidt CSV\n";
    out<<"  methods-rank-one-inverse CSV\n";
    out<<"  methods-gershgorin CSV\n";
    out<<"  methods-spectral-participation CSV\n";
    out<<"  methods-orthogonal-lasso CSV PARAMETERS...\n";
    out<<"  methods-ridge-effective-df CSV PARAMETERS...\n";
    out<<"  methods-sure-linear CSV PARAMETERS...\n";
    out<<"  methods-stability-frequency CSV PARAMETERS...\n";
    out<<"  methods-simplex-projection CSV\n";
    out<<"  methods-diagonal-kkt CSV PARAMETERS...\n";
    out<<"  methods-entropic-prox CSV PARAMETERS...\n";
    out<<"  methods-quadratic-line-search CSV\n";
    out<<"  methods-scalar-riccati CSV PARAMETERS...\n";
    out<<"  methods-ou-transition CSV PARAMETERS...\n";
    out<<"  methods-markov-occupation CSV PARAMETERS...\n";
    out<<"  methods-innovation-likelihood CSV\n";
    out<<"  methods-ma-autocov CSV PARAMETERS...\n";
    out<<"  methods-harmonic-power CSV PARAMETERS...\n";
    out<<"  methods-overlap-mean CSV PARAMETERS...\n";
    out<<"  methods-seasonal-difference CSV PARAMETERS...\n";
    out<<"  methods-weighted-effective-n CSV\n";
    out<<"  methods-stratified-mean CSV\n";
    out<<"  methods-median-of-means CSV PARAMETERS...\n";
    out<<"  methods-self-normalized-is CSV\n";
    out<<"  methods-prior-adjustment CSV PARAMETERS...\n";
    out<<"  methods-pairwise-auc CSV\n";
    out<<"  methods-multiclass-brier CSV\n";
    out<<"  methods-binary-cost-frontier CSV PARAMETERS...\n";
    out<<"  methods-log-utility-grid CSV PARAMETERS...\n";
    out<<"  methods-cara-ce CSV PARAMETERS...\n";
    out<<"  methods-regret-matrix CSV\n";
    out<<"  methods-value-information CSV\n";
    out<<"  methods-margrabe-exchange CSV PARAMETERS...\n";
    out<<"  methods-forward-start CSV PARAMETERS...\n";
    out<<"  methods-quanto-expectation CSV PARAMETERS...\n";
    out<<"  methods-power-payoff CSV PARAMETERS...\n";
    out<<"  methods-duration-immunization CSV PARAMETERS...\n";
    out<<"  methods-bond-holding-return CSV PARAMETERS...\n";
    out<<"  methods-fisher-breakeven CSV\n";
    out<<"  methods-discount-forward-roll CSV\n";
    out<<"  methods-discrete-cva CSV PARAMETERS...\n";
    out<<"  methods-joint-default CSV\n";
    out<<"  methods-tranche-loss CSV PARAMETERS...\n";
    out<<"  methods-constant-hazard-bond CSV PARAMETERS...\n";
    out<<"  methods-quote-time-weighted CSV PARAMETERS...\n";
    out<<"  methods-signed-response CSV PARAMETERS...\n";
    out<<"  methods-depth-walk CSV PARAMETERS...\n";
    out<<"  methods-depth-concentration CSV\n";
    out<<"  methods-active-factor-risk CSV PARAMETERS...\n";
    out<<"  methods-variance-shapley CSV PARAMETERS...\n";
    out<<"  methods-binary-kelly CSV PARAMETERS...\n";
    out<<"  methods-liability-surplus CSV PARAMETERS...\n";
    out<<"  methods-event-car CSV PARAMETERS...\n";
    out<<"  methods-paired-randomization CSV\n";
    out<<"  methods-ipw-balance CSV\n";
    out<<"  methods-placebo-rank CSV\n";
    out<<"  methods-debt-dynamics CSV PARAMETERS...\n";
    out<<"  methods-gdp-growth-contributions CSV\n";
    out<<"  methods-real-fx-decomposition CSV\n";
    out<<"  methods-uip-carry CSV\n";
    out<<"  methods-sequence-gap CSV\n";
    out<<"  methods-asof-staleness CSV PARAMETERS...\n";
    out<<"  methods-latency-tail CSV PARAMETERS...\n";
    out<<"  methods-position-cash-reconcile CSV PARAMETERS...\n";
    out<<"  depth-backward-error CSV\n";
    out<<"  depth-row-equilibration CSV\n";
    out<<"  depth-two-by-two-spectrum CSV\n";
    out<<"  depth-precision-conditional CSV\n";
    out<<"  depth-group-prox CSV PARAMETERS...\n";
    out<<"  depth-affine-projection CSV PARAMETERS...\n";
    out<<"  depth-bounded-budget CSV PARAMETERS...\n";
    out<<"  depth-huber-prox CSV PARAMETERS...\n";
    out<<"  depth-discrete-convolution CSV\n";
    out<<"  depth-latent-binomial CSV PARAMETERS...\n";
    out<<"  depth-exponential-tilt CSV PARAMETERS...\n";
    out<<"  depth-randomized-pit CSV\n";
    out<<"  depth-reliability-variance CSV\n";
    out<<"  depth-pairwise-missing CSV\n";
    out<<"  depth-group-jackknife CSV\n";
    out<<"  depth-trim-winsor CSV PARAMETERS...\n";
    out<<"  depth-ar-impulse CSV PARAMETERS...\n";
    out<<"  depth-ar-forecast-variance CSV PARAMETERS...\n";
    out<<"  depth-filter-frequency CSV PARAMETERS...\n";
    out<<"  depth-lag-aligned-cov CSV PARAMETERS...\n";
    out<<"  depth-paired-student CSV\n";
    out<<"  depth-equivalence-tost CSV PARAMETERS...\n";
    out<<"  depth-survey-hajek CSV PARAMETERS...\n";
    out<<"  depth-fisher-exact CSV\n";
    out<<"  depth-nystrom-schur CSV PARAMETERS...\n";
    out<<"  depth-rff-pair CSV PARAMETERS...\n";
    out<<"  depth-mahalanobis-residual CSV\n";
    out<<"  depth-outofsample-kernel-center CSV PARAMETERS...\n";
    out<<"  depth-binary-focal CSV PARAMETERS...\n";
    out<<"  depth-huberized-pinball CSV PARAMETERS...\n";
    out<<"  depth-logit-intercept-score CSV PARAMETERS...\n";
    out<<"  depth-temperature-curvature CSV PARAMETERS...\n";
    out<<"  depth-logistic-hvp CSV PARAMETERS...\n";
    out<<"  depth-softmax-jvp CSV\n";
    out<<"  depth-gradient-noise CSV\n";
    out<<"  depth-global-gradient-clip CSV PARAMETERS...\n";
    out<<"  depth-purge-interval CSV PARAMETERS...\n";
    out<<"  depth-train-only-standardization CSV\n";
    out<<"  depth-forecast-oos-r2 CSV\n";
    out<<"  depth-block-loss-decomposition CSV\n";
    out<<"  depth-mixture-predictive-variance CSV\n";
    out<<"  depth-ensemble-entropy-mi CSV\n";
    out<<"  depth-mondrian-calibration CSV PARAMETERS...\n";
    out<<"  depth-wilson-coverage CSV PARAMETERS...\n";
    out<<"  depth-weighted-group-demean CSV\n";
    out<<"  depth-train-missing-indicator CSV\n";
    out<<"  depth-ew-prior-zscore CSV PARAMETERS...\n";
    out<<"  depth-regime-covariance CSV\n";
    out<<"  depth-digital-decomposition CSV\n";
    out<<"  depth-vanna-volga CSV\n";
    out<<"  depth-delta-hedge-variance CSV PARAMETERS...\n";
    out<<"  depth-spot-vol-taylor CSV PARAMETERS...\n";
    out<<"  depth-log-discount-forward CSV\n";
    out<<"  depth-forward-swap-annuity CSV PARAMETERS...\n";
    out<<"  depth-fra-settlement CSV PARAMETERS...\n";
    out<<"  depth-overnight-accrual-bridge CSV\n";
    out<<"  depth-node-martingale CSV\n";
    out<<"  depth-numeraire-measure CSV\n";
    out<<"  depth-stopping-policy-value CSV\n";
    out<<"  depth-dual-stopping-bound CSV\n";
    out<<"  depth-forward-default CSV\n";
    out<<"  depth-margin-period-risk CSV\n";
    out<<"  depth-heterogeneous-credit-mixture CSV\n";
    out<<"  depth-exponential-hazard-mle CSV\n";
    out<<"  depth-ellipsoid-worst-mean CSV PARAMETERS...\n";
    out<<"  depth-stochastic-dominance CSV\n";
    out<<"  depth-single-asset-no-trade CSV PARAMETERS...\n";
    out<<"  depth-mean-estimation-risk CSV PARAMETERS...\n";
    out<<"  depth-forward-variance CSV\n";
    out<<"  depth-noise-signature CSV PARAMETERS...\n";
    out<<"  depth-lognormal-vol-convexity CSV\n";
    out<<"  depth-gjr-news-impact CSV PARAMETERS...\n";
    out<<"  depth-execution-kkt CSV PARAMETERS...\n";
    out<<"  depth-competing-execution-hazards CSV PARAMETERS...\n";
    out<<"  depth-queue-cancel-bounds CSV PARAMETERS...\n";
    out<<"  depth-quote-bayesian-selection CSV PARAMETERS...\n";
    out<<"  depth-bitemporal-asof CSV PARAMETERS...\n";
    out<<"  depth-event-watermark CSV PARAMETERS...\n";
    out<<"  depth-event-idempotency CSV\n";
    out<<"  depth-tick-grid CSV PARAMETERS...\n";
    out<<"  depth-unbalanced-two-way-projection CSV PARAMETERS...\n";
    out<<"  depth-manski-missing-bounds CSV PARAMETERS...\n";
    out<<"  depth-lee-trimming-bounds CSV\n";
    out<<"  depth-synthetic-control-gap CSV PARAMETERS...\n";
    out<<"  depth-leontief-demand CSV PARAMETERS...\n";
    out<<"  depth-fisher-price-index CSV\n";
    out<<"  depth-shift-share-exposure CSV\n";
    out<<"  depth-taylor-inertia CSV PARAMETERS...\n";
    out<<"  depth-mixed-nash-interior CSV\n";
    out<<"  depth-cournot-active-set CSV PARAMETERS...\n";
    out<<"  depth-auction-uniform-bid CSV PARAMETERS...\n";
    out<<"  depth-kyle-linear-equilibrium CSV\n";
    out<<"  depth-tie-aware-tail-weights CSV PARAMETERS...\n";
    out<<"  depth-size-value-six-cells CSV\n";
    out<<"  depth-beta-posterior-shrink CSV PARAMETERS...\n";
    out<<"  depth-constant-mix-diversification CSV PARAMETERS...\n";
    out<<"  model-fista-elastic CSV PARAMETERS...\n";
    out<<"  model-fused-admm CSV PARAMETERS...\n";
    out<<"  model-box-covariance-qp CSV PARAMETERS...\n";
    out<<"  model-entropy-balance-fit CSV PARAMETERS...\n";
    out<<"  model-nnls-coordinate CSV PARAMETERS...\n";
    out<<"  model-graph-smoother CSV PARAMETERS...\n";
    out<<"  model-nb2-exposure CSV PARAMETERS...\n";
    out<<"  model-zip-em CSV PARAMETERS...\n";
    out<<"  model-cox-breslow-ridge CSV PARAMETERS...\n";
    out<<"  model-tobit-known-scale CSV PARAMETERS...\n";
    out<<"  model-shrinkage-lda CSV PARAMETERS...\n";
    out<<"  model-kernel-logistic-fit CSV PARAMETERS...\n";
    out<<"  model-var-ridge-forecast CSV PARAMETERS...\n";
    out<<"  model-kalman-grid-fit CSV PARAMETERS...\n";
    out<<"  model-hawkes-fixed-decay-fit CSV PARAMETERS...\n";
    out<<"  model-har-rv-forecast CSV PARAMETERS...\n";
    out<<"  model-ou-conditional-fit CSV PARAMETERS...\n";
    out<<"  model-dcc-twoasset-grid CSV PARAMETERS...\n";
    out<<"  model-american-put-pde CSV PARAMETERS...\n";
    out<<"  model-localvol-call-pde CSV PARAMETERS...\n";
    out<<"  model-asian-independent-control CSV PARAMETERS...\n";
    out<<"  model-cds-hazard-bootstrap CSV PARAMETERS...\n";
    out<<"  model-nelson-siegel-calibration CSV PARAMETERS...\n";
    out<<"  model-risk-neutral-entropy-fit CSV PARAMETERS...\n";
    out<<"  model-rolling-elastic-portfolio CSV PARAMETERS...\n";
    out<<"  model-rolling-covariance-qp CSV PARAMETERS...\n";
    out<<"  model-risk-budget-newton CSV PARAMETERS...\n";
    out<<"  model-synthetic-control-fit CSV PARAMETERS...\n";
    out<<"  model-crossfit-dml-ridge CSV PARAMETERS...\n";
    out<<"  model-rolling-conformal-ridge CSV PARAMETERS...\n";
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
