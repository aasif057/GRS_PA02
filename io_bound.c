#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define FILE_NAME "test_file.txt"
#define NUM_LINES 100000  // lines per thread

int NUM_THREADS; // number of threads
pthread_mutex_t lock;

// function for read and write on a single file
void *io_task(void *arg) {
    int thread_id = *(int *)arg;
    
    // lock the file so that only one thread access the file at a time for write operation
    pthread_mutex_lock(&lock);
    
    // Write to the shared file
    FILE *fp = fopen(FILE_NAME, "a");
    if (!fp) {
        perror("File open error");
        pthread_mutex_unlock(&lock);
        pthread_exit(NULL);
    }
    for (int i = 0; i < NUM_LINES; i++) {
        fprintf(fp, "Thread %d - Line %d\n", thread_id, i);
    }
    fclose(fp);

    //unlock the file
    pthread_mutex_unlock(&lock);

    // lock the file so that only one thread access the file at a time for reading
    pthread_mutex_lock(&lock);
    
    // Read from shared file
    fp = fopen(FILE_NAME, "r");
    if (!fp) {
        perror("File read error");
        pthread_mutex_unlock(&lock);
        pthread_exit(NULL);
    }
    char buffer[100];
    while (fgets(buffer, sizeof(buffer), fp)) {
        //Simulating read operation
    }
    fclose(fp);
    
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
    
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    pthread_mutex_init(&lock, NULL);
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, io_task, &thread_ids[i]);
    }
    
    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("Execution Time: %f seconds\n", elapsed);
    
    pthread_mutex_destroy(&lock);
    return 0;
}
