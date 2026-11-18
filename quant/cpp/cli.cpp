#include "data.hpp"
#include "derivatives.hpp"
#include "execution.hpp"
#include "factors.hpp"
#include "io.hpp"
#include "etf.hpp"
#include "macro.hpp"
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
         << "  asof EVENT_CSV DECISION_CSV (event columns Date,Published,Received,Value)\n";
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
