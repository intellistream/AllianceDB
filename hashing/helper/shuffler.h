//
// Created by Shuhao Zhang on 6/11/19.
//

#pragma once

#include "fetcher.h"
#include "concurrentqueue.h"
#include "maps/robin_map.h"
#include "readerwriterqueue.h"
#include <list>
#include <optional>
class baseShuffler {
public:
    int nthreads;
    relation_t *relR;
    relation_t *relS;
    bool isCR = false;

    //only used by JBCR.
//    int32_t group_size = 2;//TODO: use a input parameter to tune this.

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

    virtual void push_r(Batch* batch){
        return;
    }

    virtual void push_s(Batch* batch){
        return;
    }

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
    int32_t numGrps;
    std::vector<int32_t> *grpToTh;
//    int32_t *thToGrp;

    ContRandShuffler(int nthreads, relation_t *relR, relation_t *relS, int i);

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

// fetch input with batched round robin manner
class SHJRoundRobinFetcher{
private:
    // the relation to fetch from
    relation_t* relation;
    // number of tuples a thread will fetch for a given index
    int thread_range=4096*8;
    // size of a batch (for a given index, thread_range/vector_batch_size batches will be generated)
    Batch tmp_batch;

    // current thread id
    int thread_id;
    // total thread number
    int thread_cnt;

    // current batch index
    int index;
    // tuple start loc for current index
    int cur_batch_loc;
    // tuple end loc for current index
    int cur_batch_end;
    uint64_t fetchStartTime=0;

    void set_index(int idx){
        index=idx;
        cur_batch_loc=min((index*thread_cnt+thread_id)*thread_range,(int)relation->num_tuples);
        cur_batch_end=min(cur_batch_loc+thread_range,(int)relation->num_tuples);
    }
public:
    explicit SHJRoundRobinFetcher(int thread_id,int thread_cnt,relation_t* relation,uint64_t fetch_start_time):thread_id(thread_id),thread_cnt(thread_cnt),relation(relation),tmp_batch(),fetchStartTime(fetch_start_time){
        index=0;
        set_index(index);
    }

     std::optional<Batch> fetch_batch() {
        // pull all the available tuples and form a batch
        tuple_t* readR;
        while(true){
            readR= &relation->tuples[cur_batch_loc];
            // if the current tuple can be delivered
            if(tmp_batch.size()<Batch::max_size()&&relation->payload->ts[readR->payloadID]<=(curtick() - fetchStartTime)){
                tmp_batch.add_tuple(readR);
                cur_batch_loc++;
                // outside the current range
                if(cur_batch_loc>=cur_batch_end){
                    index++;
                    set_index(index);
                    break;
                }
            }// if the timestampe of current tuple > current time
            else{
                break;
            }
        }
        if(tmp_batch.size()>0){
            auto result_batch=std::make_optional<Batch>(std::move(tmp_batch));
            tmp_batch.allocate();
            return result_batch;
        }
        return std::nullopt;

    }

    inline bool done(){
        return cur_batch_loc>=relation->num_tuples;
    }


};

class SHJShuffleQueueGroup{
private:
    uint32_t thread_cnt;
    vector<moodycamel::ConcurrentQueue<Batch>> queues;
    atomic<int> done_cnt;


public:
    explicit SHJShuffleQueueGroup(uint32_t thread_cnt):thread_cnt(thread_cnt){
        done_cnt=0;
        queues.resize(thread_cnt);
    }

    void push_batch(Batch&& batch){
        size_t hash_loc;
        vector<Batch> tmp_batches(thread_cnt);
        for(int i=0;i<batch.size();i++){
            // use naive hashing for now
            hash_loc = batch.keys_[i] % thread_cnt;
            tmp_batches[hash_loc].add_tuple(batch.keys_[i],batch.values_[i]);
        }
        for(int i=0;i<thread_cnt;i++){
            if(tmp_batches[i].size()>0){
                queues[i].enqueue(std::move(tmp_batches[i]));
            }

        }
    }

    std::optional<Batch> pull_batch(int thread_id){
        auto& queue=queues[thread_id];
        Batch b;
        bool success=queue.try_dequeue(b);
        if(success&&b.size()!=0){
            return make_optional<Batch>(std::move(b));
        }
        return std::nullopt;
    }

    void fetcher_done(){
        done_cnt++;
    }

    bool done(int thread_id){
        return done_cnt==thread_cnt&&queues[thread_id].size_approx()==0;
    }

};
