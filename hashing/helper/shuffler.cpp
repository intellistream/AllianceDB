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

//pass-in pointer points to fetch
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
    fetch_t *fetch;
    bool rt;
    if (fetchR) {
        queue = leftRecvQueue[tid].queue;
        rt = queue->try_dequeue(fetch);
        if (!rt)
            return nullptr;

    } else {
        queue = rightRecvQueue[tid].queue;
        rt = queue->try_dequeue(fetch);
        if (!rt)
            return nullptr;
    }

    return fetch;
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
    isCR = true;
    numGrps = ceil(nthreads / group_size);
    queues = new T_CQueue[nthreads];
    grpToTh = new std::vector<int32_t>[numGrps];
//    thToGrp = new int32_t[nthreads];
    // assign each thread to a group
    uint32_t curGrpId = 0;
    uint32_t count = 0;
    for (int i = 0; i < nthreads; i++) {
        if (count < group_size) {
            grpToTh[curGrpId].push_back(i);
//            thToGrp[i] = curGrpId;
        } else {
            count = 0;
            curGrpId++;
        }
        count++;
    }
}

void ContRandShuffler::push(intkey_t key, fetch_t *fetch, bool pushR) {
    // replicate R, partition S in each Group
    int32_t idx = KEY_TO_IDX(key, numGrps);

    std::vector<int32_t> curGrp = grpToTh[idx];
    // replicate R
    if (fetch->ISTuple_R) {
        DEBUGMSG("PUSH: %d, tuple: %d, R?%d\n", idx, fetch->tuple->key, fetch->ISTuple_R)
        for (auto it = curGrp.begin(); it != curGrp.end(); it++) {
            moodycamel::ConcurrentQueue<fetch_t *> *queue = queues[*it].queue;
            queue->enqueue(new fetch_t(fetch));
            DEBUGMSG("PUSH: %d, tuple: %d, queue size:%d\n", idx,
                     fetch->tuple->key,
                     queue->size_approx())
        }
    } else { // partition S
        DEBUGMSG("PUSH: %d, tuple: %d, R?%d\n", idx, fetch->tuple->key, fetch->ISTuple_R)
        int32_t idx_s = rand() % curGrp.size(); // randomly distribute to threads in the groups
        moodycamel::ConcurrentQueue<fetch_t *> *queue = queues[curGrp[idx_s]].queue;
        queue->enqueue(new fetch_t(fetch));
        DEBUGMSG("PUSH: %d, tuple: %d, queue size:%d\n", idx,
                 fetch->tuple->key,
                 queue->size_approx())
    }
}

fetch_t *ContRandShuffler::pull(int32_t tid, bool b) {

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

//    int32_t idx = TID_To_IDX(numGrps);
//    bool isLocal = false;
//    int32_t selected_index = -1;
//    for (int i=0; i<nthreads; i++) {
//        if (thToGrp[i] == idx) {
//            if (i == tid) {
//                isLocal == true;
//                selected_index = i;
//            } else if (i==tid && isLocal == false) {
//                selected_index = i;
//            }
//        }
//    }
//    if (selected_index == -1) {
//        DEBUGMSG("error in CR... %d\n", idx);
//    }
//    moodycamel::ConcurrentQueue<fetch_t *> *queue = queues[selected_index].queue;
//    fetch_t *tuple;
//    bool rt = queue->try_dequeue(tuple);
//    if (!rt)
//        return nullptr;
////          bool rt =  .try_dequeue(tuple);
//    DEBUGMSG("PULL: %d, tuple: %d, queue size:%d\n", idx,
//             tuple->tuple->key,
//             queue->size_approx());
//    return tuple;
}


