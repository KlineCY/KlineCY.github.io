#include "advanced_cli.hpp"
#include "fullstack_cli.hpp"
#include "expansion_cli_math.hpp"
#include "expansion_cli_finance.hpp"
#include "research_math_cli.hpp"
#include "research_finance_cli.hpp"
#include "crossdomain_cli.hpp"
#include "sitewide_math_ml_cli.hpp"
#include "sitewide_finance_cli.hpp"
#include "massive_stats_cli.hpp"
#include "massive_finance_cli.hpp"
#include "empirical_lab_cli.hpp"
#include "domain_lab_cli.hpp"
#include "atlas2026_cli.hpp"
#include "lab24_cli.hpp"
#include "researchflow_cli.hpp"
#include "numerical_lab_cli.hpp"
#include "data.hpp"
#include "allocation.hpp"
#include "derivatives.hpp"
#include "execution.hpp"
#include "factors.hpp"
#include "io.hpp"
#include "learning.hpp"
#include "ledger.hpp"
#include "etf.hpp"
#include "macro.hpp"
#include "attribution.hpp"
#include "simulation.hpp"
#include "validation.hpp"
#include "portfolio.hpp"
#include "risk.hpp"
#include "stats.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

void usage() {
    cerr << "Kline Quant C++17 research toolkit\n"
         << "  stats CSV COLUMN\n"
         << "  risk CSV RETURN_COLUMN [CONFIDENCE=0.95]\n"
         << "  portfolio CSV RETURN_COLUMN_A RETURN_COLUMN_B [MORE_COLUMNS...]\n"
         << "  factor CSV FACTOR_COLUMN FORWARD_RETURN_COLUMN\n"
         << "  relative CSV STRATEGY_RETURN_COLUMN BENCHMARK_RETURN_COLUMN\n"
         << "  option SPOT STRIKE RATE DIVIDEND VOLATILITY YEARS call|put\n"
         << "  implied SPOT STRIKE RATE DIVIDEND MARKET_PREMIUM YEARS call|put\n"
         << "  american SPOT STRIKE RATE DIVIDEND VOLATILITY YEARS STEPS call|put\n"
         << "  bond FACE ANNUAL_COUPON_RATE YIELD YEARS PAYMENTS_PER_YEAR\n"
         << "  quote BID ASK BID_SIZE ASK_SIZE FILL_PRICE buy|sell FUTURE_MID\n"
         << "  fills CSV ARRIVAL_PRICE buy|sell (requires Price,Quantity)\n"
         << "  bars CSV (requires Date,Open,High,Low,Close,Volume,AdjustedClose)\n"
         << "  etf CSV FUND_RETURN_COLUMN INDEX_RETURN_COLUMN\n"
         << "  curve SHORT_YEARS SHORT_ZERO_RATE LONG_YEARS LONG_ZERO_RATE\n"
         << "  fx SPOT_QUOTE_PER_BASE BASE_RATE QUOTE_RATE YEARS\n"
         << "  duration-hedge ASSET_MARKET_VALUE ASSET_DURATION HEDGE_DURATION\n"
         << "  futures-roll NEAR_PRICE FAR_PRICE\n"
         << "  walkforward OBSERVATIONS TRAIN TEST EMBARGO [EXPANDING=0|1]\n"
         << "  bootstrap CSV RETURN_COLUMN BLOCK_LENGTH REPLICATES SEED\n"
         << "  calibration CSV PROBABILITY_COLUMN LABEL_COLUMN [BINS=10]\n"
         << "  montecarlo SPOT STRIKE RATE DIVIDEND VOL YEARS call|put PAIRS SEED\n"
         << "  brinson CSV PORTFOLIO_WEIGHT BENCHMARK_WEIGHT PORTFOLIO_RETURN BENCHMARK_RETURN\n"
         << "  ridge CSV LABEL LAMBDA FEATURE [MORE_FEATURES...]\n"
         << "  ridge-forward CSV LABEL MATURITY_INDEX FIRST_TRAIN STEP WINDOW GAP LAMBDA FEATURE...\n"
         << "  ledoit-wolf CSV ASSET_RETURN_A ASSET_RETURN_B [MORE_COLUMNS...]\n"
         << "  rebalance CSV COST_BPS CASH_RETURN WEIGHT_A RETURN_A [MORE_PAIRS...]\n"
         << "  longshort CSV COST_BPS CASH_RATE FUND_RATE BORROW_RATE MAX_GROSS WEIGHT RETURN [PAIRS...]\n"
         << "  allocate CSV RISK_LAMBDA TURNOVER_PENALTY ASSET_COLUMN EXPECTED_RETURN PREVIOUS_WEIGHT [MORE_TRIPLES...]\n"
         << "  bl-pair CSV ASSET_COLUMN_A ASSET_COLUMN_B PRIOR_A PRIOR_B VIEW_A_MINUS_B VIEW_VARIANCE TAU\n"
         << "  asof EVENT_CSV DECISION_CSV (event columns Date,Published,Received,Value)\n";
    kline::advanced_cli_usage(cerr);
    kline::print_fullstack_usage(cerr);
    kline::expansion_math_usage(cerr);
    kline::expansion_finance_usage(cerr);
    kline::research_math_cli_usage(cerr);
    kline::research_finance_cli_usage(cerr);
    kline::crossdomain_cli_usage(cerr);
    kline::sitewide_math_ml_usage(cerr);
    kline::sitewide_finance_usage(cerr);
    kline::massive_stats_usage(cerr);
    kline::massive_finance_usage(cerr);
    kline::empirical_lab_usage(cerr);
    kline::domain_lab_usage(cerr);
    kline::atlas2026_usage(cerr);
    kline::lab24_usage(cerr);
    kline::researchflow_usage(cerr);
    kline::numerical_lab_usage(cerr);
}

