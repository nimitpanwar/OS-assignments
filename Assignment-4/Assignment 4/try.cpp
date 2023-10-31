// This code installs a custom signal handler for the SIGSEGV signal
// (segmentation fault) and then purposefully creates a segmentation
// fault. The custom handler `handler` is then entered, which now
// increases the instruction pointer by 1, skipping the current byte
// of the faulty instruction. This is done for as long as the faulty
// instruction is still active; in the below case, that's 2 bytes.

// Note: This is for 64 bit systems. If you prefer 32 bit, change
// `REG_RIP` to `REG_EIP`. I didn't bother putting an appropriate
// `#ifdef` here.


// Not valid this is CPP code

#include <iostream>
using namespace std;

#include <string.h>
#include <signal.h>

int flag=0;

void handler(int nSignum, siginfo_t* si, void* vcontext) {
//   std::cout << "Segmentation fault" << std::endl;
  flag=1;
  ucontext_t* context = (ucontext_t*)vcontext;
  context->uc_mcontext.gregs[REG_RIP]++;
}


int main() {
  std::cout << "Start" << std::endl;
  
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_flags = SA_SIGINFO;
  action.sa_sigaction = handler;
  sigaction(SIGSEGV, &action, NULL);
  
  int* x = 0;
  int y = *x;
    
  cout<<flag<<endl;
  std::cout << "End" << std::endl;
  
  return 0;
}