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

enum joiner {
    type_SHJJoiner, type_PMJJoiner, type_RippleJoiner
};

class localJoiner {

public:
    virtual long join(int32_t tid, tuple_t *tuple,
                      bool tuple_R, hashtable_t *htR, hashtable_t *htS, int64_t *matches,
                      void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *), void *pVoid,
                      T_TIMER *timer) = 0;

    virtual void clean(int32_t tid, tuple_t *tuple, hashtable_t *htR, hashtable_t *htS, bool cleanR) = 0;
};


class RippleJoiner : public localJoiner {

private:
    relation_t *relR;
private:
    relation_t *relS;

    t_window_list samList;


public:
    long
    join(int32_t tid, tuple_t *tuple, bool tuple_R, hashtable_t *htR, hashtable_t *htS, int64_t *matches,
         void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *), void *pVoid,
         T_TIMER *timer) override;

    RippleJoiner(relation_t *relR, relation_t *relS, int nthreads);

    void clean(int32_t tid, tuple_t *tuple, hashtable_t *htR, hashtable_t *htS, bool cleanR) override;
};

struct t_pmjjoiner {

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

    void initialize(int sizeR, int sizeS) {
        /***Initialize***/
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

class PMJJoiner : public localJoiner {

public:
    t_pmjjoiner *t_arg;

    PMJJoiner(int sizeR, int sizeS, int nthreads);

    long
    join(int32_t tid, tuple_t *tuple, bool IStuple_R, hashtable_t *htR, hashtable_t *htS, int64_t *matches,
         void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *), void *pVoid,
         T_TIMER *timer) override;

    void clean(int32_t tid, tuple_t *tuple, hashtable_t *htR, hashtable_t *htS, bool cleanR) override;
};

class SHJJoiner : public localJoiner {

public:
    long
    join(int32_t tid, tuple_t *tuple, bool tuple_R, hashtable_t *htR, hashtable_t *htS, int64_t *matches,
         void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *), void *pVoid,
         T_TIMER *timer) override;

    void clean(int32_t tid, tuple_t *tuple, hashtable_t *htR, hashtable_t *htS, bool cleanR) override;
};

long
shj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid, T_TIMER *timer);

long
pmj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid, T_TIMER *timer);

long
rpj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid, T_TIMER *timer);

#endif //ALLIANCEDB_LOCALJOINER_H
