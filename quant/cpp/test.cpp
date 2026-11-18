#include "data.hpp"
#include "derivatives.hpp"
#include "execution.hpp"
#include "factors.hpp"
#include "io.hpp"
#include "learning.hpp"
#include "ledger.hpp"
#include "macro.hpp"
#include "etf.hpp"
#include "attribution.hpp"
#include "simulation.hpp"
#include "validation.hpp"
#include "portfolio.hpp"
#include "risk.hpp"
#include "stats.hpp"
#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

int checks = 0;

void expect(bool condition, const string& message) {
    checks++;
    if (!condition) throw runtime_error("Test failed: " + message);
}

void near(double actual, double expected, double tolerance, const string& message) {
    expect(isfinite(actual) && fabs(actual - expected) <= tolerance, message);
}

void expect_error(const function<void()>& run, const string& message) {
    bool threw = false;
    try { run(); } catch (const exception&) { threw = true; }
    expect(threw, message);
}

void test_stats() {
    vector<double> x = {1, 2, 3, 4};
    near(kline::mean(x), 2.5, 1e-12, "mean");
    near(kline::variance(x), 5.0 / 3, 1e-12, "sample variance");
    near(kline::variance(x, false), 1.25, 1e-12, "population variance");
    near(kline::covariance(x, x), 5.0 / 3, 1e-12, "covariance");
    near(kline::correlation(x, x), 1, 1e-12, "correlation");
    near(kline::correlation(x, {4, 3, 2, 1}), -1, 1e-12, "negative correlation");
    near(kline::spearman({1, 2, 2, 4}, {10, 20, 20, 40}), 1, 1e-12, "tied ranks");
    auto ranked = kline::ranks({3, 2, 2, 1});
    near(ranked[1], 2.5, 1e-12, "average rank for ties");
    near(kline::quantile(x, 0.5), 2.5, 1e-12, "median interpolation");
    near(kline::quantile(x, 0), 1, 1e-12, "quantile lower boundary");
    near(kline::quantile(x, 1), 4, 1e-12, "quantile upper boundary");
    auto rolling = kline::rolling_mean(x, 2);
    expect(rolling.size() == 3, "rolling count");
    near(rolling[0], 1.5, 1e-12, "rolling sum");
    near(rolling[2], 3.5, 1e-12, "rolling final window");
    near(kline::hac_mean_standard_error({1, 2, 3, 4}, 0),
         sqrt(1.25 / 4), 1e-12, "HAC lag zero");
    auto logs = kline::log_returns({100, 110, 121});
    near(logs[0], log(1.1), 1e-12, "log return");
    near(logs[1], log(1.1), 1e-12, "log return second");
    expect_error([&]() { kline::variance({1}); }, "sample variance rejects one row");
    expect_error([&]() { kline::correlation({1, 1}, {2, 3}); },
                 "correlation rejects constant factor");
    expect_error([&]() { kline::hac_mean_standard_error(x, 4); }, "HAC lag length");
    expect_error([&]() { kline::log_returns({100, 0}); }, "price cannot be zero");
}

