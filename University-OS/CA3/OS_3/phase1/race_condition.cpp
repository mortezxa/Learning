#include "race_condition.h"
#include <pthread.h>
#include <stdio.h>
#include <sched.h>

static long long Total_Revenue = 0;
static pthread_mutex_t revenue_lock = PTHREAD_MUTEX_INITIALIZER;
static int use_lock = 0;
static int iterations_per_thread = NUM_ITERATIONS;

static void* thread_add_revenue(void*) {
    for (int i = 0; i < iterations_per_thread; i++) {
        if (use_lock) {
            pthread_mutex_lock(&revenue_lock);
            Total_Revenue += 100;
            pthread_mutex_unlock(&revenue_lock);
        } else {
            long long snapshot = Total_Revenue;
            if ((i % 64) == 0) {
                sched_yield();
            }
            Total_Revenue = snapshot + 100;
        }
    }
    return NULL;
}

static void run_test(int n_threads, int iterations, int locked) {
    pthread_t threads[n_threads];
    Total_Revenue = 0;
    use_lock = locked;
    iterations_per_thread = iterations;

    for (int i = 0; i < n_threads; i++) {
        pthread_create(&threads[i], NULL, thread_add_revenue, NULL);
    }

    for (int i = 0; i < n_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    long long expected = (long long)n_threads * iterations * 100;
    printf("  Threads=%d | K=%d | Expected=%lld | Actual=%lld | %s\n",
           n_threads,
           iterations,
           expected,
           Total_Revenue,
           (Total_Revenue == expected) ? "OK" : "Race condition");
}

void run_race_condition_demo() {
    printf("\n--- Without mutex ---\n");
    run_test(2, 1000, 0);
    run_test(8, NUM_ITERATIONS, 0);
    run_test(16, NUM_ITERATIONS, 0);

    printf("\n--- With revenue_lock mutex ---\n");
    run_test(2, 1000, 1);
    run_test(8, NUM_ITERATIONS, 1);
    run_test(16, NUM_ITERATIONS, 1);

}
