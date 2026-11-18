#include <algorithm>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

struct Release {
    int announced;
    double value;
};

struct Event {
    int start;
    int end;
};

double available_value(const std::vector<Release>& releases, int decision, int delay) {
    if (delay < 0) throw std::invalid_argument("delay must be nonnegative");
    bool found = false;
    int last = 0;
    double value = 0.0;
    for (const Release& r : releases) {
        if (!std::isfinite(r.value)) throw std::invalid_argument("nonfinite data");
        if (r.announced + delay <= decision && (!found || r.announced > last)) {
            last = r.announced;
            value = r.value;
            found = true;
        }
    }
    if (!found) throw std::invalid_argument("no information available at decision time");
    return value;
}

bool overlap(const Event& a, const Event& b) {
    if (a.start > a.end || b.start > b.end) {
        throw std::invalid_argument("invalid event interval");
    }
    return a.start <= b.end && b.start <= a.end;
}

std::vector<int> purged_train(const std::vector<Event>& train,
                              const std::vector<Event>& test,
                              int embargo) {
    if (embargo < 0 || test.empty()) throw std::invalid_argument("invalid test or embargo");
    std::vector<int> kept;
    for (int i = 0; i < static_cast<int>(train.size()); ++i) {
        const Event& a = train[i];
        if (a.start > a.end) throw std::invalid_argument("invalid training event");
        bool excluded = false;
        for (const Event& b : test) {
            if (b.start > b.end) throw std::invalid_argument("invalid test event");
            if (overlap(a, b) ||
                (a.start > b.end && static_cast<long long>(a.start) <=
                                         static_cast<long long>(b.end) + embargo)) {
                excluded = true;
                break;
            }
        }
        if (!excluded) kept.push_back(i);
    }
    return kept;
}

std::vector<double> average_ranks(const std::vector<double>& x) {
    std::vector<int> ids(x.size());
    std::iota(ids.begin(), ids.end(), 0);
    for (double v : x) {
        if (!std::isfinite(v)) throw std::invalid_argument("rank needs finite numbers");
    }
    std::sort(ids.begin(), ids.end(), [&](int a, int b) {
        return x[a] < x[b];
    });
    std::vector<double> rank(x.size());
    for (std::size_t i = 0; i < ids.size();) {
        std::size_t j = i + 1;
        while (j < ids.size() && x[ids[j]] == x[ids[i]]) ++j;
        double mid = (static_cast<double>(i) + 1.0 + static_cast<double>(j)) / 2.0;
        for (std::size_t k = i; k < j; ++k) rank[ids[k]] = mid;
        i = j;
    }
    return rank;
}

double rank_ic(const std::vector<double>& forecast, const std::vector<double>& realized) {
    if (forecast.size() != realized.size() || forecast.size() < 2) {
        throw std::invalid_argument("rank IC needs matching arrays");
    }
    std::vector<double> x = average_ranks(forecast);
    std::vector<double> y = average_ranks(realized);
    double xm = std::accumulate(x.begin(), x.end(), 0.0) / x.size();
    double ym = std::accumulate(y.begin(), y.end(), 0.0) / y.size();
    double xx = 0.0, yy = 0.0, xy = 0.0;
    for (std::size_t i = 0; i < x.size(); ++i) {
        double a = x[i] - xm, b = y[i] - ym;
        xx += a * a;
        yy += b * b;
        xy += a * b;
    }
    if (xx == 0.0 || yy == 0.0) throw std::invalid_argument("constant rank");
    return xy / std::sqrt(xx * yy);
}

double brier_score(const std::vector<double>& probabilities,
                   const std::vector<int>& outcomes) {
    if (probabilities.empty() || probabilities.size() != outcomes.size()) {
        throw std::invalid_argument("Brier input lengths do not match");
    }
    double sum = 0.0;
    for (std::size_t i = 0; i < outcomes.size(); ++i) {
        double p = probabilities[i];
        if (!std::isfinite(p) || p < 0.0 || p > 1.0 ||
            (outcomes[i] != 0 && outcomes[i] != 1)) {
            throw std::invalid_argument("invalid probability or outcome");
        }
        double e = p - outcomes[i];
        sum += e * e;
    }
    return sum / outcomes.size();
}

double trade_probability_threshold(double gain, double loss, double cost) {
    if (!std::isfinite(gain) || !std::isfinite(loss) || !std::isfinite(cost) ||
        gain <= 0.0 || loss <= 0.0 || cost < 0.0) {
        throw std::invalid_argument("invalid payoff parameters");
    }
    return (loss + cost) / (gain + loss);
}

