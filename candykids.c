#include "candykids.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <stdbool.h>
#include "stats.h"
#include "bbuff.h"



int count = 0;
_Bool signal = true;


double current_time_in_ms(void) {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return now.tv_sec * 1000.0 + now.tv_nsec/1000000.0;
}

void* factory(void* args) {
  int arg = *((int*)args);
    //loop until main() signals to exit
    while(signal) {
      
        //Pick a number of seconds which it will (later wait)
        int waitTime = rand() % 4;
        //print a message exactly as: "Factory 0 ships candy & waits 2s"
        printf("Factory %i ships candy & waits %is\n", arg, waitTime);
        //Dynamically allocate a new candy item and populate its fields
        candy_t *candy = malloc(sizeof(candy_t));
        candy->factory_number = arg;
        candy->creation_ts_ms = current_time_in_ms();
        //Add the candy item to the bounded buffer
        bbuff_blocking_insert(candy);
        stats_record_produced(arg);
        //sleep for the number of seconds in the first step
        sleep(waitTime);
    }
    //when the thread finishes, print a message
    printf("Candy-factory %d done\n", arg);
    return NULL;
}

void* kid(void* args) {
    while(true){
      //Extract a candy item from the bounded buffer
      candy_t *processedCandy = bbuff_blocking_extract();
     // printf("factory_number = %d\n", processedCandy->factory_number);
      double lifetime = current_time_in_ms() - processedCandy->creation_ts_ms;
      //printf("lifetime = %f\n", lifetime);
      stats_record_consumed(processedCandy->factory_number, lifetime);
      // process the item
      // printf("Candy from factory %i removed from buffer\n",processedCandy->factory_number);
      //sleep for 0-1 seconds
      int waitTime = rand() % 2;
      sleep(waitTime);
      free(processedCandy);
    }
}


int main(int argc, char *argv[]) {
  // 1.  Extract arguments
  if(argc!=4){
    printf("Invalid Arguments\n");
    exit(1);
  }
  int factoryCount = atoi(argv[1]);
  int kidCount = atoi(argv[2]);
  int seconds = atoi(argv[3]);
  if(factoryCount<=0 || kidCount<=0 || seconds<=0){
    printf("Invalid Arguments\n");
    exit(1);
  }
  // 2.  Initialization of modules
    stats_init(factoryCount);
    bbuff_init();
    pthread_t tid[factoryCount];
    pthread_t ktid[kidCount]; 
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    int factoryNumber[factoryCount];

  // 3.  Launch candy-factory threads
    for(int i = 0; i< factoryCount; i++) {
      //factoryNumber++;
      factoryNumber[i] = i;
      int success;
      success = pthread_create(&tid[i], NULL, factory, &factoryNumber[i]);
      if(success!=0){
        printf("Error creating thread %d\n", i);
      }
      //factoryNumber++;
    }

  // 4.  Launch kid threads
    for(int i = 0; i< kidCount; i++) {
      pthread_create(&ktid[i], NULL, kid, NULL);
    }

  // 5.  Wait for requested time
  for(int i=0; i<seconds; i++){
    sleep(1);
    printf("Time %ds\n", i);
  }

  // 6.  Stop candy-factory threads
  signal = false;
  for(int i = 0; i< factoryCount; i++){
    //pthread_cancel(tid[i]);
    pthread_join(tid[i], NULL);
  }
  // 7.  Wait until there is no more candies
  while(!bbuff_is_empty()){
    printf("Waiting for all candies to be consumed");
    sleep(1);
  }

  // 8.  Stop kid threads
  for(int i = 0; i< kidCount; i++){
    pthread_cancel(ktid[i]);
    pthread_join(ktid[i], NULL);
  }

  // 9.  Print statistics
  stats_display();

  // 10. Cleanup any allocated memory
  stats_cleanup();

  return 0;
}