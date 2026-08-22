#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>

#include "atserver/rules.hpp"
#include "atserver/serial.hpp"
#include "atserver/server.hpp"

namespace {

volatile std::sig_atomic_t g_stop = 0;

extern "C" void onSignal(int) { g_stop = 1; }

void installSignalHandlers() {
    struct sigaction action {};
    action.sa_handler = onSignal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(SIGINT, &action, nullptr);
    sigaction(SIGTERM, &action, nullptr);
    signal(SIGPIPE, SIG_IGN);
}

void printUsage(const char* program) {
    std::cerr << "usage: " << program << " <tty-device> <rule-file> [baud]\n"
              << "\n"
              << "  tty-device   serial device to listen on, e.g. /dev/ttyUSB0\n"
              << "  rule-file    expect=answer text file, or .csv\n"
              << "  baud         optional, default 115200\n";
}

}

int main(int argc, char** argv) {
    if (argc < 3 || argc > 4) {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    try {
        const std::string device = argv[1];
        const std::string ruleFile = argv[2];
        const int baudValue = (argc == 4) ? std::stoi(argv[3]) : 115200;

        const atserver::RuleSet rules =
            atserver::RuleSet::loadFromFile(ruleFile);
        if (rules.size() == 0) {
            std::cerr << "error: no usable rules in " << ruleFile << "\n";
            return EXIT_FAILURE;
        }
        std::cerr << "loaded " << rules.size() << " rules from " << ruleFile
                  << "\n";

        installSignalHandlers();

        atserver::SerialPort port(device,
                                  atserver::SerialPort::baudFromInt(baudValue));
        std::cerr << "listening on " << device << " at " << baudValue
                  << " baud, Ctrl-C to stop\n";

        atserver::Server server(port, rules);
        server.run(g_stop);

        std::cerr << "shutting down\n";
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}