#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_SIZE 4096
int n = 100000
int thread_count;

void *write_logs(void *arg) {
    int thread_id = *(int *)arg;
    char f_name[50];
    snprintf(f_name, sizeof(f_name), "log_thread_%d.txt", thread_id);

    // Open file with O_DIRECT to ensure actual disk writes
    int fd = open(f_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("File open error");
        pthread_exit(NULL);
    }

    // Simulate memory-bound task
    char buffer[BUFFER_SIZE];
    int offset = 0;

    for (int i = 0; i < n; i++) {
        int a = rand() % 1000;  // Generate random number
        total_sum += a;  // Compute sum
        int len = snprintf(buffer + offset, BUFFER_SIZE - offset, "Thread %d - Value %d\n", thread_id, a);
        offset += len;

        // If buffer is full, write to file
        if (offset >= BUFFER_SIZE - 100) {
            write(fd, buffer, offset);
            offset = 0;  // Reset buffer
        }
    }

    // Compute final average and write to file
    double avg = total_sum / (double)n;
    offset += snprintf(buffer + offset, BUFFER_SIZE - offset, "Thread %d - Average: %.2f\n", thread_id, avg);
    write(fd, buffer, offset);
    
    fsync(fd);  // Force write to disk
    close(fd);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <thread_count>\n", argv[0]);
        return 1;
    }

    thread_count = atoi(argv[1]);
    if (thread_count <= 0) {
        printf("Invalid aber of threads.\n");
        return 1;
    }

    pthread_t threads[thread_count];
    int thread_ids[thread_count];

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Create threads
    for (int i = 0; i < thread_count; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, write_logs, &thread_ids[i]);
    }

    // Join threads
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Execution Time: %f seconds\n", elapsed);
    return 0;
}
