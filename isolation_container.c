#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/wait.h>

int shared_value = 1;

// C helper function to call the pivot_root system call directly
int pivot_root(const char *new_root, const char *put_old) {
    return syscall(SYS_pivot_root, new_root, put_old);
}

int child_func(void *arg)
{
    char *new_root = (char *)arg;
    char put_old[] = "old_root";
    char put_old_path[512];

    if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) == -1) {
        perror("mount MS_PRIVATE");
        return EXIT_FAILURE;
    }
    if (mount(new_root, new_root, NULL, MS_BIND | MS_REC, NULL) == -1) {
        perror("mount MS_BIND");
        return EXIT_FAILURE;
    }
    snprintf(put_old_path, sizeof(put_old_path), "%s/%s", new_root, put_old);
    mkdir(put_old_path, 0777);

    if (chdir(new_root) == -1) {
        perror("chdir new_root");
        return EXIT_FAILURE;
    }

    if (pivot_root(".", put_old) == -1) {
        perror("pivot_root");
        return EXIT_FAILURE;
    }

    if (chdir("/") == -1) {
        perror("chdir /");
        return EXIT_FAILURE;
    }

    if (umount2(put_old, MNT_DETACH) == -1) {
        perror("umount2 old_root");
        return EXIT_FAILURE;
    }

    rmdir(put_old);


    char *child_args[] = { "/bin/sh", NULL };
    execv(child_args[0], child_args);

    
    perror("execv");
    return EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_new_root>\n", argv[0]);
        return EXIT_FAILURE;
    }

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

    // Call clone passing argv[1] as the argument pointer
    pid_t cpid = clone(child_func, stack_top, CLONE_NEWPID | SIGCHLD | CLONE_NEWNS, argv[1]);

    if (cpid == -1)
    {
        perror("clone");
        free(stack);
        return EXIT_FAILURE;
    }

    int status;
    waitpid(cpid, &status, 0);
    printf("[Parent] Child PID: %d, shared_value: %d\n", cpid, shared_value);

    free(stack);
    return EXIT_SUCCESS;
}
