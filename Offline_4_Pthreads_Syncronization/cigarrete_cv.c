#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>

int MAX_ITERATIONS = 10;

pthread_cond_t agent;
pthread_cond_t smoker[3]; // 0 for tobacco, 1 for paper, 2 for match
pthread_cond_t pusher[3]; // 0 for tobacco, 1 for paper, 2 for match

int agent_flag;
int smoker_flag[3];  // 0 for tobacco, 1 for paper, 2 for match
int pusher_flag[3]; // 0 for tobacco, 1 for paper, 2 for match

int is_paper, is_tobacco, is_match;
int finish =0;
pthread_mutex_t var_mutex;
pthread_mutex_t lock_mutex;

void *agent_(void *arg)
{
    int flag;
    for(int i=0; i<MAX_ITERATIONS; i++)
    {
        pthread_mutex_lock(&lock_mutex);
        while (!agent_flag)
        {
            pthread_cond_wait(&agent,&lock_mutex);
        }
        
        flag = rand() % 3;
        
        // printf("%d %d",num_1,num_2);
        if (flag == 0)
        {
            printf("Agent is placing tobacco and paper on the table\n");
            pusher_flag[0]=1;
            pusher_flag[1]=1;
            pthread_cond_signal(&pusher[0]);
            pthread_cond_signal(&pusher[1]);
        }
        else if (flag== 1)
        {
            printf("Agent is placing tobacco and match on the table\n");
            pusher_flag[0]=1;
            pusher_flag[2]=1;
            pthread_cond_signal(&pusher[0]);
            pthread_cond_signal(&pusher[2]);
        }
        else 
        {
            printf("Agent is placing paper and match on the table\n");
            pusher_flag[1]=1;
            pusher_flag[2]=1;
            pthread_cond_signal(&pusher[1]);
            pthread_cond_signal(&pusher[2]);
        }
        agent_flag=0;
        pthread_mutex_unlock(&lock_mutex);
    }
    while (!agent_flag)
    {
        pthread_cond_wait(&agent ,&lock_mutex);
    }
    finish =1;
    
    for(int i=0;i<3;i++)
    {
        pusher_flag[i]=1;
        smoker_flag[i]=1;
    }

    for(int i=0;i<3;i++)
    {
        pthread_cond_signal(&pusher[i]);
        pthread_cond_signal(&smoker[i]);
    }

    pthread_mutex_unlock(&lock_mutex);
    return NULL;
}


void* PusherTobacco( void* arg )
{
    while(1)
    {
        pthread_mutex_lock(&lock_mutex);
        while (!pusher_flag[0])
        {
            pthread_cond_wait(&pusher[0],&lock_mutex);
        }
         
        if (finish){
            pthread_mutex_unlock(&lock_mutex);
            break;}
        pthread_mutex_lock(&var_mutex);
        if(is_paper)
        {
            is_paper = 0;
            smoker_flag[2] = 1;
            pthread_cond_signal(&smoker[2]);
        }
        else if( is_match)
        {
            is_match = 0;
            smoker_flag[1] = 1;
            pthread_cond_signal(&smoker[1]);
        }
        else
        {
            is_tobacco = 1;
        }
         pusher_flag[0]=0;
        pthread_mutex_unlock(&var_mutex);
        pthread_mutex_unlock(&lock_mutex);
    }
    return NULL;
}

void* PusherPaper( void* arg )
{
    while(1)
    {
        pthread_mutex_lock(&lock_mutex);
        while (!pusher_flag[1])
        {
            pthread_cond_wait(&pusher[1],&lock_mutex);
        }
        if (finish){
            pthread_mutex_unlock(&lock_mutex);
            break;}
        pthread_mutex_lock(&var_mutex);
        if(is_tobacco)
        {
            is_tobacco = 0;
            smoker_flag[2] = 1;
            pthread_cond_signal(&smoker[2]);
        }
        else if( is_match)
        {
            is_match = 0;
            smoker_flag[0] = 1;
            pthread_cond_signal(&smoker[0]);
        }
        else
        {
            is_paper = 1;
        }
        pusher_flag[1]=0;
        pthread_mutex_unlock(&var_mutex);
        pthread_mutex_unlock(&lock_mutex);
    }
    return NULL;
}

