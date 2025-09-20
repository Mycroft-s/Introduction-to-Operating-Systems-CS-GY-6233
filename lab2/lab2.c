#define _POSIX_C_SOURCE 200809L
#include <unistd.h>     // getpid, read, write, close, sleep
#include <fcntl.h>      // open, O_RDONLY
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>       // time
#include <stdlib.h>     // srand, rand, exit
#include <string.h>     // strerror
#include <stdio.h>      // dprintf, snprintf

static ssize_t write_all(int fd, const void *buf, size_t count) {
    const char *p = (const char *)buf;
    size_t left = count;
    while (left > 0) {
        ssize_t n = write(fd, p, left);
        if (n < 0) {
            if (errno == EINTR) continue; //
            return -1;                    // 
        }
        p    += n;
        left -= n;
    }
    return (ssize_t)count;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        dprintf(STDERR_FILENO, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    // 1) 
    pid_t pid = getpid();
    dprintf(STDOUT_FILENO, "%d\n", (int)pid);

    // 2) 
    srand((unsigned)time(NULL));
    unsigned sec = (unsigned)(rand() % 5 + 1);
    sleep(sec);

    // 3) 
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        dprintf(STDERR_FILENO, "open(%s) failed: %s\n", argv[1], strerror(errno));
        return 1;
    }

    char buf[4096];
    for (;;) {
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n == 0) break;          // EOF
        if (n < 0) {
            if (errno == EINTR) continue;
            dprintf(STDERR_FILENO, "read failed: %s\n", strerror(errno));
            close(fd);
            return 1;
        }
        if (write_all(STDOUT_FILENO, buf, (size_t)n) < 0) {
            dprintf(STDERR_FILENO, "write failed: %s\n", strerror(errno));
            close(fd);
            return 1;
        }
    }

    if (close(fd) == -1) {
        dprintf(STDERR_FILENO, "close failed: %s\n", strerror(errno));
        return 1;
    }
    return 0;
}