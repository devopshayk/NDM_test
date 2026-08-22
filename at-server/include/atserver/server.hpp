#pragma once

#include <csignal>
#include <string>

#include "atserver/rules.hpp"
#include "atserver/serial.hpp"

namespace atserver {

class Server {
public:
    Server(SerialPort& port, const RuleSet& rules);

    void run(const volatile std::sig_atomic_t& stopFlag);

private:
    void feed(char c);
    void handleCommand(const std::string& command);

    SerialPort& port_;
    const RuleSet& rules_;
    bool echo_ = true;
    std::string line_;
};

}