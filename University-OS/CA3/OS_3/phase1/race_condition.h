#ifndef RACE_CONDITION_H
#define RACE_CONDITION_H

// تعداد تردها و تکرارها - قابل تغییر برای آزمایش
#define NUM_THREADS 4
#define NUM_ITERATIONS 100000

// اجرای بخش race condition (هر دو حالت: بدون قفل و با قفل)
void run_race_condition_demo();

#endif
