#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

#define UPPER_RANGE 500000  // Upper limit for prime checking

int prime_count = 0; // Shared counter for prime numbers
pthread_mutex_t lock;
int NUM_THREADS; // Number of threads

// Thread structure
typedef struct {
    int start, end;
} ThreadData;

// Function to check if a number is prime
int check_prime(int num) {
    if (num < 2) return 0;
    for (int i = 2; i <= sqrt(num); i++) {
        if (num % i == 0) return 0;
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
    //Acquire lock before updating the global prime_count
    pthread_mutex_lock(&lock);
    prime_count += local_count;
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    // Error check if number of threads is not passed as an argument
    if (argc != 2) {
        printf("Usage: %s <num_threads>\n", argv[0]);
        return 1;
    }
    // Error check if number of threads is invalid
    NUM_THREADS = atoi(argv[1]);
    if (NUM_THREADS <= 0) {
        printf("Invalid number of threads.\n");
        return 1;
    }
    
    pthread_t threads[NUM_THREADS]; //create threads
    ThreadData thread_data[NUM_THREADS]; 
    int range = UPPER_RANGE / NUM_THREADS;
    pthread_mutex_init(&lock, NULL);
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].start = i * range;
        thread_data[i].end = (i == NUM_THREADS - 1) ? UPPER_RANGE : (i + 1) * range;
        pthread_create(&threads[i], NULL, count_primes, &thread_data[i]);
    }
    
    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("Total prime numbers: %d\n", prime_count);
    printf("Execution Time: %f seconds\n", elapsed);
    
    pthread_mutex_destroy(&lock);
    return 0;
}
