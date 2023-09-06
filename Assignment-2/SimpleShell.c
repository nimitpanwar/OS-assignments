#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>    
#include <sys/wait.h>  
#include <string.h>

//History and exit on clicking ctrl+c not yet implemented.


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


void shell_Loop(){
    while(1){
        char input[1024]; 
        char command[1024]; 
        char* arguments[1024]; 
        printf("SimpleShell> ");
        fflush(stdout);
        read_user_input(input, sizeof(input), command, arguments);
        if(strcmp(command,"exit")==0){
            printf("Terminated\n");
            return;
        }
        // launch(command,arguments);
        // printf("%d",status);
    } 
}

int main(){
    shell_Loop();
    return 0;
}
