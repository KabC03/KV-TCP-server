#include "thread_queue.h"

typedef struct TaskNode {
    ClientRequest *clientTask; 
    struct TaskNode *next;
} TaskNode;



/* thread_queue_init */
/* Initialise a thread queue */
void thread_queue_init(ThreadQueue *queue) {
    queue->head = NULL;
    queue->tail = NULL;

    pthread_mutex_init(&(queue->mut), NULL);
    pthread_cond_init(&(queue->cond), NULL);
    return;
}


/* thread_queue_destroy */
/* Destroy a thread queue and associated memory */
void thread_queue_destroy(ThreadQueue *queue) {

    TaskNode *curr = queue->head;
    TaskNode *prev = queue->head;
    while(curr) {
        curr = curr->next;
        free(prev);
        prev = curr;
    }

    queue->head = NULL;
    queue->tail = NULL;

    pthread_mutex_destroy(&(queue->mut));
    pthread_cond_destroy(&(queue->cond));

    return;
}

/* thread_queue_enqueue */
/* Enqueue a node to a thread queue */
bool thread_queue_enqueue(ThreadQueue *queue, ClientRequest *request) {

    TaskNode *new = malloc(sizeof(TaskNode));
    if(!new) {
        return false;
    }
    new->clientTask = request;

    if(!queue->head) { //Empty queue
        queue->head = new;
        queue->tail = new;

    } else { //Non-empty queue
        queue->tail->next = new;
        queue->tail = new;
    }

    return true;
}


/* thread_queue_dequeue */
/* Dequeue a task from the queue */
void thread_queue_dequeue(ThreadQueue *queue) {
    
    TaskNode *node = queue->head;
    if(queue->head == queue->tail) { //Last item in queue
        queue->tail = NULL;
    }

    queue->head = queue->head->next;
    free(node);

    return;
}



