//
// Created by Shuhao Zhang on 6/11/19.
//

#include <iostream>
#include "shuffler.h"
#include "../joins/common_functions.h"

//TODO: Make Better Hashing..
int32_t TID_To_IDX(int32_t tid) {
    return tid;
}

int32_t KEY_TO_IDX(intkey_t key, int nthreads) {
    return key % nthreads;
}

//pass-in pointer points to state.fetch
void HashShuffler::push(intkey_t key, fetch_t *fetch, bool b) {
    int32_t idx = KEY_TO_IDX(key, nthreads);
    moodycamel::ConcurrentQueue<fetch_t *> *queue = queues[idx].queue;
    queue->enqueue(new fetch_t(fetch));
    DEBUGMSG("PUSH: %d, tuple: %d, queue size:%d\n", idx,
             fetch->tuple->key,
             queue->size_approx())
}

fetch_t *HashShuffler::pull(int32_t tid, bool b) {
    int32_t idx = TID_To_IDX(tid);
    moodycamel::ConcurrentQueue<fetch_t *> *queue = queues[idx].queue;
    fetch_t *tuple;
    bool rt = queue->try_dequeue(tuple);

    if (!rt)
        return nullptr;
//    bool rt =  .try_dequeue(tuple);
    DEBUGMSG("PULL: %d, tuple: %d, queue size:%d\n", idx,
             tuple->tuple->key,
             queue->size_approx());
    return tuple;
}

HSShuffler::HSShuffler(int nthreads, relation_t *relR, relation_t *relS)
        : baseShuffler(nthreads, relR, relS) {
    leftRecvQueue = new T_SQueue[nthreads];
    rightRecvQueue = new T_SQueue[nthreads];
    this->nthreads = nthreads;
    this->relR = relR;
    this->relS = relS;
}

void HSShuffler::push(int32_t tid, fetch_t *fetch, bool pushR) {
    moodycamel::ReaderWriterQueue<fetch_t *> *queue;
    if (pushR) {
        queue = leftRecvQueue[tid].queue;
    } else {
        queue = rightRecvQueue[tid].queue;
    }
    queue->enqueue(new fetch_t(fetch));
}

/**
 * pull left queue if fetchLeft
 * @param tid
 * @return
 */
fetch_t *HSShuffler::pull(int32_t tid, bool fetchR) {
    moodycamel::ReaderWriterQueue<fetch_t *> *queue;
    fetch_t *tuple;
    if (fetchR) {
        queue = leftRecvQueue[tid].queue;
        bool rt = queue->try_dequeue(tuple);

        if (!rt)
            return nullptr;

    } else {
        queue = rightRecvQueue[tid].queue;
        bool rt = queue->try_dequeue(tuple);
        if (!rt)
            return nullptr;

        if (tuple->flag) {
            return nullptr;
        }

//        tuple = queue->peek();//only obtain the pointer to T (fetch *),
        // and does not dequeue it actually.
//        if (!tuple)
//            return nullptr;
    }
    DEBUGMSG("PULL: %d, tuple: %d, queue size:%d\n", tid,
             (tuple)->tuple->key, queue->size_approx())
    return tuple;
}


baseShuffler::baseShuffler(int nthreads, relation_t *relR, relation_t *relS) {
    this->nthreads = nthreads;
    this->relR = relR;
    this->relS = relS;
}


HashShuffler::HashShuffler(int nthreads, relation_t *relR, relation_t *relS)
        : baseShuffler(nthreads, relR, relS) {
    queues = new T_CQueue[nthreads];
//    for (int i = 0; i < nthreads; i++) {
//        queues[i] = new moodycamel::ConcurrentQueue<fetch_t *>();
//    }
}

ContRandShuffler::ContRandShuffler(int nthreads, relation_t *relR,
                                   relation_t *relS)
        : baseShuffler(nthreads, relR, relS) {
    queues = new T_CQueue[nthreads];
}

void ContRandShuffler::push(intkey_t key, fetch_t *fetch, bool b) {

}

fetch_t *ContRandShuffler::pull(int32_t tid, bool b) {
    return nullptr;
}


