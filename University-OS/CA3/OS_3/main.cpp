// main.cpp
#include <iostream>
#include "phase1/race_condition.h"
#include "phase1/deadlock.h"
#include "phase2/scheduler.h"
#include "phase2/scenarios.h"

int main() {
    std::cout << "========================================\n";
    std::cout << "        فاز اول: Race Condition         \n";
    std::cout << "========================================\n";
    run_race_condition_demo();

    std::cout << "\n========================================\n";
    std::cout << "           فاز اول: Deadlock             \n";
    std::cout << "========================================\n";
    run_deadlock_demo();

    std::cout << "\n========================================\n";
    std::cout << "       فاز دوم: MLFQ + Heterogeneous    \n";
    std::cout << "========================================\n";
    run_all_scenarios();

    return 0;
}
