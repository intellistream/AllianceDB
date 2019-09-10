#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "generic_task_queue.h"

typedef struct taskitem_t taskitem_t;
typedef struct tasklist_t tasklist_t;

struct taskitem_t {
    void * task;
    taskitem_t * next;
    taskitem_t * prev;
};

struct tasklist_t {
    taskitem_t * tasks;
    tasklist_t * next;
    int          curr;
};

struct taskqueue_t {
    pthread_mutex_t lock;
    pthread_mutex_t alloc_lock;
    taskitem_t *    head;
    taskitem_t *    tail;
    tasklist_t *    free_list;
    int32_t         count;
    int32_t         allocsize;
};

/* gets a new taskitem slot */
inline taskitem_t *
taskqueue_getslot(taskqueue_t * tq)  __attribute__((always_inline));

/* gets a new taskitem slot */
inline taskitem_t *
taskqueue_getslot(taskqueue_t * tq)
{
    tasklist_t * l = tq->free_list;
    taskitem_t * ret;

    if(l->curr < tq->allocsize) {
        ret = &(l->tasks[l->curr]);
        l->curr++;
    }
    else {
        tasklist_t * nl = (tasklist_t *) malloc(sizeof(tasklist_t));
        nl->tasks = (taskitem_t *) malloc(tq->allocsize * sizeof(taskitem_t));
        nl->curr        = 1;
        nl->next        = tq->free_list;
        tq->free_list   = nl;
        ret             = &(nl->tasks[0]);
    }

    return ret;
}

/* initialize a task queue with given allocation block size */
taskqueue_t *
taskqueue_init(int allocsize)
{
    taskqueue_t * ret     = (taskqueue_t*) malloc(sizeof(taskqueue_t));
    ret->free_list        = (tasklist_t*) malloc(sizeof(tasklist_t));
    ret->free_list->tasks = (taskitem_t*) malloc(allocsize*sizeof(taskitem_t));
    ret->free_list->curr  = 0;
    ret->free_list->next  = NULL;
    ret->count            = 0;
    ret->allocsize        = allocsize;
    ret->head             = NULL;
    ret->tail             = NULL;
    pthread_mutex_init(&ret->lock, NULL);
    pthread_mutex_init(&ret->alloc_lock, NULL);

    return ret;
}

/** Add a new task to the front of the queue */
void
taskqueue_addfront(taskqueue_t * tq, void * t)
{
    pthread_mutex_lock(&tq->lock);
    taskitem_t * slot = taskqueue_getslot(tq);
    slot->task = t;
    slot->next = tq->head;
    slot->prev = NULL;
    tq->head   = slot;
    if(tq->tail == NULL) {
        tq->tail = slot;
    }
    tq->count ++;
    pthread_mutex_unlock(&tq->lock);
}

/** Add a new task to the tail of the queue */
void
taskqueue_addtail(taskqueue_t * tq, void * t)
{
    pthread_mutex_lock(&tq->lock);
    taskitem_t * slot = taskqueue_getslot(tq);
    slot->task = t;
    slot->next = NULL;
    slot->prev = tq->tail;
    if(tq->tail) {
        tq->tail->next = slot;
    } else {
        tq->head = slot;
    }
    tq->tail = slot;
    tq->count ++;
    pthread_mutex_unlock(&tq->lock);
}

/** get the task from the front of the queue */
void *
taskqueue_getfront(taskqueue_t * tq)
{
    pthread_mutex_lock(&tq->lock);
    void * ret = 0;
    if(tq->count > 0){
        ret = tq->head->task;
        /* memory for taskitems will be released at the end */
        tq->count --;
        tq->head = tq->head->next;
        if(tq->count == 0) {
            tq->tail = NULL;
        }
        else {
            tq->head->prev = NULL;
        }
    }
    pthread_mutex_unlock(&tq->lock);

    return ret;
}

/** get the task from the end of the queue */
void *
taskqueue_gettail(taskqueue_t * tq)
{
    pthread_mutex_lock(&tq->lock);
    void * ret = 0;
    if(tq->count > 0){
        ret = tq->tail->task;
        tq->count --;
        tq->tail = tq->tail->prev;
        if(tq->count == 0) {
            tq->head = NULL;
        }
        else {
            tq->tail->next = NULL;
        }
    }
    pthread_mutex_unlock(&tq->lock);

    return ret;
}

void
taskqueue_free(taskqueue_t * tq)
{
    tasklist_t * tmp = tq->free_list;
    while(tmp) {
        free(tmp->tasks);
        tasklist_t * tmp2 = tmp->next;
        free(tmp);
        tmp = tmp2;
    }
    free(tq);
}

