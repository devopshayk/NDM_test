#include "atserver/server.hpp"

#include "atserver/text.hpp"

namespace atserver {
namespace {

constexpr char kCarriageReturn = '\r';
constexpr char kLineFeed = '\n';
constexpr char kBackspace = '\b';
constexpr char kDelete = '\x7f';
constexpr std::size_t kMaxLineLength = 1024;

const std::string kCrLf = "\r\n";
const std::string kError = "ERROR";

}

Server::Server(SerialPort& port, const RuleSet& rules)
    : port_(port), rules_(rules) {}

void Server::run(const volatile std::sig_atomic_t& stopFlag) {
    while (stopFlag == 0) {
        char c = '\0';
        if (!port_.readByte(c)) continue;
        feed(c);
    }
}

void Server::feed(char c) {
    if (echo_) port_.write(std::string(1, c));

    if (c == kCarriageReturn || c == kLineFeed) {
        const std::string command = text::trim(line_);
        line_.clear();
        if (!command.empty()) handleCommand(command);
        return;
    }

    if (c == kBackspace || c == kDelete) {
        if (!line_.empty()) line_.pop_back();
        return;
    }

    line_.push_back(c);
    if (line_.size() > kMaxLineLength) line_.clear();
}

void Server::handleCommand(const std::string& command) {
    const std::string upper = text::toUpper(command);

    if (upper == "ATE0") {
        echo_ = false;
    } else if (upper == "ATE1" || upper == "ATE") {
        echo_ = true;
    }

    const std::string* answer = rules_.find(command);
    port_.write(kCrLf + (answer != nullptr ? *answer : kError) + kCrLf);
}

}