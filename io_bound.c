#define _GNU_SOURCE  // Required for O_DIRECT
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>

#define buff_SIZE 4096 

int n = 50000; //number of lines
int thread_counts;

void *rw_func(void *arg) {
    int thread_id = *(int *)arg;
    char filename[50];
    snprintf(filename, sizeof(filename), "thread_%d.txt", thread_id);

    void *buff; // Allocate aligned memory for O_DIRECT
    
    if (posix_memalign(&buff, 512, buff_SIZE)) {
        perror("Memory alignment error");
        pthread_exit(NULL);
    }
    memset(buff, 'A', buff_SIZE - 1);
    ((char *)buff)[buff_SIZE - 1] = '\n';

    // Open file with O_DIRECT to bypass cache
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_DIRECT, 0644);
    if (fd < 0) {
        perror("File open error");
        free(buff);
        pthread_exit(NULL);
    }

    // Write file in larger chunks to reduce CPU overhead
    for (int i = 0; i < n; i += (buff_SIZE / 100)) {
        if (write(fd, buff, buff_SIZE) < 0) {
            perror("Write error");
            break;
        }
        fsync(fd);
    }

    free(buff);
    close(fd);
    
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: %s <thread_counts>\n", argv[0]);
        return 1;
    }

    thread_counts = atoi(argv[1]);
    if (thread_counts <= 0) {
        printf("Invalid number of threads.\n");
        return 1;
    }

    pthread_t threads[thread_counts];
    int thread_ids[thread_counts];

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Create threads
    for (int i = 0; i < thread_counts; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, rw_func, &thread_ids[i]);
    }

    // Join threads
    for (int i = 0; i < thread_counts; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Execution Time: %f seconds\n", elapsed);

    sync();

    return 0;
}
