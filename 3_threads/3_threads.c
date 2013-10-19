#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>

const int ones_time = 2;
const int twos_time = 4;

ucontext_t ones_context, twos_context;


void switch_to_twos(int sig);


void switch_to_ones(int sig) {
    signal(SIGALRM, switch_to_twos);
    alarm(ones_time);
    swapcontext(&twos_context, &ones_context);
}

void switch_to_twos(int sig) {
    signal(SIGALRM, switch_to_ones);
    alarm(twos_time);
    swapcontext(&ones_context, &twos_context);
}


void print_ones(void) { while (1) printf("1"); }

void print_twos(void) { while (1) printf("2"); }


void init_thread_context(ucontext_t *context, void (*thread_func)(void))
{
    getcontext(context);
    context->uc_stack.ss_sp = malloc(SIGSTKSZ);
    context->uc_stack.ss_size = SIGSTKSZ;
    makecontext(context, thread_func, 0);
}


int main(void)
{
    init_thread_context(&ones_context, print_ones);
    init_thread_context(&twos_context, print_twos);

    signal(SIGALRM, switch_to_twos);
    alarm(ones_time);
    setcontext(&ones_context);
    
    return EXIT_SUCCESS;
}
