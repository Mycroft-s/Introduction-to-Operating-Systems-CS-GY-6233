#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define SERVER_PORT 50000
#define SERVER_ADDR "127.0.0.1"

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
    int sec = rand() % 3;
    if (sec > 0) sleep(sec);
}

static void random_sleep_upto_6999ms(void) {
    int ms = rand() % 7000;
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
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

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    int optval = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        perror("setsockopt");
        close(listen_fd);
        return EXIT_FAILURE;
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(SERVER_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        close(listen_fd);
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(listen_fd);
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        /* Child: client / producer */
        srand((unsigned)time(NULL) ^ (unsigned)getpid());

        int sockfd;
        struct sockaddr_in srv;
        memset(&srv, 0, sizeof(srv));
        srv.sin_family = AF_INET;
        srv.sin_port   = htons(SERVER_PORT);
        if (inet_pton(AF_INET, SERVER_ADDR, &srv.sin_addr) != 1) {
            fprintf(stderr, "inet_pton failed\n");
            _exit(EXIT_FAILURE);
        }

        for (;;) {
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                perror("socket(client)");
                _exit(EXIT_FAILURE);
            }

            if (connect(sockfd, (struct sockaddr *)&srv, sizeof(srv)) == 0) {
                break;
            }

            int err = errno;
            close(sockfd);
            if (err != ECONNREFUSED && err != ENETUNREACH &&
                err != ETIMEDOUT && err != EHOSTUNREACH) {
                errno = err;
                perror("connect");
                _exit(EXIT_FAILURE);
            }
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = 100000000L; 
            nanosleep(&ts, NULL);

        }

        for (int k = 0; k < n; ++k) {
            double value = init_value + k * d;
            ssize_t w = write(sockfd, &value, sizeof(value));
            if (w != (ssize_t)sizeof(value)) {
                perror("write");
                break;
            }
            rand_sleep_under_3s();
        }

        close(sockfd);
        _exit(EXIT_SUCCESS);
    }

    /* Parent: server / consumer */
    srand((unsigned)time(NULL) ^ (unsigned)getpid());
    random_sleep_upto_6999ms();

    if (listen(listen_fd, 1) == -1) {
        perror("listen");
        close(listen_fd);
        (void)waitpid(pid, NULL, 0);
        return EXIT_FAILURE;
    }

    int conn_fd = accept(listen_fd, NULL, NULL);
    if (conn_fd == -1) {
        perror("accept");
        close(listen_fd);
        (void)waitpid(pid, NULL, 0);
        return EXIT_FAILURE;
    }

    close(listen_fd);

    double value;
    ssize_t r;
    while ((r = read(conn_fd, &value, sizeof(value))) > 0) {
        if (r == (ssize_t)sizeof(value)) {
            printf("Received: %.6f\n", value);
            fflush(stdout);
        } else {
            fprintf(stderr, "Error: partial read (%zd bytes)\n", r);
            break;
        }
    }
    if (r == -1) {
        perror("read");
    }

    close(conn_fd);
    (void)waitpid(pid, NULL, 0);

    return EXIT_SUCCESS;
}
