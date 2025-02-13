#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define ARR_SIZE 100000000  // Large arr size

int *arr;
long long sum = 0; //global var to save sum
pthread_mutex_t lock;
int thread_count; // Dynamic number of threads

// Thread structure
typedef struct {
    int start, end;
} ThreadData;

//function to calculate sum
void *get_sum(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    long long local_sum = 0; // var to save the local sum on each thread
    for (int i = data->start; i < data->end; i++) {
        local_sum += arr[i];
    }
    pthread_mutex_lock(&lock); //locking global var sum to avoid race-around cond
    sum += local_sum;
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <thread_count>\n", argv[0]);
        return 1;
    }
    
    thread_count = atoi(argv[1]);
    if (thread_count <= 0) {
        printf("Invalid number of threads.\n");
        return 1;
    }
    
    // Allocate and initialize arr
    arr = (int *)malloc(ARR_SIZE * sizeof(int));
    srand(time(NULL));
    for (int i = 0; i < ARR_SIZE; i++) {
        arr[i] = rand() % 100;
    }
    
    pthread_t threads[thread_count];
    ThreadData thread_data[thread_count];
    int chunk_size = ARR_SIZE / thread_count;
    pthread_mutex_init(&lock, NULL);
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Create threads
    for (int i = 0; i < thread_count; i++) {
        thread_data[i] = (ThreadData){.start = i * chunk_size, .end = (i + 1) * chunk_size};
        if (i == thread_count - 1) thread_data[i].end = ARR_SIZE;
        pthread_create(&threads[i], NULL, get_sum, &thread_data[i]);
    }
    
    // Join threads
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("Total Sum: %lld\n", sum);
    printf("Execution Time: %f seconds\n", elapsed);
    
    pthread_mutex_destroy(&lock);
    free(arr);
    return 0;
}
