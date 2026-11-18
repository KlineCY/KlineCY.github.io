#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace kline {

inline std::vector<std::string> csv_fields(const std::string& line) {
    std::vector<std::string> fields;
    std::string value;
    bool quoted = false;
    for (int i = 0; i < int(line.size()); i++) {
        char c = line[i];
        if (c == '"') {
            if (quoted && i + 1 < int(line.size()) && line[i + 1] == '"') {
                value += '"';
                i++;
            } else {
                quoted = !quoted;
            }
        } else if (c == ',' && !quoted) {
            fields.push_back(value);
            value.clear();
        } else {
            value += c;
        }
    }
    require(!quoted, "CSV row contains an unclosed quote");
    fields.push_back(value);
    return fields;
}

inline double parse_number(const std::string& text) {
    require(!text.empty(), "Missing numeric CSV field");
    std::size_t count = 0;
    double value = std::stod(text, &count);
    require(count == text.size() && std::isfinite(value),
            "CSV numeric field must be finite and unambiguous");
    return value;
}

struct CsvTable {
    std::vector<std::string> header;
    std::vector<std::vector<std::string>> rows;

    int column(const std::string& name) const {
        auto found = std::find(header.begin(), header.end(), name);
        require(found != header.end(), "Required CSV column not found");
        return int(found - header.begin());
    }

    std::vector<double> numbers(const std::string& name) const {
        int index = column(name);
        std::vector<double> values;
        for (const auto& row : rows) values.push_back(parse_number(row[index]));
        return values;
    }
};

inline CsvTable read_csv(const std::string& path) {
    std::ifstream input(path);
    require(bool(input), "Cannot open CSV");
    std::string line;
    require(bool(std::getline(input, line)), "CSV is empty");
    CsvTable table;
    table.header = csv_fields(line);
    require(!table.header.empty(), "CSV has no columns");
    for (int i = 0; i < int(table.header.size()); i++) {
        require(!table.header[i].empty(), "CSV header has empty field");
        for (int j = 0; j < i; j++)
            require(table.header[i] != table.header[j], "Duplicate CSV column");
    }
    while (std::getline(input, line)) {
        if (line.empty()) continue;
        auto row = csv_fields(line);
        require(row.size() == table.header.size(), "CSV row does not match header width");
        table.rows.push_back(row);
        require(table.rows.size() <= 250000, "CSV exceeds row limit");
    }
    require(!table.rows.empty(), "CSV contains no data rows");
    return table;
}

inline std::vector<std::vector<double>> asset_returns_from_columns(
    const CsvTable& table, const std::vector<std::string>& names) {
    require(!names.empty(), "At least one asset column required");
    std::vector<std::vector<double>> assets;
    for (const auto& name : names) assets.push_back(table.numbers(name));
    return assets;
}

}  // namespace kline
