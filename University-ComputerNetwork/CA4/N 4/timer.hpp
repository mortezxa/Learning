#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>

using namespace std;

class Timer {
private:
    chrono::time_point<chrono::high_resolution_clock> start_time;
public:
    void start() {
        start_time = chrono::high_resolution_clock::now();
    }
    double elapsed_ms() {
        auto now = chrono::high_resolution_clock::now();
        return chrono::duration<double, milli>(now - start_time).count();
    }
};

#endif
