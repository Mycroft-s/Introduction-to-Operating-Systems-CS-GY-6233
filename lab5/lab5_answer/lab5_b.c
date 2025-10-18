#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

static double compute_init_value(void) {
    const char *first = getenv("FIRST_NAME");
    const char *last  = getenv("LAST_NAME");
    if (!first || !first[0]) first = "Hongdao";
    if (!last  || !last[0])  last  = "Meng";
    int a = toupper((unsigned char)first[0]);
    int b = toupper((unsigned char)last[0]);
    return (double)a + ((double)b / 10.0);
}

static bool parse_int(const char *s, int *out) {
    char *end = NULL;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (errno || !end || *end != '\0') return false;
    if (v < INT32_MIN || v > INT32_MAX) return false;
    *out = (int)v;
    return true;
}

static bool parse_double(const char *s, double *out) {
    char *end = NULL;
    errno = 0;
    double v = strtod(s, &end);
    if (errno || !end || *end != '\0') return false;
    *out = v;
    return true;
}

static void rand_sleep_under_3s(void) {
    int sec = rand() % 3;   // 0, 1, or 2
    if (sec > 0) sleep(sec);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s n d\n", argv[0]);
        fprintf(stderr, "  n: integer, > 1\n");
        fprintf(stderr, "  d: double (common difference)\n");
        return EXIT_FAILURE;
    }
    int n;
    double d;
    if (!parse_int(argv[1], &n) || n <= 1) {
        fprintf(stderr, "Error: n must be an integer > 1\n");
        return EXIT_FAILURE;
    }
    if (!parse_double(argv[2], &d)) {
        fprintf(stderr, "Error: d must be a valid double\n");
        return EXIT_FAILURE;
    }

    double init_value = compute_init_value();

    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(fd[0]); close(fd[1]);
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        /* ---------------- Child: Producer (write end) ---------------- */
        close(fd[0]); // close read end
        srand((unsigned)time(NULL) ^ (unsigned)getpid());

        for (int k = 0; k < n; ++k) {
            double value = init_value + k * d;
            ssize_t w = write(fd[1], &value, sizeof(value));
            if (w != (ssize_t)sizeof(value)) {
                perror("write");
                break;
            }
            rand_sleep_under_3s();
        }
        close(fd[1]);
        _exit(0);
    }

    /* ---------------- Parent: Consumer (read end) ---------------- */
    close(fd[1]); // close write end

    double value;
    ssize_t r;
    while ((r = read(fd[0], &value, sizeof(value))) > 0) {
        if (r == (ssize_t)sizeof(value)) {
            printf("Received: %.6f\n", value);
            fflush(stdout);
        } else {
            /* Partial read of a double is unexpected; treat as error. */
            fprintf(stderr, "Error: partial read (%zd bytes)\n", r);
            break;
        }
    }
    if (r == -1) perror("read");

    close(fd[0]);
    int status = 0;
    (void)waitpid(pid, &status, 0);

    return EXIT_SUCCESS;
}
