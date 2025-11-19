#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

int main(void)
{
    int fd;
    char buf[256];
    ssize_t n;
    struct timeval t1, t2;
    long user_ms;
    unsigned long k_ms = 0;
    int k_ppid = -1;
    pid_t u_ppid;

    gettimeofday(&t1, NULL);

    fd = open("/dev/lab7dev", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    n = read(fd, buf, sizeof(buf) - 1);
    if (n < 0) {
        perror("read");
        close(fd);
        return 1;
    }
    buf[n] = '\0';

    gettimeofday(&t2, NULL);
    close(fd);

    user_ms = (t2.tv_sec - t1.tv_sec) * 1000L +
              (t2.tv_usec - t1.tv_usec) / 1000L;

    sscanf(buf, "elapsed: %lu ms\nppid: %d", &k_ms, &k_ppid);
    u_ppid = getppid();

    printf("=== Kernel Output ===\n%s\n", buf);
    printf("Kernel elapsed: %lu ms\n", k_ms);
    printf("Kernel ppid: %d\n", k_ppid);
    printf("User measured elapsed: %ld ms\n", user_ms);
    printf("User getppid(): %d\n", u_ppid);

    return 0;
}
