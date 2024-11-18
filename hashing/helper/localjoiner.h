//
// Created by Shuhao Zhang on 1/11/19.
//

#ifndef ALLIANCEDB_LOCALJOINER_H
#define ALLIANCEDB_LOCALJOINER_H

#include "../utils/xxhash64.h"

#include "../timer/t_timer.h"  /* startTimer, stopTimer */
#include "../utils/generator.h"          /* numa_localize() */
#include "../joins/npj_types.h"          /* bucket_t, hashtable_t, bucket_buffer_t */
#include "../joins/npj_params.h"         /* constant parameters */
#include "../joins/common_functions.h"
#include "../joins/batcher.h"
#include "pmj_helper.h"
#include <list>
#include <unordered_map>

///** To keep track of the input relation pairs fitting into L2 cache */
//typedef struct tuplepair_t tuplepair_t;
//
///** To keep track of the input relation pairs fitting into L2 cache */
//struct tuplepair_t {
//    tuple_t *R;
//    tuple_t *S;
//};

enum joiner_type {
    type_SHJJoiner, type_PMJJoiner, type_RippleJoiner
};

class baseJoiner {
public:

    virtual void join(int32_t tid, tuple_t *tuple, bool tuple_R, int64_t *matches,
            /*void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *),*/ void *pVoid) = 0;

    virtual void clean(int32_t tid, tuple_t *tuple, bool cleanR) = 0;

    virtual long
    merge(int32_t tid, int64_t *matches, /*void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *),*/
          void *pVoid) {
        //do nothing.
        return -1;
    }

    virtual void clean(int32_t tid, tuple_t *fat_tuple, int fat_tuple_size, bool cleanR) {
        //only used in PMJ.
    }

    virtual void join(int32_t tid, tuple_t *fat_tuple, int fat_tuple_size, bool IStuple_R, int64_t *matches,
            /*void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *),*/ void *pVoid) {
        //only supported by PMJ.
    }

    virtual void join_batched(int32_t tid, Batch* tuple, bool ISTupleR, int64_t *matches, void *pVoid){
        MSG("Batched Join not Implemented")
    }

//every joiner has its own timer --> this completely avoids interference.
//dump the measurement into file and analysis the results when program exit.
    int64_t matches = 0;
//#ifndef NO_TIMING
    T_TIMER *timer = new T_TIMER();
//#endif
};


class RippleJoiner : public baseJoiner {

private:
    relation_t *relR;
    relation_t *relS;
    t_window_list samList;


public:
    void
    join(int32_t tid, tuple_t *tuple, bool ISTupleR, int64_t *matches,
            /*void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *),*/ void *pVoid) override;

    RippleJoiner(relation_t *relR, relation_t *relS, int nthreads);

    void clean(int32_t tid, tuple_t *tuple, bool cleanR) override;
};

struct t_pmj {

    tuple_t *tmp_relR;
    int innerPtrR;
    int outerPtrR;
    int extraPtrR;
    int sizeR;
    tuple_t *tmp_relS;
    int innerPtrS;
    int outerPtrS;
    int extraPtrS;
    int sizeS;
    std::vector<run> Q;//let Q be an empty set;

    size_t relRsz;
    tuple_t *out_relR;
    tuple_t *out_relS;

    /***Initialize***/
    t_pmj(int sizeR, int sizeS) {
        /**** allocate temporary space for sorting ****/
        this->sizeR = sizeR;
        this->sizeS = sizeS;
        relRsz = sizeR * sizeof(tuple_t)
                 + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.

        tmp_relR = (tuple_t *) malloc_aligned(relRsz);


        relRsz = sizeS * sizeof(tuple_t)
                 + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.

        tmp_relS = (tuple_t *) malloc_aligned(relRsz);
        out_relR = (tuple_t *) malloc_aligned(relRsz);
        out_relS = (tuple_t *) malloc_aligned(relRsz);

        innerPtrR = 0;
        outerPtrR = 0;
        innerPtrS = 0;
        outerPtrS = 0;
        extraPtrR = 0;
        extraPtrS = 0;
    }
};

class PMJJoiner : public baseJoiner {
private:
    t_pmj *t_arg;

public:
    int progressive_step_R = 640;//#number of tuples to sort at each iteration. It must be multiple cacheline size (64).
    int progressive_step_S = 640;//#number of tuples to sort at each iteration. It must be multiple cacheline size (64).
    int merge_step = 10000;//#runs to merge at each iteration.

    PMJJoiner(int sizeR, int sizeS, int nthreads);

    void
    join(int32_t tid, tuple_t *tuple, bool IStuple_R, int64_t *matches,
            /*void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *),*/ void *pVoid) override;

    void join(int32_t tid, tuple_t *fat_tuple, int fat_tuple_size, bool IStuple_R, int64_t *matches,
            /*void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *),*/ void *pVoid) override;

    long
    merge(int32_t tid, int64_t *matches, /*void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *),*/
          void *pVoid) override;

    void clean(int32_t tid, tuple_t *tuple, bool cleanR) override;

    void clean(int32_t tid, tuple_t *fat_tuple, int fat_tuple_size, bool cleanR) override;

    void keep_tuple_single(tuple_t *tmp_rel, int outerPtr, tuple_t *tuple, int fat_tuple_size);

    void join_tuple_single(int32_t tid, tuple_t *tmp_rel, int *outerPtr, tuple_t *tuple, int fat_tuple_size,
                           int64_t *matches,
                           T_TIMER *timer, t_pmj *pPmj, bool IStuple_R, chainedtuplebuffer_t *pChainedtuplebuffer);
};

class SHJJoiner : public baseJoiner {

private:
#ifdef USE_CUSTOM_HASHTABLE
    hashtable_t *htR;
    hashtable_t *htS;
#endif

#ifndef USE_CUSTOM_HASHTABLE
    unordered_multimap<key_t,tuple_t> htR;
    unordered_multimap<key_t,tuple_t> htL;
#endif

public:
    virtual ~SHJJoiner();

    SHJJoiner(int sizeR, int sizeS);

    void
    join(int32_t tid, tuple_t *tuple, bool ISTupleR, int64_t *matches,
            /*void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *),*/ void *pVoid) override;

    void join_batched(int32_t tid, Batch* tuple, bool ISTupleR, int64_t *matches, void *pVoid) override;
    void clean(int32_t tid, tuple_t *tuple, bool cleanR) override;
};

SHJJoiner
shj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid);

PMJJoiner
pmj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid);

RippleJoiner
rpj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid);

#endif //ALLIANCEDB_LOCALJOINER_H
