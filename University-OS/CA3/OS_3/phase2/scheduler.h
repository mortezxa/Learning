#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include <deque>
#include <string>
#include <vector>

constexpr int P_CORE_POWER     = 5;
constexpr int E_CORE_POWER     = 1;
constexpr int P_CORE_SPEED     = 2;
constexpr int E_CORE_SPEED     = 1;
constexpr int Q1_QUANTUM       = 2;
constexpr int Q2_QUANTUM       = 4;
constexpr int AGING_THRESHOLD  = 10;

enum class CoreType { P_CORE, E_CORE };

struct Core {
    CoreType type;
    int id;
    Process* running;
};

struct GanttEntry {
    int tick;
    std::string p_core;
    std::string e_core_1;
    std::string e_core_2;
};

class Scheduler {
public:
    Scheduler();

    void add_process(Process process);
    void run();
    void print_gantt() const;
    void print_stats() const;
    void print_energy() const;

private:
    std::vector<Process> processes;
    std::deque<Process*> q1;
    std::deque<Process*> q2;
    std::deque<Process*> q3;
    Core cores[3];
    int current_tick;
    int total_busy_core_ticks;
    std::vector<GanttEntry> gantt;

    void admit_new_processes();
    void apply_aging();
    void check_preemption();
    void assign_processes();
    void tick_cores();
    void handle_completion_and_demotion();
    void record_gantt();

    bool all_done() const;
    bool is_idle(int core_index) const;
    void start_on_core(int core_index, Process* process);
    std::string describe_core(int core_index) const;
    int total_energy() const;
    int get_turnaround(const Process& process) const;
    int get_waiting(const Process& process) const;
};

#endif
