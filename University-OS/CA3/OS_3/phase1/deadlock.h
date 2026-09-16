#ifndef DEADLOCK_H
#define DEADLOCK_H

#include <pthread.h>

typedef struct {
    int account_id;
    double balance;
    pthread_mutex_t lock;
} Account;

void run_deadlock_demo();

#endif
