#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

// Global variables for the number of CPUs and time slice
int NCPUs;
int TSLICE;

// Flag to indicate whether to print history
int end_print_history = 0;

// Signal handler for handling SIGINT (Ctrl+C)
static void my_handler(int signum) {
    if (signum == SIGINT) {
        end_print_history = 1;
    }
}

// Function to retrieve the state of a process by its PID
char* getProcessState(pid_t pid) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/status", pid);

    FILE *procfile = fopen(path, "r");
    if (procfile == NULL) {
        perror("Error opening /proc status file");
        return "Not Found";
    }

    char line[256];
    char* state = "Unknown";

    while (fgets(line, sizeof(line), procfile)) {
        if (strncmp(line, "State:", 6) == 0) {
            state = line + 7;
            break;
        }
    }

    fclose(procfile);
    return state;
}

// Define a structure to represent a process
typedef struct {
    int pid;
    char name[100];
    char first_arg[100];
    time_t prev_queued_time;
    double wait_time;
    time_t execution_time;
    int priority;
} process;

// Define a structure for a priority queue
typedef struct {
    process arr[100];
    int size;
    int capacity;
    sem_t mutex;
    time_t first_arrival;
} priority_queue;

// Helper function to insert a process into the priority queue
void insertHelper(priority_queue* pq, int index) {
    // Store parent of element at index in the parent variable
    int parent = (index - 1) / 2;

    if (pq->arr[parent].priority > pq->arr[index].priority) {
        // Swapping when child is smaller than parent element
        process temp = pq->arr[parent];
        pq->arr[parent] = pq->arr[index];
        pq->arr[index] = temp;
        insertHelper(pq, parent);
    } else if (pq->arr[parent].priority == pq->arr[index].priority) {
        if (pq->arr[parent].prev_queued_time < pq->arr[index].prev_queued_time) {
            process temp = pq->arr[parent];
            pq->arr[parent] = pq->arr[index];
            pq->arr[index] = temp;
            insertHelper(pq, parent);
        }
    }
}

// Function to insert a process into the priority queue
void insert(priority_queue* pq, process p) {
    if (pq->size < pq->capacity) {
        pq->arr[pq->size] = p;
        insertHelper(pq, pq->size);
        pq->size++;
    }
}

// Helper function to maintain the min-heap property
void minHeapify(priority_queue* pq, int index) {
    int left = index * 2 + 1;
    int right = index * 2 + 2;
    int min = index;

    if (left >= pq->size || left < 0)
        left = -1;
    if (right >= pq->size || right < 0)
        right = -1;

    if (left != -1 && pq->arr[left].priority < pq->arr[index].priority) {
        min = left;
    } else if (left != -1 && pq->arr[left].priority == pq->arr[index].priority) {
        if (pq->arr[left].prev_queued_time < pq->arr[index].prev_queued_time) {
            min = left;
        }
    }

    if (right != -1 && pq->arr[right].priority < pq->arr[index].priority)
        min = right;
    else if (right != -1 && pq->arr[right].priority == pq->arr[index].priority) {
        if (pq->arr[right].prev_queued_time < pq->arr[index].prev_queued_time) {
            min = right;
        }
    }

    if (min != index) {
        process temp = pq->arr[min];
        pq->arr[min] = pq->arr[index];
        pq->arr[index] = temp;
        minHeapify(pq, min);
    }
}

// Function to extract the minimum process from the priority queue
process extractMin(priority_queue* pq) {
    process deleteItem;
    deleteItem = pq->arr[0];
    pq->arr[0] = pq->arr[pq->size - 1];
    pq->size--;
    minHeapify(pq, 0);
    return deleteItem;
}

int main(int argv, char** argc) {
    // Parse command-line arguments
    NCPUs = atoi(argc[1]);
    TSLICE = atoi(argc[2]);

    // Set up a signal handler for SIGINT
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = my_handler;
    sigaction(SIGINT, &sig, NULL);

    // Open a shared memory region with error handling
    int fd = shm_open("sm2", O_RDWR, 0666);
    if (fd == -1) {
        perror("Error opening shared memory");
        exit(1); // Exit with an error code
    }

    // Map the shared memory region to the priority queue
    priority_queue* ready_queue = mmap(0, sizeof(priority_queue), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    // Check if the mapping was successful
    if (ready_queue == MAP_FAILED) {
        perror("Error mapping shared memory");
        exit(1); // Exit with an error code
    }

    // Array to store information about all processes
    process all_processes[1000];
    int process_number = 0;

    // Main loop to manage processes
    while (!end_print_history) {
        // Array to hold running processes
        process running_array[NCPUs];
        int running_count = 0;

        // Check if there are processes in the ready queue
        if (ready_queue->size > 0) {
            sem_wait(&ready_queue->mutex);
            while (ready_queue->size != 0 && running_count < NCPUs) {
                // Print a space (for visual separation) and extract a process from the ready queue
                printf(" ");
                running_array[running_count] = extractMin(ready_queue);
                // Resume the process by sending a SIGCONT signal
                kill(running_array[running_count].pid, SIGCONT);
                // Record the starting time to calculate wait time
                time_t starting_time;
                time(&starting_time);
                running_array[running_count].wait_time += difftime(starting_time, running_array->prev_queued_time);
                running_count++;
            }
            sem_post(&ready_queue->mutex);
        }

        // Check if there are running processes
        if (running_count > 0) {
            usleep(TSLICE*1000); // Simulate time slice
            sem_wait(&ready_queue->mutex);
            for (int i = 0; i < running_count; i++) {
                if (getProcessState(running_array[i].pid)[0] != 'Z') {
                    // If the process is not in a Zombie state, pause it (send SIGSTOP)
                    kill(running_array[i].pid, SIGSTOP);
                    running_array[i].prev_queued_time = time(NULL);
                    // Reinsert the process back into the ready queue
                    insert(ready_queue, running_array[i]);
                } else {
                    // If the process has completed (Zombie state), record its execution time and details
                    running_array[i].execution_time = time(NULL);
                    all_processes[process_number] = running_array[i];
                    process_number++;
                }
            }
            sem_post(&ready_queue->mutex);
        }
    }

    // Print the history of processes
    printf("\n--------------------------------\n");
    printf("Name   PID   Wait Time    Execution Time\n");
    for (int i = 0; i < process_number; i++) {
        printf("%s ", all_processes[i].name);
        if (strcmp(all_processes[i].first_arg, "NULL") != 0) {
            printf("%s ", all_processes[i].first_arg);
        }
        printf("%d ", all_processes[i].pid);
        printf("%.2lf seconds ", all_processes[i].wait_time);
        printf("%.2lf seconds\n", difftime(all_processes[i].execution_time, ready_queue->first_arrival));
    }

    return 0;
}
