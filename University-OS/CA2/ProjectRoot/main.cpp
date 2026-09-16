#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdlib>

#include "include/stage1.hpp"
#include "include/stage2.hpp"
#include "include/stage3.hpp"

#define FIFO_STAGE1_TO_STAGE2 "/tmp/fifo_s1_s2"
#define FIFO_STAGE3_TO_STAGE2 "/tmp/fifo_s3_s2"

static void logMsg(const std::string& msg) {
    std::cerr << "[Father] " << msg << std::endl;
}

static void execStage(const char* selfPath, const char* role, int fd) {
    std::string fdStr = std::to_string(fd);
    execl(selfPath, selfPath, role, fdStr.c_str(), static_cast<char*>(nullptr));
    perror("execl");
    _exit(1);
}

int main(int argc, char* argv[]) {
    if (argc == 3) {
        std::string role = argv[1];
        int fd = std::atoi(argv[2]);
        if (role == "--stage1") {
            runStage1(fd);
            return 0;
        }
        if (role == "--stage2") {
            runStage2(fd);
            return 0;
        }
        if (role == "--stage3") {
            runStage3(fd);
            return 0;
        }
    }

    std::string booksFile = "books.csv";
    std::string ratingsFile = "ratings.csv";

    logMsg("Started.");
    logMsg("Books file: " + booksFile);
    logMsg("Ratings file: " + ratingsFile);

    // Create named pipes (FIFOs)
    unlink(FIFO_STAGE1_TO_STAGE2);
    unlink(FIFO_STAGE3_TO_STAGE2);

    if (mkfifo(FIFO_STAGE1_TO_STAGE2, 0666) < 0) {
        perror("mkfifo s1->s2");
        return 1;
    }
    if (mkfifo(FIFO_STAGE3_TO_STAGE2, 0666) < 0) {
        perror("mkfifo s3->s2");
        return 1;
    }
    logMsg("Named pipes created.");

    // Unnamed pipe: Father -> Stage 1 (send Books.csv filename)
    int pipeToS1[2];
    if (pipe(pipeToS1) < 0) { perror("pipe pipeToS1"); return 1; }

    // Unnamed pipe: Father -> Stage 3 (send Ratings.csv filename)
    int pipeToS3[2];
    if (pipe(pipeToS3) < 0) { perror("pipe pipeToS3"); return 1; }

    // Unnamed pipe: Stage 2 -> Father (send final results)
    int pipeFromS2[2];
    if (pipe(pipeFromS2) < 0) { perror("pipe pipeFromS2"); return 1; }

    logMsg("Unnamed pipes created.");

    // Fork Stage 1
    pid_t pid1 = fork();
    if (pid1 < 0) { perror("fork stage1"); return 1; }
    if (pid1 == 0) {
        // Child: Stage 1
        close(pipeToS1[1]);   // close write end
        close(pipeToS3[0]);
        close(pipeToS3[1]);
        close(pipeFromS2[0]);
        close(pipeFromS2[1]);
        execStage(argv[0], "--stage1", pipeToS1[0]);
    }
    logMsg("Forked Stage 1 (pid=" + std::to_string(pid1) + ").");

    // Fork Stage 3
    pid_t pid3 = fork();
    if (pid3 < 0) { perror("fork stage3"); return 1; }
    if (pid3 == 0) {
        // Child: Stage 3
        close(pipeToS3[1]);   // close write end
        close(pipeToS1[0]);
        close(pipeToS1[1]);
        close(pipeFromS2[0]);
        close(pipeFromS2[1]);
        execStage(argv[0], "--stage3", pipeToS3[0]);
    }
    logMsg("Forked Stage 3 (pid=" + std::to_string(pid3) + ").");

    // Fork Stage 2
    pid_t pid2 = fork();
    if (pid2 < 0) { perror("fork stage2"); return 1; }
    if (pid2 == 0) {
        // Child: Stage 2
        close(pipeFromS2[0]); // close read end
        close(pipeToS1[0]);
        close(pipeToS1[1]);
        close(pipeToS3[0]);
        close(pipeToS3[1]);
        execStage(argv[0], "--stage2", pipeFromS2[1]);
    }
    logMsg("Forked Stage 2 (pid=" + std::to_string(pid2) + ").");

    // Father: close unused pipe ends
    close(pipeToS1[0]);
    close(pipeToS3[0]);
    close(pipeFromS2[1]);

    // Send filenames to Stage 1 and Stage 3
    write(pipeToS1[1], booksFile.c_str(), booksFile.size());
    close(pipeToS1[1]);
    logMsg("Sent Books.csv path to Stage 1.");

    write(pipeToS3[1], ratingsFile.c_str(), ratingsFile.size());
    close(pipeToS3[1]);
    logMsg("Sent Ratings.csv path to Stage 3.");

    // Read final results from Stage 2
    logMsg("Waiting for results from Stage 2...");
    {
        char buf[4096] = {};
        std::string leftover;
        ssize_t n;
        while ((n = read(pipeFromS2[0], buf, sizeof(buf) - 1)) > 0) {
            buf[n] = '\0';
            leftover += std::string(buf, n);
            size_t pos;
            while ((pos = leftover.find('\n')) != std::string::npos) {
                std::string entry = leftover.substr(0, pos);
                leftover = leftover.substr(pos + 1);
                if (entry.empty()) continue;
                std::istringstream ss(entry);
                std::string id;
                double score;
                if (ss >> id >> score) {
                    logMsg("Received result -> book_id=" + id +
                           " BookScore=" + std::to_string(score));
                    std::cout << "book_id=" << id
                              << "  BookScore=" << score << std::endl;
                }
            }
        }
        close(pipeFromS2[0]);
    }

    // Wait for all children
    waitpid(pid1, nullptr, 0);
    logMsg("Stage 1 finished.");
    waitpid(pid2, nullptr, 0);
    logMsg("Stage 2 finished.");
    waitpid(pid3, nullptr, 0);
    logMsg("Stage 3 finished.");

    // Cleanup FIFOs
    unlink(FIFO_STAGE1_TO_STAGE2);
    unlink(FIFO_STAGE3_TO_STAGE2);

    logMsg("All processes done. Exiting.");
    return 0;
}
