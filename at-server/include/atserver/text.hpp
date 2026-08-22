#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace atserver {
    namespace text {

    std::string trim(std::string s);
    std::string unescape(const std::string& in);
    std::string toUpper(std::string s);
    std::size_t findUnescaped(const std::string& s, char needle);
    std::vector<std::string> parseCsvLine(const std::string& line, char separator);

    }
}