#define _POSIX_C_SOURCE 200809L

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROCESS_COUNT 5
#define MAX_EVENTS 256
#define DEFAULT_QUANTUM 2

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int priority;
    int remaining_time;
    int completion_time;
} process_t;

typedef struct {
    int pid;
    int start_time;
    int end_time;
} event_t;

static const process_t sample_processes[PROCESS_COUNT] = {
    {1, 0, 8, 2, 8, 0},
    {2, 1, 4, 1, 4, 0},
    {3, 2, 9, 3, 9, 0},
    {4, 3, 5, 2, 5, 0},
    {5, 4, 2, 1, 2, 0}
};

static void reset_processes(process_t processes[]) {
    int i;

    memcpy(processes, sample_processes, sizeof(sample_processes));
    for (i = 0; i < PROCESS_COUNT; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].completion_time = 0;
    }
}

static void add_event(event_t events[], int *event_count, int pid, int start,
                      int end) {
    if (start == end) {
        return;
    }

    if (*event_count > 0 && events[*event_count - 1].pid == pid &&
        events[*event_count - 1].end_time == start) {
        events[*event_count - 1].end_time = end;
        return;
    }

    if (*event_count >= MAX_EVENTS) {
        fprintf(stderr, "Error: too many scheduling events.\n");
        exit(1);
    }

    events[*event_count].pid = pid;
    events[*event_count].start_time = start;
    events[*event_count].end_time = end;
    (*event_count)++;
}

static int all_complete(const process_t processes[]) {
    int i;

    for (i = 0; i < PROCESS_COUNT; i++) {
        if (processes[i].remaining_time > 0) {
            return 0;
        }
    }

    return 1;
}

static int earliest_unfinished_arrival(const process_t processes[]) {
    int i;
    int earliest = INT_MAX;

    for (i = 0; i < PROCESS_COUNT; i++) {
        if (processes[i].remaining_time > 0 &&
            processes[i].arrival_time < earliest) {
            earliest = processes[i].arrival_time;
        }
    }

    return earliest;
}

static void print_input_table(void) {
    int i;

    printf("Input processes:\n");
    printf("PID  Arrival  Burst  Priority\n");
    for (i = 0; i < PROCESS_COUNT; i++) {
        printf("P%-3d %-8d %-6d %-8d\n",
               sample_processes[i].pid,
               sample_processes[i].arrival_time,
               sample_processes[i].burst_time,
               sample_processes[i].priority);
    }
    printf("\n");
}

static void print_results(const char *title, const process_t processes[],
                          const event_t events[], int event_count) {
    int i;
    double total_waiting = 0.0;
    double total_turnaround = 0.0;

    printf("=== %s ===\n", title);
    printf("Gantt chart: ");
    for (i = 0; i < event_count; i++) {
        if (events[i].pid == 0) {
            printf("[%d-%d idle] ", events[i].start_time, events[i].end_time);
        } else {
            printf("[%d-%d P%d] ", events[i].start_time, events[i].end_time,
                   events[i].pid);
        }
    }
    printf("\n");

    printf("PID  Completion  Turnaround  Waiting\n");
    for (i = 0; i < PROCESS_COUNT; i++) {
        int turnaround = processes[i].completion_time -
                         processes[i].arrival_time;
        int waiting = turnaround - processes[i].burst_time;

        total_turnaround += turnaround;
        total_waiting += waiting;

        printf("P%-3d %-11d %-11d %-7d\n",
               processes[i].pid,
               processes[i].completion_time,
               turnaround,
               waiting);
    }

    printf("Average turnaround time: %.2f\n",
           total_turnaround / PROCESS_COUNT);
    printf("Average waiting time: %.2f\n\n", total_waiting / PROCESS_COUNT);
}

static void schedule_fcfs(void) {
    process_t processes[PROCESS_COUNT];
    event_t events[MAX_EVENTS];
    int event_count = 0;
    int completed[PROCESS_COUNT] = {0};
    int complete_count = 0;
    int time = 0;

    reset_processes(processes);

    while (complete_count < PROCESS_COUNT) {
        int best = -1;
        int i;

        for (i = 0; i < PROCESS_COUNT; i++) {
            if (!completed[i] && processes[i].arrival_time <= time &&
                (best == -1 ||
                 processes[i].arrival_time < processes[best].arrival_time ||
                 (processes[i].arrival_time == processes[best].arrival_time &&
                  processes[i].pid < processes[best].pid))) {
                best = i;
            }
        }

        if (best == -1) {
            int next_time = earliest_unfinished_arrival(processes);
            add_event(events, &event_count, 0, time, next_time);
            time = next_time;
            continue;
        }

        add_event(events, &event_count, processes[best].pid, time,
                  time + processes[best].burst_time);
        time += processes[best].burst_time;
        processes[best].remaining_time = 0;
        processes[best].completion_time = time;
        completed[best] = 1;
        complete_count++;
    }

    print_results("First-Come, First-Served", processes, events, event_count);
}

