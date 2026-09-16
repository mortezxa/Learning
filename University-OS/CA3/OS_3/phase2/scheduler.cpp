#include "scheduler.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <numeric>

Scheduler::Scheduler() : current_tick(0), total_busy_core_ticks(0) {
    cores[0] = {CoreType::P_CORE, 0, nullptr};
    cores[1] = {CoreType::E_CORE, 1, nullptr};
    cores[2] = {CoreType::E_CORE, 2, nullptr};
}

void Scheduler::add_process(Process process) {
    processes.push_back(std::move(process));
}

void Scheduler::run() {
    while (!all_done()) {
        admit_new_processes();
        apply_aging();
        check_preemption();
        assign_processes();
        record_gantt();
        tick_cores();
        handle_completion_and_demotion();
        current_tick++;
    }
}

void Scheduler::admit_new_processes() {
    for (auto& process : processes) {
        if (process.state == ProcessState::NEW && process.arrival_time == current_tick) {
            process.state = ProcessState::READY;
            process.priority_queue = 1;
            process.wait_ticks = 0;
            process.ticks_in_quantum = 0;
            q1.push_back(&process);
        }
    }
}

void Scheduler::apply_aging() {
    auto age_queue = [this](std::deque<Process*>& queue, int queue_level) {
        std::deque<Process*> remaining;
        while (!queue.empty()) {
            Process* process = queue.front();
            queue.pop_front();
            process->wait_ticks++;

            if (process->wait_ticks >= AGING_THRESHOLD) {
                process->priority_queue = 1;
                process->wait_ticks = 0;
                process->ticks_in_quantum = 0;
                q1.push_back(process);
                std::cout << "Tick " << current_tick << ": Aging promoted "
                          << process->name << " from Q" << queue_level << " to Q1\n";
            } else {
                remaining.push_back(process);
            }
        }
        queue.swap(remaining);
    };

    for (Process* process : q1) {
        process->wait_ticks++;
    }
    age_queue(q2, 2);
    age_queue(q3, 3);
}

void Scheduler::check_preemption() {
    Process* running_on_p = cores[0].running;
    if (!q1.empty() && running_on_p != nullptr && running_on_p->priority_queue == 2) {
        running_on_p->state = ProcessState::READY;
        running_on_p->wait_ticks = 0;
        q2.push_front(running_on_p);
        cores[0].running = nullptr;
        std::cout << "Tick " << current_tick << ": Preempted " << running_on_p->name
                  << " from P-Core and returned it to front of Q2\n";
    }
}

void Scheduler::assign_processes() {
    if (is_idle(0) && !q1.empty()) {
        Process* process = q1.front();
        q1.pop_front();
        start_on_core(0, process);
    }

    for (int core_index = 0; core_index < 3 && !q2.empty(); core_index++) {
        if (is_idle(core_index)) {
            Process* process = q2.front();
            q2.pop_front();
            start_on_core(core_index, process);
        }
    }

    for (int core_index = 1; core_index < 3 && !q3.empty(); core_index++) {
        if (is_idle(core_index)) {
            Process* process = q3.front();
            q3.pop_front();
            start_on_core(core_index, process);
        }
    }
}

void Scheduler::tick_cores() {
    for (auto& core : cores) {
        if (core.running == nullptr) {
            continue;
        }

        Process* process = core.running;
        const bool is_p_core = core.type == CoreType::P_CORE;
        const int speed = is_p_core ? P_CORE_SPEED : E_CORE_SPEED;
        process->remaining_burst = std::max(0, process->remaining_burst - speed);
        process->ticks_in_quantum++;
        if (is_p_core) {
            process->p_core_ticks++;
        } else {
            process->e_core_ticks++;
        }
        total_busy_core_ticks++;
    }
}

void Scheduler::handle_completion_and_demotion() {
    for (auto& core : cores) {
        Process* process = core.running;
        if (process == nullptr) {
            continue;
        }

        if (process->remaining_burst == 0) {
            process->state = ProcessState::DONE;
            process->completion_time = current_tick + 1;
            core.running = nullptr;
            continue;
        }

        if (process->priority_queue == 1 && process->ticks_in_quantum >= Q1_QUANTUM) {
            process->priority_queue = 2;
            process->wait_ticks = 0;
            process->ticks_in_quantum = 0;
            process->state = ProcessState::READY;
            q2.push_back(process);
            core.running = nullptr;
        } else if (process->priority_queue == 2 && process->ticks_in_quantum >= Q2_QUANTUM) {
            process->priority_queue = 3;
            process->wait_ticks = 0;
            process->ticks_in_quantum = 0;
            process->state = ProcessState::READY;
            q3.push_back(process);
            core.running = nullptr;
        }
    }
}

