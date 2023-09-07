#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>    
#include <sys/wait.h>  
#include <string.h>
#include <time.h>

// pipes & bonus are left


struct cmnd_Elt {
    char command[1024];
    int pid;
    time_t start_time;
    double execution_time;
};

struct cmnd_Elt cmnd_Array[100];

int cmnd_count = 0;

int ctrl_clicked=0;


static void my_handler(int signum) {
    if(signum == SIGINT) {
        ctrl_clicked=1;
    } 
}



int read_user_input(char* input,int size, char*command, char** arguments){
    fgets(input,size,stdin);
    if(ctrl_clicked){
        return 5;
    }
    if (cmnd_count < 100) {
            strcpy(cmnd_Array[cmnd_count].command, input);
            cmnd_count++;
    }

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
    return 0;
}

int create_process_and_run(char* command, char** arguments) {
    // time(&(cmnd_Array[cmnd_count - 1].start_time));
    int child_PID = fork();
    if(child_PID < 0) {
        printf("Something went wrong.\n");
        exit(10);
    } 
    else if(child_PID == 0) {
        // printf("I am the child process.\n");
        char* args[3] = {command,arguments[1],NULL};
        if (execvp(command, arguments) == -1) {
            perror("execvp");
            exit(10);
        }
        // time(&(cmnd_Array[cmnd_count - 1].start_time));
        printf("I should never get printed.\n");
    } 
    else {
        int ret;
        // time(&(cmnd_Array[cmnd_count - 1].start_time));
        time_t startTime;
        time(&startTime);
        waitpid(child_PID, &ret, 0);
        time_t endTime;
        time(&endTime);
        if(WIFEXITED(ret)) {
            cmnd_Array[cmnd_count - 1].execution_time=difftime(endTime,startTime);
            cmnd_Array[cmnd_count - 1].pid=child_PID;
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

void cd_Func(char *path) {
    if (chdir(path) != 0) {
        perror("cd");
    }
}


void print_History(){
    for (int i = 0; i < cmnd_count; i++) {
        printf("%d- %s \n", i + 1, cmnd_Array[i].command);
        int j=1;
    }
}

void print_On_Exit(){
    printf("\nNo. : Command    PID    Start Time   Execution Time\n");
    for (int i = 0; i < cmnd_count; i++) {
        printf("%d : %s    %d    %s    %.2lf seconds\n",i+1, 
        cmnd_Array[i].command, cmnd_Array[i].pid,
        asctime(localtime(&cmnd_Array[i].start_time)),
        cmnd_Array[i].execution_time);
    }
}


void shell_Loop() {
    char input[1024];
    char command[1024];
    char* arguments[1024];
    // signal(SIGINT, my_handler);
    do {
        printf("(SimpleShell) ");
        fflush(stdout);
        int sig_recieved= read_user_input(input, sizeof(input), command, arguments);
        if(sig_recieved){
            print_On_Exit();
            return;
        }
        if(strcmp(command,"history")==0){
            print_History();
        }
        else if (strcmp(command, "cd") == 0) {
            if (arguments[1] != NULL) {
                cd_Func(arguments[1]);
            } 
        }
        else{
            launch(command, arguments);
        }
    } 
    while (1);
}


int main(){
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = my_handler;
    sigaction(SIGINT, &sig, NULL);

    shell_Loop();

    return 0;
}
