#include "deadlock.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static Account accounts[2];

struct TransferArgs {
    int from;
    int to;
    double amount;
};

static void destroy_accounts() {
    for (int i = 0; i < 2; i++) {
        pthread_mutex_destroy(&accounts[i].lock);
    }
}

static void init_accounts() {
    for (int i = 0; i < 2; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = 1000.0;
        pthread_mutex_init(&accounts[i].lock, NULL);
    }
}

static void* transfer_deadlock_prone(void* arg) {
    TransferArgs* transfer = static_cast<TransferArgs*>(arg);
    Account* from = &accounts[transfer->from];
    Account* to = &accounts[transfer->to];

    printf("  Thread_%d: locking Account_%d\n", transfer->from + 1, from->account_id);
    pthread_mutex_lock(&from->lock);
    usleep(100000);

    printf("  Thread_%d: trying Account_%d (will block in real deadlock)\n",
           transfer->from + 1, to->account_id);
    int second_lock_result = pthread_mutex_trylock(&to->lock);
    if (second_lock_result != 0) {
        printf("  Thread_%d: cannot lock Account_%d -> deadlock detected\n",
               transfer->from + 1, to->account_id);
        pthread_mutex_unlock(&from->lock);
        return NULL;
    }

    from->balance -= transfer->amount;
    to->balance += transfer->amount;
    pthread_mutex_unlock(&to->lock);
    pthread_mutex_unlock(&from->lock);
    return NULL;
}

static void* transfer_safe(void* arg) {
    TransferArgs* transfer = static_cast<TransferArgs*>(arg);
    Account* from = &accounts[transfer->from];
    Account* to = &accounts[transfer->to];
    Account* first = from->account_id < to->account_id ? from : to;
    Account* second = from->account_id < to->account_id ? to : from;

    printf("  Thread_%d: locking Account_%d then Account_%d\n",
           transfer->from + 1, first->account_id, second->account_id);
    pthread_mutex_lock(&first->lock);
    usleep(100000);
    pthread_mutex_lock(&second->lock);

    from->balance -= transfer->amount;
    to->balance += transfer->amount;
    printf("  Thread_%d: transferred %.1f from Account_%d to Account_%d\n",
           transfer->from + 1, transfer->amount, from->account_id, to->account_id);

    pthread_mutex_unlock(&second->lock);
    pthread_mutex_unlock(&first->lock);
    return NULL;
}

void run_deadlock_demo() {
    pthread_t thread_1;
    pthread_t thread_2;
    TransferArgs first_transfer = {0, 1, 200.0};
    TransferArgs second_transfer = {1, 0, 150.0};

    printf("\n--- Deadlock simulation: opposite lock order ---\n");
    init_accounts();
    pthread_create(&thread_1, NULL, transfer_deadlock_prone, &first_transfer);
    pthread_create(&thread_2, NULL, transfer_deadlock_prone, &second_transfer);
    pthread_join(thread_1, NULL);
    pthread_join(thread_2, NULL);
    printf("  Result: both threads held one account and needed the other. trylock avoids hanging the demo.\n");
    destroy_accounts();

    printf("\n--- Fixed version: Resource Ordering by smaller account_id ---\n");
    init_accounts();
    pthread_create(&thread_1, NULL, transfer_safe, &first_transfer);
    pthread_create(&thread_2, NULL, transfer_safe, &second_transfer);
    pthread_join(thread_1, NULL);
    pthread_join(thread_2, NULL);
    printf("  Final balances: Account_0=%.1f | Account_1=%.1f\n",
           accounts[0].balance, accounts[1].balance);
    destroy_accounts();
}
