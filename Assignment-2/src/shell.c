#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>    
#include <sys/wait.h>  
#include <string.h>

//History not yet implemented.

// volatile sig_atomic_t ctrl_c_pressed = 0;


static void my_handler(int signum) {
    static int counter = 0;
    if(signum == SIGINT) {
        char buff1[23] = "\nCaught SIGINT signal\n";
        write(STDOUT_FILENO, buff1, 23);
        if(counter==0) {
            char buff2[20] = "Cannot handle more\n";
            write(STDOUT_FILENO, buff2, 20);
            exit(10);
        }
    } 
}


void read_user_input(char* input,int size, char*command, char** arguments){
    fgets(input,size,stdin);

    input[strcspn(input, "\n")] = '\0';
    char * temp= strtok(input," \n");
    strcpy(command, temp);

    int i = 0;
    while (temp != NULL) {
        arguments[i] = temp;
        temp = strtok(NULL, " \n");
        i++;
    }
    arguments[i] = NULL;

    printf("%s\n",command);
}

int create_process_and_run(char* command, char** arguments) {
    int child_PID = fork();
    if(child_PID < 0) {
        printf("Something bad happened.\n");
        exit(10);
    } 
    else if(child_PID == 0) {
        // printf("I am the child process.\n");
        char* args[3] = {command,arguments[1],NULL};
        if (execvp(command, arguments) == -1) {
            perror("execvp");
            exit(10);
        }
        printf("I should never get printed.\n");
    } 
    else {
        int ret;
        waitpid(child_PID, &ret, 0);
        if(WIFEXITED(ret)) {
            printf("%d Exit =%d\n",child_PID,WEXITSTATUS(ret));
        } 
        else {
            printf("Abnormal termination of %d\n",child_PID);
        }
    }
    return 0;
}

int launch (char* command, char** arguments) {
    int status;
    status = create_process_and_run(command,arguments);
    return status;
}

void shell_Loop() {
    char input[1024];
    char command[1024];
    char* arguments[1024];
    // signal(SIGINT, my_handler);
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = my_handler;
    sigaction(SIGINT, &sig, NULL);
    
    do {
        printf("SimpleShell> ");
        fflush(stdout);
        read_user_input(input, sizeof(input), command, arguments);
        launch(command, arguments);
    } while (1);
}

int main(){
    shell_Loop();
    return 0;
}
