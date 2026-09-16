#include "stage2.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <set>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

#define FIFO_STAGE1_TO_STAGE2 "/tmp/fifo_s1_s2"
#define FIFO_STAGE3_TO_STAGE2 "/tmp/fifo_s3_s2"

static void logMsg(const std::string& msg) {
    std::cerr << "[Stage2] " << msg << std::endl;
}

void runStage2(int writeFd) {
    logMsg("Started.");

    int fd1 = open(FIFO_STAGE1_TO_STAGE2, O_RDONLY);
    if (fd1 < 0) {
        logMsg("Failed to open FIFO from Stage 1.");
        close(writeFd);
        return;
    }
    logMsg("Opened FIFO from Stage 1.");

    int fd3 = open(FIFO_STAGE3_TO_STAGE2, O_RDONLY);
    if (fd3 < 0) {
        logMsg("Failed to open FIFO from Stage 3.");
        close(fd1);
        close(writeFd);
        return;
    }
    logMsg("Opened FIFO from Stage 3.");

    std::map<std::string, double> meanPriceMap;
    std::map<std::string, double> meanRatingMap;
    std::set<std::string> emittedIds;
    std::string buf1;
    std::string buf3;
    bool eof1 = false;
    bool eof3 = false;

    auto tryEmit = [&](const std::string& id) {
        if (emittedIds.count(id) != 0) return;
        auto priceIt = meanPriceMap.find(id);
        auto ratingIt = meanRatingMap.find(id);
        if (priceIt == meanPriceMap.end() || ratingIt == meanRatingMap.end()) return;
        if (ratingIt->second == 0.0) return;

        const double bookScore = priceIt->second / ratingIt->second;
        std::ostringstream oss;
        oss << id << " " << bookScore << "\n";
        const std::string out = oss.str();
        if (write(writeFd, out.c_str(), out.size()) < 0) {
            logMsg("Failed to send BookScore for book_id=" + id);
            return;
        }
        emittedIds.insert(id);
        logMsg("Sent to Father -> book_id=" + id +
               " BookScore=" + std::to_string(bookScore));
        meanPriceMap.erase(priceIt);
        meanRatingMap.erase(ratingIt);
    };

    auto consumeBuffer = [&](std::string& buffer, bool isPriceStream) {
        size_t pos;
        while ((pos = buffer.find('\n')) != std::string::npos) {
            std::string entry = buffer.substr(0, pos);
            buffer.erase(0, pos + 1);
            if (entry.empty()) continue;
            std::istringstream ss(entry);
            std::string id;
            double val;
            if (!(ss >> id >> val)) continue;
            if (isPriceStream) {
                meanPriceMap[id] = val;
                logMsg("Received MeanPrice -> book_id=" + id +
                       " val=" + std::to_string(val));
            } else {
                meanRatingMap[id] = val;
                logMsg("Received MeanRating -> book_id=" + id +
                       " val=" + std::to_string(val));
            }
            tryEmit(id);
        }
    };

    logMsg("Processing started.");

    while (!eof1 || !eof3) {
        fd_set readSet;
        FD_ZERO(&readSet);
        int maxFd = -1;
        if (!eof1) {
            FD_SET(fd1, &readSet);
            if (fd1 > maxFd) maxFd = fd1;
        }
        if (!eof3) {
            FD_SET(fd3, &readSet);
            if (fd3 > maxFd) maxFd = fd3;
        }

        int ready = select(maxFd + 1, &readSet, nullptr, nullptr, nullptr);
        if (ready < 0) {
            if (errno == EINTR) continue;
            logMsg("select() failed.");
            break;
        }

        if (!eof1 && FD_ISSET(fd1, &readSet)) {
            char chunk[4096];
            ssize_t n = read(fd1, chunk, sizeof(chunk));
            if (n > 0) {
                buf1.append(chunk, n);
                consumeBuffer(buf1, true);
            } else if (n == 0) {
                eof1 = true;
                consumeBuffer(buf1, true);
                logMsg("Stage 1 stream closed.");
            }
        }

        if (!eof3 && FD_ISSET(fd3, &readSet)) {
            char chunk[4096];
            ssize_t n = read(fd3, chunk, sizeof(chunk));
            if (n > 0) {
                buf3.append(chunk, n);
                consumeBuffer(buf3, false);
            } else if (n == 0) {
                eof3 = true;
                consumeBuffer(buf3, false);
                logMsg("Stage 3 stream closed.");
            }
        }
    }

    close(writeFd);
    close(fd1);
    close(fd3);
    logMsg("Finished.");
    logMsg("All results sent. Exiting.");
}
