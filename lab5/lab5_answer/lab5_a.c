#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUF_SZ 5
#define SHM_NAME "/lab5_shm"

typedef struct {
    double buffer[BUF_SZ];
    int head;     // index to read from
    int tail;     // index to write to
    int count;    // number of elements currently in buffer
    sem_t mutex;  // binary semaphore for mutual exclusion
    sem_t empty;  // counts available slots
    sem_t full;   // counts available items
} shm_queue_t;

/* Compute init_value from FIRST_NAME and LAST_NAME.
 * defaults to "Hongdao" and "Meng". */
static double compute_init_value(void) {
    const char *first = getenv("FIRST_NAME");
    const char *last  = getenv("LAST_NAME");
    if (!first || !first[0]) first = "Hongdao";
    if (!last  || !last[0])  last  = "Meng";
    int a = toupper((unsigned char)first[0]);
    int b = toupper((unsigned char)last[0]);
    return (double)a + ((double)b / 10.0);
}

/* Convert string to long / double with basic validation. */
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

/* Random sleep: 0 <= seconds < 3 (integer seconds is fine for the spec). */
static void rand_sleep_under_3s(void) {
    int sec = rand() % 3;   // 0, 1, or 2
    if (sec > 0) sleep(sec);
    // If sec==0, no sleep (still satisfies 0 <= t < 3)
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

    /* Create / open shared memory (named, POSIX). */
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return EXIT_FAILURE;
    }
    if (ftruncate(shm_fd, (off_t)sizeof(shm_queue_t)) == -1) {
        perror("ftruncate");
        close(shm_fd);
        shm_unlink(SHM_NAME); // best effort
        return EXIT_FAILURE;
    }

    shm_queue_t *q = mmap(NULL, sizeof(*q),
                          PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (q == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return EXIT_FAILURE;
    }

    /* Initialize queue and semaphores (process-shared = 1). */
    q->head = q->tail = q->count = 0;
    if (sem_init(&q->mutex, 1, 1) == -1 ||
        sem_init(&q->empty, 1, BUF_SZ) == -1 ||
        sem_init(&q->full,  1, 0) == -1) {
        perror("sem_init");
        munmap(q, sizeof(*q));
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        munmap(q, sizeof(*q));
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        /* ---------------- Child: Producer ---------------- */
        /* It's safe that both processes keep shm_fd open; kernel tracks refs. */
        srand((unsigned)time(NULL) ^ (unsigned)getpid());
        for (int k = 0; k < n; ++k) {
            double value = init_value + k * d;

            /* Wait for an empty slot, then lock mutex to write one item. */
            sem_wait(&q->empty);
            sem_wait(&q->mutex);

            q->buffer[q->tail] = value;
            q->tail = (q->tail + 1) % BUF_SZ;
            q->count++;

            sem_post(&q->mutex);
            sem_post(&q->full);

            rand_sleep_under_3s();
        }
        /* Child done: simply exit. Parent will clean up. */
        _exit(0);
    }

    /* ---------------- Parent: Consumer ---------------- */
    int received = 0;
    while (received < n) {
        sem_wait(&q->full);
        sem_wait(&q->mutex);

        double value = q->buffer[q->head];
        q->head = (q->head + 1) % BUF_SZ;
        q->count--;

        sem_post(&q->mutex);
        sem_post(&q->empty);

        printf("Received: %.6f\n", value);
        fflush(stdout);
        received++;
    }

    /* Wait for child to exit, then destroy semaphores and unlink shm. */
    int status = 0;
    (void)waitpid(pid, &status, 0);

    /* Destroy semaphores (safe now: child has exited). */
    sem_destroy(&q->full);
    sem_destroy(&q->empty);
    sem_destroy(&q->mutex);

    /* Detach and remove shared memory object. */
    if (munmap(q, sizeof(*q)) == -1) perror("munmap");
    if (close(shm_fd) == -1) perror("close(shm_fd)");
    if (shm_unlink(SHM_NAME) == -1) perror("shm_unlink");

    return EXIT_SUCCESS;
}
