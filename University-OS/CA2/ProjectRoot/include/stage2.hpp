#ifndef STAGE2_HPP
#define STAGE2_HPP

// Stage 2: Score aggregator
// Receives MeanPrice from Stage 1 and MeanRating from Stage 3 via named pipes
// When both values available for a book_id, computes BookScore = MeanPrice / MeanRating
// Sends final result to Father via unnamed pipe

void runStage2(int writeFd);
// writeFd: unnamed pipe fd to send final results to Father

#endif
