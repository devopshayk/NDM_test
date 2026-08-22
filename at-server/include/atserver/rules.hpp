#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "atserver/pattern.hpp"

namespace atserver {

struct Rule {
    Pattern pattern;
    std::string answer;
};

class RuleSet {
public:
    static RuleSet loadFromFile(const std::string& path);

    const std::string* find(const std::string& command) const;

    std::size_t size() const noexcept { return rules_.size(); }

private:
    static RuleSet parseText(const std::string& path);
    static RuleSet parseCsv(const std::string& path);

    std::vector<Rule> rules_;
};

}