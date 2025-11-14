#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define NUM_THREADS 4
#define NUM_POINTS 1000000

static int count = 0;
static sem_t sem;

void *WorkerThread(void *arg) {
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)pthread_self();

    for (int i = 0; i < NUM_POINTS; ++i) {
        double x = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;
        double y = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;

        double r2 = x*x + y*y;
        if (r2 <= 1.0) {
            sem_wait(&sem);
            count++;
            sem_post(&sem);
        }
    }

    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREADS];

    sem_init(&sem, 0, 1);

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_create(&threads[i], NULL, WorkerThread, NULL);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&sem);

    long total_points = (long)NUM_THREADS * (long)NUM_POINTS;
    double pi_estimate = 4.0 * ((double)count / (double)total_points);

    printf("Estimated area of the circle (pi): %.6f\n", pi_estimate);

    return 0;
}
