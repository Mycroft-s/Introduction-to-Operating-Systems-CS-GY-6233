#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
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

int main(int argc, char *argv[])
{
    if (argc < 3) return 1;

    n = atoi(argv[1]);
    d = atof(argv[2]);
    if (n <= 1) return 1;

    printf("[Consumer] n = %d, d = %f\n", n, d);
    printf("[Consumer] Address of n = %p\n", (void*)&n);

    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) return 1;

    shared_data_t *shm_ptr = mmap(NULL, sizeof(shared_data_t),
                                  PROT_READ | PROT_WRITE,
                                  MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) return 1;

    printf("[Consumer] Shared buffer address = %p\n", (void*)shm_ptr->buffer);

    for (int i = 0; i < n; ++i) {
        while (shm_ptr->in == shm_ptr->out) {}

        double value = shm_ptr->buffer[shm_ptr->out];
        shm_ptr->out = (shm_ptr->out + 1) % BUF_SZ;

        printf("[Consumer] Consumed: %f (i=%d)\n", value, i);
        fflush(stdout);
    }

    munmap(shm_ptr, sizeof(shared_data_t));
    close(shm_fd);
    return 0;
}