void test_risk() {
    auto dd = kline::drawdown_from_equity({100, 110, 100, 88, 112, 100});
    near(dd.worst, -0.2, 1e-12, "drawdown from prior peak");
    expect(dd.longest_underwater == 2, "underwater duration");
    expect(dd.current_underwater == 1, "current underwater duration");
    near(kline::historical_var({1, 2, 10}, 0.5), 2, 1e-12, "historical VaR");
    near(kline::historical_expected_shortfall({1, 2, 10}, 0.5),
         11.0 / 1.5, 1e-12, "fractional sample tail mass");
    near(kline::historical_expected_shortfall({1, 2, 10}, 0.95), 10, 1e-12,
         "extreme empirical ES");
    near(kline::historical_expected_shortfall({10, 10, 1, 2}, 0.5), 10, 1e-12,
         "tail ties");
    near(kline::downside_deviation({0.1, -0.2, -0.1, 0}), sqrt(0.05 / 4), 1e-12,
         "downside second moment");
    near(kline::compounded_return({0.1, -0.1}), -0.01, 1e-12, "compounding");
    near(kline::cagr({0.1, -0.1}, 2), -0.01, 1e-12, "annualization");
    near(kline::stress_pnl({0.6, 0.4}, {-0.1, 0.2}), 0.02, 1e-12, "stress PnL");
    auto perf = kline::relative_performance({0.02, 0, -0.02},
                                            {0.01, 0, -0.01});
    near(perf.beta, 2, 1e-12, "CAPM beta");
    near(perf.alpha_per_period, 0, 1e-12, "periodic alpha");
    near(perf.tracking_error_annual, sqrt(0.0001 * 252), 1e-12,
         "tracking error annualization");
    near(perf.information_ratio, 0, 1e-12, "zero mean active return");
    expect_error([&]() { kline::historical_expected_shortfall({1, 2}, 1); },
                 "confidence cannot be one");
    expect_error([&]() { kline::compounded_return({-1}); }, "bankruptcy guard");
    expect_error([&]() { kline::drawdown_from_equity({100, -1}); },
                 "negative equity rejected");
}

void test_portfolio() {
    kline::Matrix cov = {{0.04, 0}, {0, 0.01}};
    auto minimum = kline::minimum_variance_long_only(cov);
    near(minimum[0], 0.2, 1e-7, "minimum variance weight A");
    near(minimum[1], 0.8, 1e-7, "minimum variance weight B");
    near(kline::portfolio_variance(cov, minimum), 0.008, 1e-8, "minimum variance");
    auto inverse = kline::inverse_volatility_weights(cov);
    near(inverse[0], 1.0 / 3, 1e-12, "inverse vol A");
    near(inverse[1], 2.0 / 3, 1e-12, "inverse vol B");
    auto contribution = kline::volatility_risk_contributions(cov, minimum);
    near(contribution[0] + contribution[1], sqrt(0.008), 1e-8,
         "Euler contributions add up to volatility");
    auto projected = kline::project_simplex({-1, 2, 2});
    near(projected[0], 0, 1e-12, "simplex forbids negative weight");
    near(projected[1] + projected[2], 1, 1e-12, "simplex sum to one");
    auto shrunk = kline::shrink_to_diagonal({{0.04, 0.01}, {0.01, 0.09}}, 0.5);
    near(shrunk[0][1], 0.005, 1e-12, "off-diagonal shrinkage");
    near(shrunk[0][0], 0.04, 1e-12, "variance preserved");
    near(kline::one_way_turnover({0.6, 0.4}, {0.3, 0.7}), 0.3, 1e-12,
         "one way turnover");
    near(kline::trading_cost({0.6, 0.4}, {0.3, 0.7}, 10), 0.0006, 1e-12,
         "both sides charged");
    auto drift = kline::drifted_weights({0.5, 0.5}, {0.1, 0});
    near(drift[0], 1.1 / 2.1, 1e-12, "drifted holding");
    auto cov_est = kline::sample_covariance_matrix({{1, 2, 3}, {2, 4, 6}});
    near(cov_est[1][1], 4 * cov_est[0][0], 1e-12, "matrix covariance");
    expect_error([&]() { kline::check_covariance({{1, 0.2}, {0.3, 1}}); },
                 "reject asymmetric covariance");
    expect_error([&]() { kline::shrink_to_diagonal(cov, 1.1); },
                 "reject invalid shrinkage intensity");
    expect_error([&]() { kline::trading_cost({1}, {1, 0}, 1); },
                 "reject mismatched weights");
}

