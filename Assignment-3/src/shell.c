#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

int NCPU;
int TSLICE;

#define MAX_PROCESSES 100

struct Process {
    int pid;
    char pname[100];
};

struct Process processes[MAX_PROCESSES]; //array containing processes;
int process_count = 0; // stores processes present currently

void custom_signal_handler(int signum) {
    if (signum == SIGUSR1) {
        if (process_count > 0) {
            // Move the child process to the ready queue or take any other desired action
        } else {
            printf("No processes in the array to move to the ready queue.\n");
        }
    }
}

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

int create_process_and_run(char* command, char** arguments) {
    int child_PID = fork();
    if (child_PID < 0) {
        printf("Something went wrong.\n");
        exit(10);
    } else if (child_PID == 0) {
        if (execvp(command, arguments) == -1) {
            perror("execvp");
            exit(10);
        }
    } else {
        int ret;
        time_t startTime;
        time(&startTime);
        waitpid(child_PID, &ret, 0);
        time_t endTime;
        time(&endTime);
        if (WIFEXITED(ret)) {
            cmnd_Array[cmnd_count - 1].execution_time = difftime(endTime, startTime);
            cmnd_Array[cmnd_count - 1].pid = child_PID;
            printf("%d Exit = %d\n", child_PID, WEXITSTATUS(ret));
        } else {
            printf("Abnormal termination of %d\n", child_PID);
        }
    }
    return 0;
}

int launch(char* command, char** arguments) {
    int status;
    status = create_process_and_run(command, arguments);
    return status;
}

void print_History() {
    for (int i = 0; i < cmnd_count; i++) {
        printf("%d- %s \n", i + 1, cmnd_Array[i].command);
    }
}

void print_On_Exit() {
    printf("\nNo. : Command    PID    Start Time   Execution Time\n");
    for (int i = 0; i < cmnd_count; i++) {
        printf("%d : %s    %d    %s    %.2lf seconds\n", i + 1,
               cmnd_Array[i].command, cmnd_Array[i].pid,
               asctime(localtime(&cmnd_Array[i].start_time)),
               cmnd_Array[i].execution_time);
    }
}

void print_processes() {
    printf("Processes Array:\n");
    for (int i = 0; i < process_count; i++) {
        printf("PID: %d, pname: %s\n", processes[i].pid, processes[i].pname);
    }
}

void shell_Loop() {
    char input[1024];
    char command[1024];
    char* arguments[1024];
    do {
        printf(">>> $ ");
        fflush(stdout);
        int sig_received = read_user_input(input, sizeof(input), command, arguments);
        if (sig_received) {
            print_On_Exit();
            return;
        }
        if (strcmp(command, "history") == 0) {
            print_History();
        }// Inside the "submit" command handler

        // Inside the "submit" command handler
        if (strcmp(command, "submit") == 0) {
            // Check for valid input and arguments
            if (arguments[1] != NULL) {
                if (process_count < MAX_PROCESSES) {
                    int child_PID = fork();
                    if (child_PID < 0) {
                        printf("Something went wrong.\n");
                        exit(10);
                    } else if (child_PID == 0) {
                        // Child process
                        // Use the custom_signal_handler function declared above
                        signal(SIGUSR1, custom_signal_handler);

                        // Sleep or pause to wait for the signal from the scheduler
                        pause();
                    } else {
                        // Parent process
                        struct Process new_process;
                        new_process.pid = child_PID;
                        strcpy(new_process.pname, arguments[1]);
                        processes[process_count] = new_process;
                        process_count++;

                        // Send the signal only if the array was previously empty
                        if (process_count == 1) {
                        // Trigger the signal (e.g., SIGUSR1)
                            kill(child_PID, SIGUSR1);
                        }
                    }
                } else {
                    printf("Maximum process limit reached.\n");
                }
            } else {
                printf("Usage: submit <pname>\n");
            }
        }else if (strcmp(command, "print_processes") == 0) {
            // Call the function to print the processes array
            print_processes();
        }else {
            launch(command, arguments);
        }
    } while (1);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <integer1> <integer2>\n", argv[0]);
        return 1;
    }

    NCPU = atoi(argv[1]);
    TSLICE = atoi(argv[2]);

    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = my_handler;
    sigaction(SIGINT, &sig, NULL);

    shell_Loop();

    return 0;
}
