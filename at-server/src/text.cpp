#include "atserver/text.hpp"

#include <algorithm>
#include <cctype>

namespace atserver {
namespace text {

std::string trim(std::string s) {
    const auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

std::string unescape(const std::string& in) {
    std::string out;
    out.reserve(in.size());

    for (std::size_t i = 0; i < in.size(); ++i) {
        if (in[i] != '\\' || i + 1 >= in.size()) {
            out.push_back(in[i]);
            continue;
        }
        switch (in[i + 1]) {
            case 'r':  out.push_back('\r'); ++i; break;
            case 'n':  out.push_back('\n'); ++i; break;
            case 't':  out.push_back('\t'); ++i; break;
            case '=':  out.push_back('=');  ++i; break;
            case '\\': out.push_back('\\'); ++i; break;
            default:   out.push_back(in[i]);     break;
        }
    }
    return out;
}

std::string toUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return s;
}

std::size_t findUnescaped(const std::string& s, char needle) {
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\') { ++i; continue; }
        if (s[i] == needle) return i;
    }
    return std::string::npos;
}

std::vector<std::string> parseCsvLine(const std::string& line, char separator) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;

    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];

        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    field.push_back('"');
                    ++i;
                } else {
                    inQuotes = false;
                }
            } else {
                field.push_back(c);
            }
        } else if (c == '"') {
            inQuotes = true;
        } else if (c == separator) {
            fields.push_back(field);
            field.clear();
        } else {
            field.push_back(c);
        }
    }
    fields.push_back(field);
    return fields;
}

}
}