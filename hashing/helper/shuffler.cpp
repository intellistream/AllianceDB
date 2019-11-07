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
void HashShuffler::push(intkey_t key, fetch_t *fetch) {
    int32_t idx = KEY_TO_IDX(key, nthreads);
    moodycamel::ConcurrentQueue<fetch_t *> *queue = queues[idx].queue;
    queue->enqueue(new fetch_t(fetch));
    DEBUGMSG(1, "PUSH: %d, tuple: %d, queue size:%d\n", idx, fetch->tuple->key, queue->size_approx())
}

fetch_t *HashShuffler::pull(int32_t tid) {
    int32_t idx = TID_To_IDX(tid);
    moodycamel::ConcurrentQueue<fetch_t *> *queue = queues[idx].queue;
    fetch_t *tuple;
    bool rt = queue->try_dequeue(tuple);

    if (!rt)
        return nullptr;
//    bool rt =  .try_dequeue(tuple);
    DEBUGMSG(1, "PULL: %d, tuple: %d, queue size:%d\n", idx, tuple->tuple->key, queue->size_approx());
    return tuple;
}


BaseShuffler::BaseShuffler(int nthreads, relation_t *relR, relation_t *relS) {
    this->nthreads = nthreads;
    this->relR = relR;
    this->relS = relS;
}

HashShuffler::HashShuffler(int nthreads, relation_t *relR, relation_t *relS)
        : BaseShuffler(nthreads, relR, relS) {
    queues = new T_queue[nthreads];
//    for (int i = 0; i < nthreads; i++) {
//        queues[i] = new moodycamel::ConcurrentQueue<fetch_t *>();
//    }
}

ContRandShuffler::ContRandShuffler(int nthreads, relation_t *relR, relation_t *relS)
        : BaseShuffler(nthreads, relR, relS) {
    queues = new T_queue[nthreads];
}

void ContRandShuffler::push(intkey_t key, fetch_t *fetch) {

}

fetch_t *ContRandShuffler::pull(int32_t tid) {
    return nullptr;
}


