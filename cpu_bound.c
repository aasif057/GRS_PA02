#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

#define UPPER_RANGE 500000  // Upper limit for prime checking

int count = 0; // Shared counter for prime numbers
pthread_mutex_t lock;
int thread_count; // Number of threads

// Thread structure
typedef struct {
    int start, end;
} ThreadData;

// Function to check if a number is prime
int check_prime(int n) {
    if (n < 2) 
        return 0;
    for (int i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) 
            return 0;
    }
    return 1;
}

// Thread function to count primes in a range
void *count_primes(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int local_count = 0;
    for (int i = data->start; i < data->end; i++) {
        if (check_prime(i)) {
            local_count++;
        }
    }
    //Acquire lock before updating the global count
    pthread_mutex_lock(&lock);
    count += local_count;
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    // Error check if number of threads is not passed as an argument
    if (argc != 2) {
        printf("Usage: %s <thread_count>\n", argv[0]);
        return 1;
    }
    // Error check if number of threads is invalid
    thread_count = atoi(argv[1]);
    if (thread_count <= 0) {
        printf("Invalid number of threads.\n");
        return 1;
    }
    
    pthread_t threads[thread_count]; //create threads
    ThreadData thread_data[thread_count]; 
    int range = UPPER_RANGE / thread_count;
    pthread_mutex_init(&lock, NULL);
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Create threads
    for (int i = 0; i < thread_count; i++) {
        thread_data[i].start = i * range;
        thread_data[i].end = (i == thread_count - 1) ? UPPER_RANGE : (i + 1) * range;
        pthread_create(&threads[i], NULL, count_primes, &thread_data[i]);
    }
    
    // Join threads
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("Total prime numbers: %d\n", count);
    printf("Execution Time: %f seconds\n", elapsed);
    
    pthread_mutex_destroy(&lock);
    return 0;
}
