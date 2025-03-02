#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

int BUFFER_SIZE;
int thread_pairs; 

//Shared buffer structiure
typedef struct {
    int *buffer;  
    int curr_size;
    pthread_mutex_t mut;
    pthread_cond_t not_empty, not_full;
} SharedBuff;

//Array for buffer
SharedBuff *buffers; 

//Func to get current time
double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
}

//Func for producer thread
void *producer(void *arg) {
    int id = *(int *)arg;

    for (int i = 0; i < 100; i++) { 
        int num = rand() %1000;  //producer thread generate a random number
        //lock the buffer for producer
        pthread_mutex_lock(&buffers[id].mut);

        // If buffer is full then wait
        while (buffers[id].curr_size == BUFFER_SIZE) {
            pthread_cond_wait(&buffers[id].not_full, &buffers[id].mut);
        }

        // add the generated random number in the buffer
        buffers[id].buffer[buffers[id].curr_size++] = num;

        // cond var to signal the consumer that buffer is not empty
        pthread_cond_signal(&buffers[id].not_empty);
        
        //unlock the buffer
        pthread_mutex_unlock(&buffers[id].mut);
    }
    return NULL;
}

// Func for consumer
void *consumer(void *arg) {
    int id = *(int *)arg;

    for (int i = 0; i < 100; i++) { 
        //lock the buffer before consuming
        pthread_mutex_lock(&buffers[id].mut);

        //if the buffer is empty then wait
        while (buffers[id].curr_size == 0) 
            pthread_cond_wait(&buffers[id].not_empty, &buffers[id].mut);

        //remove the val from the buffer and transform it by squaring
        int val = buffers[id].buffer[--buffers[id].curr_size]; 
        int squared_no = val * val;

        //cond var to signal the producer
        pthread_cond_signal(&buffers[id].not_full);

        //unlock the buffer
        pthread_mutex_unlock(&buffers[id].mut);

        // printf("consumer %d processed val: %d -> %d\n", id, val, squared_no);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s [thread_pairs] [Buffer_Size]\n", argv[0]);
        return 1;
    }

    //read number of prod-cons thread pair
    thread_pairs = atoi(argv[1]);

    // read size for buffer
    BUFFER_SIZE = atoi(argv[2]); 

    //define producer adn consumer threads
    pthread_t producers[thread_pairs], consumers[thread_pairs];
    int ids[thread_pairs];

    // Allocate buffers
    buffers = malloc(thread_pairs * sizeof(SharedBuff));
    for (int i = 0; i < thread_pairs; i++) {
        buffers[i].buffer = malloc(BUFFER_SIZE * sizeof(int));
        buffers[i].curr_size = 0;
        //initialize mutex and cond var
        pthread_mutex_init(&buffers[i].mut, NULL);
        pthread_cond_init(&buffers[i].not_empty, NULL);
        pthread_cond_init(&buffers[i].not_full, NULL);
    }

    //start timer for computing time
    double start_time = get_time();

    // intialize producer and consumer threads
    for (int i = 0; i < thread_pairs; i++) {
        ids[i] = i;
        pthread_create(&producers[i], NULL, producer, &ids[i]);
        pthread_create(&consumers[i], NULL, consumer, &ids[i]);
    }

    // Join threads
    for (int i = 0; i < thread_pairs; i++) {
        pthread_join(producers[i], NULL);
        pthread_join(consumers[i], NULL);
    }

    double end_time = get_time(); // End timer
    double total_time = end_time - start_time;
    double throughput = (N * 100) / total_time;

    printf("\nExecution Time: %.6f seconds\n", total_time);
    printf("Throughput: %.2f items/second\n", throughput);

    //free the memory
    for (int i = 0; i < N; i++) {
        free(buffers[i].buffer);
    }
    free(buffers);

    return 0;
}
