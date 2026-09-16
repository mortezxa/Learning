#include "scenarios.h"
#include <iostream>

static void run_scenario(const std::string& title, void (*load)(Scheduler&)) {
    std::cout << "\n========================================\n";
    std::cout << title << '\n';
    std::cout << "========================================\n";

    Scheduler scheduler;
    load(scheduler);
    scheduler.run();
    scheduler.print_gantt();
    scheduler.print_stats();
    scheduler.print_energy();
}

void scenario1(Scheduler& scheduler) {
    scheduler.add_process(Process(1, "P1", 0, 10));
    scheduler.add_process(Process(2, "P2", 0, 12));
    scheduler.add_process(Process(3, "P3", 1, 2));
}

void scenario2(Scheduler& scheduler) {
    scheduler.add_process(Process(1, "P1", 0, 20));
    scheduler.add_process(Process(2, "P2", 2, 25));
    scheduler.add_process(Process(3, "P3", 5, 5));
}

void scenario3(Scheduler& scheduler) {
    scheduler.add_process(Process(1, "P1", 0, 8));
    scheduler.add_process(Process(2, "P2", 3, 15));
    scheduler.add_process(Process(3, "P3", 10, 6));
    scheduler.add_process(Process(4, "P4", 12, 3));
}

void run_all_scenarios() {
    run_scenario("Scenario 1: پایه و تعاملی", scenario1);
    run_scenario("Scenario 2: محاسبات سنگین و Aging", scenario2);
    run_scenario("Scenario 3: ورودهای متناوب و Preemption", scenario3);
}
