#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int create_process_tree(void) {
    pid_t pid;
    pid = fork();
    if (pid == 0)
        return 1;
    else if (pid < 0)
        return -1;
    pid = fork();
    if (pid == 0) {
        pid = fork();
        if (pid == 0)
            return 3;
        else if (pid > 0)
            return 2;
        else
            return -1;
    } else if (pid > 0)
        return 0;
    else
        return -1;
}

int main(void) {
    int who = create_process_tree();
    printf("ret=%d, PID=%d, PPID=%d\n", who, getpid(), getppid());
    return 0;
}
