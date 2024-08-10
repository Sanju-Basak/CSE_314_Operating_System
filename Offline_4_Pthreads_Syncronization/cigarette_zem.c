#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include "zemaphore.h"

#define NUM_OF_ITERATIONS 10
#define AGENT 0
#define SMOKER_HAS_MATCH 1 // has match
#define SMOKER_HAS_PAPER 2 // has paper
#define SMOKER_HAS_TOBACCO 3 // has tobacco
#define MATCH 4 // produced match
#define PAPER 5 // produced paper
#define TOBACCO 6 // produced tobacco

zem_t zem_array[7];
int is_match, is_paper, is_tobacco, finished = 0;
zem_t lock;

void *Agent(void *arg){
    // int id = *((int *)arg);
    for(int iter = 0; iter < NUM_OF_ITERATIONS; iter++)
    {
        zem_down(&zem_array[AGENT]);
        int random = rand() % 3;
        switch(random)
        {
            case 0: // tobacco and paper
                printf("Agent is placing tobacco and paper on the table\n");
                zem_up(&zem_array[TOBACCO]);
                zem_up(&zem_array[PAPER]);
                // zem_down(&zem_array[AGENT]);
                break;
            case 1: // tobacco and match
                printf("Agent is placing tobacco and match on the table\n");
                zem_up(&zem_array[TOBACCO]);
                zem_up(&zem_array[MATCH]);
                // zem_down(&zem_array[AGENT]);
                break;
            case 2: // match and paper
                printf("Agent is placing match and paper on the table\n");
                zem_up(&zem_array[MATCH]);
                zem_up(&zem_array[PAPER]);
                // zem_down(&zem_array[AGENT]);
                break;
        }
    }
    // all threads sleeping at the moment
    zem_down(&zem_array[AGENT]);
    finished = 1;
    for (int i = SMOKER_HAS_MATCH; i < (TOBACCO + 1); i++)
        zem_up(&zem_array[i]);   
    return NULL;
}

void *PusherMatch(void *arg){
    while(1) {
        zem_down(&zem_array[MATCH]);
        if (finished) break;
        zem_down(&lock); // locking mutex
        if (is_tobacco) {
            is_tobacco = 0;
            zem_up(&zem_array[SMOKER_HAS_PAPER]);
        }
        else if (is_paper) {
            is_paper = 0;
            zem_up(&zem_array[SMOKER_HAS_TOBACCO]);
        }
        else is_match = 1;
        zem_up(&lock);
    }
    return NULL;
}

void *PusherPaper(void *arg){
    while(1) {
        zem_down(&zem_array[PAPER]);
        if (finished) break;
        zem_down(&lock); // locking mutex
        if (is_tobacco) {
            is_tobacco = 0;
            zem_up(&zem_array[SMOKER_HAS_MATCH]);
        }
        else if (is_match) {
            is_match = 0;
            zem_up(&zem_array[SMOKER_HAS_TOBACCO]);
        }
        else is_paper = 1;
        zem_up(&lock);
    }
    return NULL;
}

void *PusherTobacco(void *arg){
    while(1) {
        zem_down(&zem_array[TOBACCO]);
        if (finished) break;
        zem_down(&lock); // locking mutex
        if (is_paper) {
            is_paper = 0;
            zem_up(&zem_array[SMOKER_HAS_MATCH]);
        }
        else if (is_match) {
            is_match = 0;
            zem_up(&zem_array[SMOKER_HAS_PAPER]);
        }
        else is_tobacco = 1;
        zem_up(&lock);
    }
    return NULL;
}


void* SmokerHasMatch(void *arg){
    while(1) {
        zem_down(&zem_array[SMOKER_HAS_MATCH]);
        if (finished) break;
        printf("Smoker (Match) is smoking\n");
        zem_up(&zem_array[AGENT]);
    }
    return NULL;
}


void* SmokerHasPaper(void *arg){
    while(1) {
        zem_down(&zem_array[SMOKER_HAS_PAPER]);
        if (finished) break;
        printf("Smoker (Paper) is smoking\n");
        zem_up(&zem_array[AGENT]);
    }
    return NULL;
}


void* SmokerHasTobacco(void *arg){
    while(1) {
        zem_down(&zem_array[SMOKER_HAS_TOBACCO]);
        if (finished) break;
        printf("Smoker (Tobacco) is smoking\n");
        zem_up(&zem_array[AGENT]);
    }
    return NULL;
}

int main(){

    pthread_t threads[7];

    zem_init(&zem_array[AGENT], 1);
    for (int i = SMOKER_HAS_MATCH; i < (TOBACCO + 1); i++)
        zem_init(&zem_array[i], 0);
    zem_init(&lock, 1);

    pthread_create(&threads[AGENT], NULL, Agent, NULL);
    pthread_create(&threads[SMOKER_HAS_MATCH], NULL, SmokerHasMatch, NULL);
    pthread_create(&threads[SMOKER_HAS_PAPER], NULL, SmokerHasPaper, NULL);
    pthread_create(&threads[SMOKER_HAS_TOBACCO], NULL, SmokerHasTobacco, NULL);
    pthread_create(&threads[MATCH], NULL, PusherMatch, NULL);
    pthread_create(&threads[PAPER], NULL, PusherPaper, NULL);
    pthread_create(&threads[TOBACCO], NULL, PusherTobacco, NULL);
    
    for(int i = 0; i < 7; i++)
        pthread_join(threads[i], NULL);

    return 0;
}