void Scheduler::record_gantt() {
    gantt.push_back({current_tick, describe_core(0), describe_core(1), describe_core(2)});
}

void Scheduler::print_gantt() const {
    std::cout << "\n===== Execution Gantt Chart =====\n";
    std::cout << std::left << std::setw(8) << "Tick"
              << std::setw(16) << "P-Core"
              << std::setw(16) << "E-Core 1"
              << std::setw(16) << "E-Core 2" << '\n';
    std::cout << std::string(56, '-') << '\n';

    for (const auto& entry : gantt) {
        std::cout << std::left << std::setw(8) << entry.tick
                  << std::setw(16) << entry.p_core
                  << std::setw(16) << entry.e_core_1
                  << std::setw(16) << entry.e_core_2 << '\n';
    }
}

void Scheduler::print_stats() const {
    std::cout << "\n===== Process Statistics =====\n";
    std::cout << std::left << std::setw(8) << "PID"
              << std::setw(10) << "Arrival"
              << std::setw(8) << "Burst"
              << std::setw(10) << "P-Ticks"
              << std::setw(10) << "E-Ticks"
              << std::setw(12) << "Finish"
              << std::setw(14) << "Turnaround"
              << std::setw(10) << "Waiting"
              << std::setw(10) << "Response" << '\n';
    std::cout << std::string(92, '-') << '\n';

    double total_turnaround = 0.0;
    double total_waiting = 0.0;
    double total_response = 0.0;

    for (const auto& process : processes) {
        const int turnaround = get_turnaround(process);
        const int waiting = get_waiting(process);
        total_turnaround += turnaround;
        total_waiting += waiting;
        total_response += process.response_time;

        std::cout << std::left << std::setw(8) << process.name
                  << std::setw(10) << process.arrival_time
                  << std::setw(8) << process.burst_time
                  << std::setw(10) << process.p_core_ticks
                  << std::setw(10) << process.e_core_ticks
                  << std::setw(12) << process.completion_time
                  << std::setw(14) << turnaround
                  << std::setw(10) << waiting
                  << std::setw(10) << process.response_time << '\n';
    }

    const double process_count = static_cast<double>(processes.size());
    const double total_core_slots = static_cast<double>(std::max(1, current_tick) * 3);
    const double cpu_utilization = (static_cast<double>(total_busy_core_ticks) * 100.0) / total_core_slots;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\nAverage Turnaround Time: " << total_turnaround / process_count << '\n';
    std::cout << "Average Waiting Time   : " << total_waiting / process_count << '\n';
    std::cout << "Average Response Time  : " << total_response / process_count << '\n';
    std::cout << "CPU Utilization        : " << cpu_utilization << "%\n";
    std::cout << "Total Energy           : " << total_energy() << " watt-ticks\n";
}

void Scheduler::print_energy() const {
    std::cout << "\n===== Energy Details =====\n";
    for (const auto& process : processes) {
        const int energy = process.p_core_ticks * P_CORE_POWER + process.e_core_ticks * E_CORE_POWER;
        std::cout << process.name << ": (" << process.p_core_ticks << " * 5) + ("
                  << process.e_core_ticks << " * 1) = " << energy << " watt-ticks\n";
    }
    std::cout << "Total Energy Consumed: " << total_energy() << " watt-ticks\n";
}

bool Scheduler::all_done() const {
    return std::all_of(processes.begin(), processes.end(), [](const Process& process) {
        return process.state == ProcessState::DONE;
    });
}

bool Scheduler::is_idle(int core_index) const {
    return cores[core_index].running == nullptr;
}

void Scheduler::start_on_core(int core_index, Process* process) {
    process->state = ProcessState::RUNNING;
    process->wait_ticks = 0;
    if (process->first_run) {
        process->response_time = current_tick - process->arrival_time;
        process->first_run = false;
    }
    cores[core_index].running = process;
}

std::string Scheduler::describe_core(int core_index) const {
    const Process* process = cores[core_index].running;
    if (process == nullptr) {
        return "IDLE";
    }
    return process->name + "(Q" + std::to_string(process->priority_queue) + ")";
}

int Scheduler::total_energy() const {
    int energy = 0;
    for (const auto& process : processes) {
        energy += process.p_core_ticks * P_CORE_POWER + process.e_core_ticks * E_CORE_POWER;
    }
    return energy;
}

int Scheduler::get_turnaround(const Process& process) const {
    return process.completion_time - process.arrival_time;
}

int Scheduler::get_waiting(const Process& process) const {
    return get_turnaround(process) - (process.p_core_ticks + process.e_core_ticks);
}