void test_derivatives() {
    auto call = kline::black_scholes(100, 100, 0, 0, 0.2, 1, true);
    auto put = kline::black_scholes(100, 100, 0, 0, 0.2, 1, false);
    near(call.value, 7.9655674554058, 1e-9, "known at-the-money call");
    near(put.value, call.value, 1e-10, "zero-carry put-call equality");
    near(call.delta - put.delta, 1, 1e-12, "delta put-call parity");
    near(call.gamma, put.gamma, 1e-12, "same gamma");
    near(call.vega, put.vega, 1e-12, "same vega");
    near(kline::put_call_parity_residual(call.value, put.value, 100, 100, 0, 0, 1),
         0, 1e-10, "European parity");
    double recovered = kline::option_implied_volatility(call.value, 100, 100, 0, 0, 1);
    near(recovered, 0.2, 1e-10, "recover quoted implied vol");
    double american_put = kline::american_binomial(100, 100, 0.05, 0, 0.2, 1, 250, false);
    double european_put = kline::black_scholes(100, 100, 0.05, 0, 0.2, 1, false).value;
    expect(american_put >= european_put - 0.01, "American put at least European");
    double american_call = kline::american_binomial(100, 100, 0.05, 0, 0.2, 1, 250, true);
    double european_call = kline::black_scholes(100, 100, 0.05, 0, 0.2, 1, true).value;
    near(american_call, european_call, 0.03, "non-dividend American call convergence");
    auto bond = kline::fixed_coupon_bond(100, 0, 0.05, 2, 2);
    near(bond.price, 100 / pow(1 + 0.05 / 2, 4), 1e-10, "zero coupon price");
    near(bond.macaulay_duration, 2, 1e-12, "zero coupon Macaulay duration");
    near(bond.modified_duration, 2 / 1.025, 1e-12, "modified duration");
    near(bond.dv01, bond.price * bond.modified_duration * 0.0001, 1e-12, "DV01");
    expect_error([&]() { kline::option_implied_volatility(100, 100, 100, 0, 0, 1); },
                 "option premium no-arbitrage bound");
    expect_error([&]() { kline::american_binomial(100, 100, 0, 0, 0.2, 1, 0); },
                 "invalid binomial time steps");
}

void test_factors() {
    auto cap = kline::winsorize({0, 1, 2, 3, 100}, 0.2, 0.8);
    near(cap[0], 0.8, 1e-12, "winsorize lower");
    near(cap[4], 22.4, 1e-10, "winsorize upper");
    auto z = kline::zscore({1, 2, 3, 4});
    near(kline::mean(z), 0, 1e-12, "cross-sectional zscore mean");
    near(kline::variance(z, false), 1, 1e-12, "cross-sectional zscore variance");
    auto neutral = kline::group_neutralize({1, 3, 10, 14}, {0, 0, 1, 1});
    near(neutral[0] + neutral[1], 0, 1e-12, "industry neutral first group");
    near(neutral[2] + neutral[3], 0, 1e-12, "industry neutral second group");
    auto residual = kline::residualize_factor({2, 3, 6, 7}, {1, 2, 3, 4});
    near(kline::mean(residual), 0, 1e-12, "regression residual zero mean");
    near(kline::covariance(residual, {1, 2, 3, 4}), 0, 1e-12,
         "regression residual orthogonal to control");
    near(kline::quantile_long_short_spread({1, 2, 3, 4}, {0.01, 0.02, 0.03, 0.04}, 1),
         0.03, 1e-12, "factor long short quintile spread");
    near(kline::factor_coverage({1, 2, 3, 4}, {true, false, true, false}),
         0.5, 1e-12, "factor valid fraction");
    near(kline::feature_decay({1, 2, 3}, {2, 4, 6}), 1, 1e-12,
         "factor monotonic preservation");
    auto ic = kline::information_coefficients({{1, 2, 3}, {3, 2, 1}},
                                                {{2, 3, 4}, {1, 2, 3}});
    near(ic[0], 1, 1e-12, "positive IC");
    near(ic[1], -1, 1e-12, "negative IC");
    near(kline::ic_information_ratio({0.1, 0.3, 0.2}), 2, 0.1,
         "ICIR sample std convention");
    expect_error([&]() { kline::zscore({1, 1}); }, "constant factor cannot z-score");
    expect_error([&]() { kline::quantile_long_short_spread({1, 2}, {1, 2}, 2); },
                 "overlapping long and short sides rejected");
}