static void schedule_sjf(void) {
    process_t processes[PROCESS_COUNT];
    event_t events[MAX_EVENTS];
    int event_count = 0;
    int time = 0;

    reset_processes(processes);

    while (!all_complete(processes)) {
        int best = -1;
        int i;

        for (i = 0; i < PROCESS_COUNT; i++) {
            if (processes[i].remaining_time > 0 &&
                processes[i].arrival_time <= time &&
                (best == -1 ||
                 processes[i].burst_time < processes[best].burst_time ||
                 (processes[i].burst_time == processes[best].burst_time &&
                  processes[i].arrival_time < processes[best].arrival_time))) {
                best = i;
            }
        }

        if (best == -1) {
            int next_time = earliest_unfinished_arrival(processes);
            add_event(events, &event_count, 0, time, next_time);
            time = next_time;
            continue;
        }

        add_event(events, &event_count, processes[best].pid, time,
                  time + processes[best].burst_time);
        time += processes[best].burst_time;
        processes[best].remaining_time = 0;
        processes[best].completion_time = time;
    }

    print_results("Shortest Job First", processes, events, event_count);
}

static void schedule_priority(void) {
    process_t processes[PROCESS_COUNT];
    event_t events[MAX_EVENTS];
    int event_count = 0;
    int time = 0;

    reset_processes(processes);

    while (!all_complete(processes)) {
        int best = -1;
        int i;

        for (i = 0; i < PROCESS_COUNT; i++) {
            if (processes[i].remaining_time > 0 &&
                processes[i].arrival_time <= time &&
                (best == -1 ||
                 processes[i].priority < processes[best].priority ||
                 (processes[i].priority == processes[best].priority &&
                  processes[i].arrival_time < processes[best].arrival_time))) {
                best = i;
            }
        }

        if (best == -1) {
            int next_time = earliest_unfinished_arrival(processes);
            add_event(events, &event_count, 0, time, next_time);
            time = next_time;
            continue;
        }

        add_event(events, &event_count, processes[best].pid, time,
                  time + processes[best].burst_time);
        time += processes[best].burst_time;
        processes[best].remaining_time = 0;
        processes[best].completion_time = time;
    }

    print_results("Priority Scheduling", processes, events, event_count);
}

static int select_round_robin_process(const process_t processes[], int time,
                                      int start_index, int priority_mode) {
    int best_priority = INT_MAX;
    int offset;

    if (priority_mode) {
        int i;

        for (i = 0; i < PROCESS_COUNT; i++) {
            if (processes[i].remaining_time > 0 &&
                processes[i].arrival_time <= time &&
                processes[i].priority < best_priority) {
                best_priority = processes[i].priority;
            }
        }
    }

    for (offset = 0; offset < PROCESS_COUNT; offset++) {
        int index = (start_index + offset) % PROCESS_COUNT;

        if (processes[index].remaining_time > 0 &&
            processes[index].arrival_time <= time &&
            (!priority_mode ||
             processes[index].priority == best_priority)) {
            return index;
        }
    }

    return -1;
}

static void schedule_round_robin(const char *title, int quantum,
                                 int priority_mode) {
    process_t processes[PROCESS_COUNT];
    event_t events[MAX_EVENTS];
    int event_count = 0;
    int time = 0;
    int next_index = 0;

    reset_processes(processes);

    while (!all_complete(processes)) {
        int selected = select_round_robin_process(processes, time, next_index,
                                                  priority_mode);
        int run_time;

        if (selected == -1) {
            int next_time = earliest_unfinished_arrival(processes);
            add_event(events, &event_count, 0, time, next_time);
            time = next_time;
            continue;
        }

        run_time = processes[selected].remaining_time < quantum
                       ? processes[selected].remaining_time
                       : quantum;

        add_event(events, &event_count, processes[selected].pid, time,
                  time + run_time);
        time += run_time;
        processes[selected].remaining_time -= run_time;

        if (processes[selected].remaining_time == 0) {
            processes[selected].completion_time = time;
        }

        next_index = (selected + 1) % PROCESS_COUNT;
    }

    print_results(title, processes, events, event_count);
}

static int parse_quantum(const char *text, int *quantum) {
    char *endptr;
    long parsed;

    parsed = strtol(text, &endptr, 10);
    if (*endptr != '\0' || parsed <= 0 || parsed > 1000) {
        return -1;
    }

    *quantum = (int)parsed;
    return 0;
}

int main(int argc, char *argv[]) {
    int quantum = DEFAULT_QUANTUM;

    if (argc > 2) {
        fprintf(stderr, "Usage: %s [time_quantum]\n", argv[0]);
        return 1;
    }

    if (argc == 2 && parse_quantum(argv[1], &quantum) == -1) {
        fprintf(stderr, "Error: time_quantum must be a positive integer.\n");
        return 1;
    }

    print_input_table();
    printf("Round-robin quantum: %d\n\n", quantum);

    schedule_fcfs();
    schedule_sjf();
    schedule_priority();
    schedule_round_robin("Round Robin", quantum, 0);
    schedule_round_robin("Priority with Round Robin", quantum, 1);

    return 0;
}
