#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>    
#include <sys/wait.h>  
#include <string.h>
#include <time.h>




#define READY 0
#define RUNNING 1

typedef struct Process{
    char name[100];
    int ID;
    int state;
    int flag;
    float time;
}Process;

Process process_table[100];
int front=-1;
int rear=-1;

int filled=0;

void unlock(){}
void lock(){}

// make a queue

Process dequeue (Process process_table){

}

Process enqueue (Process process_table){
    
}

void swtch(int i ,Process p){

}

void scheduler() {
    while(1) {
        lock(process_table);
        for (int i=0;i<filled;i++) {
            if(process_table[i].state != READY) {
                continue;
            }
            process_table[i].state = RUNNING;
            unlock(process_table);
            // swtch(scheduler_process, process_table[i]);
            swtch(1, process_table[i]);
            // p is done for now..
            lock(process_table);
        }
        unlock(process_table);
    }
}

// lock the process_table
// dequeue a process which is in ready state
// change its state to running state 
// unlock process_table and context switch to the new process from the old one
// lock the process table 



void setup(){}
void init_all_cpus(){}
void start_init_process(){}

// void launch(int n, void* scheduler){}                          

void kernel_main() {
    setup();
    init_all_cpus();
    start_init_process(); // on CPU-0
    for(int cpu=1;cpu<NCPU;cpu++){
        // launch(cpu, scheduler);
    }
    scheduler(); // on CPU-0
}
