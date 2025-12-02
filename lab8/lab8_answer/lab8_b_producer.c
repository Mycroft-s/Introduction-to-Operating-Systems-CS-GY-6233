#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define SHM_NAME "/lab8_shm"
#define BUF_SZ 5

typedef struct {
    double buffer[BUF_SZ];
    volatile int in;
    volatile int out;
} shared_data_t;

int n = 1;
double d = 1.0;

static double compute_init_value(void) {
    char first = 'H';
    char last  = 'M';
    return (double)first + (double)last / 10.0;
}

int main(int argc, char *argv[])
{
    if (argc < 3) return 1;

    n = atoi(argv[1]);
    d = atof(argv[2]);
    if (n <= 1) return 1;

    double init_value = compute_init_value();
    printf("[Producer] n = %d, d = %f, init_value = %f\n", n, d, init_value);
    printf("[Producer] Address of n = %p\n", (void*)&n);

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) return 1;

    if (ftruncate(shm_fd, sizeof(shared_data_t)) == -1) return 1;

    shared_data_t *shm_ptr = mmap(NULL, sizeof(shared_data_t),
                                  PROT_READ | PROT_WRITE,
                                  MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) return 1;

    printf("[Producer] Shared buffer address = %p\n", (void*)shm_ptr->buffer);

    shm_ptr->in  = 0;
    shm_ptr->out = 0;

    srand((unsigned int)(time(NULL) ^ getpid()));

    for (int k = 0; k < n; ++k) {
        double value = init_value + k * d;

        while (((shm_ptr->in + 1) % BUF_SZ) == shm_ptr->out) {}

        shm_ptr->buffer[shm_ptr->in] = value;
        shm_ptr->in = (shm_ptr->in + 1) % BUF_SZ;

        printf("[Producer] Produced: %f (k=%d)\n", value, k);
        fflush(stdout);

        int sleep_time = rand() % 3;
        sleep(sleep_time);
    }

    munmap(shm_ptr, sizeof(shared_data_t));
    close(shm_fd);
    return 0;
}
