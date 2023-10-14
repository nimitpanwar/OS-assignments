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

int NCPU;
int TSLICE;

typedef struct{
    int pid;
    char name[100];
    time_t prev_queued_time;
    double wait_time;
    time_t execution_time;
} process;

typedef struct  {
    process array[100];
    int front;
    int rear;
    sem_t mutex;
}queue;



struct cmnd_Elt {
    char command[1024];
    int pid;
    time_t start_time;
    double execution_time;
};

struct cmnd_Elt cmnd_Array[100];
int cmnd_count = 0;
int ctrl_clicked = 0;

static void my_handler(int signum) {
    if (signum == SIGINT) {
        ctrl_clicked = 1;
    }
}



int read_user_input(char* input, int size, char* command, char** arguments) {
    fgets(input, size, stdin);
    if (ctrl_clicked) {
        return 5;
    }
    if (cmnd_count < 100) {
        strcpy(cmnd_Array[cmnd_count].command, input);
        cmnd_count++;
    }

    input[strcspn(input, "\n")] = '\0';
    char* temp = strtok(input, " \n");
    strcpy(command, temp);

    int i = 0;
    while (temp != NULL) {
        arguments[i] = temp;
        temp = strtok(NULL, " \n");
        i++;
    }
    arguments[i] = NULL;
    return 0;
}

void enqueue (queue* q,process p){
    q->rear++;
    q->array[q->rear]=p;
}



void create_process_for_scheduler(queue* ready_queue, char** arguments) {
    char executable[1024];
    strcpy(executable,arguments[1]);

    int length = strlen(executable);

    if (length < 3) {
        printf("Invalid input: %s\n", executable);
        return;
    }

    char* new_arguments[1024];

    char first_argument[length - 1];
    int j = 0;

    for (int i = 2; i < length; i++) {
        first_argument[j] = executable[i];
        j++;
    }
    first_argument[j] = '\0';

    new_arguments[0]=first_argument;
    int i=1;
    int k=2;
    while(arguments[k]!=NULL){
        new_arguments[i]=arguments[k];
        // printf("%s\n",new_arguments[i]);
        i++;
        k++;
    }

    new_arguments[i]=NULL;

    int child_PID = fork();
    if (child_PID < 0) {
        printf("Something went wrong.\n");
        exit(10);
    } else if (child_PID == 0) {
        int my_pid= getpid();
        kill(my_pid,SIGSTOP);
        if (execvp(executable, new_arguments) == -1) {
            perror("execlp");
            exit(10);
        }
    } else {

        process p;
        strcpy(p.name,executable);
        p.pid=child_PID;
        p.prev_queued_time=time(NULL);
        p.wait_time=0;
        sem_wait(&ready_queue->mutex);
        // printf("IN THE SHELL\n");
        enqueue(ready_queue,p);
        // printf("in shell- %d\n",)
        sem_post(&ready_queue->mutex);
        // printf("in shell- %d",child_PID);
    }
}



int launch(char* command, char** arguments) {
    int status;
    // status = create_process_and_run(command, arguments);
    return status;
}


void shell_Loop(queue* ready_queue) {
    char input[1024];
    char command[1024];
    char* arguments[1024];
    char cNCPU[3];
    char cTSLICE[4];

    int sched_pid=fork();
    if(sched_pid==0){                           
        snprintf(cNCPU, sizeof(cNCPU), "%d", NCPU);
        snprintf(cTSLICE, sizeof(cTSLICE), "%d", TSLICE);              
        execlp("./sched", "sched",cNCPU, cTSLICE,NULL);
    }
    else{
        do {
            printf(">>> $ ");
            fflush(stdout);
            int sig_received = read_user_input(input, sizeof(input), command, arguments);
            if (sig_received) {
                kill(sched_pid,SIGINT);
                sem_destroy(&ready_queue->mutex); 
                munmap(ready_queue, sizeof(queue));
                shm_unlink("shared memory");
                return;
            }
            if (strcmp(command, "history") == 0) {
            }

            if (strcmp(command, "submit") == 0) {
                if (arguments[1] != NULL) {
                    if (ready_queue->rear< 100-1) {
                        create_process_for_scheduler(ready_queue,arguments);
                    } else {
                        printf("Maximum process limit reached.\n");
                    }
                } else {
                    printf("Usage: submit <pname>\n");
                }
            }else {
                launch(command, arguments);
            }
        } while (1);
    }
}

int main(int argv, char**argc) {
    
    NCPU = atoi(argc[1]);  
    TSLICE = atoi(argc[2]);  

    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = my_handler;
    sigaction(SIGINT, &sig, NULL);

    int fd = shm_open("shared memory", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(queue));

    queue* ready_queue = mmap(0, sizeof(queue), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    sem_init(&ready_queue->mutex, 1, 1);

    ready_queue->front=-1;
    ready_queue->rear=-1;
    shell_Loop(ready_queue);

    return 0;
}
