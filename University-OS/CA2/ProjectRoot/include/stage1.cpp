#include "stage1.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>

#define FIFO_STAGE1_TO_STAGE2 "/tmp/fifo_s1_s2"

static void logMsg(const std::string& msg) {
    std::cerr << "[Stage1] " << msg << std::endl;
}

void runStage1(int readFd) {
    logMsg("Started.");

    // Receive Books.csv filename from Father via unnamed pipe
    char buf[512] = {};
    ssize_t n = read(readFd, buf, sizeof(buf) - 1);
    if (n <= 0) {
        logMsg("Failed to read filename from Father.");
        close(readFd);
        return;
    }
    close(readFd);

    std::string filename(buf, n);
    // trim newline/whitespace
    while (!filename.empty() &&
           (filename.back() == '\n' || filename.back() == '\r' || filename.back() == ' '))
        filename.pop_back();

    logMsg("Received filename: " + filename);

    // Open FIFO to Stage 2 for writing
    int fifoFd = open(FIFO_STAGE1_TO_STAGE2, O_WRONLY);
    if (fifoFd < 0) {
        logMsg("Failed to open FIFO to Stage 2.");
        return;
    }
    logMsg("Opened FIFO to Stage 2.");

    std::ifstream file(filename);
    if (!file.is_open()) {
        logMsg("Failed to open file: " + filename);
        close(fifoFd);
        return;
    }

    logMsg("Processing started.");

    std::string line;
    std::getline(file, line);

    std::string currentId;
    double priceSum = 0.0;
    std::size_t count = 0;

    auto flushCurrent = [&]() {
        if (currentId.empty() || count == 0) return;
        const double meanPrice = priceSum / static_cast<double>(count);
        std::ostringstream oss;
        oss << currentId << " " << meanPrice << "\n";
        const std::string out = oss.str();
        if (write(fifoFd, out.c_str(), out.size()) < 0) {
            logMsg("Failed to send MeanPrice for book_id=" + currentId);
            return;
        }
        logMsg("Sent to Stage2 -> book_id=" + currentId +
               " MeanPrice=" + std::to_string(meanPrice));
    };

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::istringstream ss(line);
        std::string id, priceStr;
        if (!std::getline(ss, id, ',')) continue;
        if (!std::getline(ss, priceStr, ',')) continue;

        double price = 0.0;
        try { price = std::stod(priceStr); }
        catch (...) { continue; }

        if (!currentId.empty() && id != currentId) {
            flushCurrent();
            currentId = id;
            priceSum = 0.0;
            count = 0;
        } else if (currentId.empty()) {
            currentId = id;
        }

        priceSum += price;
        count += 1;
    }
    flushCurrent();

    file.close();
    close(fifoFd);
    logMsg("Processing finished. FIFO closed.");
    logMsg("Finished.");
}
