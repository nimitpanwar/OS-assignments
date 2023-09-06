#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024

volatile sig_atomic_t ctrl_c_pressed = 0;

void sigint_handler(int signum) {
    ctrl_c_pressed = 1;
}

void execute_command(char *command) {
    // Tokenize the command into arguments
    char *args[MAX_INPUT_SIZE];
    int i = 0;
    args[i] = strtok(command, " \n");
    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, " \n");
    }
    args[i] = NULL;

    // Create a child process to execute the command
    pid_t pid = fork();

    if (pid == 0) {
        // This is the child process
        if (execvp(args[0], args) == -1) {
            perror("Shell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("Shell");
    } else {
        // This is the parent process
        int status;
        waitpid(pid, &status, 0);
    }
}

int main() {
    char input[MAX_INPUT_SIZE];

    // Set up the SIGINT (Ctrl+C) signal handler
    signal(SIGINT, sigint_handler);

    do {
        printf("SimpleShell> "); // Display a customizable prompt
        if (fgets(input, sizeof(input), stdin) == NULL) {
            perror("Shell");
            exit(EXIT_FAILURE);
        }

        if (strcmp(input, "exit\n") == 0) {
            break;
        }

        if (ctrl_c_pressed) {
            printf("Ctrl+C pressed. Exiting.\n");
            // Add process pid, time at which the command was executed, total duration
            break;
        }

        execute_command(input);
    } while (1);

    return 0;
}