void test_execution() {
    kline::Quote q{99, 101, 30, 10};
    near(kline::quoted_spread(q), 2, 1e-12, "quoted spread");
    near(kline::relative_quoted_spread_bps(q), 200, 1e-12, "relative spread");
    near(kline::microprice(q), 100.5, 1e-12, "microprice uses opposite liquidity");
    near(kline::effective_spread_bps(q, 101, 1), 200, 1e-12, "buy effective spread");
    near(kline::effective_spread_bps(q, 99, -1), 200, 1e-12, "sell effective spread");
    near(kline::realized_spread_bps(q, 101, 100.5, 1), 100, 1e-12,
         "realized spread after price movement");
    vector<kline::ExecutionFill> fills = {{100, 10}, {102, 30}};
    near(kline::vwap(fills), 101.5, 1e-12, "execution VWAP");
    near(kline::execution_shortfall_bps(100, fills, 1), 150, 1e-12,
         "implementation shortfall");
    near(kline::fill_ratio(30, 100), 0.3, 1e-12, "partial fill");
    near(kline::executed_participation(50, 1000), 0.05, 1e-12, "POV");
    auto schedule = kline::volume_weighted_schedule({2, 3, 5}, 100);
    near(schedule[0], 20, 1e-12, "VWAP schedule");
    near(schedule[2], 50, 1e-12, "VWAP final interval");
    auto pov = kline::pov_schedule({10, 100, 100}, 12, 0.1);
    near(pov[0], 1, 1e-12, "POV first interval");
    near(pov[1], 10, 1e-12, "POV second interval");
    near(pov[2], 1, 1e-12, "POV remaining order");
    near(kline::square_root_impact_bps(0.02, 0.25, 0.5), 50, 1e-12,
         "square root impact in bps");
    expect_error([&]() { kline::quoted_spread({101, 99, 1, 1}); },
                 "crossed quote rejected");
    expect_error([&]() { kline::fill_ratio(120, 100); },
                 "overfill rejected");
    expect_error([&]() { kline::vwap({{100, -1}}); },
                 "negative executed quantity");
}

void test_data() {
    vector<kline::PriceBar> bars = {
        {"2026-01-01", 100, 105, 98, 101, 1000, 100, true},
        {"2026-01-02", 102, 106, 100, 104, 1200, 105, true},
        {"2026-01-03", 105, 108, 101, 106, 1100, 107, true}
    };
    kline::validate_bars(bars);
    auto total = kline::total_return_series(bars);
    near(total[0], 0.05, 1e-12, "adjusted return");
    near(total[1], 107.0 / 105 - 1, 1e-12, "adjusted second day");
    expect_error([&]() {
        auto wrong = bars;
        wrong[1].high = 99;
        kline::validate_bars(wrong);
    }, "invalid OHLC high");
    expect_error([&]() {
        auto wrong = bars;
        wrong[1].date = wrong[0].date;
        kline::validate_bars(wrong);
    }, "duplicate dates");
    expect_error([&]() {
        auto wrong = bars;
        wrong[1].has_adjusted_close = false;
        kline::total_return_series(wrong);
    }, "requires adjustment to call a return adjusted");
    auto joined = kline::asof_join(
        {{"2026-01-01", "2026-01-01", "2026-01-03", "rev1", 5},
         {"2026-01-02", "2026-01-04", "2026-01-04", "rev2", 8}},
        {"2026-01-02", "2026-01-03", "2026-01-04"});
    expect(!joined[0].first, "unavailable event must not leak");
    near(joined[1].second, 5, 1e-12, "first available event");
    near(joined[2].second, 8, 1e-12, "revision becomes usable after release");
    expect(!kline::labels_overlap({1, 3}, {4, 5}), "nonoverlapping intervals");
    expect(kline::labels_overlap({1, 3}, {3, 5}), "shared end boundary overlaps");
    auto keep = kline::purge_training_labels({{1, 2}, {3, 4}, {7, 8}},
                                              {{5, 6}}, 1);
    expect(keep.size() == 2 && keep[0] == 0 && keep[1] == 1, "purged overlap and embargo");
    auto labels = kline::forward_simple_returns({100, 110, 99, 105}, 2);
    near(labels[0], -0.01, 1e-12, "forward label at horizon two");
    expect(labels.size() == 2, "forward label count");
    auto dates = kline::common_dates({{"A", bars}, {"B", {bars[0], bars[2]}}});
    expect(dates.size() == 2, "aligned common dates");
    expect(dates[1] == "2026-01-03", "aligned final date");
    auto fields = kline::csv_fields("A,\"B,C\",\"say \"\"hi\"\"\"");
    expect(fields.size() == 3 && fields[1] == "B,C" && fields[2] == "say \"hi\"",
           "CSV quote escaping");
    near(kline::parse_number("12.5"), 12.5, 1e-12, "strict numeric CSV parser");
    expect_error([&]() { kline::parse_number("1.2usd"); }, "ambiguous numeric text");
    expect_error([&]() { kline::csv_fields("\"bad"); }, "unclosed quote");
}

