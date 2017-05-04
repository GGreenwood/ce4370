#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define NUM_TASKS   5

#define MIN_PERIOD  200
#define MAX_PERIOD  2000

#define MIN_LENGTH  50
#define MAX_LENGTH_RATIO    .5

#define MAX_UTILIZATION     .99

typedef struct {
    int id;
    int period;
    int length;
    int deadline;
    int laxity;
    bool is_critical = false;
    bool is_ready = true;
} Task;

// Returns time since first call in milliseconds
struct timeval start;
int get_time(void) {
    if(start.tv_sec == 0) {
        gettimeofday(&start, NULL);
        return 0;
    }

    struct timeval stop;
    gettimeofday(&stop, NULL);

    return (int)(stop.tv_usec - start.tv_usec) / 1000 + (int)(stop.tv_sec - start.tv_sec) * 1000;
}

// Sleeps a number of milliseconds
void msleep(int ms) {
    usleep(ms * 1000);
}

// Returns an integer in the range [0, n).
int randint(int lower, int upper) {
    int n = upper - lower;

    long end = RAND_MAX / n; // truncate skew
    end *= n;
    int r;
    while ((r = rand()) >= end);
    return lower + (r % n);
}

// Compare tasks by their periods, for finding the critical set
// for use by qsort
int cmp_task_period(const void *a, const void *b) {
    const Task *task1 = (Task*) a;
    const Task *task2 = (Task*) b;

    if(task1->period < task2->period) {
        return -1;
    } else if(task1->period > task2->period) {
        return 1;
    } else {
        return 0;
    }
}

// Finds the next task to execute
// Compare tasks by readiness, criticality, then by inverse laxity
// for use by qsort
int cmp_task_next(const void *a, const void *b) {
    const Task *task1 = (Task*) a;
    const Task *task2 = (Task*) b;

    // Sort by readiness
    if(task1->is_ready && !task2->is_ready) {
        return -1;
    } else if(!task1->is_ready && task2->is_ready) {
        return 1;
    } else if(!task1->is_ready && !task2->is_ready) {
        return 0;
    }

    // If both tasks are ready, continue
    // Next, sort by criticality
    if(task1->is_critical && !task2->is_critical) {
        return -1;
    } else if(!task1->is_critical && task2->is_critical) {
        return 1;
    } else if(!task1->is_critical && !task2->is_critical) {
        return 0;
    }

    // If both tasks are ready and critical, continue
    // Next, sort by inverse laxity(dynamic priority)
    if(task1->laxity < task2->laxity) {
        return -1;
    } else if(task1->laxity > task2->laxity) {
        return 1;
    } else {
        return 0;
    }

    // If all is equal, sort arbitrarily(FCFS)
    return 0;
}

void update_task(Task *task, int cur_time) {
    if(task->deadline <= cur_time + task->length) {
        if(task->is_ready) {
            // Deadline is reached, and task has not been executed
            printf("Task %i deadline missed. Increasing deadline by one period.\n", task->id);
            task->deadline += task->period;
            return;
        } else {
            // Deadline is reached, and task has been executed
            task->deadline += task->period;
            task->is_ready = true;
            return;
        }
    }

    // Update laxity
    task->laxity = task->deadline - cur_time - task->length;
    //printf("%i\n", task->laxity);
}

void execute_task(Task *task) {
    printf("Executing task %i...\n", task->id);
    task->is_ready = false;
    msleep(task->length);
}

void print_task(Task *task, bool verbose) {
    printf("### Task %i ###\n", task->id);
    printf("Period: \t%4i\n", task->period);
    printf("Length: \t%4i\n", task->length);

    if(verbose) {
        printf("Deadline: \t%12i\n", task->deadline);
        printf("Laxity:   \t%12i\n", task->laxity);
        printf("Is critical: \t%5s\n", task->is_critical ? "true" : "false");
        printf("Is ready:    \t%5s\n", task->is_ready ? "true" : "false");
    }

    printf("\n");
}

int main(int argc, char **argv) {
    srand(time(NULL));

    Task tasks[NUM_TASKS];

    // Generate some tasks randomly
    printf("GENERATING RANDOM TASKS...\n");
    for(int i = 0; i < NUM_TASKS; i++) {
        tasks[i].id = i;
        tasks[i].period = randint(MIN_PERIOD, MAX_PERIOD);
        tasks[i].length = randint(MIN_LENGTH, tasks[i].period * MAX_LENGTH_RATIO);
        tasks[i].deadline = tasks[i].period;

        print_task(&tasks[i], false);
    }

    // Sort tasks by period
    printf("SORTING TASKS BY PERIOD...\n");
    qsort(tasks, NUM_TASKS, sizeof(Task), cmp_task_period);
    for(int i = 0; i < NUM_TASKS; i++) {
        printf("Task %i: %i\n", tasks[i].id, tasks[i].period);
    }
    printf("\n");

    // Find critical set of tasks
    printf("FINDING CRITICAL SET OF TASKS...\n");
    double total_utilization = 0;
    for(int i = 0; i < NUM_TASKS; i++) {
        double utilization = (double) tasks[i].length / tasks[i].period;
        if(utilization + total_utilization < MAX_UTILIZATION) {
            tasks[i].is_critical = true;
            printf("Task %i is critical\n", tasks[i].id);
            total_utilization += utilization;
        } else {
            break;
            printf("Task %i is not critical\n", tasks[i].id);
        }
    }
    printf("Total critical utilization is %f\n\n", total_utilization);

    // Enter execution loop
    printf("ENTERING EXECUTION LOOP...\n");
    for(;;) {
        int cur_time = get_time();
        // Update each tasks state
        for(int i = 0; i < NUM_TASKS; i++) {
            update_task(&tasks[i], cur_time);
        }
        // Find next task to run
        qsort(tasks, NUM_TASKS, sizeof(Task), cmp_task_next);

        for(int i = 0; i < NUM_TASKS; i++) {
            //print_task(&tasks[i], true);
        }
        execute_task(&tasks[0]);
    }

    return 0;
}
