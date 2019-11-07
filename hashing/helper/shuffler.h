//
// Created by Shuhao Zhang on 6/11/19.
//

#ifndef ALLIANCEDB_SHUFFLER_H
#define ALLIANCEDB_SHUFFLER_H

#include "fetcher.h"
#include "concurrentqueue.h"
#include "maps/robin_map.h"

class BaseShuffler {
public:
    int nthreads;
    relation_t *relR;
    relation_t *relS;

    BaseShuffler(int tid, relation_t *relR,
                 relation_t *relS);

    virtual void push(intkey_t key, fetch_t *fetch) = 0;

    virtual fetch_t *pull(int32_t tid) = 0;

};

struct T_queue {
    moodycamel::ConcurrentQueue<fetch_t *> *queue = new moodycamel::ConcurrentQueue<fetch_t *>();
};

class HashShuffler : public BaseShuffler {

public:
    //in a extreme case, hash-partition the queue.
    T_queue *queues;//simple hashing first.

    HashShuffler(int nthreads, relation_t *relR, relation_t *relS);

    void push(intkey_t key, fetch_t *fetch) override;

    fetch_t *pull(int32_t tid) override;
};


class ContRandShuffler : public BaseShuffler {
public:

    //in a extreme case, hash-partition the queue.
    //moodycamel::ConcurrentQueue<fetch_t> queues[];//simple hashing first.
    T_queue *queues;//simple hashing first.

    ContRandShuffler(int nthreads, relation_t *relR, relation_t *relS);

    void push(intkey_t key, fetch_t *fetch) override;

    fetch_t *pull(int32_t tid) override;
};

#endif //ALLIANCEDB_SHUFFLER_H
