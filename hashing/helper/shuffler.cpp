//
// Created by Shuhao Zhang on 6/11/19.
//

#include <iostream>
#include "shuffler.h"

bool HashShuffler::finish(int32_t i) {
    return false;
}

intkey_t ID_To_Key(int32_t tid) {
    return tid;
}

fetch_t *HashShuffler::pull(int32_t tid) {
    intkey_t key = ID_To_Key(tid);
    moodycamel::ConcurrentQueue<fetch_t *> *queue = queues[key];

    fetch_t *tuple;

    queue->try_dequeue(tuple);

//    bool rt =  .try_dequeue(tuple);
    return tuple;
}

void HashShuffler::push(intkey_t key, fetch_t *pFetch) {
    moodycamel::ConcurrentQueue<fetch_t *> *queue = queues[key];
    queue->enqueue(pFetch);
}


BaseShuffler::BaseShuffler(int nthreads, relation_t *relR, relation_t *relS) {
    this->nthreads = nthreads;
    this->relR = relR;
    this->relS = relS;
}

HashShuffler::HashShuffler(int nthreads, relation_t *relR, relation_t *relS)
        : BaseShuffler(nthreads, relR, relS) {

    *queues = new moodycamel::ConcurrentQueue<fetch_t *>[relS->num_tuples / nthreads];

}

ContRandShuffler::ContRandShuffler(int nthreads, relation_t *relR, relation_t *relS)
        : BaseShuffler(nthreads, relR, relS) {
    *queues = new moodycamel::ConcurrentQueue<fetch_t *>[relS->num_tuples / nthreads];

}

void ContRandShuffler::push(intkey_t key, fetch_t *pFetch) {

}

bool ContRandShuffler::finish(int32_t i) {
    return false;
}

fetch_t *ContRandShuffler::pull(int32_t tid) {
    return nullptr;
}


