#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define FILE_NAME "test_file.txt"
#define LINES_COUNT 10000  // Total lines to write

int thread_count;
pthread_mutex_t lock;

// Write Task
void *write_func(void *arg) {
    int thread_id = *(int *)arg;

    pthread_mutex_lock(&lock);
    FILE *fp = fopen(FILE_NAME, "a");
    if (!fp) {
        perror("File open error");
        pthread_mutex_unlock(&lock);
        pthread_exit(NULL);
    }
    for (int i = 0; i < (LINES_COUNT / thread_count); i++) {
        fprintf(fp, "Thread %d - Line %d\n", thread_id, i);
    }
    fclose(fp);
    pthread_mutex_unlock(&lock);

    pthread_exit(NULL);
}

// Read Task 
void *read_func(void *arg) {
    int thread_id = *(int *)arg;

    FILE *fp = fopen(FILE_NAME, "r");
    if (!fp) {
        perror("File read error");
        pthread_exit(NULL);
    }
    char buffer[100];
    while (fgets(buffer, sizeof(buffer), fp)) {
        // Simulating read operation
    }
    fclose(fp);

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

    pthread_t threads[thread_count];
    int thread_ids[thread_count];
    pthread_mutex_init(&lock, NULL);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Writing Phase (Parallel)
    for (int i = 0; i < thread_count; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, write_func, &thread_ids[i]);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    // Reading Phase (Parallel)
    for (int i = 0; i < thread_count; i++) {
        pthread_create(&threads[i], NULL, read_func, &thread_ids[i]);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Execution Time: %f seconds\n", elapsed);

    pthread_mutex_destroy(&lock);
    return 0;
}