double one_period_equity(double equity, double units, double start_price,
                         double end_price, double dividend, double costs) {
    if (!std::isfinite(equity) || !std::isfinite(units) ||
        !std::isfinite(start_price) || !std::isfinite(end_price) ||
        !std::isfinite(dividend) || !std::isfinite(costs) ||
        equity <= 0.0 || start_price <= 0.0 || end_price < 0.0 || costs < 0.0) {
        throw std::invalid_argument("invalid portfolio ledger");
    }
    return equity + units * (end_price - start_price + dividend) - costs;
}

double maximum_drawdown(const std::vector<double>& wealth) {
    if (wealth.empty()) throw std::invalid_argument("wealth series is empty");
    double peak = 0.0, worst = 0.0;
    for (double w : wealth) {
        if (!std::isfinite(w) || w <= 0.0) throw std::invalid_argument("invalid wealth");
        peak = std::max(peak, w);
        worst = std::max(worst, 1.0 - w / peak);
    }
    return worst;
}

double empirical_es(std::vector<double> losses, double confidence) {
    if (losses.empty() || !std::isfinite(confidence) ||
        confidence < 0.0 || confidence >= 1.0) {
        throw std::invalid_argument("invalid ES input");
    }
    for (double x : losses) {
        if (!std::isfinite(x)) throw std::invalid_argument("loss is not finite");
    }
    std::sort(losses.begin(), losses.end(), std::greater<double>());
    double remaining = (1.0 - confidence) * losses.size();
    double tail_mass = remaining;
    double tail_total = 0.0;
    for (double x : losses) {
        if (remaining <= 0.0) break;
        double mass = std::min(1.0, remaining);
        tail_total += mass * x;
        remaining -= mass;
    }
    return tail_total / tail_mass;
}

double hac_variance_of_mean(const std::vector<double>& observations, int lag) {
    int n = static_cast<int>(observations.size());
    if (n < 3 || lag < 0 || lag >= n) throw std::invalid_argument("invalid HAC lag");
    for (double x : observations) {
        if (!std::isfinite(x)) throw std::invalid_argument("nonfinite observation");
    }
    double mean = std::accumulate(observations.begin(), observations.end(), 0.0) / n;
    double variance = 0.0;
    for (int i = 0; i < n; ++i) {
        double d = observations[i] - mean;
        variance += d * d;
    }
    variance /= n;
    for (int k = 1; k <= lag; ++k) {
        double covariance = 0.0;
        for (int i = k; i < n; ++i) {
            covariance += (observations[i] - mean) * (observations[i-k] - mean);
        }
        covariance /= n;
        double weight = 1.0 - static_cast<double>(k) / (lag + 1.0);
        variance += 2.0 * weight * covariance;
    }
    return std::max(0.0, variance / n);
}


double fx_home_return(double foreign_return, double spot_start, double spot_end) {
    if (!std::isfinite(foreign_return) || !std::isfinite(spot_start) ||
        !std::isfinite(spot_end) || foreign_return < -1.0 ||
        spot_start <= 0.0 || spot_end <= 0.0) {
        throw std::invalid_argument("invalid FX or foreign return");
    }
    return (1.0 + foreign_return) * spot_end / spot_start - 1.0;
}

double no_trade_adjustment(double signal, double curvature, double cost) {
    if (!std::isfinite(signal) || !std::isfinite(curvature) ||
        !std::isfinite(cost) || curvature <= 0.0 || cost < 0.0) {
        throw std::invalid_argument("invalid no-trade parameters");
    }
    if (signal > cost) return (signal - cost) / curvature;
    if (signal < -cost) return (signal + cost) / curvature;
    return 0.0;
}

double dv01(double price, double modified_duration) {
    if (!std::isfinite(price) || !std::isfinite(modified_duration) ||
        price <= 0.0 || modified_duration < 0.0) {
        throw std::invalid_argument("invalid bond inputs");
    }
    return price * modified_duration * 0.0001;
}

double delta_gamma_change(double delta, double gamma, double price_change) {
    if (!std::isfinite(delta) || !std::isfinite(gamma) ||
        !std::isfinite(price_change)) {
        throw std::invalid_argument("invalid option sensitivity");
    }
    return delta * price_change + 0.5 * gamma * price_change * price_change;
}

