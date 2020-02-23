//
// Created by Shuhao Zhang on 6/11/19.
//

#ifndef ALLIANCEDB_SHUFFLER_H
#define ALLIANCEDB_SHUFFLER_H

#include "fetcher.h"
#include "concurrentqueue.h"
#include "maps/robin_map.h"
#include "readerwriterqueue.h"
#include <list>

class baseShuffler {
public:
    int nthreads;
    relation_t *relR;
    relation_t *relS;
    bool isCR = false;

    baseShuffler(int tid, relation_t *relR,
                 relation_t *relS);

    virtual void push(intkey_t key, fetch_t *fetch, bool pushR) = 0;

    /**
     *
     * @param tid
     * @param fetchLeft  only used in HS mode.
     * @return
     */
    virtual fetch_t *pull(int32_t tid, bool fetchLeft) = 0;

};

/**
 * Try other queues in future.
 */
struct T_SQueue {
    moodycamel::ReaderWriterQueue<fetch_t *> *queue = new moodycamel::ReaderWriterQueue<fetch_t *>();
};

struct T_CQueue {
    moodycamel::ConcurrentQueue<fetch_t *> *queue = new moodycamel::ConcurrentQueue<fetch_t *>();
};

class HashShuffler : public baseShuffler {

public:
    //in a extreme case, hash-partition the queue.
    T_CQueue *queues;//simple hashing first.

    HashShuffler(int nthreads, relation_t *relR, relation_t *relS);

    void push(intkey_t key, fetch_t *fetch, bool b) override;

    fetch_t *pull(int32_t tid, bool b) override;
};


class ContRandShuffler : public baseShuffler {
public:

    //in a extreme case, hash-partition the queue.
    //moodycamel::ConcurrentQueue<fetch_t> queues[];//simple hashing first.
    T_CQueue *queues;//simple hashing first.
    int32_t group_size = 2;//TODO: use a input parameter to tune this.
    int32_t numGrps;
    std::vector<int32_t> *grpToTh;
//    int32_t *thToGrp;

    ContRandShuffler(int nthreads, relation_t *relR, relation_t *relS);

    void push(intkey_t key, fetch_t *fetch, bool pushR) override;

    fetch_t *pull(int32_t tid, bool b) override;
};


/**
 * some conventions:
 * thread 0 reads R and is identified as ``left" core.
 */
class HSShuffler : public baseShuffler {

    /**
     * Though supportive of increased parallelism, asynchronous communication between cores can bear an important risk.
     * "Missing" Matches.
     */
public:
    T_SQueue *leftRecvQueue;//a group of input queue of R each core.
    T_SQueue *rightRecvQueue;//a group of input queue of S each core.

//    T_SQueue *rWindow;//a group of input queue of each core.
//    T_SQueue *sWindow;//a group of output queue of each core.

    HSShuffler(int nthreads, relation_t *relR, relation_t *relS);

    void push(int32_t tid, fetch_t *fetch, bool pushR) override;

    fetch_t *pull(int32_t tid, bool fetchR) override;

};

#endif //ALLIANCEDB_SHUFFLER_H
