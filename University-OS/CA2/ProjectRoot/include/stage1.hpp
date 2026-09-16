#ifndef STAGE1_HPP
#define STAGE1_HPP

// Stage 1: Book price analysis
// Reads Books.csv line by line, computes MeanPrice per book_id
// Sends results to Stage 2 via named pipe (FIFO)

void runStage1(int readFd);
// readFd: unnamed pipe fd to receive Books.csv filename from Father

#endif
