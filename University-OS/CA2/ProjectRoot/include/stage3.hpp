#ifndef STAGE3_HPP
#define STAGE3_HPP

// Stage 3: User rating analysis
// Reads Ratings.csv line by line, computes MeanRating per book_id
// Sends results to Stage 2 via named pipe (FIFO)

void runStage3(int readFd);
// readFd: unnamed pipe fd to receive Ratings.csv filename from Father

#endif
