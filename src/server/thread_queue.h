#ifndef THREAD_QUEUE_H
#define THREAD_QUEUE_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "hashmap.h"
#include "../commands.h"
#include "../message.h"

typedef struct TaskNode TaskNode;
typedef struct ThreadQueue {
    TaskNode *head;
    TaskNode *tail;

    pthread_mutex_t mut; //Prevent race conditions
    pthread_cond_t cond; //Workers sleep while tasks arrive
} ThreadQueue;




#endif

