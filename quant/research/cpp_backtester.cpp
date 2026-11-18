#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

struct Bar {
    double open;
    double close;
};

struct Market {
    vector<string> dates;
    vector<string> tickers;
    map<string, vector<Bar>> prices;
};

vector<string> parseCsvRow(const string& line) {
    vector<string> result;
    string field;
    bool quoted = false;
    for (int i = 0; i < int(line.size()); i++) {
        char c = line[i];
        if (c == '"') {
            if (quoted && i + 1 < int(line.size()) && line[i + 1] == '"') {
                field += '"';
                i++;
            } else {
                quoted = !quoted;
            }
        } else if (c == ',' && !quoted) {
            result.push_back(field);
            field.clear();
        } else {
            field += c;
        }
    }
    if (quoted) throw runtime_error("Unclosed CSV quotation");
    result.push_back(field);
    return result;
}

string trim(const string& text) {
    size_t first = text.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

string normalize(const string& text) {
    string result;
    for (char c : text) {
        if (c >= 'A' && c <= 'Z') result += char(c - 'A' + 'a');
        else if (c != ' ' && c != '\r') result += c;
    }
    return result;
}

string tickerName(const string& text) {
    string result;
    for (char c : text) {
        if (c >= 'a' && c <= 'z') result += char(c - 'a' + 'A');
        else result += c;
    }
    return result.empty() ? "ASSET" : result;
}

double finiteDouble(const string& text) {
    size_t consumed = 0;
    double value = stod(text, &consumed);
    if (consumed != text.size() || !isfinite(value))
        throw runtime_error("Invalid numeric CSV field");
    return value;
}

Market readMarket(const string& path) {
    ifstream input(path);
    if (!input) throw runtime_error("Cannot open CSV input file");
    string line;
    if (!getline(input, line)) throw runtime_error("CSV has no header");
    vector<string> header = parseCsvRow(line);
    map<string, int> columns;
    for (int i = 0; i < int(header.size()); i++) {
        string column = normalize(header[i]);
        if (!column.empty()) columns[column] = i;
    }
    if (!columns.count("open") || !columns.count("close"))
        throw runtime_error("CSV must contain Open and Close");
    string dateColumn;
    for (string key : {"date", "datetime", "timestamp", "time"}) {
        if (columns.count(key)) { dateColumn = key; break; }
    }
    if (dateColumn.empty()) throw runtime_error("CSV needs Date");
    map<string, map<string, Bar>> raw;
    int rows = 0;
    while (getline(input, line)) {
        if (line.empty()) continue;
        vector<string> item = parseCsvRow(line);
        auto field = [&](const string& name) -> string {
            int column = columns.at(name);
            if (column >= int(item.size())) throw runtime_error("Incomplete CSV row");
            return trim(item[column]);
        };
        string date = field(dateColumn);
        if (date.size() < 10) throw runtime_error("Malformed date");
        date = date.substr(0, 10);
        replace(date.begin(), date.end(), '/', '-');
        if (date[4] != '-' || date[7] != '-')
            throw runtime_error("Dates must be ISO YYYY-MM-DD");
        string ticker = columns.count("ticker") ? tickerName(field("ticker")) : "ASSET";
        if (ticker == "__ALL__" || ticker == "__BENCHMARK__" || ticker.size() > 45)
            throw runtime_error("Invalid ticker");
        double open = finiteDouble(field("open"));
        double close = finiteDouble(field("close"));
        if (open <= 0 || close <= 0) throw runtime_error("Open and Close must be positive");
        if (raw[ticker].count(date)) throw runtime_error("Duplicate ticker and date");
        raw[ticker][date] = {open, close};
        rows++;
        if (rows > 120000) throw runtime_error("Too many CSV rows");
    }
    if (raw.empty() || raw.size() > 40)
        throw runtime_error("Asset count must be between 1 and 40");
    Market result;
    for (const auto& item : raw) result.tickers.push_back(item.first);
    for (const auto& item : raw.begin()->second) {
        const string& date = item.first;
        bool common = true;
        for (const auto& asset : raw) {
            if (!asset.second.count(date)) common = false;
        }
        if (common) result.dates.push_back(date);
    }
    if (result.dates.size() < 35 || result.dates.size() > 5000)
        throw runtime_error("Common trading dates must be between 35 and 5000");
    for (const auto& asset : raw) {
        vector<Bar> aligned;
        for (const string& date : result.dates)
            aligned.push_back(asset.second.at(date));
        result.prices[asset.first] = aligned;
    }
    return result;
}

vector<double> chooseWeights(const Market& market, int end,
                             const string& strategy, int lookback) {
    int n = int(market.tickers.size());
    vector<double> weight(n, 0);
    if (strategy == "equal") {
        for (int i = 0; i < n; i++) weight[i] = 1.0 / n;
    } else if (strategy == "trend") {
        vector<int> selected;
        for (int i = 0; i < n; i++) {
            const auto& bars = market.prices.at(market.tickers[i]);
            if (end < lookback) continue;
            double average = 0;
            for (int t = end - lookback; t < end; t++)
                average += bars[t].close / lookback;
            if (bars[end - 1].close > average) selected.push_back(i);
        }
        for (int index : selected) weight[index] = 1.0 / selected.size();
    } else if (strategy == "momentum") {
        if (n < 2 || end <= lookback) return weight;
        vector<pair<double, int>> scores;
        for (int i = 0; i < n; i++) {
            const auto& bars = market.prices.at(market.tickers[i]);
            double performance = bars[end - 1].close / bars[end - lookback - 1].close - 1;
            scores.push_back({performance, i});
        }
        sort(scores.begin(), scores.end());
        weight[scores.back().second] = 0.5;
        weight[scores.front().second] = -0.5;
    } else {
        throw runtime_error("Strategy must be equal, trend, or momentum");
    }
    return weight;
}

struct Report {
    double net = 1;
    double benchmark = 1;
    double peak = 1;
    double maxDrawdown = 0;
    double turnover = 0;
    vector<double> returns;
};

Report run(const Market& market, const string& strategy, int lookback,
           double commissionBps, double slippageBps, double borrowBps) {
    if (lookback < 1 || commissionBps < 0 || slippageBps < 0 || borrowBps < 0
        || commissionBps > 500 || slippageBps > 500 || borrowBps > 5000)
        throw runtime_error("Invalid strategy period or costs");
    int n = int(market.tickers.size());
    vector<double> weight(n, 0), previousReturn(n, 0);
    double previousGrowth = 1.0;
    bool hasPrevious = false;
    Report report;
    cout << "Date,Strategy,Benchmark,Return,Turnover\n";
    cout << market.dates[1] << ",1,1,0,0\n";
    double costRate = (commissionBps + slippageBps) / 10000.0;
    for (int t = 1; t < int(market.dates.size()) - 1; t++) {
        vector<double> requested = chooseWeights(market, t, strategy, lookback);
        vector<double> old(n, 0);
        if (hasPrevious) {
            for (int i = 0; i < n; i++)
                old[i] = weight[i] * (1 + previousReturn[i]) / previousGrowth;
        }
        double gross = 0;
        for (double w : requested) gross += fabs(w);
        if (gross > 1.000000001) throw runtime_error("Gross exposure exceeds one");
        double executionCost = 0;
        for (int k = 0; k < 20; k++) {
            double nextCost = 0;
            for (int i = 0; i < n; i++)
                nextCost += fabs(requested[i] * (1 - executionCost) - old[i]);
            nextCost *= costRate;
            if (fabs(nextCost - executionCost) < 1e-13) {
                executionCost = nextCost;
                break;
            }
            executionCost = nextCost;
        }
        double traded = 0;
        for (int i = 0; i < n; i++)
            traded += fabs(requested[i] * (1 - executionCost) - old[i]);
        double borrowCost = 0;
        for (double w : requested)
            if (w < 0) borrowCost += -w * borrowBps / 10000.0 / 252;
        vector<double> marketReturn(n, 0);
        double portfolioReturn = 0, benchmarkReturn = 0;
        for (int i = 0; i < n; i++) {
            const auto& bars = market.prices.at(market.tickers[i]);
            marketReturn[i] = bars[t + 1].open / bars[t].open - 1;
            portfolioReturn += requested[i] * marketReturn[i];
            benchmarkReturn += marketReturn[i] / n;
        }
        double growth = 1 + portfolioReturn - borrowCost;
        double netReturn = (1 - executionCost) * growth - 1;
        if (netReturn <= -1) throw runtime_error("Portfolio lost all capital");
        report.net *= 1 + netReturn;
        report.benchmark *= 1 + benchmarkReturn;
        report.peak = max(report.peak, report.net);
        report.maxDrawdown = min(report.maxDrawdown, report.net / report.peak - 1);
        report.turnover += traded;
        report.returns.push_back(netReturn);
        cout << market.dates[t + 1] << ","
             << setprecision(15) << report.net << ","
             << report.benchmark << "," << netReturn << "," << traded << "\n";
        weight = requested;
        previousReturn = marketReturn;
        previousGrowth = growth;
        hasPrevious = true;
    }
    return report;
}

void printMetrics(const Report& report) {
    int n = int(report.returns.size());
    double average = 0, variance = 0;
    for (double value : report.returns) average += value / n;
    for (double value : report.returns)
        variance += (value - average) * (value - average) / (n - 1);
    double annualReturn = pow(report.net, 252.0 / n) - 1;
    double volatility = sqrt(variance) * sqrt(252.0);
    cerr << setprecision(15)
         << "periods=" << n
         << " annualized=" << annualReturn
         << " volatility=" << volatility
         << " sharpe=";
    if (variance > 1e-24) cerr << average * sqrt(252.0) / sqrt(variance);
    else cerr << "n/a";
    cerr << " max_drawdown=" << report.maxDrawdown
         << " turnover_oneway=" << report.turnover / 2 << "\n";
}

int main(int argc, char** argv) {
    try {
        if (argc < 3 || argc > 7) {
            cerr << "Usage: cpp_backtester DATA.csv equal|trend|momentum "
                 << "[lookback=20] [commission_bps=3] [slippage_bps=2] [borrow_bps_annual=0]\n";
            return 2;
        }
        int lookback = argc > 3 ? stoi(argv[3]) : 20;
        double fee = argc > 4 ? finiteDouble(argv[4]) : 3;
        double slip = argc > 5 ? finiteDouble(argv[5]) : 2;
        double borrow = argc > 6 ? finiteDouble(argv[6]) : 0;
        Market market = readMarket(argv[1]);
        Report report = run(market, argv[2], lookback, fee, slip, borrow);
        printMetrics(report);
        return 0;
    } catch (const exception& error) {
        cerr << "Backtest error: " << error.what() << "\n";
        return 1;
    }
}
