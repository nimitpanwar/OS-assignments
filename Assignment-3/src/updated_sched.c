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

int NCPUs;
int TSLICE;

int end_print_history=0;

static void my_handler(int signum) {
    if (signum == SIGINT) {
        end_print_history=1;
    }
}



typedef struct{
    int pid;
    char name[100];
    char first_arg[100];
    time_t prev_queued_time;
    double wait_time;
    time_t execution_time;
    int priority;
} process;

typedef struct {
    process arr[100];
    int size;
    int capacity;
    sem_t mutex;
    time_t first_arrival;
}priority_queue;
 

void insertHelper(priority_queue* pq, int index)
{
 
    // Store parent of element at index
    // in parent variable
    int parent = (index - 1) / 2;
 
    if (pq->arr[parent].priority > pq->arr[index].priority) {
        // Swapping when child is smaller
        // than parent element
        process temp = pq->arr[parent];
        pq->arr[parent] = pq->arr[index];
        pq->arr[index] = temp;
 
        // Recursively calling insertHelper
        insertHelper(pq, parent);
    }
    else if(pq->arr[parent].priority == pq->arr[index].priority){
        if(pq->arr[parent].prev_queued_time < pq->arr[index].prev_queued_time){
            process temp = pq->arr[parent];
            pq->arr[parent] = pq->arr[index];
            pq->arr[index] = temp;  
            insertHelper(pq, parent);
        }
    }
}


void insert(priority_queue* pq, process p)
{
 
    if (pq->size < pq->capacity) {
        pq->arr[pq->size] = p;
        insertHelper(pq, pq->size);
        pq->size++;
    }
}


void minHeapify(priority_queue* pq, int index)
{
    int left = index * 2 + 1;
    int right = index * 2 + 2;
    int min = index;
 
    // Checking whether our left or child element
    // is at right index or not to avoid index error
    if (left >= pq->size || left < 0)
        left = -1;
    if (right >= pq->size || right < 0)
        right = -1;
 
    // store left or right element in min if
    // any of these is smaller that its parent
    if (left != -1 && pq->arr[left].priority < pq->arr[index].priority){
        min = left;
    }
    else if(left != -1 && pq->arr[left].priority == pq->arr[index].priority){
        if(pq->arr[left].prev_queued_time < pq->arr[index].prev_queued_time){
           min = left; 
        }
    }

    if (right != -1 && pq->arr[right].priority < pq->arr[index].priority)
        min = right;
    else if(right != -1 && pq->arr[right].priority == pq->arr[index].priority){
        if(pq->arr[right].prev_queued_time < pq->arr[index].prev_queued_time){
           min = right;
        }
    }
    

    // Swapping the nodes
    if (min != index) {
        process temp = pq->arr[min];
        pq->arr[min] = pq->arr[index];
        pq->arr[index] = temp;
 
        // recursively calling for their child elements
        // to maintain min heap
        minHeapify(pq, min);
    }
}

process extractMin(priority_queue* pq)
{
    process deleteItem;
 
    // Checking if the heap is empty or not
 
    // Store the node in deleteItem that
    // is to be deleted.
    deleteItem = pq->arr[0];
 
    // Replace the deleted node with the last node
    pq->arr[0] = pq->arr[pq->size - 1];
    // Decrement the size of heap
    pq->size--;
 
    // Call minheapify_top_down for 0th index
    // to maintain the heap property
    minHeapify(pq, 0);
    return deleteItem;
}




int main(int argv, char**argc){

    NCPUs = atoi(argc[1]);  
    TSLICE = atoi(argc[2]); 


    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = my_handler;
    sigaction(SIGINT, &sig, NULL);

    int fd = shm_open("sm2", O_RDWR, 0666);

    priority_queue* ready_queue = mmap(0, sizeof(priority_queue), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    // scheduler(ready_queue);

    typedef struct {
        int arr[NCPUs];
        sem_t mutex;
        int count;
    }flag_array;

    int fd2 =shm_open("sm3", O_CREAT | O_RDWR, 0666);
    ftruncate(fd2, sizeof(flag_array));
    flag_array* terminated_array = mmap(NULL, sizeof(flag_array), PROT_READ | PROT_WRITE, MAP_SHARED, fd2 , 0);
    close(fd2);
    sem_init(&terminated_array->mutex,1,1);
    terminated_array->count=0;
    for (size_t i = 0; i < NCPUs; i++) {
        terminated_array->arr[i] = 0;  
    }


    process all_processes[1000];
    int process_number=0;

    while(!end_print_history){
        // printf("Yes\n");
        process running_array[NCPUs];
        int running_count = 0;
        // printf("%d\n",ready_queue->size);
        if(ready_queue->size>0){
            // printf("Yes\n");
            sem_wait(&ready_queue->mutex);
            while(ready_queue->size!=0 && running_count<NCPUs){
                // printf("i - %d\n",running_count);
                printf(" ");
                running_array[running_count]=extractMin(ready_queue);
                // printf("in sched- %d\n",running_array[running_count]);
                kill(running_array[running_count].pid, SIGCONT);
                time_t starting_time;
                time(&starting_time);
                running_array[running_count].wait_time+= difftime(starting_time,running_array->prev_queued_time);
                // printf("in the sched - %d\n",running_array[running_count].pid);
                running_count++; 
            }
            sem_post(&ready_queue->mutex);
        }


        if(running_count>0){
            sleep(TSLICE);
            sem_wait(&ready_queue->mutex);
            for (int i = 0; i < running_count; i++) {
                // if(getProcessState(running_array[i].pid)[0]!='Z'){
                int terminated=0;
                // printf("YEs\n");
                // printf("in the sched- %d\n",terminated_array->count);
                for(int j=0;j<terminated_array->count;j++){
                    // printf("%d\n",terminated_array->arr[j]);
                    if(terminated_array->arr[j]==running_array[i].pid){
                        // printf("in the sched- %d\n",terminated_array->arr[j]);
                        // printf("YEs\n");
                        terminated=1;
                        break;
                    }
                }
                if(terminated==0){
                    // printf("running -%s\n",running_array[i].name);
                    kill(running_array[i].pid, SIGSTOP);
                    running_array[i].prev_queued_time=time(NULL);
                    insert(ready_queue, running_array[i]);
                }
                else{
                    // printf("terminated - %s\n",running_array[i].name);
                    running_array[i].execution_time=time(NULL);
                    all_processes[process_number]=running_array[i];
                    process_number++;
                }
            }
            sem_post(&ready_queue->mutex);

            sem_wait(&terminated_array->mutex);
            for (size_t i = 0; i < NCPUs; i++) {
                terminated_array->arr[i] = 0;  
            }
            terminated_array->count=0;
            sem_post(&terminated_array->mutex);
        }

    }  
    
    printf("\n--------------------------------\n");
    printf("Name   PID   Wait Time    Execution Time\n");
    for(int i=0;i<process_number;i++){
        printf("%s ",all_processes[i].name);
        if(strcmp(all_processes[i].first_arg,"NULL")!=0){
            // printf("%s ",all_processes[i].first_arg);
        }
        printf("%d ",all_processes[i].pid);
        // printf("%s ",ctime(&all_processes[i].prev_queued_time));
        printf("%.2lf seconds ",all_processes[i].wait_time);
        // printf("%s \n",ctime(&all_processes[i].execution_time));
        printf("%.2lf seconds\n",difftime(all_processes[i].execution_time,ready_queue->first_arrival));
    }

    munmap("sm3",sizeof(flag_array));
    shm_unlink("sm3");
    return 0;
}



// program priority
// program 
// program arguments priority
