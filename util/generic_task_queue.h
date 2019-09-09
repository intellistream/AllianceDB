/**
 * @file   generic_task_queue.h
 * @author Cagri Balkesen <cagri.balkesen@inf.ethz.ch>
 * @date   Tue Nov  6 14:39:33 2012
 *
 * @brief  Implements a generic task queue facility for the thread pool.
 *
 * (c) 2012, ETH Zurich, Systems Group
 *
 */
#ifndef ALLIANCEDB_GENERIC_TASK_QUEUE_H
#define ALLIANCEDB_GENERIC_TASK_QUEUE_H


/**
 * @defgroup GenericTaskQueue Generic Task Queue Implementation
 * @{
 */

typedef struct taskqueue_t taskqueue_t;


/* initialize a task queue with given allocation block size */
taskqueue_t *
taskqueue_init(int allocsize);

/** Add a new task to the front of the queue */
void
taskqueue_addfront(taskqueue_t * tq, void * t);

/** Add a new task to the tail of the queue */
void
taskqueue_addtail(taskqueue_t * tq, void * t);

/** get the task from the front of the queue */
void *
taskqueue_getfront(taskqueue_t * tq);

/** get the task from the end of the queue */
void *
taskqueue_gettail(taskqueue_t * tq);

void
taskqueue_free(taskqueue_t * tq);

/** @} */

#endif //ALLIANCEDB_GENERIC_TASK_QUEUE_H
