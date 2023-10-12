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


typedef struct  {
    int array[100];
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

void enqueue (queue* q,int p){
    q->rear++;
    q->array[q->rear]=p;
}


void create_process_for_scheduler(queue* ready_queue, char* executable) {
    int length = strlen(executable);

    // Check if the length is less than 3 (minimum for "./x")
    if (length < 3) {
        printf("Invalid input: %s\n", executable);
        return;
    }

    char argument[length - 1];
    int j = 0;

    // Skip the first two characters (typically "./")
    for (int i = 2; i < length; i++) {
        argument[j] = executable[i];
        j++;
    }
    argument[j] = '\0';

    int child_PID = fork();
    if (child_PID < 0) {
        printf("Something went wrong.\n");
        exit(10);
    } else if (child_PID == 0) {
        int my_pid= getpid();
        kill(my_pid,SIGSTOP);
        if (execlp(executable, argument, NULL) == -1) {
            perror("execlp");
            exit(10);
        }
    } else {

        sem_wait(&ready_queue->mutex);
        enqueue(ready_queue, child_PID);
        // printf("%d\n",child_PID);
        sem_post(&ready_queue->mutex);

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

    if(fork()==0){                           
        snprintf(cNCPU, sizeof(cNCPU), "%d", NCPU);
        snprintf(cTSLICE, sizeof(cTSLICE), "%d", TSLICE);              
        execlp("./simpleScheduler", "simpleScheduler",cNCPU, cTSLICE,NULL);
    }
    else{
        do {
            printf(">>> $ ");
            fflush(stdout);
            int sig_received = read_user_input(input, sizeof(input), command, arguments);
            if (sig_received) {
                shm_unlink("shared memory");
                return;
            }
            if (strcmp(command, "history") == 0) {
            }

            if (strcmp(command, "submit") == 0) {
                if (arguments[1] != NULL) {
                    if (ready_queue->rear< 100-1) {
                        create_process_for_scheduler(ready_queue,arguments[1]);
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

    sem_init(&ready_queue->mutex, 1, 1);

    ready_queue->front=-1;
    ready_queue->rear=-1;
    shell_Loop(ready_queue);

    return 0;
}
