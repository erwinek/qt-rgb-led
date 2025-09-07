// SerialReader.cpp
#include "SerialReader.h"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <iostream>

SerialReader::SerialReader(const std::string& device, int baudrate)
    : device_(device), baudrate_(baudrate), fd_(-1), running_(false) {}

SerialReader::~SerialReader() {
    stop();
}

void SerialReader::setCommandHandler(std::function<void(const std::string&)> handler) {
    handler_ = handler;
}

void SerialReader::start() {
    fd_ = open(device_.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd_ < 0) {
        perror("UART ERROR open");
        return;
    }

    termios options{};
    tcgetattr(fd_, &options);
    cfsetispeed(&options, baudrate_);
    cfsetospeed(&options, baudrate_);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    tcsetattr(fd_, TCSANOW, &options);

    running_ = true;
    thread_ = std::thread(&SerialReader::run, this);
}

void SerialReader::stop() {
    running_ = false;
    if (thread_.joinable()) thread_.join();
    if (fd_ >= 0) close(fd_);
}

void SerialReader::run() {
    char buffer[256];
    std::string line;
    while (running_) {
        int n = read(fd_, buffer, sizeof(buffer));
        if (n > 0) {
            line.append(buffer, n);
            size_t pos;
            while ((pos = line.find('\n')) != std::string::npos) {
                std::string cmd = line.substr(0, pos);
                line.erase(0, pos + 1);
                if (handler_) handler_(cmd);
            }
        } else {
            usleep(10000);
        }
    }
}
