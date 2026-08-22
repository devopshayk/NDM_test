#pragma once

#include <string>

#include <termios.h>

namespace atserver {

class SerialPort {
public:
    SerialPort(const std::string& device, speed_t baud);
    ~SerialPort();

    SerialPort(const SerialPort&) = delete;
    SerialPort& operator=(const SerialPort&) = delete;
    SerialPort(SerialPort&& other) noexcept;
    SerialPort& operator=(SerialPort&& other) noexcept;

    bool readByte(char& out) const;
    void write(const std::string& data) const;

    static speed_t baudFromInt(int value);

private:
    void reset() noexcept;

    int fd_ = -1;
    termios saved_{};
    bool hasSaved_ = false;
};

}