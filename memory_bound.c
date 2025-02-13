#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define ARRAY_SIZE 100000000  // Large array size

int *array;
long long sum = 0; //global var to save sum
pthread_mutex_t lock;
int NUM_THREADS; // Dynamic number of threads

// Thread structure
typedef struct {
    int start_idx, end_idx;
} ThreadData;

//function to calculate sum
void *get_sum(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    long long local_sum = 0; // var to save the local sum on each thread
    for (int i = data->start_idx; i < data->end_idx; i++) {
        local_sum += array[i];
    }
    pthread_mutex_lock(&lock); //locking global var sum to avoid race-around cond
    sum += local_sum;
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <num_threads>\n", argv[0]);
        return 1;
    }
    
    NUM_THREADS = atoi(argv[1]);
    if (NUM_THREADS <= 0) {
        printf("Invalid number of threads.\n");
        return 1;
    }
    
    // Allocate and initialize array
    array = (int *)malloc(ARRAY_SIZE * sizeof(int));
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = rand() % 100;
    }
    
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    int chunk_size = ARRAY_SIZE / NUM_THREADS;
    pthread_mutex_init(&lock, NULL);
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i] = (ThreadData){.start_idx = i * chunk_size, .end_idx = (i + 1) * chunk_size};
        if (i == NUM_THREADS - 1) thread_data[i].end_idx = ARRAY_SIZE;
        pthread_create(&threads[i], NULL, get_sum, &thread_data[i]);
    }
    
    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("Total Sum: %lld\n", sum);
    printf("Execution Time: %f seconds\n", elapsed);
    
    pthread_mutex_destroy(&lock);
    free(array);
    return 0;
}
