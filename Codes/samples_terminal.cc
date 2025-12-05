#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <chrono>
#include <cctype>

volatile bool keepRunning = true;
void intHandler(int) { keepRunning = false; }

int main() {
    const char* portname = "/dev/ttyUSB0";
    int baudrate = B115200;

    // Open serial port
    int serial_port = open(portname, O_RDWR | O_NOCTTY);
    if (serial_port < 0) {
        std::cerr << "Error: Unable to open serial port " << portname << std::endl;
        return 1;
    }

    // Configure serial port
    struct termios tty{};
    if (tcgetattr(serial_port, &tty) != 0) {
        std::cerr << "Error: Could not get terminal attributes." << std::endl;
        return 1;
    }

    cfsetospeed(&tty, baudrate);
    cfsetispeed(&tty, baudrate);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag = IGNPAR;
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | CSTOPB | CRTSCTS);
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    tcflush(serial_port, TCIFLUSH);
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        std::cerr << "Error: Could not set terminal attributes." << std::endl;
        return 1;
    }

    std::ofstream outfile("data.csv");
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open data.csv for writing." << std::endl;
        return 1;
    }

    // Write CSV header
    outfile << "Time_us,Sample" << std::endl;

    std::cout << "Reading from " << portname << " at 115200 baud..." << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    signal(SIGINT, intHandler);

    char buf[256];
    std::string line;
    ssize_t n = 0;

    // Record start time
    auto start_time = std::chrono::steady_clock::now();

    while (keepRunning) {
        n = read(serial_port, buf, sizeof(buf));
        if (n > 0) {
            for (ssize_t i = 0; i < n; i++) {
                unsigned char c = buf[i];

                // Skip ANSI escape sequences and non-digit stuff
                if (c == 0x1B || c == '[' || c == 'u')
                    continue;

                if (c == ' ') {
                    if (!line.empty()) {
                        auto now = std::chrono::steady_clock::now();
                        auto elapsed_us =
                            std::chrono::duration_cast<std::chrono::microseconds>(now - start_time).count();
                        outfile << elapsed_us << "," << line << std::endl;
                        outfile.flush();
                        line.clear();
                    }
                    continue;
                }

                if (std::isdigit(c) || c == '-' || c == '+') {
                    line += c;
                }
            }
        } else if (n < 0) {
            perror("read");
            break;
        }
    }

    close(serial_port);
    outfile.close();
    std::cout << "\nData saved to data.csv" << std::endl;
    return 0;
}
