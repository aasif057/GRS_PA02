#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_ITEMS 1000

typedef struct {
    int buffer[NUM_ITEMS];
    int count;
    int done;
    pthread_mutex_t mtx;
    pthread_cond_t cv;
} SharedBuffer;

void* producer(void* arg) {
    SharedBuffer* sharedBuffer = (SharedBuffer*)arg;
    srand(time(NULL));

    for (int i = 0; i < NUM_ITEMS; ++i) {
        int num = rand() % 100 + 1;
        pthread_mutex_lock(&sharedBuffer->mtx);
        sharedBuffer->buffer[sharedBuffer->count++] = num;
        pthread_mutex_unlock(&sharedBuffer->mtx);
        pthread_cond_signal(&sharedBuffer->cv);
        usleep(10000);
    }

    pthread_mutex_lock(&sharedBuffer->mtx);
    sharedBuffer->done = 1;
    pthread_mutex_unlock(&sharedBuffer->mtx);
    pthread_cond_signal(&sharedBuffer->cv);
    return NULL;
}

void* consumer(void* arg) {
    SharedBuffer* sharedBuffer = (SharedBuffer*)arg;
    int sum = 0, count = 0;

    while (1) {
        pthread_mutex_lock(&sharedBuffer->mtx);
        while (sharedBuffer->count == 0 && !sharedBuffer->done) {
            pthread_cond_wait(&sharedBuffer->cv, &sharedBuffer->mtx);
        }
        while (sharedBuffer->count > 0) {
            sum += sharedBuffer->buffer[--sharedBuffer->count];
            count++;
        }
        if (sharedBuffer->done && sharedBuffer->count == 0) {
            pthread_mutex_unlock(&sharedBuffer->mtx);
            break;
        }
        pthread_mutex_unlock(&sharedBuffer->mtx);
    }
    printf("Consumer processed %d items. Average: %.2f\n", count, count ? (double)sum / count : 0);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <num_threads>\n", argv[0]);
        return 1;
    }

    int num_pairs = atoi(argv[1]);
    if (num_pairs <= 0) {
        printf("Please enter a positive number of threads.\n");
        return 1;
    }

    clock_t start_time = clock();
    pthread_t producers[num_pairs], consumers[num_pairs];
    SharedBuffer buffers[num_pairs];

    for (int i = 0; i < num_pairs; ++i) {
        buffers[i].count = 0;
        buffers[i].done = 0;
        pthread_mutex_init(&buffers[i].mtx, NULL);
        pthread_cond_init(&buffers[i].cv, NULL);
        pthread_create(&producers[i], NULL, producer, &buffers[i]);
        pthread_create(&consumers[i], NULL, consumer, &buffers[i]);
    }

    for (int i = 0; i < num_pairs; ++i) {
        pthread_join(producers[i], NULL);
        pthread_join(consumers[i], NULL);
        pthread_mutex_destroy(&buffers[i].mtx);
        pthread_cond_destroy(&buffers[i].cv);
    }

    clock_t end_time = clock();
    double elapsed = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Execution time: %.2f seconds\n", elapsed);
    return 0;
}