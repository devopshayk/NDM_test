#include "atserver/rules.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "atserver/text.hpp"

namespace atserver {
namespace {

bool endsWith(const std::string& value, const std::string& suffix) {
    if (value.size() < suffix.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), value.rbegin(),
                      [](char a, char b) {
                          return std::tolower(static_cast<unsigned char>(a)) ==
                                 std::tolower(static_cast<unsigned char>(b));
                      });
}

std::ifstream openOrThrow(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open rule file: " + path);
    return in;
}

void stripCarriageReturn(std::string& line) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
}

bool isSkippable(const std::string& line) {
    return line.empty() || line[0] == '#';
}

}

RuleSet RuleSet::loadFromFile(const std::string& path) {
    return endsWith(path, ".csv") ? parseCsv(path) : parseText(path);
}

RuleSet RuleSet::parseText(const std::string& path) {
    std::ifstream in = openOrThrow(path);

    RuleSet result;
    std::string line;
    std::size_t lineNumber = 0;

    while (std::getline(in, line)) {
        ++lineNumber;
        stripCarriageReturn(line);
        line = text::trim(line);
        if (isSkippable(line)) continue;

        const std::size_t separator = text::findUnescaped(line, '=');
        if (separator == std::string::npos) {
            std::cerr << path << ':' << lineNumber
                      << ": no separator found, line skipped\n";
            continue;
        }

        Rule rule{
            Pattern(text::unescape(text::trim(line.substr(0, separator)))),
            text::unescape(text::trim(line.substr(separator + 1)))};
        result.rules_.push_back(std::move(rule));
    }
    return result;
}

RuleSet RuleSet::parseCsv(const std::string& path) {
    std::ifstream in = openOrThrow(path);

    RuleSet result;
    std::string line;
    std::size_t lineNumber = 0;

    while (std::getline(in, line)) {
        ++lineNumber;
        stripCarriageReturn(line);
        if (isSkippable(text::trim(line))) continue;

        const std::vector<std::string> fields = text::parseCsvLine(line, ',');
        if (fields.size() < 2) {
            std::cerr << path << ':' << lineNumber
                      << ": expected two columns, line skipped\n";
            continue;
        }

        Rule rule{Pattern(text::trim(fields[0])),
                  text::unescape(text::trim(fields[1]))};
        result.rules_.push_back(std::move(rule));
    }
    return result;
}

const std::string* RuleSet::find(const std::string& command) const {
    for (const Rule& rule : rules_) {
        if (rule.pattern.matches(command)) return &rule.answer;
    }
    return nullptr;
}

}