double square_root_impact(double coefficient, double volatility, double participation) {
    if (!std::isfinite(coefficient) || !std::isfinite(volatility) ||
        !std::isfinite(participation) || coefficient < 0.0 ||
        volatility < 0.0 || participation < 0.0 || participation > 1.0) {
        throw std::invalid_argument("invalid impact inputs");
    }
    return coefficient * volatility * std::sqrt(participation);
}

double markout(int side, double execution_price, double later_mid) {
    if ((side != -1 && side != 1) || !std::isfinite(execution_price) ||
        !std::isfinite(later_mid) || execution_price <= 0.0 || later_mid <= 0.0) {
        throw std::invalid_argument("invalid execution markout");
    }
    return side * (later_mid - execution_price);
}

double futures_two_contract_pnl(int contracts, double multiplier,
                                double old_start, double old_close,
                                double new_open, double new_close) {
    if (!std::isfinite(multiplier) || multiplier <= 0.0 ||
        !std::isfinite(old_start) || !std::isfinite(old_close) ||
        !std::isfinite(new_open) || !std::isfinite(new_close) ||
        old_start <= 0.0 || old_close <= 0.0 ||
        new_open <= 0.0 || new_close <= 0.0) {
        throw std::invalid_argument("invalid futures inputs");
    }
    return contracts * multiplier *
           ((old_close - old_start) + (new_close - new_open));
}

void expect_close(const char* label, double actual, double expected) {
    if (!std::isfinite(actual) || std::abs(actual - expected) > 1e-9) {
        throw std::runtime_error(label);
    }
}

void expect_true(const char* label, bool actual) {
    if (!actual) throw std::runtime_error(label);
}

int main() {
    try {
        expect_close("as-of publication time",
            available_value({{9, 100.0}, {12, 120.0}}, 11, 1), 100.0);

        std::vector<Event> train = {{1, 4}, {5, 10}, {12, 13}, {16, 17}};
        std::vector<Event> test = {{8, 11}};
        std::vector<int> kept = purged_train(train, test, 2);
        expect_true("purging and after-test embargo",
            kept.size() == 2 && kept[0] == 0 && kept[1] == 3);

        expect_close("Rank IC", rank_ic({1, 2, 3}, {1, 3, 2}), 0.5);
        expect_close("Brier score", brier_score({0.1, 0.9}, {0, 1}), 0.01);
        expect_close("cost-sensitive threshold",
                     trade_probability_threshold(2.0, 1.0, 0.2), 0.4);
        expect_close("total-return portfolio ledger",
                     one_period_equity(100.0, 1.0, 100.0, 98.0, 2.0, 0.5), 99.5);
        expect_close("maximum drawdown",
                     maximum_drawdown({100.0, 80.0, 96.0}), 0.2);
        expect_close("fractional-tail ES",
                     empirical_es({1.0, 2.0, 3.0, 10.0}, 0.75), 10.0);
        expect_close("HAC nonnegative variance",
                     hac_variance_of_mean({1, 2, 1, 3, 2, 4}, 2) >= 0.0 ? 1.0 : 0.0, 1.0);

        bool rejected = false;
        try {
            available_value({{12, 120.0}}, 11, 1);
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        expect_true("future disclosure rejected", rejected);

        rejected = false;
        try {
            brier_score({1.2}, {1});
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        expect_true("invalid probability rejected", rejected);


        expect_close("FX translated return",
                     fx_home_return(0.05, 7.0, 6.3), -0.055);
        expect_close("sparse no-trade target",
                     no_trade_adjustment(0.8, 2.0, 0.5), 0.15);
        expect_close("no-trade dead zone",
                     no_trade_adjustment(0.3, 2.0, 0.5), 0.0);
        expect_close("DV01",
                     dv01(100.0, 5.0), 0.05);
        expect_close("delta-gamma approximation",
                     delta_gamma_change(0.5, 0.02, 2.0), 1.04);
        expect_close("square-root impact",
                     square_root_impact(0.5, 0.02, 0.01), 0.001);
        expect_close("post-trade markout",
                     markout(1, 100.0, 99.9), -0.1);
        expect_close("futures roll without artificial jump",
                     futures_two_contract_pnl(1, 1.0, 100.0, 101.0, 104.0, 106.0), 3.0);

        std::cout << std::fixed << std::setprecision(4);
        std::cout << "As-of, purging, Rank IC, calibration, net equity, "
                     "drawdown, ES, HAC, FX, costs, Greeks and futures checks passed\n";
    } catch (const std::exception& e) {
        std::cerr << "Validation failed: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
