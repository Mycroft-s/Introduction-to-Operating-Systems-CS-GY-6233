#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s n d\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]);
    int d = atoi(argv[2]);
    if (n <= 0) {
        fprintf(stderr, "n must be > 0\n");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    } else if (pid == 0) {
        for (int k = 0; k < n; ++k) {
            if (k) printf(" ");
            printf("%d", k * d);
        }
        printf("\n");
        _exit(0);
    } else {
        wait(NULL);
        printf("%d %d\n", n * d, (n + 1) * d);
    }
    return 0;
}