void* PusherMatch( void* arg )
{
    while(1)
    {
        pthread_mutex_lock(&lock_mutex);
        while (!pusher_flag[2])
        {
            pthread_cond_wait(&pusher[2],&lock_mutex);
        }
        if (finish){
            pthread_mutex_unlock(&lock_mutex);
            break;}
        pthread_mutex_lock(&var_mutex);
        if(is_tobacco)
        {
            is_tobacco = 0;
            smoker_flag[1] = 1;
            pthread_cond_signal(&smoker[1]);
        }
        else if( is_paper)
        {
            is_paper = 0;
            smoker_flag[0] = 1;
            pthread_cond_signal(&smoker[0]);
        }
        else
        {
            is_match = 1;
        }
        pusher_flag[2]=0;
        pthread_mutex_unlock(&var_mutex);
        pthread_mutex_unlock(&lock_mutex);
    }
    return NULL;
}

void* SmokerTobacco( void* arg )
{
    while(1)
    {
        pthread_mutex_lock(&lock_mutex);
        while (!smoker_flag[0])
        {
            pthread_cond_wait(&smoker[0],&lock_mutex);
        }
        if (finish){
            pthread_mutex_unlock(&lock_mutex);
            break;}
        pthread_mutex_lock(&var_mutex);
        printf("Smoker(tobacco) is smoking\n");
        agent_flag = 1;
        pthread_cond_signal(&agent);
        smoker_flag[0]=0;
        pthread_mutex_unlock(&var_mutex);
        pthread_mutex_unlock(&lock_mutex);
    }
    return NULL;
}

void* SmokerPaper( void* arg )
{
    while(1)
    {
        pthread_mutex_lock(&lock_mutex);
        while (!smoker_flag[1])
        {
            pthread_cond_wait(&smoker[1],&lock_mutex);
        }
        if (finish){
            pthread_mutex_unlock(&lock_mutex);
            break;}
        pthread_mutex_lock(&var_mutex);
        printf("Smoker(paper) is smoking\n");
        agent_flag = 1;
        pthread_cond_signal(&agent);
        smoker_flag[1]=0;
        pthread_mutex_unlock(&var_mutex);
        pthread_mutex_unlock(&lock_mutex);
    }
    return NULL;
}

void* SmokerMatch( void* arg )
{
    while(1)
    {
        pthread_mutex_lock(&lock_mutex);
        while (!smoker_flag[2])
        {
            pthread_cond_wait(&smoker[2],&lock_mutex);
        }
        if (finish){
            pthread_mutex_unlock(&lock_mutex);
            break;}
        pthread_mutex_lock(&var_mutex);
        printf("Smoker(match) is smoking\n");
        agent_flag = 1;
        pthread_cond_signal(&agent);
        smoker_flag[2]=0;
        pthread_mutex_unlock(&var_mutex);
        pthread_mutex_unlock(&lock_mutex);
    }
    return NULL;
}

int main()
{
    pthread_t agent_thread;
    pthread_t pusher_tobacco_thread;
    pthread_t pusher_paper_thread;
    pthread_t pusher_match_thread;
    pthread_t smoker_tobacco_thread;
    pthread_t smoker_paper_thread;
    pthread_t smoker_match_thread;

    pthread_mutex_init(&lock_mutex, NULL);
    pthread_mutex_init(&var_mutex, NULL);
    pthread_cond_init(&agent, NULL);
    for(int i=0; i<3; i++)
    {
        pthread_cond_init(&pusher[i], NULL);
        pthread_cond_init(&smoker[i], NULL);
    }
    pthread_create(&agent_thread, NULL, agent_, NULL);
    pthread_create(&pusher_tobacco_thread, NULL, PusherTobacco, NULL);
    pthread_create(&pusher_paper_thread, NULL, PusherPaper, NULL);
    pthread_create(&pusher_match_thread, NULL, PusherMatch, NULL);
    pthread_create(&smoker_tobacco_thread, NULL, SmokerTobacco, NULL);
    pthread_create(&smoker_paper_thread, NULL, SmokerPaper, NULL);
    pthread_create(&smoker_match_thread, NULL, SmokerMatch, NULL);

    agent_flag = 1;
    pthread_cond_signal(&agent);

    pthread_join(agent_thread, NULL);
    pthread_join(pusher_tobacco_thread, NULL);
    pthread_join(pusher_paper_thread, NULL);
    pthread_join(pusher_match_thread, NULL);
    pthread_join(smoker_tobacco_thread, NULL);
    pthread_join(smoker_paper_thread, NULL);
    pthread_join(smoker_match_thread, NULL);

    return 0;
}