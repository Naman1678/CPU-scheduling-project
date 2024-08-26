/*
I am Naman Goyal, and I am taking CS-30200 course in the Spring 2024 semester. 
The due date for this programming assignment is (Thursday) 8 February 2024.
Description of the assignment : This project simulates the execution of real-time and interactive processes, including CPU, TTY, and disk processes. 
It calculates various metrics such as CPU and disk busy fractions.

Functions:
- initializeQueue: Initializes a queue.
  Arguments: Queue* q (pointer to the queue to be initialized)

- isQueueEmpty: Checks if a queue is empty.
  Arguments: Queue* q (pointer to the queue to be checked)
  Returns: 1 if the queue is empty, 0 otherwise

- isQueueFull: Checks if a queue is full.
  Arguments: Queue* q (pointer to the queue to be checked)
  Returns: 1 if the queue is full, 0 otherwise

- enqueue: Adds a process to a queue.
  Arguments: Queue* q (pointer to the queue), Process* process (pointer to the process to be added)

- dequeue: Removes a process from a queue.
  Arguments: Queue* q (pointer to the queue)
  Returns: Pointer to the dequeued process

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 100
#define MAX_QUEUE_SIZE 100

typedef enum {
    READY,
    RUNNING,
    WAITING
} ProcessStatus;

typedef enum {
    REAL_TIME,
    INTERACTIVE
} ProcessClass;

typedef enum {
    CPU_PROCESS,
    TTY_PROCESS,
    DISK_PROCESS
} InteractiveProcessType;

typedef struct InteractiveProcess {
    InteractiveProcessType type;
    int duration;
} InteractiveProcess;

typedef struct {
    int sequence_number;
    ProcessClass class;
    int arrival_time;
    ProcessStatus status;
    int remaining_cpu_time;
    int deadline;
    struct InteractiveProcess* interactive_process; // Add this field
} Process;

typedef struct {
    Process* processes[MAX_PROCESSES];
    int front;
    int rear;
    int size;
} Queue;

Queue realTimeQueue, interactiveQueue;
int current_time = 0;
int total_real_time_processes = 0;
int missed_deadline_count = 0;
int total_interactive_processes = 0;
int total_disk_accesses = 0;
int total_disk_time = 0;
int cpu_busy_time = 0;
int disk_busy_time = 0;
float cpu_utilization = 0.0;
float disk_utilization = 0.0;
float cpu_busy_fraction = 0.0;
float disk_busy_fraction = 0.0;

void initializeQueue(Queue* q) {
    q->front = 0;
    q->rear = -1;
    q->size = 0;
}

int isQueueEmpty(Queue* q) {
    return (q->size == 0);
}

int isQueueFull(Queue* q) {
    return (q->size == MAX_QUEUE_SIZE);
}

void enqueue(Queue* q, Process* process) {
    if (isQueueFull(q)) {
        printf("Queue is full\n");
        return;
    }
    q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
    q->processes[q->rear] = process;
    q->size++;
}

Process* dequeue(Queue* q) {
    if (isQueueEmpty(q)) {
        printf("Queue is empty\n");
        return NULL;
    }
    Process* process = q->processes[q->front];
    q->front = (q->front + 1) % MAX_QUEUE_SIZE;
    q->size--;
    return process;
}

void executeProcess(Process* process) {
    if (process->class == REAL_TIME) {
        total_real_time_processes++;
        if (current_time > process->deadline)
            missed_deadline_count++;
    } else { // Interactive process execution
        total_interactive_processes++;
        printf("%d. %s\tSimulated time: %d ms\n", process->sequence_number,
               (process->class == REAL_TIME) ? "REAL-TIME" : "INTERACTIVE", current_time);

        // Execute interactive process
        InteractiveProcess* interactive_process = process->interactive_process;
        switch (interactive_process->type) {
            case CPU_PROCESS:
                cpu_busy_time += interactive_process->duration;
                printf("%d.\t%s\tCPU\tSimulated time: %d ms\n", process->sequence_number,
                       (process->class == REAL_TIME) ? "REAL-TIME" : "INTERACTIVE", current_time);
                break;
            case TTY_PROCESS:
                printf("%d.\t%s\tTTY\tSimulated time: %d ms\n", process->sequence_number,
                       (process->class == REAL_TIME) ? "REAL-TIME" : "INTERACTIVE", current_time);
                break;
            case DISK_PROCESS:
                total_disk_accesses++; // Increment total disk accesses for each disk access
                disk_busy_time += interactive_process->duration;
                total_disk_time += interactive_process->duration;
                printf("%d.\t%s\tDISK\tSimulated time: %d ms\n", process->sequence_number,
                       (process->class == REAL_TIME) ? "REAL-TIME" : "INTERACTIVE", current_time);
                break;
            default:
                break;
        }
        current_time += interactive_process->duration;
    }

    // Free memory allocated for interactive process
    if (process->interactive_process)
        free(process->interactive_process);
}

int main() {
    initializeQueue(&realTimeQueue);
    initializeQueue(&interactiveQueue);

    // Process variables
    char process_class_str[20];
    int start_time;
    int cpu_time;
    int deadline;

    // Interactive process variables
    char interactive_process_type[20];
    int duration;
    int interactive_sequence_number = 1; // Sequence number for interactive processes

    // Open input file
    FILE *input_file = fopen("input.txt", "r");
    if (input_file == NULL) {
        printf("Error opening input file.\n");
        return 1;
    }

    // Loop to read input and enqueue processes
    while (fscanf(input_file, "%s %d", process_class_str, &start_time) == 2) {
        Process* process = (Process*)malloc(sizeof(Process));
        process->sequence_number = (strcmp(process_class_str, "REAL-TIME") == 0) ? total_real_time_processes + 1 : interactive_sequence_number++;

        if (strcmp(process_class_str, "REAL-TIME") == 0) {
            process->class = REAL_TIME;
            fscanf(input_file, "%d", &deadline);
            process->deadline = deadline;
        } else {
            process->class = INTERACTIVE;

            // Read interactive process type and duration
            fscanf(input_file, "%s %d", interactive_process_type, &duration);

            // Initialize interactive process
            InteractiveProcess* interactive_process = (InteractiveProcess*)malloc(sizeof(InteractiveProcess));
            if (strcmp(interactive_process_type, "CPU") == 0)
                interactive_process->type = CPU_PROCESS;
            else if (strcmp(interactive_process_type, "TTY") == 0)
                interactive_process->type = TTY_PROCESS;
            else if (strcmp(interactive_process_type, "DISK") == 0)
                interactive_process->type = DISK_PROCESS; // Set type as DISK_PROCESS for disk access
            interactive_process->duration = duration;

            // Assign interactive process to the process
            process->interactive_process = interactive_process;
        }

        enqueue((process->class == REAL_TIME) ? &realTimeQueue : &interactiveQueue, process);
    }

    fclose(input_file);

    while (!isQueueEmpty(&realTimeQueue) || !isQueueEmpty(&interactiveQueue)) {
        Process* next_process = NULL;
        if (!isQueueEmpty(&realTimeQueue)) {
            next_process = dequeue(&realTimeQueue);
        } else if (!isQueueEmpty(&interactiveQueue)) {
            next_process = dequeue(&interactiveQueue);
        }
        if (next_process) {
            executeProcess(next_process);
            free(next_process);
        }
    }

    // Calculate CPU and disk busy fractions
    cpu_busy_fraction = (float)cpu_busy_time / current_time;
    disk_busy_fraction = (float)disk_busy_time / current_time;

    printf("\n--- Summary ---\n");
    printf("The number of real time processes completed: %d\n", total_real_time_processes);
    if (total_real_time_processes > 0) {
        printf("The percentage of real time processes that miss their deadline: %.2f%%\n",
               (float)missed_deadline_count / total_real_time_processes * 100);
    } else {
        printf("The percentage of real time processes that miss their deadline: N/A\n");
    }
    printf("The total number of interactive processes completed: %d\n", total_interactive_processes);
    printf("The total number of disk access: %d\n", total_disk_accesses);
    if (total_disk_accesses > 0) {
        printf("The average duration of disk access (including the waiting time in the disk queue): %.2f ms\n",
               (float)total_disk_time / total_disk_accesses);
    } else {
        printf("The average duration of disk access: N/A\n");
    }
    printf("The total time elapsed since the start of the first process: %d ms\n", current_time);
    printf("The amount of time CPU was busy: %.2f\n", cpu_busy_fraction);
    printf("The amount of time the disk was busy: %.2f\n", disk_busy_fraction);

    return 0;
}
