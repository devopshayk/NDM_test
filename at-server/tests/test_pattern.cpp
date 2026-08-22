#include <cstddef>
#include <iostream>
#include <string>

#include "atserver/pattern.hpp"
#include "atserver/text.hpp"

namespace {

int g_failures = 0;
int g_checks = 0;

void expectMatch(const std::string& pattern, const std::string& input,
                 bool expected) {
    ++g_checks;
    const bool actual = atserver::Pattern(pattern).matches(input);
    if (actual != expected) {
        ++g_failures;
        std::cerr << "FAIL  pattern=\"" << pattern << "\" input=\"" << input
                  << "\" expected=" << std::boolalpha << expected
                  << " actual=" << actual << "\n";
    }
}

void expectEqual(const std::string& actual, const std::string& expected,
                 const std::string& label) {
    ++g_checks;
    if (actual != expected) {
        ++g_failures;
        std::cerr << "FAIL  " << label << " expected=\"" << expected
                  << "\" actual=\"" << actual << "\"\n";
    }
}

void literals() {
    expectMatch("AT", "AT", true);
    expectMatch("AT", "at", true);
    expectMatch("AT", "At", true);
    expectMatch("AT", "ATI", false);
    expectMatch("AT", "", false);
    expectMatch("", "", true);
    expectMatch("", "AT", false);
}

void anyOne() {
    expectMatch("AT.", "ATI", true);
    expectMatch("AT.", "AT", false);
    expectMatch("AT.", "ATIX", false);
    expectMatch("...", "ATI", true);
    expectMatch("A.I", "ATI", true);
}

void star() {
    expectMatch("AT*", "AT", true);
    expectMatch("AT*", "ATI", true);
    expectMatch("AT*", "AT+CPIN=1234", true);
    expectMatch("*", "", true);
    expectMatch("*", "anything", true);
    expectMatch("**", "AT", true);
    expectMatch("*AT*", "xxATyy", true);
    expectMatch("A*E", "AE", true);
    expectMatch("A*E", "AbE", true);
    expectMatch("A*E", "AbcdE", true);
    expectMatch("A*E", "AbcdxyzE", true);
    expectMatch("A*E", "AbcdxyzF", false);
    expectMatch("A*B*C", "AxxByyC", true);
    expectMatch("A*B*C", "AxxCyyB", false);
    expectMatch("*A*A*A*A*B", "AAAAAAAAAAAAAAAAAAAA", false);
}

void sets() {
    expectMatch("ATE[01]", "ATE0", true);
    expectMatch("ATE[01]", "ATE1", true);
    expectMatch("ATE[01]", "ATE2", false);
    expectMatch("ATE[1,0]", "ATE0", true);
    expectMatch("ATE[1,0]", "ATE1", true);
    expectMatch("ATE[1,0]", "ATE,", true);
    expectMatch("ATE[1,0]", "ATE2", false);
    expectMatch("ATE[^01]", "ATE2", true);
    expectMatch("ATE[^01]", "ATE0", false);
    expectMatch("AT[", "AT[", true);
}

void baseCommandSet() {
    expectMatch("AT", "AT", true);
    expectMatch("ATI", "ATI", true);
    expectMatch("AT+COPS?", "AT+COPS?", true);
    expectMatch("AT+COPS=?", "AT+COPS=?", true);
    expectMatch("AT+COPS=*", "AT+COPS=0,0", true);
    expectMatch("AT+CPIN?", "AT+CPIN?", true);
    expectMatch("AT+CPIN=*", "AT+CPIN=1234", true);
}

void textHelpers() {
    expectEqual(atserver::text::trim("  AT  "), "AT", "trim");
    expectEqual(atserver::text::unescape("a\\r\\nb"), "a\r\nb", "unescape crlf");
    expectEqual(atserver::text::unescape("AT+CPIN\\=1"), "AT+CPIN=1",
                "unescape equals");
    expectEqual(atserver::text::toUpper("at+cpin"), "AT+CPIN", "toUpper");

    ++g_checks;
    if (atserver::text::findUnescaped("AT+COPS\\=?=OK", '=') != 10) {
        ++g_failures;
        std::cerr << "FAIL  findUnescaped returned wrong index\n";
    }

    const auto fields = atserver::text::parseCsvLine("\"AT+CPIN=?\",\"OK\"", ',');
    ++g_checks;
    if (fields.size() != 2 || fields[0] != "AT+CPIN=?" || fields[1] != "OK") {
        ++g_failures;
        std::cerr << "FAIL  parseCsvLine\n";
    }
}

}

int main() {
    literals();
    anyOne();
    star();
    sets();
    baseCommandSet();
    textHelpers();

    if (g_failures == 0) {
        std::cout << "all " << g_checks << " checks passed\n";
        return 0;
    }
    std::cout << g_failures << " of " << g_checks << " checks FAILED\n";
    return 1;
}