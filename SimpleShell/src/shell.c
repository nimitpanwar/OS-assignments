#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>    
#include <sys/wait.h>  
#include <string.h>
#include <time.h>



struct cmnd_Elt {
    char command[1000];
    int pid;
    time_t start_time;
    long long execution_time;
};

struct cmnd_Elt cmnd_Array[100];

int cmnd_count = 0;

int ctrl_clicked=0;


static void my_handler(int signum) {
    if(signum == SIGINT) {
        ctrl_clicked=1;
    } 
}

void remove_and(char* str) {
    for(int i=0;i<strlen(str);i++){
        if(str[i]=='&'){
            str[i]='\0';
        }
    }
}

void background_process_creation(char* command, char** arguments){
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    time(&(cmnd_Array[cmnd_count-1].start_time));
    int child_PID = fork();
    if(child_PID < 0) {
        perror("fork");
        exit(10);
    } 
    else if(child_PID == 0) {
        int grandchild_PID=fork();
        if(grandchild_PID<0) {
            perror("fork");
        }
        else if (grandchild_PID == 0) {
            if (execvp(command, arguments) == -1) {
                perror("execvp");
                exit(10);
            }
        } 
        else {
            cmnd_Array[cmnd_count - 1].pid=child_PID;
            _exit(0);
        }
    } 
    else {
        printf("%d\n",child_PID);
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        cmnd_Array[cmnd_count - 1].execution_time = (end_time.tv_sec - start_time.tv_sec) * 1000LL + (end_time.tv_nsec - start_time.tv_nsec) / 1000000LL;
        cmnd_Array[cmnd_count - 1].pid=child_PID;
    }
}


int read_user_input(char* input,int size, char*command, char** arguments){
    fgets(input,size,stdin);
    if(ctrl_clicked){
        return 1;
    }
    if (cmnd_count < 100) {
            strcpy(cmnd_Array[cmnd_count].command, input);
            cmnd_count++;
    }

    if(strstr(input,"|") && strstr(input,"&") ){
        remove_and(input);
        return 4;
    }
    else if(strstr(input,"|")){
        return 2;
    }
    else if(strstr(input,"&")){
        remove_and(input);
        // printf("%s",input);
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
        return 3;
    }
    else{
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
    }
    return 0;
}

