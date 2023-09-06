#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define MAX_INPUT_SIZE 1024
#define MAX_HISTORY_SIZE 100

// Structure to store command history details
struct CommandHistory {
    char command[MAX_INPUT_SIZE];
    pid_t pid;
    time_t start_time;
    double execution_time;
};

// Global flag to indicate if Ctrl+C was pressed
volatile sig_atomic_t ctrl_c_pressed = 0;

// Global flag to track whether the shell is terminating
int is_terminating = 0;

// Global variable to keep track of the history count
int history_count = 0;

// Global array to store command history
struct CommandHistory history[MAX_HISTORY_SIZE];

// Signal handler for Ctrl+C
void sigint_handler(int signum) {
    // Handling Ctrl+C when not terminating
    if (!is_terminating) {
        printf("\nCtrl+C pressed. To exit the shell, type 'exit'.\n");
        ctrl_c_pressed = 1;
    } else {
        // Display command details for entries in history when terminating
        for (int i = 0; i < history_count; i++) {
            printf("%d: %s (PID: %d, Start Time: %s, Execution Time: %.2lf seconds)\n",
                   i + 1, history[i].command, history[i].pid,
                   asctime(localtime(&history[i].start_time)),
                   history[i].execution_time);
        }
        exit(EXIT_SUCCESS);
    }
}

// Function to parse user input into command and arguments
void parse_input(char *input, char *command, char **arguments) {
    char *token = strtok(input, " \n");
    strcpy(command, token);

    int i = 0;
    while (token != NULL) {
        arguments[i++] = token;
        token = strtok(NULL, " \n");
    }
    arguments[i] = NULL;
}

int main() {
    char input[MAX_INPUT_SIZE];
    char command[MAX_INPUT_SIZE];
    char *arguments[MAX_INPUT_SIZE];

    // Set up the Ctrl+C signal handler
    signal(SIGINT, sigint_handler);

    while (1) {
        // Check if Ctrl+C was pressed
        if (ctrl_c_pressed) {
            ctrl_c_pressed = 0;
            continue;
        }

        printf("SimpleShell> ");
        fflush(stdout);

        // Read user input
        if (fgets(input, sizeof(input), stdin) == NULL) {
            perror("fgets");
            exit(EXIT_FAILURE);
        }

        // Remove newline character from input
        input[strcspn(input, "\n")] = '\0';

        // Handle history command
        if (strcmp(input, "history") == 0) {
            // Display command history
            for (int i = 0; i < history_count; i++) {
                printf("%d: %s\n", i + 1, history[i].command);
            }
            continue;
        }

        // Save command to history
        if (history_count < MAX_HISTORY_SIZE) {
            strcpy(history[history_count].command, input);
            history_count++;
        }

        // Parse user input
        parse_input(input, command, arguments);

        // Exit shell if the user enters "exit"
        if (strcmp(command, "exit") == 0) {
            printf("Terminating the SimpleShell.\n");
            is_terminating = 1;
            raise(SIGINT); // Trigger the Ctrl+C handler to display history
        }

        // Fork a child process to execute the command
        pid_t child_pid = fork();

        if (child_pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (child_pid == 0) {
            // Child process
            time(&history[history_count - 1].start_time); // Record start time
            if (execvp(command, arguments) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else {
            // Parent process
            int status;
            time_t start_time, end_time;
            time(&start_time); // Record start time
            waitpid(child_pid, &status, 0);
            time(&end_time); // Record end time
            if (WIFEXITED(status)) {
                history[history_count - 1].pid = child_pid;
                history[history_count - 1].execution_time = difftime(end_time, start_time);
                printf("Command exited with status %d\n", WEXITSTATUS(status));
            }
        }
    }

    return 0;
}