void test_macro_and_etf() {
    near(kline::continuous_discount_factor(0.05, 2), exp(-0.1), 1e-12,
         "continuous zero discount");
    near(kline::continuous_forward_rate(0.03, 1, 0.04, 2), 0.05, 1e-12,
         "bootstrapped continuously compounded forward");
    near(kline::term_spread_bps(0.045, 0.035), 100, 1e-10,
         "term spread basis points");
    near(kline::breakeven_inflation(0.04, 0.015), 0.025, 1e-12,
         "nominal minus real yield");
    near(kline::duration_hedge_notional(1000000, 5, 10), -500000, 1e-9,
         "short duration hedge convention");
    near(kline::approximate_parallel_shift_pnl(1000000, 5, 40, 0.01),
         -48000, 1e-8, "duration and convexity shock PnL");
    near(kline::fx_forward_quote_per_base(1.2, 0.02, 0.05, 1),
         1.2 * 1.05 / 1.02, 1e-12, "FX covered parity direction");
    near(kline::futures_roll_yield(98, 100), -0.02, 1e-12,
         "contango simplified roll return");
    near(kline::excess_bond_return_approx(0.04, 3, 0.01, 15),
         0.01075, 1e-12, "bond return carry and convexity");

    near(kline::nav_premium_discount(102, 100), 0.02, 1e-12, "ETF NAV premium");
    near(kline::tracking_difference_annualized({0.01, 0.02}, {0.011, 0.021}),
         -0.252, 1e-12, "arithmetic tracking difference");
    near(kline::tracking_error_annualized({0.01, 0.02, 0.03},
                                          {0.009, 0.020, 0.031}),
         0.001 * sqrt(252.0), 1e-12, "annual ETF tracking error");
    near(kline::daily_expense_drag(0), 0, 1e-12, "zero expense ratio");
    double one_day = kline::daily_expense_drag(0.01);
    near(pow(1 - one_day, 252), 0.99, 1e-12,
         "daily expense compounds to one-year annual ratio");
    near(kline::gross_exposure({0.6, -0.4, 0.2}), 1.2, 1e-12,
         "ETF/CTA gross exposure");
    near(kline::net_exposure({0.6, -0.4, 0.2}), 0.4, 1e-12,
         "ETF/CTA net exposure");
    near(kline::volatility_target_multiplier(0.2, 0.1, 1.2), 0.5, 1e-12,
         "volatility target deleveraging");
    near(kline::volatility_target_multiplier(0.05, 0.2, 1.5), 1.5, 1e-12,
         "volatility target leverage cap");
    near(kline::total_return_with_cash_distribution(100, 98, 3), 0.01, 1e-12,
         "simple return with cash distribution");
    near(kline::exchange_traded_fund_liquidity_cost_bps(20, 2, 3),
         15, 1e-12, "half spread plus commission and impact");
    expect_error([&]() { kline::fx_forward_quote_per_base(1, -2, 0.04, 1); },
                 "covered parity invalid discount");
    expect_error([&]() { kline::nav_premium_discount(100, 0); },
                 "ETF nonpositive NAV");
    expect_error([&]() { kline::volatility_target_multiplier(0, 0.1, 2); },
                 "volatility target zero observed risk");
}

