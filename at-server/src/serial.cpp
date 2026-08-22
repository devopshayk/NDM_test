#include "atserver/serial.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <utility>

#include <fcntl.h>
#include <unistd.h>

namespace atserver {
namespace {

[[noreturn]] void fail(const std::string& what) {
    throw std::runtime_error(what + ": " + std::strerror(errno));
}

}

SerialPort::SerialPort(const std::string& device, speed_t baud) {
    fd_ = ::open(device.c_str(), O_RDWR | O_NOCTTY);
    if (fd_ < 0) fail("open " + device);

    if (tcgetattr(fd_, &saved_) != 0) {
        ::close(fd_);
        fd_ = -1;
        fail("tcgetattr");
    }
    hasSaved_ = true;

    termios settings = saved_;
    cfmakeraw(&settings);
    cfsetispeed(&settings, baud);
    cfsetospeed(&settings, baud);

    settings.c_cflag |= CLOCAL | CREAD;
    settings.c_cc[VMIN] = 1;
    settings.c_cc[VTIME] = 0;

    if (tcsetattr(fd_, TCSANOW, &settings) != 0) {
        ::close(fd_);
        fd_ = -1;
        hasSaved_ = false;
        fail("tcsetattr");
    }

    tcflush(fd_, TCIOFLUSH);
}

SerialPort::~SerialPort() { reset(); }

SerialPort::SerialPort(SerialPort&& other) noexcept
    : fd_(other.fd_), saved_(other.saved_), hasSaved_(other.hasSaved_) {
    other.fd_ = -1;
    other.hasSaved_ = false;
}

SerialPort& SerialPort::operator=(SerialPort&& other) noexcept {
    if (this != &other) {
        reset();
        fd_ = other.fd_;
        saved_ = other.saved_;
        hasSaved_ = other.hasSaved_;
        other.fd_ = -1;
        other.hasSaved_ = false;
    }
    return *this;
}

void SerialPort::reset() noexcept {
    if (fd_ < 0) return;
    if (hasSaved_) tcsetattr(fd_, TCSANOW, &saved_);
    ::close(fd_);
    fd_ = -1;
    hasSaved_ = false;
}

bool SerialPort::readByte(char& out) const {
    for (;;) {
        const ssize_t n = ::read(fd_, &out, 1);
        if (n == 1) return true;
        if (n == 0) return false;
        if (errno == EINTR) return false;
        fail("read");
    }
}

void SerialPort::write(const std::string& data) const {
    const char* cursor = data.data();
    std::size_t remaining = data.size();

    while (remaining > 0) {
        const ssize_t written = ::write(fd_, cursor, remaining);
        if (written < 0) {
            if (errno == EINTR) continue;
            fail("write");
        }
        cursor += written;
        remaining -= static_cast<std::size_t>(written);
    }
}

speed_t SerialPort::baudFromInt(int value) {
    switch (value) {
        case 9600:   return B9600;
        case 19200:  return B19200;
        case 38400:  return B38400;
        case 57600:  return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
#ifdef B460800
        case 460800: return B460800;
#endif
#ifdef B921600
        case 921600: return B921600;
#endif
        default:
            throw std::runtime_error("unsupported baud rate: " +
                                     std::to_string(value));
    }
}

}