int create_process_and_run(char* command, char** arguments) {
    int child_PID = fork();
    if(child_PID < 0) {
        perror("fork");
        exit(10);
    } 
    else if(child_PID == 0) {
        if (execvp(command, arguments) == -1) {
            perror("execvp error");
            exit(10);
        }
    } 
    else {
        int ret;
        struct timespec start_time, end_time;
        time(&(cmnd_Array[cmnd_count-1].start_time));
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        waitpid(child_PID, &ret, 0);
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        if(WIFEXITED(ret)) {
            // cmnd_Array[cmnd_count - 1].execution_time=difftime(endTime,startTime);
            cmnd_Array[cmnd_count - 1].execution_time = (end_time.tv_sec - start_time.tv_sec) * 1000LL + (end_time.tv_nsec - start_time.tv_nsec) / 1000000LL;
            cmnd_Array[cmnd_count - 1].pid=child_PID;

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
    time(&(cmnd_Array[cmnd_count-1].start_time));
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    if (chdir(path) != 0) {
        perror("cd");
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    cmnd_Array[cmnd_count - 1].execution_time= (end_time.tv_sec - start_time.tv_sec) * 1000LL + (end_time.tv_nsec - start_time.tv_nsec) / 1000000LL;

}

void print_History(){
    // time(&cmnd_Array[cmnd_count-1].start_time);
    time(&(cmnd_Array[cmnd_count-1].start_time));
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    for (int i = 0; i < cmnd_count; i++) {
        printf("%d- %s", i + 1, cmnd_Array[i].command);
        int j=1;
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    cmnd_Array[cmnd_count - 1].execution_time= (end_time.tv_sec - start_time.tv_sec) * 1000LL + (end_time.tv_nsec - start_time.tv_nsec) / 1000000LL;
}

void print_On_Exit(){
    printf("\nNo. : Command    PID    Start Time   Execution Time\n");
    for (int i = 0; i < cmnd_count; i++) {
        printf("%d : %s    %d    %s    %.2lld milliseconds\n",i+1, 
        cmnd_Array[i].command, cmnd_Array[i].pid,
        asctime(localtime(&cmnd_Array[i].start_time)),
        cmnd_Array[i].execution_time);
    }
}



int read_input_piped(char* input,char*command, char** arguments){

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



void execute_piped_commands(char** input_List, int num_Commands) {

    int fd[num_Commands - 1][2];
    int pids[num_Commands];

    for (int i = 0; i < num_Commands - 1; i++) {
        if (pipe(fd[i]) == -1) {
            perror("pipe");
            return;
        }
    }

    for (int i = 0; i < num_Commands; i++) {
        char input[1024];
        char command[1024];
        char* arguments[1024];
        strcpy(input, input_List[i]);
        read_input_piped(input, command, arguments);
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            return;
        }
        if (pids[i] == 0) {
            if (i == 0) {
                dup2(fd[i][1], STDOUT_FILENO);
                for (int j = 0; j < num_Commands - 1; j++) {
                    close(fd[j][0]);
                    close(fd[j][1]);
                }
                if (execvp(command, arguments) == -1) {
                    perror("execvp");
                    return;
                }
            } else if (i == num_Commands - 1) {
                dup2(fd[i - 1][0], STDIN_FILENO);
                for (int j = 0; j < num_Commands - 1; j++) {
                    close(fd[j][0]);
                    close(fd[j][1]);
                }
                if (execvp(command, arguments) == -1) {
                    perror("execvp");
                    return;
                }
            } else {
                dup2(fd[i - 1][0], STDIN_FILENO);
                dup2(fd[i][1], STDOUT_FILENO);
                for (int j = 0; j < num_Commands - 1; j++) {
                    close(fd[j][0]);
                    close(fd[j][1]);
                }
                if (execvp(command, arguments) == -1) {
                    perror("execvp");
                    return;
                }
            }
        }
    }

    for (int j = 0; j < num_Commands - 1; j++) {
        close(fd[j][0]);
        close(fd[j][1]);
    }

    time(&(cmnd_Array[cmnd_count-1].start_time));
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    for (int i = 0; i < num_Commands; i++) {
        int status;
        if (waitpid(pids[i], &status, 0) == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Child process %d exited with an error status\n", pids[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);  
    cmnd_Array[cmnd_count - 1].pid=pids[0];
    cmnd_Array[cmnd_count - 1].execution_time=(end_time.tv_sec - start_time.tv_sec) * 1000LL + (end_time.tv_nsec - start_time.tv_nsec) / 1000000LL;
    return;
}

void remove_Spaces(char* str) {
    if(str[0]==' '){
        for(int i=0;i<strlen(str)-1;i++){
            str[i]=str[i+1];
        }
    }
    if(str[strlen(str)-1]=' ' ){
        str[strlen(str)-1]='\0';
    }
}

void process_piped_commands(char* piped_Input){
    char* input_List[1024];
    int num_Commands=0;
    piped_Input[strcspn(piped_Input, "\n")] = '\0';

    char* tok=strtok(piped_Input,"|");
    while (tok != NULL) { 
        input_List[num_Commands] = tok;
        tok= strtok(NULL, "|");
        num_Commands++;
    }

    for(int i=0;i<num_Commands;i++){
        remove_Spaces(input_List[i]);
    }

    execute_piped_commands(input_List,num_Commands);
}


// to run shell-script run 'rs <filename>' (rs short for runscript)
void execute_shell_script(const char* scriptFileName) {
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    time(&(cmnd_Array[cmnd_count-1].start_time));
    char command[256];
    FILE* scriptFile = fopen(scriptFileName, "r");

    if (scriptFile == NULL) {
        perror("Error opening script file");
        return;
    }

    while (fgets(command, sizeof(command), scriptFile) != NULL) {
        command[strcspn(command, "\n")] = '\0';

        pid_t child_pid = fork();

        if (child_pid == -1) {
            perror("Error creating child process");
        } else if (child_pid == 0) { 
            int result = execl("/bin/sh", "sh", "-c", command, (char *)NULL);

            if (result == -1) {
                perror("Error executing command");
                exit(EXIT_FAILURE);
            }
        } else { 
            int status;
            if (waitpid(child_pid, &status, 0) == -1) {
                perror("Error waiting for child process");
            } else if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                printf("Command returned non-zero exit status\n");
            }
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    cmnd_Array[cmnd_count - 1].execution_time=(end_time.tv_sec - start_time.tv_sec) * 1000LL + (end_time.tv_nsec - start_time.tv_nsec) / 1000000LL;
    fclose(scriptFile);

}


void background_with_piped_creation(char* piped_Input){
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    char* input_List[1024];
    int num_Commands=0;
    piped_Input[strcspn(piped_Input, "\n")] = '\0';

    char* tok=strtok(piped_Input,"|");
    while (tok != NULL) { 
        input_List[num_Commands] = tok;
        tok= strtok(NULL, "|");
        num_Commands++;
    }

    for(int i=0;i<num_Commands;i++){
        remove_Spaces(input_List[i]);
    }

    
    int fd[num_Commands - 1][2];
    int pids[num_Commands];

    for (int i = 0; i < num_Commands - 1; i++) {
        if (pipe(fd[i]) == -1) {
            perror("pipe");
            return;
        }
    }

    for (int i = 0; i < num_Commands; i++) {
        char input[1024];
        char command[1024];
        char* arguments[1024];
        strcpy(input, input_List[i]);
        read_input_piped(input, command, arguments);
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            return;
        }
        if (pids[i] == 0) {
            if (i == 0) {
                dup2(fd[i][1], STDOUT_FILENO);
                for (int j = 0; j < num_Commands - 1; j++) {
                    close(fd[j][0]);
                    close(fd[j][1]);
                }
                if (execvp(command, arguments) == -1) {
                    perror("execvp");
                    return;
                }
            } else if (i == num_Commands - 1) {
                dup2(fd[i - 1][0], STDIN_FILENO);
                for (int j = 0; j < num_Commands - 1; j++) {
                    close(fd[j][0]);
                    close(fd[j][1]);
                }
                if (execvp(command, arguments) == -1) {
                    perror("execvp");
                    return;
                }
            } else {
                dup2(fd[i - 1][0], STDIN_FILENO);
                dup2(fd[i][1], STDOUT_FILENO);
                for (int j = 0; j < num_Commands - 1; j++) {
                    close(fd[j][0]);
                    close(fd[j][1]);
                }
                if (execvp(command, arguments) == -1) {
                    perror("execvp");
                    return;
                }
            }
        }
    }
    // time(&cmnd_Array[cmnd_count-1].start_time);
    time(&(cmnd_Array[cmnd_count-1].start_time));
    for (int j = 0; j < num_Commands - 1; j++) {
        close(fd[j][0]);
        close(fd[j][1]);
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    cmnd_Array[cmnd_count-1].execution_time=(end_time.tv_sec - start_time.tv_sec) * 1000LL + (end_time.tv_nsec - start_time.tv_nsec) / 1000000LL;
    cmnd_Array[cmnd_count - 1].pid=pids[0];
    return;

}

void shell_Loop() {
    char input[1000];
    char command[1000];
    char* arguments[1000];
    do {
        printf(">>> simpleshell $ ");
        fflush(stdout);
        int sig_recieved= read_user_input(input, sizeof(input), command, arguments);
        if(sig_recieved==1){
            print_On_Exit();
            return;
        }
        if(sig_recieved==4){
            background_with_piped_creation(input);
        }
        else if(sig_recieved==2){
            process_piped_commands(input);
        }
        else if(sig_recieved==3){
            background_process_creation(command,arguments);
        }
        else if(strcmp(command,"history")==0){
            print_History();
        }
        else if (strcmp(command, "cd") == 0) {
            if (arguments[1] != NULL) {
                cd_Func(arguments[1]);
            } 
        }
        else if (strcmp(command, "rs") == 0) {
            if (arguments[1] != NULL) {
                execute_shell_script(arguments[1]);
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