void test_validation_simulation_attribution() {
    auto folds = kline::walk_forward_folds(120, 60, 20, 5);
    expect(folds.size() == 2, "Walk-forward number of nonoverlapping test folds");
    expect(folds[0].train_begin == 0 && folds[0].train_end == 60 &&
           folds[0].test_begin == 65 && folds[0].test_end == 85,
           "Time split with embargo");
    expect(folds[1].train_begin == 20 && folds[1].test_begin == 85,
           "Rolling training window");
    auto expanding = kline::walk_forward_folds(120, 60, 20, 5, true);
    expect(expanding[1].train_begin == 0, "Expanding training window");
    auto idx1 = kline::moving_block_resample(20, 4, 42);
    auto idx2 = kline::moving_block_resample(20, 4, 42);
    expect(idx1 == idx2 && idx1.size() == 20, "Seeded block bootstrap reproducibility");
    for (int index : idx1) expect(index >= 0 && index < 20, "Bootstrap indices valid");
    auto bootstrap = kline::block_bootstrap_means({1, 2, 3, 4, 5, 6}, 2, 30, 42);
    expect(bootstrap.size() == 30, "Bootstrap replicate count");
    near(kline::bonferroni_adjusted_p(0.01, 10), 0.1, 1e-12, "Bonferroni adjustment");
    near(kline::bonferroni_adjusted_p(0.6, 10), 1, 1e-12, "Bonferroni cap");
    auto fdr = kline::benjamini_hochberg({0.03, 0.001, 0.02, 0.2}, 0.05);
    expect(fdr[0] && fdr[1] && fdr[2] && !fdr[3], "BH step-up retains ordering");
    near(kline::brier_score({0.2, 0.8}, {0, 1}), 0.04, 1e-12, "Brier score");
    auto bins = kline::reliability_bins({0.1, 0.2, 0.8, 0.9}, {0, 1, 1, 1}, 2);
    expect(bins[0].observations == 2 && bins[1].observations == 2,
           "Calibration sample bins");
    near(bins[0].observed_frequency, 0.5, 1e-12, "Calibration observed frequency");
    near(bins[1].predicted_frequency, 0.85, 1e-12, "Calibration average probability");
    expect_error([&]() { kline::walk_forward_folds(50, 0, 5); },
                 "Zero training window disallowed");
    expect_error([&]() { kline::moving_block_resample(10, 11, 1); },
                 "Bootstrap block longer than sample");
    expect_error([&]() { kline::brier_score({1.2}, {1}); },
                 "Probabilities above one rejected");

    auto deterministic = kline::geometric_brownian_path(100, 0.05, 0, 1,
                                                        {0, 1, -1});
    near(deterministic[1], 100 * exp(0.05), 1e-10,
         "GBM zero volatility solves deterministic exponential");
    expect(deterministic.size() == 4, "GBM includes initial time");
    auto ou = kline::ornstein_uhlenbeck_path(2, 0.5, 1, 0, 1, {0, 0});
    near(ou[1], 1.5, 1e-12, "OU mean reversion");
    near(ou[2], 1.25, 1e-12, "OU second deterministic step");
    auto cir = kline::cir_full_truncation_path(0.001, 2, 0.001, 2, 1, {-5, 0});
    for (double value : cir) expect(value >= 0, "CIR nonnegative truncated path");
    auto markov = kline::markov_distribution_after_steps(
        {{0.9, 0.1}, {0.2, 0.8}}, {1, 0}, 2);
    near(markov[0], 0.83, 1e-12, "Markov two-step distribution");
    near(markov[0] + markov[1], 1, 1e-12, "Markov total probability");
    auto mc = kline::monte_carlo_european_option(
        100, 100, 0.05, 0, 0.2, 1, true, 30000, 2026);
    auto bs = kline::black_scholes(100, 100, 0.05, 0, 0.2, 1, true);
    expect(std::fabs(mc.estimate - bs.value) < 0.2,
           "Antithetic Monte Carlo within pricing tolerance");
    expect(mc.standard_error > 0 && mc.simulated_pairs == 30000,
           "Monte Carlo uncertainty and number of antithetic pairs");
    auto mc_again = kline::monte_carlo_european_option(
        100, 100, 0.05, 0, 0.2, 1, true, 30000, 2026);
    near(mc.estimate, mc_again.estimate, 1e-12, "Fixed Monte Carlo seed");
    expect_error([&]() {
        kline::markov_distribution_after_steps({{0.5, 0.4}, {0.2, 0.8}}, {1, 0}, 2);
    }, "Markov probabilities must be normalized");

    auto brinson = kline::brinson_fachler_single_period(
        {0.6, 0.4}, {0.5, 0.5}, {0.08, 0.03}, {0.05, 0.02});
    near(brinson.active_return, 0.025, 1e-12, "Portfolio benchmark active return");
    near(brinson.explained_active_return, brinson.active_return, 1e-12,
         "Brinson allocation, selection and interaction reconcile");
    near(brinson.allocation_by_sector[0] + brinson.allocation_by_sector[1],
         0.003, 1e-12, "Brinson-Fachler allocation");
    near(brinson.selection_by_sector[0] + brinson.selection_by_sector[1],
         0.02, 1e-12, "Brinson selection");
    near(brinson.interaction_by_sector[0] + brinson.interaction_by_sector[1],
         0.002, 1e-12, "Brinson interaction");
    auto factor_attr = kline::factor_return_attribution({0.8, -0.2},
                                                        {0.05, 0.01}, 0.06);
    near(factor_attr.systematic_return, 0.038, 1e-12,
         "Linear factor explained return");
    near(factor_attr.idiosyncratic_return, 0.022, 1e-12,
         "Unexplained factor return");
    auto contributions = kline::asset_return_contributions({0.6, 0.4},
                                                            {0.05, -0.01});
    near(kline::single_period_active_return(contributions), 0.026, 1e-12,
         "Portfolio beginning-weight return contribution");
    expect_error([&]() {
        kline::brinson_fachler_single_period({0.5, 0.4}, {0.5, 0.5},
                                             {0.1, 0}, {0.05, 0.01});
    }, "Brinson weights must sum to one");
}


