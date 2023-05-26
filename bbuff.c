
#include "bbuff.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

int input_ind = 0;
int output_ind = 0;
sem_t empty;
sem_t full;
pthread_mutex_t mutex;
void *buff_array[BUFFER_SIZE];


void bbuff_init(void) {
    pthread_mutex_init(&mutex, NULL);
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
};

void bbuff_blocking_insert(void* item) {
    // wait until empty is non-zero
    sem_wait(&empty);

    // lock mutex
    pthread_mutex_lock(&mutex);

    // insert into buff, increment input index
    buff_array[input_ind] = item;
    input_ind = (input_ind + 1) % BUFFER_SIZE;

    // unlock mutex and increment full
    pthread_mutex_unlock(&mutex);
    sem_post(&full);
};

void *bbuff_blocking_extract(void) {
    // wait until full is non-zero
    sem_wait(&full);
    
    // lock mutex
    pthread_mutex_lock(&mutex);

    // extract item from buff, increment output index
    void *item = buff_array[output_ind];
    output_ind = (output_ind + 1) % BUFFER_SIZE;

    // unlock mutex and increment empty
    pthread_mutex_unlock(&mutex);
    sem_post(&empty);
    return item;
};

_Bool bbuff_is_empty(void) {
    int sem_value;

    // set sem_value to be equal to the value of semaphore 'full'
    sem_getvalue(&full, &sem_value);

    // if no full elements, buff is empty
    if(sem_value == 0) {
        return true;
    }
    else {
        return false;
    }
};