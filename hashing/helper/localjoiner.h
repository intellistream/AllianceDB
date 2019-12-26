//
// Created by Shuhao Zhang on 1/11/19.
//

#ifndef ALLIANCEDB_LOCALJOINER_H
#define ALLIANCEDB_LOCALJOINER_H

#include "../utils/xxhash64.h"
#include "../utils/t_timer.h"  /* startTimer, stopTimer */
#include "../utils/generator.h"          /* numa_localize() */
#include "../joins/npj_types.h"          /* bucket_t, hashtable_t, bucket_buffer_t */
#include "../joins/npj_params.h"         /* constant parameters */
#include "../joins/common_functions.h"
#include "pmj_helper.h"
#include <list>

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
                      void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *), void *pVoid) = 0;

    virtual long join(int32_t tid, tuple_t **fat_tuple, bool IStuple_R, int64_t *matches,
                      void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *), void *pVoid) {
        //only supported by PMJ.
    }

    virtual long
    cleanup(int32_t tid, int64_t *matches, void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *),
            void *pVoid) {
        //do nothing.
    }

    virtual void clean(int32_t tid, tuple_t *tuple, bool cleanR) = 0;

    virtual void clean(int32_t tid, tuple_t **tuple, bool cleanR) {
        //only used in PMJ.
    }

//every joiner has its own timer --> this completely avoids interference.
//dump the measurement into file and analysis the results when program exit.
    int64_t matches = 0;
#ifndef NO_TIMING
    T_TIMER timer;
#endif
};


class RippleJoiner : public baseJoiner {

private:
    relation_t *relR;
    relation_t *relS;
    t_window_list samList;


public:
    void
    join(int32_t tid, tuple_t *tuple, bool tuple_R, int64_t *matches,
         void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *), void *pVoid) override;

    RippleJoiner(relation_t *relR, relation_t *relS, int nthreads);

    void clean(int32_t tid, tuple_t *tuple, bool cleanR) override;
};

struct t_pmj {

    tuple_t *tmp_relR;
    int innerPtrR;
    int outerPtrR;
    int sizeR;
    tuple_t *tmp_relS;
    int innerPtrS;
    int outerPtrS;
    int sizeS;
    std::vector<run> Q;//let Q be an empty set;

    size_t relRsz;
    tuple_t *outptrR;
    tuple_t *outptrS;

    /***Initialize***/
    t_pmj(int sizeR, int sizeS) {
        /**** allocate temporary space for sorting ****/
        this->sizeR = sizeR;
        this->sizeS = sizeS;
        relRsz = sizeR * sizeof(tuple_t)
                 + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
        tmp_relR = (tuple_t *) malloc_aligned(relRsz);

        outptrR = (tuple_t *) malloc_aligned(relRsz);

        relRsz = sizeS * sizeof(tuple_t)
                 + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
        tmp_relS = (tuple_t *) malloc_aligned(relRsz);
        outptrS = (tuple_t *) malloc_aligned(relRsz);

        innerPtrR = 0;
        outerPtrR = 0;
        innerPtrS = 0;
        outerPtrS = 0;
    }
};

class PMJJoiner : public baseJoiner {
private:
    t_pmj *t_arg;
public:

    PMJJoiner(int sizeR, int sizeS, int nthreads);

    void
    join(int32_t tid, tuple_t *tuple, bool IStuple_R, int64_t *matches,
         void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *), void *pVoid);

    long join(int32_t tid, tuple_t **fat_tuple, bool IStuple_R, int64_t *matches,
              void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *), void *pVoid);

    long
    cleanup(int32_t tid, int64_t *matches, void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *),
            void *pVoid) override;

    void clean(int32_t tid, tuple_t *tuple, bool cleanR) override {
        //not implemented.
    }


    void clean(int32_t tid, tuple_t **tuple, bool cleanR);
};

class SHJJoiner : public baseJoiner {

private:
    hashtable_t *htR;
    hashtable_t *htS;

public:
    virtual ~SHJJoiner();

    SHJJoiner(int sizeR, int sizeS);

    void
    join(int32_t tid, tuple_t *tuple, bool tuple_R, int64_t *matches,
         void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *), void *pVoid) override;

    void clean(int32_t tid, tuple_t *tuple, bool cleanR) override;
};

SHJJoiner
shj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid);

PMJJoiner
pmj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid);

long
rpj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid, T_TIMER *timer);

#endif //ALLIANCEDB_LOCALJOINER_H
