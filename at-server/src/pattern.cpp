#include "atserver/pattern.hpp"

#include <cctype>
#include <utility>

namespace atserver {
namespace {

char upper(char c) {
    return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
}

}

Pattern::Pattern(std::string text)
    : text_(std::move(text)), tokens_(compile(text_)) {}

std::vector<Pattern::Token> Pattern::compile(const std::string& text) {
    std::vector<Token> tokens;

    for (std::size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];

        if (c == '.') {
            Token t;
            t.kind = Kind::AnyOne;
            tokens.push_back(std::move(t));
        } else if (c == '*') {
            if (!tokens.empty() && tokens.back().kind == Kind::Star) continue;
            Token t;
            t.kind = Kind::Star;
            tokens.push_back(std::move(t));
        } else if (c == '[') {
            Token t;
            t.kind = Kind::Set;

            std::size_t j = i + 1;
            if (j < text.size() && text[j] == '^') {
                t.negated = true;
                ++j;
            }
            while (j < text.size() && text[j] != ']') {
                t.set.push_back(upper(text[j]));
                ++j;
            }
            if (j == text.size()) {
                Token literal;
                literal.kind = Kind::Literal;
                literal.literal = '[';
                tokens.push_back(std::move(literal));
                continue;
            }
            i = j;
            tokens.push_back(std::move(t));
        } else {
            Token t;
            t.kind = Kind::Literal;
            t.literal = upper(c);
            tokens.push_back(std::move(t));
        }
    }
    return tokens;
}

bool Pattern::tokenMatches(const Token& token, char c) {
    const char u = upper(c);

    switch (token.kind) {
        case Kind::Literal:
            return token.literal == u;
        case Kind::AnyOne:
            return true;
        case Kind::Set: {
            const bool inSet = token.set.find(u) != std::string::npos;
            return token.negated ? !inSet : inSet;
        }
        case Kind::Star:
            return false;
    }
    return false;
}

bool Pattern::matches(const std::string& input) const {
    const std::size_t m = input.size();
    const std::size_t n = tokens_.size();

    std::vector<char> previous(n + 1, 0);
    std::vector<char> current(n + 1, 0);

    previous[0] = 1;
    for (std::size_t j = 1; j <= n; ++j)
        previous[j] = previous[j - 1] && tokens_[j - 1].kind == Kind::Star;

    for (std::size_t i = 1; i <= m; ++i) {
        current[0] = 0;
        for (std::size_t j = 1; j <= n; ++j) {
            if (tokens_[j - 1].kind == Kind::Star)
                current[j] = current[j - 1] || previous[j];
            else
                current[j] = previous[j - 1] &&
                             tokenMatches(tokens_[j - 1], input[i - 1]);
        }
        previous.swap(current);
    }
    return previous[n] != 0;
}

}