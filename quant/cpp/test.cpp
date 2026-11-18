#include "data.hpp"
#include "derivatives.hpp"
#include "execution.hpp"
#include "factors.hpp"
#include "io.hpp"
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

int main() {
    try {
        test_stats();
        test_risk();
        test_portfolio();
        test_derivatives();
        test_factors();
        test_execution();
        test_data();
        cout << "PASS: " << checks << " independent C++17 numerical and safety assertions\n";
        return 0;
    } catch (const exception& error) {
        cerr << error.what() << "\n";
        return 1;
    }
}
