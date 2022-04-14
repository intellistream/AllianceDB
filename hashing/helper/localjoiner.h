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
#include "../joins/count_min_sketch.h"         /* constant parameters */
#include "../joins/common_functions.h"
#include "pmj_helper.h"
#include <list>
#include <queue>

#define SAMPLE_ON
#define NO_SET_PR
#define NO_ALWAYS_PROBE
#define PRESAMPLE
#define AVX_RAND
#define NO_MEM_LIM
#define NO_RESERVOIR_STRATA
#define NO_MARTERIAL_SAMPLE
#define NO_PROBEHASH


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
    int que_head = 0;
    uint32_t *rand_que = NULL;
    int pre_smp_thrshld;
    int div_prmtr;
    uint32_t hash_a;
    uint32_t hash_b;
    int count_pre;
    std::unordered_map<intkey_t, int> *pre_smp[2];
    uint64_t gm[3][3];
    uint32_t Bernoulli_q;
    uint32_t Universal_p;
    uint32_t Epsilon;
    double Epsilon_d;
    double epsilon_r;
    double epsilon_s;
    int reservior_size;
    int rand_buffer_size;
    int presample_size;


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

    // uint32_t rand4reservoir();
    uint32_t rand4sample();

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
    uint32_t htR_Universal_p;
    uint32_t htR_Bernoulli_q;
    uint32_t htS_Universal_p;
    uint32_t htS_Bernoulli_q;

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

class PrioQueMember{
public:
    unsigned int est;
    tuple_t tuple;

    PrioQueMember(unsigned int _est, tuple_t _tuple) : est(_est), tuple(_tuple){}
    PrioQueMember() = default;

    bool operator < (const PrioQueMember &b) const
    {
        return est < b.est;
    }
    bool operator > (const PrioQueMember &b) const
    {
        return est > b.est;
    }
    bool operator == (const PrioQueMember &b) const
    {
        return est == b.est;
    }
    bool operator <= (const PrioQueMember &b) const
    {
        return est <= b.est;
    }
    bool operator >= (const PrioQueMember &b) const
    {
        return est >= b.est;
    }
};

class SHJJoiner : public baseJoiner {

private:
    hashtable_t *htR;
    hashtable_t *htS;
    
#ifdef RESERVOIR_STRATA
    hashtable_t *htR_strata[RESERVOIR_STRATA_NUM];
    hashtable_t *htS_strata[RESERVOIR_STRATA_NUM];
#endif

#ifdef PROBEHASH
    CountMinSketch skch[2];
    std::priority_queue<PrioQueMember, std::vector<PrioQueMember>, std::greater<PrioQueMember> > prio_q[2];
    tuple_t prb_buf[PROB_BUFF];
#endif
public:
    virtual ~SHJJoiner();

    SHJJoiner(int sizeR, int sizeS);

    void
    join(int32_t tid, tuple_t *tuple, bool ISTupleR, int64_t *matches,
            /*void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *),*/ void *pVoid) override;

    void clean(int32_t tid, tuple_t *tuple, bool cleanR) override;

};

SHJJoiner
shj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid);

PMJJoiner
pmj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid);

RippleJoiner
rpj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid);

#endif //ALLIANCEDB_LOCALJOINER_H
