#define _GNU_SOURCE  // Required for O_DIRECT
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>

#define BUFFER_SIZE 4096
#define NUM_ENTRIES 10000  // 

int thread_count;
pthread_mutex_t sum_lock;  // Mutex to lock gloval sum
int total_sum = 0; 

// function for log processing simulation
void *write_logs(void *arg) {
    int thread_id = *(int *)arg;
    char f_name[50];
    snprintf(f_name, sizeof(f_name), "log_thread_%d.txt", thread_id);

    // Allocate aligned memory for O_DIRECT
    void *buff;
    if (posix_memalign(&buff, 512, BUFFER_SIZE)) {
        perror("Memory alignment error");
        pthread_exit(NULL);
    }
    memset(buff, 'A', BUFFER_SIZE - 1);
    ((char *)buff)[BUFFER_SIZE - 1] = '\n'; 

    // Open file with O_DIRECT to bypass OS cache
    int fd = open(f_name, O_WRONLY | O_CREAT | O_TRUNC | O_DIRECT, 0644);
    if (fd < 0) {
        perror("File open error");
        free(buff);
        pthread_exit(NULL);
    }

    // Simulate memory-bound task
    long long local_sum = 0;  

    for (int i = 0; i < NUM_ENTRIES; i++) {
        int a = rand() % 1000;  
        local_sum += a;  // Compute sum locally

        if (write(fd, buff, BUFFER_SIZE) < 0) {
            perror("Write error");
            break;
        }
    }

    char avg_buff[100];
    int len = snprintf(avg_buff, sizeof(avg_buff), "Thread %d - Average: %.2f\n", thread_id, local_sum / (double)NUM_ENTRIES);
    write(fd, avg_buff, len);

    fsync(fd); 
    close(fd);
    free(buff);

    pthread_mutex_lock(&sum_lock);
    total_sum += local_sum;
    pthread_mutex_unlock(&sum_lock);

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
    pthread_mutex_init(&sum_lock, NULL);

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
    // printf("Total Sum: %d\n", total_sum);

    pthread_mutex_destroy(&sum_lock);
    sync();  // Ensure all writes are flushed to disk
    return 0;
}
