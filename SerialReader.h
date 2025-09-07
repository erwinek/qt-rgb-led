#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <functional>   // <-- dodane!
#include <termios.h>    // <-- dodane dla B115200 itp.

class SerialReader {
public:
    SerialReader(const std::string& device, int baudrate);
    ~SerialReader();

    void setCommandHandler(std::function<void(const std::string&)> handler);
    void start();
    void stop();

private:
    void run();

    std::string device_;
    int baudrate_;
    int fd_;
    std::thread thread_;
    std::atomic<bool> running_;
    std::function<void(const std::string&)> handler_;
};