void test_learning_and_ledger() {
    auto solution = kline::solve_linear_system({{3, 2}, {1, 2}}, {7, 5});
    near(solution[0], 1, 1e-12, "pivoted linear solve first coefficient");
    near(solution[1], 2, 1e-12, "pivoted linear solve second coefficient");
    expect_error([&]() {
        kline::solve_linear_system({{1, 1}, {2, 2}}, {3, 6});
    }, "reject singular normal equations");

    kline::Matrix x = {{0}, {1}, {2}, {3}, {4}};
    vector<double> y = {1, 3, 5, 7, 9};
    auto exact = kline::fit_ridge(x, y, 0);
    near(kline::ridge_predict(exact, {5}), 11, 1e-10,
         "OLS limit of ridge recovers intercept and slope");
    near(kline::ridge_training_mse(exact, x, y), 0, 1e-12, "fitted model residual");
    auto regularized = kline::fit_ridge(x, y, 1);
    near(kline::ridge_predict(regularized, {4}), 7, 1e-12,
         "ridge penalty shrinks standardized coefficient");
    near(kline::ridge_predict(regularized, {2}), 5, 1e-12,
         "ridge leaves the intercept unpenalized");
    auto flat = kline::fit_ridge({{5}, {5}, {5}}, {1, 2, 3}, 1);
    near(kline::ridge_predict(flat, {5}), 2, 1e-12,
         "ridge handles a constant predictor");
    expect_error([&]() {
        kline::fit_ridge({{1, 2}, {2, 4}, {3, 6}}, {1, 2, 3}, 0);
    }, "unpenalized collinear normal equations are rejected");
    expect_error([&]() { kline::fit_ridge(x, y, -1); },
                 "reject negative ridge penalty");
    expect_error([&]() { kline::ridge_predict(exact, {1, 2}); },
                 "predictor count must match training");

    kline::Matrix features;
    vector<double> labels;
    vector<int> matured;
    for (int i = 0; i < 12; i++) {
        features.push_back({double(i)});
        labels.push_back(1 + 2.0 * i);
        matured.push_back(i + 2);
    }
    auto predictions = kline::forward_ridge(features, labels, matured,
                                             5, 2, 5, 1, 0);
    expect(predictions.size() == 7, "forward forecast count");
    near(predictions[0].prediction, 11, 1e-11,
         "first forecast uses mature training labels only");
    expect(predictions[0].fitted_at == 5 && predictions[0].training_rows == 3,
           "purging and embargo remove immature labels");
    auto revised = labels;
    for (int i = 3; i < int(revised.size()); i++) revised[i] = -999;
    auto unchanged = kline::forward_ridge(features, revised, matured,
                                           5, 2, 5, 1, 0);
    near(unchanged.front().prediction, predictions.front().prediction, 1e-12,
         "future labels cannot affect the previous forecast");
    expect_error([&]() {
        kline::forward_ridge(features, labels, matured, 2, 2, 3, 1, 0);
    }, "reject folds without two mature labels");
    expect_error([&]() {
        auto invalid_end = matured;
        invalid_end[0] = 0;
        kline::forward_ridge(features, labels, invalid_end, 5, 2, 5, 1, 0);
    }, "label maturity must be after feature time");

    auto shrink = kline::ledoit_wolf_identity({{1, 3, 5, 7}, {7, 5, 3, 1}});
    expect(shrink.intensity >= 0 && shrink.intensity <= 1,
           "shrinkage stays within its statistical bounds");
    near(shrink.covariance[0][0], shrink.covariance[1][1], 1e-12,
         "equal asset variances stay equal");
    near(shrink.covariance[0][1], shrink.covariance[1][0], 1e-12,
         "shrunk covariance is symmetric");
    expect(kline::portfolio_variance(shrink.covariance, {1, -1}) >= 0,
           "shrunk covariance stays nonnegative for a long-short portfolio");
    auto identical = kline::ledoit_wolf_identity({{1, 1, 1}, {1, 1, 1}});
    near(identical.covariance[0][0], 0, 1e-12, "constant input covariance is zero");
    expect_error([&]() { kline::ledoit_wolf_identity({{1}, {2}}); },
                 "reject covariance with one time observation");

    auto periods = kline::simulate_long_only_rebalance(
        {{1}, {1}, {0}}, {{0.1}, {0}, {0}}, 100);
    near(periods[0].ending_equity, 1.1 / 1.01, 1e-12,
         "first entry pays the exact cash-financed commission");
    near(periods[1].trading_cost, 0, 1e-12,
         "no cost when target equals price-drifted exposure");
    near(periods[2].ending_equity,
         (1.1 / 1.01) * 0.99, 1e-12,
         "full exit pays a percentage of the sold notional");
    auto equity = kline::equity_curve_from_periods(periods);
    expect(equity.size() == 4, "equity curve includes the starting balance");
    near(equity.back(), periods.back().ending_equity, 1e-12,
         "period ledger reconciles with total equity");
    auto two_assets = kline::simulate_long_only_rebalance(
        {{0.5, 0.5}, {0.5, 0.5}}, {{0.1, -0.1}, {0, 0}}, 100);
    near(two_assets[0].gross_return, 0, 1e-12,
         "equally weighted opposing returns net to zero before costs");
    expect(two_assets[1].traded_notional > 0,
           "price drift creates turnover during rebalancing");
    expect_error([&]() {
        kline::simulate_long_only_rebalance({{0.7, 0.6}}, {{0, 0}}, 10);
    }, "reject infeasible cash budget");
    expect_error([&]() {
        kline::simulate_long_only_rebalance({{1}}, {{-1}}, 0);
    }, "reject exhausted portfolio equity");
}

int main() {
    try {
        test_stats();
        test_risk();
        test_portfolio();
        test_derivatives();
        test_factors();
        test_execution();
        test_data();
        test_macro_and_etf();
        test_validation_simulation_attribution();
        test_learning_and_ledger();
        cout << "PASS: " << checks << " independent C++17 numerical and safety assertions\n";
        return 0;
    } catch (const exception& error) {
        cerr << error.what() << "\n";
        return 1;
    }
}
