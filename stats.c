#include"stats.h"
#include<stdlib.h>
#include<stdio.h>
#include<semaphore.h>

typedef struct stats{
    int factoryNumber;
    int made;
    int eaten;
    double minDelay;
    double avgDelay;
    double maxDelay;
}stats_t;

int numFactories = 0;
sem_t statsmutex;
stats_t* factory_stats;


void stats_init(int num_producers){
    numFactories = num_producers;
    factory_stats = malloc(num_producers*sizeof(stats_t));
    sem_init(&statsmutex, 0, 1);
    for(int i=0; i<num_producers; i++){
        factory_stats[i].factoryNumber = i;
        factory_stats[i].made = 0;
        factory_stats[i].eaten = 0;
        factory_stats[i].minDelay = 0;
        factory_stats[i].avgDelay = 0;
        factory_stats[i].maxDelay = 0;
    }
}

void stats_cleanup(void){
    free(factory_stats);
    factory_stats = NULL;
}

void stats_record_produced(int factory_number){
    sem_wait(&statsmutex);

    factory_stats[factory_number].made++;
    sem_post(&statsmutex);
}

void stats_record_consumed(int factory_number, double delay_in_ms){
    sem_wait(&statsmutex);
    
    factory_stats[factory_number].eaten++;
    if(factory_stats[factory_number].maxDelay==0 && factory_stats[factory_number].minDelay==0){
        factory_stats[factory_number].minDelay = delay_in_ms;
        factory_stats[factory_number].avgDelay = delay_in_ms; 
        factory_stats[factory_number].maxDelay = delay_in_ms;
    }
    else{
        if(factory_stats[factory_number].minDelay > delay_in_ms){
            factory_stats[factory_number].minDelay = delay_in_ms;
        }
        int x = factory_stats[factory_number].eaten;
        double oldAvg = factory_stats[factory_number].avgDelay;
        factory_stats[factory_number].avgDelay = ((x-1)*(oldAvg) + delay_in_ms)/x;
        if(factory_stats[factory_number].maxDelay < delay_in_ms){
            factory_stats[factory_number].maxDelay = delay_in_ms;
        }
        

    }
    sem_post(&statsmutex);
}

void stats_display(void){
    printf("Statistics:\n");
    printf("%8s%10s%10s%20s%20s%20s\n", "Factory#", "#Made", "#Eaten", "Min Delay[ms]", "Avg Delay[ms]", "Max Delay[ms]");
    for(int i=0; i<numFactories; i++){
        printf("%8d%10d%10d%20.5f%20.5f%20.5f\n", 
                                                factory_stats[i].factoryNumber,
                                                factory_stats[i].made,
                                                factory_stats[i].eaten,
                                                factory_stats[i].minDelay,
                                                factory_stats[i].avgDelay,
                                                factory_stats[i].maxDelay);
        if(factory_stats[i].eaten != factory_stats[i].made){
            printf("ERROR: Mismatch between number made and eaten.\n");
        }
    }
}

