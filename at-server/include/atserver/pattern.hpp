#pragma once

#include <string>
#include <vector>

namespace atserver {

class Pattern {
public:
    explicit Pattern(std::string text);

    bool matches(const std::string& input) const;

    const std::string& text() const noexcept { return text_; }

private:
    enum class Kind { Literal, AnyOne, Star, Set };

    struct Token {
        Kind kind = Kind::Literal;
        char literal = '\0';
        std::string set;
        bool negated = false;
    };

    static std::vector<Token> compile(const std::string& text);
    static bool tokenMatches(const Token& token, char c);

    std::string text_;
    std::vector<Token> tokens_;
};

}