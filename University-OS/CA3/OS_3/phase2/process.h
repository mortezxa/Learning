#ifndef PROCESS_H
#define PROCESS_H

#include <string>

enum class ProcessState {
    NEW,
    READY,
    RUNNING,
    DONE
};

struct Process {
    int pid;
    int id;
    std::string name;
    int arrival_time;
    int burst_time;
    int remaining_burst;

    int p_core_ticks;
    int e_core_ticks;

    int priority_queue;
    int wait_ticks;
    int ticks_in_quantum;

    int response_time;
    int completion_time;
    bool first_run;

    ProcessState state;

    Process(int pid_, std::string name_, int arrival, int burst)
        : pid(pid_), id(pid_), name(std::move(name_)), arrival_time(arrival),
          burst_time(burst), remaining_burst(burst), p_core_ticks(0),
          e_core_ticks(0), priority_queue(1), wait_ticks(0),
          ticks_in_quantum(0), response_time(-1), completion_time(-1),
          first_run(true), state(ProcessState::NEW) {}
};

#endif
