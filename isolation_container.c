#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>

int shared_value = 1;

int child_func(void *arg)
{
    shared_value = 10;
    //should print 1 because the child process is isolated 
    printf("[Child] PID: %d\n", getpid());
    return 0;
}

int main(void)
{
    // Allocating a 4 Mb stack
    const int stack_size = 1024 * 1024 * 4;
    void *stack = malloc(stack_size);
    if (!stack)
    {
        perror("malloc");
        return EXIT_FAILURE;
    }
    // Stacks grow downwards, so point to the top
    void *stack_top = stack + stack_size;

    // Call clone sharing virtual memory (CLONE_VM) or a standard process with SIGCHLD
    pid_t cpid = clone(child_func, stack_top, CLONE_NEWPID | SIGCHLD | CLONE_NEWNS, NULL);
    if (cpid == -1)
    {
        perror("clone");
        free(stack);
        return EXIT_FAILURE;
    }

    int status;
    waitpid(cpid, &status, 0);
    //the pid printing should be valid because it is called by parent 
    printf("[Parent] Child PID: %d, shared_value: %d\n", cpid, shared_value);

    free(stack);
    return EXIT_SUCCESS;
}