double number(const char* value) {
    return kline::parse_number(value);
}

void require_arguments(int argc, int needed) {
    kline::require(argc == needed, "Incorrect number of arguments");
}

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            usage();
            return 2;
        }
        string command = argv[1];
        cout << fixed << setprecision(8);
        if (command == "stats") {
            require_arguments(argc, 4);
            auto x = kline::read_csv(argv[2]).numbers(argv[3]);
            cout << "Count " << x.size() << "\n"
                 << "Mean " << kline::mean(x) << "\n"
                 << "SampleVariance " << kline::variance(x) << "\n"
                 << "Median " << kline::quantile(x, 0.5) << "\n"
                 << "HAC_SE_1 " << kline::hac_mean_standard_error(x, 1) << "\n";
        } else if (command == "risk") {
            kline::require(argc == 4 || argc == 5, "Use risk CSV COLUMN [CONFIDENCE]");
            auto returns = kline::read_csv(argv[2]).numbers(argv[3]);
            double confidence = argc == 5 ? number(argv[4]) : 0.95;
            vector<double> losses, equity = {1};
            for (double r : returns) {
                kline::require(r > -1, "Returns cannot be <= -100%");
                losses.push_back(-r);
                equity.push_back(equity.back() * (1 + r));
            }
            auto dd = kline::drawdown_from_equity(equity);
            cout << "HistoricalVaR " << kline::historical_var(losses, confidence) << "\n"
                 << "HistoricalES " << kline::historical_expected_shortfall(losses, confidence) << "\n"
                 << "AnnualVolatility " << kline::annual_volatility(returns) << "\n"
                 << "CAGR " << kline::cagr(returns) << "\n"
                 << "MaximumDrawdown " << dd.worst << "\n"
                 << "LongestUnderwaterDays " << dd.longest_underwater << "\n";
            if (kline::variance(returns) > 0)
                cout << "SharpeNoRiskFree " << kline::sharpe_ratio(returns) << "\n";
        } else if (command == "portfolio") {
            kline::require(argc >= 5, "Portfolio requires two or more return columns");
            auto table = kline::read_csv(argv[2]);
            vector<string> names;
            for (int i = 3; i < argc; i++) names.push_back(argv[i]);
            auto cov = kline::sample_covariance_matrix(
                kline::asset_returns_from_columns(table, names));
            auto minimum = kline::minimum_variance_long_only(cov);
            auto inverse = kline::inverse_volatility_weights(cov);
            auto contributions = kline::volatility_risk_contributions(cov, minimum);
            cout << "Asset,MinVarianceWeight,InverseVolWeight,VolatilityContribution\n";
            for (int i = 0; i < int(names.size()); i++)
                cout << names[i] << "," << minimum[i] << "," << inverse[i]
                     << "," << contributions[i] << "\n";
            cout << "PortfolioVolatility " << sqrt(kline::portfolio_variance(cov, minimum))
                 << "\n";
        } else if (command == "factor") {
            require_arguments(argc, 5);
            auto table = kline::read_csv(argv[2]);
            auto x = table.numbers(argv[3]), next = table.numbers(argv[4]);
            auto capped = kline::winsorize(x, 0.05, 0.95);
            int side = max(1, int(x.size()) / 5);
            cout << "RawIC " << kline::sample_information_coefficient(x, next, false) << "\n"
                 << "RankIC " << kline::sample_information_coefficient(x, next) << "\n"
                 << "CappedIC " << kline::sample_information_coefficient(capped, next, false) << "\n"
                 << "LongShortReturn " << kline::quantile_long_short_spread(x, next, side)
                 << "\n";
        } else if (command == "relative") {
            require_arguments(argc, 5);
            auto table = kline::read_csv(argv[2]);
            auto a = table.numbers(argv[3]), b = table.numbers(argv[4]);
            auto result = kline::relative_performance(a, b);
            cout << "Beta " << result.beta << "\n"
                 << "AlphaPerPeriod " << result.alpha_per_period << "\n"
                 << "TrackingErrorAnnual " << result.tracking_error_annual << "\n"
                 << "InformationRatio " << result.information_ratio << "\n";
        } else if (command == "option" || command == "implied") {
            require_arguments(argc, 9);
            double spot = number(argv[2]), strike = number(argv[3]), rate = number(argv[4]);
            double dividend = number(argv[5]), parameter = number(argv[6]);
            double years = number(argv[7]);
            string type = argv[8];
            kline::require(type == "call" || type == "put", "Option type is call or put");
            bool call = type == "call";
            if (command == "implied") {
                cout << "ImpliedVolatility " <<
                    kline::option_implied_volatility(parameter, spot, strike,
                                                     rate, dividend, years, call) << "\n";
            } else {
                auto result = kline::black_scholes(spot, strike, rate, dividend,
                                                   parameter, years, call);
                cout << "Premium " << result.value << "\n"
                     << "Delta " << result.delta << "\n"
                     << "Gamma " << result.gamma << "\n"
                     << "VegaPerUnitVol " << result.vega << "\n"
                     << "ThetaPerYear " << result.theta_per_year << "\n"
                     << "RhoPerUnitRate " << result.rho << "\n";
            }
        } else if (command == "american") {
            require_arguments(argc, 10);
            string type = argv[9];
            kline::require(type == "call" || type == "put", "Option type is call or put");
            cout << "AmericanOption " <<
                kline::american_binomial(number(argv[2]), number(argv[3]),
                                         number(argv[4]), number(argv[5]),
                                         number(argv[6]), number(argv[7]),
                                         stoi(argv[8]), type == "call") << "\n";
        } else if (command == "bond") {
            require_arguments(argc, 7);
            auto result = kline::fixed_coupon_bond(number(argv[2]), number(argv[3]),
                                                    number(argv[4]), stoi(argv[5]),
                                                    stoi(argv[6]));
            cout << "Price " << result.price << "\n"
                 << "MacaulayDuration " << result.macaulay_duration << "\n"
                 << "ModifiedDuration " << result.modified_duration << "\n"
                 << "DV01 " << result.dv01 << "\n"
                 << "Convexity " << result.convexity << "\n";
        } else if (command == "quote") {
            require_arguments(argc, 9);
            kline::Quote quote{number(argv[2]), number(argv[3]),
                               number(argv[4]), number(argv[5])};
            string direction = argv[7];
            kline::require(direction == "buy" || direction == "sell",
                           "Trade side is buy or sell");
            int side = direction == "buy" ? 1 : -1;
            cout << "QuotedSpreadBps " << kline::relative_quoted_spread_bps(quote) << "\n"
                 << "Microprice " << kline::microprice(quote) << "\n"
                 << "EffectiveSpreadBps " << kline::effective_spread_bps(quote, number(argv[6]), side) << "\n"
                 << "RealizedSpreadBps " << kline::realized_spread_bps(
                    quote, number(argv[6]), number(argv[8]), side) << "\n";
        } else if (command == "fills") {
            require_arguments(argc, 5);
            auto table = kline::read_csv(argv[2]);
            auto prices = table.numbers("Price"), quantities = table.numbers("Quantity");
            vector<kline::ExecutionFill> fills;
            for (int i = 0; i < int(prices.size()); i++)
                fills.push_back({prices[i], quantities[i]});
            string direction = argv[4];
            kline::require(direction == "buy" || direction == "sell",
                           "Trade side is buy or sell");
            int side = direction == "buy" ? 1 : -1;
            cout << "VWAP " << kline::vwap(fills) << "\n"
                 << "ImplementationShortfallBps " <<
                    kline::execution_shortfall_bps(number(argv[3]), fills, side) << "\n";
        } else if (command == "bars") {
            require_arguments(argc, 3);
            auto table = kline::read_csv(argv[2]);
            vector<kline::PriceBar> bars;
            int date_column = table.column("Date");
            auto open = table.numbers("Open"), high = table.numbers("High");
            auto low = table.numbers("Low"), close = table.numbers("Close");
            auto volume = table.numbers("Volume"), adjusted = table.numbers("AdjustedClose");
            for (int i = 0; i < int(table.rows.size()); i++)
                bars.push_back({table.rows[i][date_column], open[i], high[i], low[i],
                                close[i], volume[i], adjusted[i], true});
            auto returns = kline::total_return_series(bars);
            cout << "Bars " << bars.size() << "\n"
                 << "MeanAdjustedSimpleReturn " << kline::mean(returns) << "\n"
                 << "AdjustedCumulativeReturn " << kline::compounded_return(returns) << "\n";
        } else if (command == "etf") {
            require_arguments(argc, 5);
            auto table = kline::read_csv(argv[2]);
            auto fund = table.numbers(argv[3]), index = table.numbers(argv[4]);
            cout << "AnnualTrackingDifference " <<
                kline::tracking_difference_annualized(fund, index) << "\n"
                 << "AnnualTrackingError " <<
                kline::tracking_error_annualized(fund, index) << "\n";
        } else if (command == "curve") {
            require_arguments(argc, 6);
            double short_years = number(argv[2]), short_rate = number(argv[3]);
            double long_years = number(argv[4]), long_rate = number(argv[5]);
            cout << "ForwardContinuousZeroRate " <<
                kline::continuous_forward_rate(short_rate, short_years, long_rate, long_years)
                 << "\n"
                 << "LongDiscountFactor " <<
                kline::continuous_discount_factor(long_rate, long_years) << "\n";
        } else if (command == "fx") {
            require_arguments(argc, 6);
            cout << "CoveredParityForwardQuotePerBase " <<
                kline::fx_forward_quote_per_base(number(argv[2]), number(argv[3]),
                                                 number(argv[4]), number(argv[5])) << "\n";
        } else if (command == "duration-hedge") {
            require_arguments(argc, 5);
            cout << "HedgeNotional " <<
                kline::duration_hedge_notional(number(argv[2]), number(argv[3]),
                                               number(argv[4])) << "\n";
        } else if (command == "futures-roll") {
            require_arguments(argc, 4);
            cout << "StaticRollYield " <<
                kline::futures_roll_yield(number(argv[2]), number(argv[3])) << "\n";
        } else if (command == "walkforward") {
            kline::require(argc == 6 || argc == 7, "Invalid walkforward arguments");
            bool expanding = argc == 7 ? stoi(argv[6]) != 0 : false;
            auto folds = kline::walk_forward_folds(stoi(argv[2]), stoi(argv[3]),
                                                  stoi(argv[4]), stoi(argv[5]), expanding);
            cout << "TrainBegin,TrainEndExclusive,TestBegin,TestEndExclusive\n";
            for (const auto& fold : folds)
                cout << fold.train_begin << "," << fold.train_end << ","
                     << fold.test_begin << "," << fold.test_end << "\n";
        } else if (command == "bootstrap") {
            require_arguments(argc, 7);
            auto x = kline::read_csv(argv[2]).numbers(argv[3]);
            auto means = kline::block_bootstrap_means(x, stoi(argv[4]),
                                                       stoi(argv[5]), stoul(argv[6]));
            cout << "BootstrapMean " << kline::mean(means) << "\n"
                 << "Bootstrap025 " << kline::quantile(means, 0.025) << "\n"
                 << "Bootstrap975 " << kline::quantile(means, 0.975) << "\n";
        } else if (command == "calibration") {
            kline::require(argc == 5 || argc == 6, "Invalid calibration arguments");
            auto table = kline::read_csv(argv[2]);
            auto p = table.numbers(argv[3]), raw = table.numbers(argv[4]);
            vector<int> labels;
            for (double y : raw) {
                kline::require(y == 0 || y == 1, "Calibration labels must be 0 or 1");
                labels.push_back(int(y));
            }
            int bins = argc == 6 ? stoi(argv[5]) : 10;
            cout << "Brier " << kline::brier_score(p, labels) << "\n";
            auto groups = kline::reliability_bins(p, labels, bins);
            cout << "Bin,Count,MeanPredicted,ObservedFrequency\n";
            for (int i = 0; i < int(groups.size()); i++)
                cout << i << "," << groups[i].observations << ","
                     << groups[i].predicted_frequency << ","
                     << groups[i].observed_frequency << "\n";
        } else if (command == "montecarlo") {
            require_arguments(argc, 11);
            string type = argv[8];
            kline::require(type == "call" || type == "put", "Option type must be call or put");
            auto result = kline::monte_carlo_european_option(
                number(argv[2]), number(argv[3]), number(argv[4]), number(argv[5]),
                number(argv[6]), number(argv[7]), type == "call",
                stoi(argv[9]), stoul(argv[10]));
            cout << "MonteCarloPremium " << result.estimate << "\n"
                 << "MonteCarloStandardError " << result.standard_error << "\n"
                 << "AntitheticPairs " << result.simulated_pairs << "\n";
        } else if (command == "brinson") {
            require_arguments(argc, 7);
            auto table = kline::read_csv(argv[2]);
            auto result = kline::brinson_fachler_single_period(
                table.numbers(argv[3]), table.numbers(argv[4]),
                table.numbers(argv[5]), table.numbers(argv[6]));
            cout << "Sector,Allocation,Selection,Interaction\n";
            for (int i = 0; i < int(result.allocation_by_sector.size()); i++)
                cout << i << "," << result.allocation_by_sector[i] << ","
                     << result.selection_by_sector[i] << ","
                     << result.interaction_by_sector[i] << "\n";
            cout << "ActiveReturn " << result.active_return << "\n"
                 << "ExplainedActiveReturn " << result.explained_active_return << "\n";
        } else if (command == "ridge") {
            kline::require(argc >= 6, "Use ridge CSV LABEL LAMBDA FEATURE...");
            auto table = kline::read_csv(argv[2]);
            auto y = table.numbers(argv[3]);
            kline::Matrix x(y.size());
            vector<string> features;
            for (int arg = 5; arg < argc; arg++) {
                features.push_back(argv[arg]);
                auto column = table.numbers(argv[arg]);
                for (int t = 0; t < int(y.size()); t++) x[t].push_back(column[t]);
            }
            auto model = kline::fit_ridge(x, y, number(argv[4]));
            double raw_intercept = model.intercept;
            for (int j = 0; j < int(features.size()); j++)
                raw_intercept -= model.coefficients[j] *
                                 model.feature_mean[j] / model.feature_scale[j];
            cout << "Intercept " << raw_intercept << "\n";
            cout << "Feature,CoefficientPerRawUnit\n";
            for (int j = 0; j < int(features.size()); j++)
                cout << features[j] << "," <<
                    model.coefficients[j] / model.feature_scale[j] << "\n";
            cout << "TrainingMSE " << kline::ridge_training_mse(model, x, y) << "\n";
        } else if (command == "ridge-forward") {
            kline::require(argc >= 11,
                           "Use ridge-forward CSV LABEL MATURITY FIRST_TRAIN STEP WINDOW GAP LAMBDA FEATURE...");
            auto table = kline::read_csv(argv[2]);
            auto y = table.numbers(argv[3]);
            auto maturity_raw = table.numbers(argv[4]);
            vector<int> maturity;
            for (double value : maturity_raw) {
                kline::require(value >= 0 && value <= int(y.size()) + 1000000 &&
                               value == std::floor(value),
                               "Maturity index must be a nonnegative integer");
                maturity.push_back(int(value));
            }
            kline::Matrix x(y.size());
            for (int arg = 10; arg < argc; arg++) {
                auto column = table.numbers(argv[arg]);
                for (int t = 0; t < int(y.size()); t++) x[t].push_back(column[t]);
            }
            auto forecasts = kline::forward_ridge(x, y, maturity,
                stoi(argv[5]), stoi(argv[6]), stoi(argv[7]), stoi(argv[8]),
                number(argv[9]));
            cout << "Index,ModelFitAt,TrainingRows,Prediction,RealizedLabel\n";
            for (const auto& forecast : forecasts)
                cout << forecast.index << "," << forecast.fitted_at << ","
                     << forecast.training_rows << "," << forecast.prediction << ","
                     << y[forecast.index] << "\n";
        } else if (command == "ledoit-wolf") {
            kline::require(argc >= 5, "Use ledoit-wolf CSV ASSET_A ASSET_B...");
            auto table = kline::read_csv(argv[2]);
            vector<vector<double>> assets;
            vector<string> names;
            for (int arg = 3; arg < argc; arg++) {
                names.push_back(argv[arg]);
                assets.push_back(table.numbers(argv[arg]));
            }
            auto result = kline::ledoit_wolf_identity(assets);
            cout << "ShrinkageIntensity " << result.intensity << "\n";
            cout << "Asset";
            for (const auto& name : names) cout << "," << name;
            cout << "\n";
            for (int i = 0; i < int(names.size()); i++) {
                cout << names[i];
                for (double value : result.covariance[i]) cout << "," << value;
                cout << "\n";
            }
        } else if (command == "rebalance") {
            kline::require(argc >= 7 && (argc - 5) % 2 == 0,
                           "Use rebalance CSV COST_BPS CASH_RETURN WEIGHT RETURN...");
            auto table = kline::read_csv(argv[2]);
            kline::Matrix weights(table.rows.size()), returns(table.rows.size());
            for (int arg = 5; arg < argc; arg += 2) {
                auto weight = table.numbers(argv[arg]);
                auto asset_return = table.numbers(argv[arg + 1]);
                for (int t = 0; t < int(weight.size()); t++) {
                    weights[t].push_back(weight[t]);
                    returns[t].push_back(asset_return[t]);
                }
            }
            auto history = kline::simulate_long_only_rebalance(
                weights, returns, number(argv[3]), number(argv[4]));
            cout << "Index,BeginningEquity,TradedNotional,TradingCost,GrossReturn,NetReturn,EndingEquity\n";
            for (const auto& point : history)
                cout << point.index << "," << point.beginning_equity << ","
                     << point.traded_notional << "," << point.trading_cost << ","
                     << point.gross_return << "," << point.net_return << ","
                     << point.ending_equity << "\n";
        } else if (command == "longshort") {
            kline::require(argc >= 10 && (argc - 8) % 2 == 0,
                           "Use longshort CSV COST_BPS CASH_RATE FUND_RATE BORROW_RATE MAX_GROSS WEIGHT RETURN...");
            auto table = kline::read_csv(argv[2]);
            kline::Matrix weights(table.rows.size()), returns(table.rows.size());
            for (int arg = 8; arg < argc; arg += 2) {
                auto weight = table.numbers(argv[arg]);
                auto asset_return = table.numbers(argv[arg + 1]);
                for (int t = 0; t < int(weight.size()); t++) {
                    weights[t].push_back(weight[t]);
                    returns[t].push_back(asset_return[t]);
                }
            }
            auto history = kline::simulate_long_short_rebalance(
                weights, returns, number(argv[3]), number(argv[4]),
                number(argv[5]), number(argv[6]), number(argv[7]));
            cout << "Index,BeginningEquity,TradedNotional,TradingCost,ReturnBeforeTradingCost,NetReturn,EndingEquity\n";
            for (const auto& point : history)
                cout << point.index << "," << point.beginning_equity << ","
                     << point.traded_notional << "," << point.trading_cost << ","
                     << point.gross_return << "," << point.net_return << ","
                     << point.ending_equity << "\n";
        } else if (command == "allocate") {
            kline::require(argc >= 11 && (argc - 5) % 3 == 0,
                           "Use allocate CSV RISK TURNOVER RETURN_COLUMN MU PREVIOUS_WEIGHT...");
            auto table = kline::read_csv(argv[2]);
            vector<string> names;
            vector<vector<double>> returns;
            vector<double> expected, previous;
            for (int arg = 5; arg < argc; arg += 3) {
                names.push_back(argv[arg]);
                returns.push_back(table.numbers(argv[arg]));
                expected.push_back(number(argv[arg + 1]));
                previous.push_back(number(argv[arg + 2]));
            }
            auto shrink = kline::ledoit_wolf_identity(returns);
            auto result = kline::optimize_long_only(
                expected, shrink.covariance, previous,
                number(argv[3]), number(argv[4]));
            cout << "ShrinkageIntensity " << shrink.intensity << "\n";
            cout << "Asset,TargetWeight\n";
            for (int i = 0; i < int(names.size()); i++)
                cout << names[i] << "," << result.weights[i] << "\n";
            cout << "ExpectedReturn " << result.expected_return << "\n"
                 << "ExpectedVolatility " << result.expected_volatility << "\n"
                 << "HalfTurnover " << result.one_way_turnover << "\n"
                 << "PenalizedObjective " << result.objective << "\n";
        } else if (command == "bl-pair") {
            require_arguments(argc, 10);
            auto table = kline::read_csv(argv[2]);
            auto shrink = kline::ledoit_wolf_identity(
                {table.numbers(argv[3]), table.numbers(argv[4])});
            vector<double> prior = {number(argv[5]), number(argv[6])};
            auto posterior = kline::black_litterman_posterior(
                prior, shrink.covariance, number(argv[9]),
                {{1, -1}}, {number(argv[7])}, {number(argv[8])});
            cout << "ShrinkageIntensity " << shrink.intensity << "\n"
                 << "PosteriorA " << posterior[0] << "\n"
                 << "PosteriorB " << posterior[1] << "\n";
        } else if (command == "asof") {
            require_arguments(argc, 4);
            auto events = kline::read_csv(argv[2]);
            auto decisions = kline::read_csv(argv[3]);
            int date = events.column("Date"), published = events.column("Published");
            int received = events.column("Received");
            auto value = events.numbers("Value");
            vector<kline::EventRecord> e;
            for (int i = 0; i < int(events.rows.size()); i++)
                e.push_back({events.rows[i][date], events.rows[i][published],
                             events.rows[i][received], "", value[i]});
            int decision_col = decisions.column("Decision");
            vector<string> times;
            for (const auto& row : decisions.rows) times.push_back(row[decision_col]);
            auto joined = kline::asof_join(e, times);
            cout << "Decision,Value\n";
            for (int i = 0; i < int(times.size()); i++) {
                cout << times[i] << ",";
                if (joined[i].first) cout << joined[i].second;
                cout << "\n";
            }
        } else if (kline::run_advanced_cli(command,argc,argv,cout)) {
        } else if (kline::run_fullstack_cli(command,argc,argv,cout)) {
        } else if (kline::run_expansion_math(command,argc,argv,cout)) {
        } else if (kline::run_expansion_finance(command,argc,argv,cout)) {
        } else if (kline::run_research_math_cli(command,argc,argv,cout)) {
        } else if (kline::run_research_finance_cli(command,argc,argv,cout)) {
        } else if (kline::run_crossdomain_cli(command,argc,argv,cout)) {
        } else if (kline::run_sitewide_math_ml_cli(command,argc,argv,cout)) {
        } else if (kline::run_sitewide_finance_cli(command,argc,argv,cout)) {
        } else if (kline::run_massive_stats_cli(command,argc,argv,cout)) {
        } else if (kline::run_massive_finance_cli(command,argc,argv,cout)) {
        } else if (kline::run_empirical_lab_cli(command,argc,argv,cout)) {
        } else if (kline::run_domain_lab_cli(command,argc,argv,cout)) {
        } else if (kline::run_atlas2026_cli(command,argc,argv,cout)) {
        } else if (kline::run_lab24_cli(command,argc,argv,cout)) {
        } else if (kline::run_researchflow_cli(command,argc,argv,cout)) {
        } else if (kline::run_numerical_lab_cli(command,argc,argv,cout)) {
        } else {
            usage();
            return 2;
        }
        return 0;
    } catch (const exception& error) {
        cerr << "Quant research input error: " << error.what() << "\n";
        return 1;
    }
}
