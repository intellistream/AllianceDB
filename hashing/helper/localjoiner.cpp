//
// Created by Shuhao Zhang on 1/11/19.
//

#include <cmath>
#include <vector>
#include <assert.h>
#include <zconf.h>
#include "avxsort.h"
#include "sort_common.h"
#include "localjoiner.h"
#include "pmj_helper.h"
#include "../joins/eagerjoin_struct.h"
#include <pxart/pxart.hpp>
#include <random>
#include <array>
#include <pxart/simd256/mt19937.hpp>
#include <pxart/utility/pun_cast.hpp>
/**
 * As an example of join execution, consider a join with join predicate T1.attr1 = T2.attr2.
 * The join operator will incrementally load a hash table H1 for T1 by hashing attr1 using hash function f1,
 * and another hash table H2 for T2 by hashing attr2 using hash function f2.
 * The symmetric hash join operator starts by
 * (1): getting a tuple from T1, hashing its attr1 field using f1, and inserting it into H1.
 * (2): it probes H2 by applying f2 to attr1 of the current T1 tuple, returning any matched tuple pairs.
 *
 * (3): it gets a tuple from T2, hashes it by applying f2 to attr2, and inserts it into H2.
 * (4): it probes H1 by applying f1 to attr2 of the current T2 tuple, and returns any matches.
 * (5): This continues until all tuples from T1 and T2 have been consumed.
 *
 * @param relR
 * @param relS
 * @param nthreads
 * @return
 */
SHJJoiner
shj(int32_t tid, relation_t* rel_R, relation_t* rel_S, void* pVoid) {

    SHJJoiner joiner(rel_R->num_tuples, rel_S->num_tuples);
    uint32_t index_R = 0;//index of rel_R
    uint32_t index_S = 0;//index of rel_S


#ifndef NO_TIMING
    START_MEASURE(joiner.timer)
#endif
    do {
        if (index_R < rel_R->num_tuples) {
            joiner.join(tid, &rel_R->tuples[index_R], true, &joiner.matches, /*NULL,*/ pVoid);
            index_R++;
        }
        if (index_S < rel_S->num_tuples) {
            joiner.join(tid, &rel_S->tuples[index_S], false, &joiner.matches,/* NULL,*/ pVoid);
            index_S++;
        }
    } while (index_R < rel_R->num_tuples || index_S < rel_S->num_tuples);

#ifndef NO_TIMING
    END_MEASURE(joiner.timer)
#endif

    return joiner;
}


// uint32_t rand4sample(int &que_head, uint32_t *&rand_que)
uint32_t baseJoiner::rand4sample()
{
#ifdef AVX_RAND
    using rand_t = uint32_t;
    static rand_t mod = int(1e9)+6, quelen = RANDOM_BUFFER_SIZE;
    // static rand_t *rand_que = NULL;
    static pxart::simd256::mt19937 rng{random_device{}};
    static constexpr auto rand_stp = sizeof(rng()) / sizeof(rand_t);


    if (rand_que == NULL)
    {
        rand_que = (rand_t *)malloc_aligned(sizeof(rand_t)*quelen+256);
    }
    if (que_head == quelen)
        que_head = 0;
    if (que_head == 0)
    {
        for (int i = 0; i < quelen; i += rand_stp)
        {
            // const auto vrnd = pxart::simd256::uniform<type>(rng, -9, 9);
            const auto vrnd = pxart::uniform<rand_t>(rng, 0, mod);
            // const auto srnd = pxart::pun_cast<array<rand_t, rand_stp>>(vrnd);
            // for (int j = 0; j < rand_stp; ++j) {
            //     rand_que[i+j] = srnd[j];
            memcpy(rand_que+i, &vrnd, sizeof(vrnd));
        }
    }
    return rand_que[que_head++];
#endif

#ifdef NO_AVX_RAND
    static uint32_t mod = int(1e9)+7;
    return rand()%mod;
#endif
}

uint32_t rand4reservoir(int &que_head, uint32_t *&rand_que)
// uint32_t baseJoiner::rand4reservoir()
{
#ifdef AVX_RAND
    using rand_t = uint32_t;
    static int quelen = RANDOM_BUFFER_SIZE;
    static int atc_res_size = RESERVOIR_SIZE/RESERVOIR_STRATA_NUM;
    // static rand_t *rand_que = NULL;
    static pxart::simd256::mt19937 rng{random_device{}};
    static constexpr auto rand_stp = sizeof(rng()) / sizeof(rand_t);


    if (rand_que == NULL)
    {
        rand_que = (rand_t *)malloc_aligned(sizeof(rand_t)*quelen+256);
    }
    if (que_head == quelen)
        que_head = 0;
    if (que_head == 0)
    {
        for (int i = 0; i < quelen; i += rand_stp)
        {
            // const auto vrnd = pxart::simd256::uniform<type>(rng, -9, 9);

#ifdef RESERVOIR_STRATA
            const auto vrnd = pxart::uniform<rand_t>(rng, 0, atc_res_size - 1);
#else
            const auto vrnd = pxart::uniform<rand_t>(rng, 0, RESERVOIR_SIZE - 1);
#endif
            // const auto srnd = pxart::pun_cast<array<rand_t, rand_stp>>(vrnd);
            // for (int j = 0; j < rand_stp; ++j) {
            //     rand_que[i+j] = srnd[j];
            memcpy(rand_que+i, &vrnd, sizeof(vrnd));
        }
    }
    return rand_que[que_head++];
#endif

#ifdef NO_AVX_RAND
    return rand()%RESERVOIR_SIZE;
#endif

}

/*

uint32_t rand4sample(int &que_head, uint32_t *&rand_que)
{
    static uint32_t res = 19260817, a = int(1e7)+9, mod = int(1e9)+7, b = 233;
    res = (1ll*res*a+b)%mod;
    return res;
}
*/


/**
 * SHJ algorithm to be used in each thread.
 * @param tid
 * @param tuple
 * @param ISTupleR
 * @param htR
 * @param htS
 * @param matches
 * @param pVoid
 * @param timer
 * @return
 */
void SHJJoiner::join(int32_t tid, tuple_t* tuple, bool ISTupleR, int64_t* matches,
    /*void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *),*/ void* out) {

    const uint32_t hashmask_R = htR->hash_mask;
    const uint32_t skipbits_R = htR->skip_bits;
    const uint32_t hashmask_S = htS->hash_mask;
    const uint32_t skipbits_S = htS->skip_bits;

#ifdef PROBEHASH
#endif

#ifdef SAMPLE_ON
    static int mod = 1e9+7;
#ifdef PRESAMPLE
    
    if(count_pre < PRESAMPLING_SIZE)
    {
        count_pre++;

        if (ISTupleR)
        {
            int tmp = (*(pre_smp[0]))[tuple->key];
            int tt = (*(pre_smp[1]))[tuple->key];
            (*(pre_smp[0]))[tuple->key] = tmp + 1;
            gm[1][1] += tt;
            gm[1][2] += 1ll*tt*tt;
            gm[2][1] += 2ll*tmp*tt + tt;
            gm[2][2] += 2ll*tmp*tt*tt + 1ll*tt*tt;

        }
        else
        {
            int tmp = (*(pre_smp[1]))[tuple->key];
            int tt = (*(pre_smp[0]))[tuple->key];
            (*(pre_smp[1]))[tuple->key] = tmp + 1;
            gm[1][1] += tt;
            gm[1][2] += 2ll*tmp*tt + tt;
            gm[2][1] += 1ll*tt*tt;
            gm[2][2] += 2ll*tmp*tt*tt + 1ll*tt*tt;

        }
        
        // htR->pre_set->insert(tuple->key);
        if (count_pre == PRESAMPLING_SIZE)
        {
            // if(htR->pre_set->size() > htR->div_prmtr)
            // {
            //     htR->Universal_p = (mod*1);
            //     htR->Bernoulli_q = int(mod*0.001);
            // }
            // else
            // {
            //     htR->Universal_p = (mod*1);
            //     htR->Bernoulli_q = int(mod*0.001);
            // }
            
#ifdef CORRECTION_P
            double pre_r = count_pre*1.0 / (sizeR + sizeS); 
            double p = ((gm[2][2]/(pre_r*pre_r) - gm[2][1]/pre_r - gm[1][2]/pre_r)/gm[1][1] + 1)*epsilon_r*epsilon_s;
#else
            double p = (1.0*(gm[2][2] - gm[2][1] - gm[1][2])/gm[1][1] + 1)*epsilon_r*epsilon_s;
#endif

            p = sqrt(p);
            p = max(p, epsilon_r);
            p = max(p, epsilon_s);
            p = min(1.0, p);
            htR->Universal_p = p*mod;
            htR->Bernoulli_q = epsilon_r/p*mod;
            htS->Universal_p = p*mod;
            htS->Bernoulli_q = epsilon_s/p*mod;
            htR->Data_utilization = data_utilization_r*mod;
            htS->Data_utilization = data_utilization_s*mod;

            hash_key_p = p;
        }

        if(rand4sample() >= Epsilon)
        {
#ifdef ALWAYS_PROBE
#ifdef MATCH
            if (ISTupleR)
                probe_hashtable_single(htS, tuple, hashmask_S, skipbits_S, matches, /*thread_fun,*/ timer, ISTupleR,
                                out);//(2)
            else
                probe_hashtable_single(htR, tuple, hashmask_R, skipbits_R, matches, /*thread_fun,*/ timer, ISTupleR,
                               out);//(4)
#endif
#endif
            return;
        }
    }
    else
    {
        if (ISTupleR)
        {
            if((1ll*tuple->key*tuple->key%mod*hash_a%mod + hash_b)%mod >= htR->Universal_p )
            {
// #ifdef ALWAYS_PROBE
// #ifdef MATCH
//                     probe_hashtable_single(htS, tuple, hashmask_S, skipbits_S, matches, /*thread_fun,*/ timer, ISTupleR,
//                                     out);//(2)
// #endif
// #endif
                return;
            }
            if(rand4sample() >= htR->Bernoulli_q)
            {
#ifdef MATCH
#ifdef ALWAYS_PROBE
#else
                if (rand4sample() < htR->Data_utilization)
#endif
                    probe_hashtable_single(htS, tuple, hashmask_S, skipbits_S, matches, /*thread_fun,*/ timer, ISTupleR,
                                out);//(2)
#endif
                return;
            }
        }
        else
        {
            if((1ll*tuple->key*tuple->key%mod*hash_a%mod + hash_b)%mod >= htS->Universal_p )
            {
// #ifdef ALWAYS_PROBE
// #ifdef MATCH
//                     probe_hashtable_single(htR, tuple, hashmask_R, skipbits_R, matches, /*thread_fun,*/ timer, ISTupleR,
//                                 out);//(4)
// #endif
// #endif
                return;
            }
            if(rand4sample() >= htS->Bernoulli_q)
            {
#ifdef MATCH
#ifdef ALWAYS_PROBE
#else
                if (rand4sample() < htS->Data_utilization)
#endif
                    probe_hashtable_single(htR, tuple, hashmask_R, skipbits_R, matches, /*thread_fun,*/ timer, ISTupleR,
                                out);//(4)
#endif
                return;
            }
        }
    }
// SET PR
#else 
    if (count_pre == 0)
    {
        count_pre++;
        htR->Universal_p = Universal_p;
        htR->Bernoulli_q = Bernoulli_q;
        htS->Universal_p = Universal_p;
        htS->Bernoulli_q = Bernoulli_q;
        htR->Data_utilization = data_utilization_r*mod;
        htS->Data_utilization = data_utilization_s*mod;
    }

    if (ISTupleR)
    {
        if((1ll*tuple->key*tuple->key%mod*hash_a%mod + hash_b)%mod >= htR->Universal_p )
        {
// #ifdef ALWAYS_PROBE
// #ifdef MATCH
//                     probe_hashtable_single(htS, tuple, hashmask_S, skipbits_S, matches, /*thread_fun,*/ timer, ISTupleR,
//                                     out);//(2)
// #endif
// #endif
            return;
        }
        if(rand4sample() >= htR->Bernoulli_q)
        {
#ifdef MATCH
#ifdef ALWAYS_PROBE
#else
            if (rand4sample() < htR->Data_utilization)
#endif
                probe_hashtable_single(htS, tuple, hashmask_S, skipbits_S, matches, /*thread_fun,*/ timer, ISTupleR,
                                out);//(2)
#endif
            return;
        }
    }
    else
    {
        if((1ll*tuple->key*tuple->key%mod*hash_a%mod + hash_b)%mod >= htS->Universal_p )
        {
// #ifdef ALWAYS_PROBE
// #ifdef MATCH
//                     probe_hashtable_single(htR, tuple, hashmask_R, skipbits_R, matches, /*thread_fun,*/ timer, ISTupleR,
//                                 out);//(4)
// #endif
// #endif
            return;
        }
        if(rand4sample() >= htS->Bernoulli_q)
        {
#ifdef MATCH
#ifdef ALWAYS_PROBE
#else
            if (rand4sample() < htS->Data_utilization)
#endif
                probe_hashtable_single(htR, tuple, hashmask_R, skipbits_R, matches, /*thread_fun,*/ timer, ISTupleR,
                            out);//(4)
#endif
            return;
        }
    }
#endif
#endif

#ifdef PROBEHASH
    static int mod = 1e9 + 7;
    int q_on = ISTupleR ^ 1, q_off = ISTupleR;
    hashtable_t *ht[2] = {htR, htS};
    const uint32_t hshmsk[2] = {hashmask_R, hashmask_S};
    const uint32_t skpbits[2] = {skipbits_R, skipbits_S};
    ht[q_on]->cnt++;
    double w_exp = 1.0 / skch[q_off].estimate(tuple->key);
    double u_base = rand4sample() / double(mod);
    prio_q[q_on].push( PrioQueMember(-pow(u_base, w_exp), *tuple) );
    if (ht[q_on]->cnt > RESERVOIR_SIZE)
    {
        ht[q_on]->cnt--;
        tuple_t tmp4del = prio_q[q_on].top().tuple;
        prio_q[q_on].pop();

        delete_hashtable_single(ht[q_on], &tmp4del, hshmsk[q_on], skpbits[q_on]);
    }
#endif

#ifdef RESERVOIR_STRATA
    int stra = rand4sample()%RESERVOIR_STRATA_NUM;
    static int atc_res_size = RESERVOIR_SIZE/RESERVOIR_STRATA_NUM;
    
    fflush(stderr);
    if (ISTupleR) {
        
// #ifdef SAMPLE_ON
        if (htR_strata[stra]->cnt < atc_res_size)
            htR_strata[stra]->rsv[htR_strata[stra]->cnt] = *tuple;
            
        htR_strata[stra]->cnt++;
        if (htR_strata[stra]->cnt > atc_res_size)
        {
            if (rand4sample()%htR_strata[stra]->cnt > atc_res_size)
            {
#ifdef MATCH
#ifdef ALWAYS_PROBE
#else
                if (rand4sample() < htR->Data_utilization)
#endif
                    probe_hashtable_single(htS_strata[stra], tuple, hashmask_S, skipbits_S, matches, /*thread_fun,*/ timer, ISTupleR, out);//(2)
#endif
                return;
            }
            int del_ind = rand4reservoir(htR_strata[stra]->rsv_que_head, htR_strata[stra]->rsv_rand_que);
            
            tuple_t tmp4del = htR_strata[stra]->rsv[del_ind];
            htR_strata[stra]->rsv[del_ind] = htR_strata[stra]->rsv[atc_res_size];
            delete_hashtable_single(htR_strata[stra], &tmp4del, hashmask_R, skipbits_R);
        }

        build_hashtable_single(htR_strata[stra], tuple, hashmask_R, skipbits_R);//(1)
#ifdef MATCH
        probe_hashtable_single(htS_strata[stra], tuple, hashmask_S, skipbits_S, matches, /*thread_fun,*/ timer, ISTupleR, out);//(2)
#endif
    } else {

// #ifdef SAMPLE_ON
        if (htS_strata[stra]->cnt < atc_res_size)
           htS_strata[stra]->rsv[htS_strata[stra]->cnt] = *tuple;
           
        htS_strata[stra]->cnt++;
        if (htS_strata[stra]->cnt > atc_res_size)
        {
            if (rand4sample()%htS_strata[stra]->cnt > atc_res_size)
            {
#ifdef MATCH
#ifdef ALWAYS_PROBE
#else
                if (rand4sample() < htS->Data_utilization)
#endif
                    probe_hashtable_single(htR_strata[stra], tuple, hashmask_R, skipbits_R, matches, /*thread_fun,*/ timer, ISTupleR, out);//(4)
#endif
                return;
            }
            int del_ind = rand4reservoir(htS_strata[stra]->rsv_que_head, htS_strata[stra]->rsv_rand_que);
            tuple_t tmp4del = htS_strata[stra]->rsv[del_ind];
            htS_strata[stra]->rsv[del_ind] = htS_strata[stra]->rsv[atc_res_size];
            delete_hashtable_single(htS_strata[stra], &tmp4del, hashmask_S, skipbits_S);
        }
// #endif

        build_hashtable_single(htS_strata[stra], tuple, hashmask_S, skipbits_S);//(3)
#ifdef MATCH
        probe_hashtable_single(htR_strata[stra], tuple, hashmask_R, skipbits_R, matches, /*thread_fun,*/ timer, ISTupleR, out);//(4)
#endif
    }

    return;
#endif

    if (ISTupleR) {
        
// #ifdef SAMPLE_ON
#ifdef MEM_LIM
        if (htR->cnt < RESERVOIR_SIZE)
            htR->rsv[htR->cnt] = *tuple;
            
        htR->cnt++;
        if (htR->cnt > RESERVOIR_SIZE)
        {
            if (rand4sample()%htR->cnt > RESERVOIR_SIZE)
            {
#ifdef MATCH
#ifdef ALWAYS_PROBE
#else
                if (rand4sample() < htR->Data_utilization)
#endif
                    probe_hashtable_single(htS, tuple, hashmask_S, skipbits_S, matches, /*thread_fun,*/ timer, ISTupleR, out);//(2)
#endif
                return;
            }
            int del_ind = rand4reservoir(htR->rsv_que_head, htR->rsv_rand_que);
            
            tuple_t tmp4del = htR->rsv[del_ind];
            htR->rsv[del_ind] = htR->rsv[RESERVOIR_SIZE];
            delete_hashtable_single(htR, &tmp4del, hashmask_R, skipbits_R);
        }
#endif
// #endif

        build_hashtable_single(htR, tuple, hashmask_R, skipbits_R);//(1)
#ifdef MATCH
        probe_hashtable_single(htS, tuple, hashmask_S, skipbits_S, matches, /*thread_fun,*/ timer, ISTupleR, out);//(2)
#endif
    } else {

// #ifdef SAMPLE_ON
#ifdef MEM_LIM
        if (htS->cnt < RESERVOIR_SIZE)
           htS->rsv[htS->cnt] = *tuple;
           
        htS->cnt++;
        if (htS->cnt > RESERVOIR_SIZE)
        {
            if (rand4sample()%htS->cnt > RESERVOIR_SIZE)
            {
#ifdef MATCH
#ifdef ALWAYS_PROBE
#else
                if (rand4sample() < htS->Data_utilization)
#endif
                    probe_hashtable_single(htR, tuple, hashmask_R, skipbits_R, matches, /*thread_fun,*/ timer, ISTupleR, out);//(4)
#endif
                return;
            }
            int del_ind = rand4reservoir(htS->rsv_que_head, htS->rsv_rand_que);
            tuple_t tmp4del = htS->rsv[del_ind];
            htS->rsv[del_ind] = htS->rsv[RESERVOIR_SIZE];
            delete_hashtable_single(htS, &tmp4del, hashmask_S, skipbits_S);
        }
#endif
// #endif

        build_hashtable_single(htS, tuple, hashmask_S, skipbits_S);//(3)
#ifdef MATCH
        probe_hashtable_single(htR, tuple, hashmask_R, skipbits_R, matches, /*thread_fun,*/ timer, ISTupleR, out);//(4)
#endif
    }
}

/**     
 * Clean state stored in local thread, basically used in HS mode
 * @param tid
 * @param tuple
 * @param htR
 * @param htS
 * @param cleanR
 */
void SHJJoiner::clean(int32_t tid, tuple_t* tuple, bool cleanR) {
    if (cleanR) {
        //if SHJ is used, we need to clean up hashtable of R.
        debuild_hashtable_single(htR, tuple, htR->hash_mask, htR->skip_bits);
    } else {
        debuild_hashtable_single(htS, tuple, htS->hash_mask, htS->skip_bits);
    }
}

SHJJoiner::SHJJoiner(int sizeR, int sizeS) {
    //allocate two hashtables.
    this->sizeR = sizeR;
    this->sizeS = sizeS;
    int nbucketsR = sizeR/BUCKET_SIZE;
#ifdef RESERVOIR_STRATA
    for (int i = 0; i < RESERVOIR_STRATA_NUM; ++i)
        allocate_hashtable(&(htR_strata[i]), nbucketsR);
// #else
#endif
    allocate_hashtable(&htR, nbucketsR);

    assert(nbucketsR > 0);
    int nbucketsS = sizeS/BUCKET_SIZE;
#ifdef RESERVOIR_STRATA
    for (int i = 0; i < RESERVOIR_STRATA_NUM; ++i)
        allocate_hashtable(&(htS_strata[i]), nbucketsS);
#endif
// #else
    allocate_hashtable(&htS, nbucketsS);
    assert(nbucketsS > 0);
}

SHJJoiner::~SHJJoiner() {
    destroy_hashtable(htR);
    destroy_hashtable(htS);
}

/**
 *  The main idea of PMJ is to read as much data as can fit in memory.
 *  Then, in-memory data is sorted and is joined together, and then is flushed into disk.
 *  When all data is received, PMJ joins the disk-resident data using a refinement version
 *  of the sort-merge join that allows producing join results while merging.
 *
 *  We change it to read up to progressive_step of data. Then sort and join, and then push aside at rest.
 *  When all data is received, join the rest data using refinement version of SMJ.
 *
 * @param tid
 * @param rel_R
 * @param rel_S
 * @param pVoid
 * @param timer
 * @return
 */
PMJJoiner
pmj(int32_t tid, relation_t* rel_R, relation_t* rel_S, void* output) {

    PMJJoiner joiner(rel_R->num_tuples, rel_S->num_tuples, 1);

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t *chainedbuf = (chainedtuplebuffer_t *) output;
#else
    chainedtuplebuffer_t* chainedbuf = nullptr;
#endif

#ifndef NO_TIMING
    START_MEASURE(joiner.timer)
#endif
    //Phase 1 ('Join during run creation')
    int sizeR = rel_R->num_tuples;
    int sizeS = rel_S->num_tuples;
    int i = 0;
    int j = 0;

    int progressive_stepR =
        joiner.progressive_step_R;//ALIGN_NUMTUPLES((int) (progressive_step * sizeR));//cacheline aligned.
    int progressive_stepS = joiner.progressive_step_S;//ALIGN_NUMTUPLES((int) (progressive_step * sizeS));

    assert(progressive_stepR > 0 && progressive_stepS > 0);

    std::vector<run> Q;//let Q be an empty set;

    /***Initialize***/
    /**** allocate temporary space for sorting ****/
    size_t relRsz;
    tuple_t* outptrR;
    tuple_t* outptrS;

    relRsz = sizeR*sizeof(tuple_t)
        + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.

    outptrR = (tuple_t*) malloc_aligned(relRsz);

    relRsz = sizeS*sizeof(tuple_t)
        + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.

    outptrS = (tuple_t*) malloc_aligned(relRsz);

    /***Sorting***/
    do {
        sorting_phase(tid, rel_R, rel_S, sizeR, sizeS, progressive_stepR, progressive_stepS, &i, &j, &joiner.matches,
                      &Q, outptrR + i, outptrS + j, joiner.timer, chainedbuf);

    } while (i < sizeR - progressive_stepR && j < sizeS - progressive_stepS);//while R!=null, S!=null.

    /***Handling Left-Over***/
    progressive_stepR = sizeR - i;
    progressive_stepS = sizeS - j;
    sorting_phase(tid, rel_R, rel_S, sizeR, sizeS, progressive_stepR, progressive_stepS, &i, &j, &joiner.matches, &Q,
                  outptrR + i, outptrS + j, joiner.timer, chainedbuf);

    //    MSG("Join during run creation:%ld, Q size:%ld\n", joiner.matches, Q.size());

    merging_phase(&joiner.matches, &Q, joiner.timer, chainedbuf);

#ifndef NO_TIMING
    END_MEASURE(joiner.timer)
#endif
    //    MSG("Join during run merge matches:%ld\n", joiner.matches);
    return joiner;
}

void PMJJoiner::keep_tuple_single(tuple_t* tmp_rel, const int outerPtr,
                                  tuple_t* tuple, int fat_tuple_size) {
    for (auto i = 0; i < fat_tuple_size; i++) {
        tmp_rel[outerPtr + i] = tuple[i];//deep copy tuples.
        //        tmp_rel[outerPtr + i].key = -1;//to check if it's deep copy.
        //        DEBUGMSG("%d, %d", std::addressof( tmp_rel[outerPtr + i]), std::addressof(tuple[i]))
    }
}

void
PMJJoiner::join_tuple_single(int32_t tid, tuple_t* tmp_rel, int* outerPtr, tuple_t* tuple, int fat_tuple_size,
                             int64_t* matches,
                             T_TIMER* timer, t_pmj* pPmj, bool IStuple_R, chainedtuplebuffer_t* chainedbuf) {

    if (*outerPtr > 0 && fat_tuple_size > 0) {
        tuple_t* out_relR;
        tuple_t* out_relS;
        if (IStuple_R) {
            auto relRsz = fat_tuple_size*sizeof(tuple_t)
                + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
            out_relR = (tuple_t*) malloc_aligned(relRsz);

            relRsz = *outerPtr*sizeof(tuple_t)
                + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
            out_relS = (tuple_t*) malloc_aligned(relRsz);

            sorting_phase(tid,
                          tuple,
                          fat_tuple_size,
                          tmp_rel,
                          *outerPtr,
                          matches,
                          &pPmj->Q,
                          out_relR,
                          out_relS,
                          timer, chainedbuf);

            pPmj->extraPtrR += fat_tuple_size;
            pPmj->extraPtrS += *outerPtr;

        } else {
            auto relRsz = fat_tuple_size*sizeof(tuple_t)
                + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
            out_relS = (tuple_t*) malloc_aligned(relRsz);

            relRsz = *outerPtr*sizeof(tuple_t)
                + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
            out_relR = (tuple_t*) malloc_aligned(relRsz);

            sorting_phase(tid,
                          tmp_rel,
                          *outerPtr,
                          tuple,
                          fat_tuple_size,
                          matches,
                          &pPmj->Q,
                          out_relR,
                          out_relS,
                          timer, chainedbuf);
            pPmj->extraPtrR += *outerPtr;
            pPmj->extraPtrS += fat_tuple_size;
        }
    }
}

/**
 * PMJ algorithm to be used in each thread.
 * First store enough tuples from R and S, then call PMJ algorithm.
 * This is outdated and previously used in HS scheme.
 * @param tid
 * @param tuple
 * @param IStuple_R
 * @param htR
 * @param htS
 * @param matches
 * @param pVoid
 * @param timer
 * @return
 */
void PMJJoiner::join(int32_t tid, tuple_t* tuple, int fat_tuple_size, bool IStuple_R, int64_t* matches,
    /*void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *),*/ void* output) {
    auto* arg = (t_pmj*) t_arg;

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t *chainedbuf = (chainedtuplebuffer_t *) output;
#else
    chainedtuplebuffer_t* chainedbuf = nullptr;
#endif

    //store tuples.
    if (IStuple_R) {
        DEBUGMSG("TID %d: before store R is %s.", tid,
                 print_tuples(arg->tmp_relR, arg->outerPtrR).c_str())

        keep_tuple_single(arg->tmp_relR, arg->outerPtrR, tuple, fat_tuple_size);
        arg->outerPtrR += fat_tuple_size;

        DEBUGMSG("TID %d: after store R is %s.", tid,
                 print_tuples(arg->tmp_relR, arg->outerPtrR).c_str())

        DEBUGMSG("TID: %d Sorting in normal stage start", tid)

        auto inputS = copy_tuples(arg->tmp_relS, arg->outerPtrS);

        join_tuple_single(tid, inputS, &arg->outerPtrS,
                          tuple, fat_tuple_size, matches, timer, arg,
                          IStuple_R, chainedbuf);
        DEBUGMSG("TID: %d Join during run creation:%d", tid, *matches)

    } else {
        DEBUGMSG("TID %d: before store S is %s.", tid,
                 print_tuples(arg->tmp_relS, arg->outerPtrS).c_str())

        keep_tuple_single(arg->tmp_relS, arg->outerPtrS, tuple, fat_tuple_size);
        arg->outerPtrS += fat_tuple_size;

        DEBUGMSG("TID %d: after store S is %s.", tid,
                 print_tuples(arg->tmp_relS, arg->outerPtrS).c_str())

        DEBUGMSG("TID: %d Sorting in normal stage start", tid)
        auto inputR = copy_tuples(arg->tmp_relR, arg->outerPtrR);
        join_tuple_single(tid, inputR, &arg->outerPtrR, tuple, fat_tuple_size,
                          matches, timer, arg, IStuple_R, chainedbuf);
        DEBUGMSG("TID: %d Join during run creation:%d", tid, *matches)

    }

}

void PMJJoiner::clean(int32_t tid, tuple_t* tuple, bool cleanR) {
    std::cout << "this method should not be called" << std::endl;
    //    if (cleanR) {
    //        auto idx = find_index(this->t_arg->tmp_relR, this->t_arg->outerPtrR + this->t_arg->innerPtrR, tuple);
    //        this->t_arg->tmp_relR[idx] = this->t_arg->tmp_relR[this->t_arg->outerPtrR + this->t_arg->innerPtrR - 1];
    //        this->t_arg->innerPtrR--;
    //        if (this->t_arg->innerPtrR < 0) {
    //            this->t_arg->outerPtrR -= progressive_step_tupleR;
    //            this->t_arg->innerPtrR = progressive_step_tupleR - 1;
    //        }
    //
    //#ifdef DEBUG
    //        if (tid == 0) {
    //            window0.R_Window.remove(tuple->key);
    //            DEBUGMSG("T0 after remove R (expected): %s, actual: %s", print_window(window0.R_Window).c_str(),
    //                     print_tuples(this->t_arg->tmp_relR, this->t_arg->outerPtrR + this->t_arg->innerPtrR).c_str())
    //        } else {
    //            window1.R_Window.remove(tuple->key);
    //            DEBUGMSG("T1 after remove R (expected): %s, actual: %s", print_window(window1.R_Window).c_str(),
    //                     print_tuples(this->t_arg->tmp_relR, this->t_arg->outerPtrR + this->t_arg->innerPtrR).c_str())
    //        }
    //#endif
    //
    //    } else {
    //        auto idx = find_index(this->t_arg->tmp_relS, this->t_arg->outerPtrS + this->t_arg->innerPtrS, tuple);
    //        this->t_arg->tmp_relS[idx] = this->t_arg->tmp_relS[this->t_arg->outerPtrS + this->t_arg->innerPtrS - 1];
    //        this->t_arg->innerPtrS--;
    //        if (this->t_arg->innerPtrS < 0) {
    //            this->t_arg->outerPtrS -= progressive_step_tupleS;
    //            this->t_arg->innerPtrS = progressive_step_tupleS - 1;
    //        }
    //
    //#ifdef DEBUG
    //        if (tid == 0) {
    //            window0.S_Window.remove(tuple->key);
    //            DEBUGMSG("T0 after remove S (expected): %s,(actual): %s", print_window(window0.S_Window).c_str(),
    //                     print_tuples(this->t_arg->tmp_relS, this->t_arg->outerPtrS + this->t_arg->innerPtrS).c_str())
    //        } else {
    //            window1.S_Window.remove(tuple->key);
    //            DEBUGMSG("T1 after remove S (expected): %s,(actual): %s", print_window(window1.S_Window).c_str(),
    //                     print_tuples(this->t_arg->tmp_relS, this->t_arg->outerPtrS + this->t_arg->innerPtrS).c_str())
    //        }
    //#endif
    //    }
}

/**
 * HS cleaner
 * @param tid
 * @param tuple
 * @param htR
 * @param htS
 * @param cleanR
 */
void PMJJoiner::
clean(int32_t tid, tuple_t* fat_tuple, int fat_tuple_size, bool cleanR) {

    if (cleanR) {

        DEBUGMSG("Start clean %d; TID:%d, Initial R: %s", fat_tuple[0].key, tid,
                 print_relation(this->t_arg->tmp_relR, this->t_arg->outerPtrR).c_str())

        for (auto i = 0; i < fat_tuple_size; i++) {
            /*
                            int idx = find_index(this->t_arg->tmp_relR,
                                                 this->t_arg->outerPtrR,
                                                 tuple[i]);
                            if (idx == -1) {
                                 DEBUGMSG(
                                        "Clean %d not find correct position. something is wrong. TID:%d, Initial R: %s",
                                        tuple[i]->key, tid, print_relation(this->t_arg->tmp_relR, this->t_arg->outerPtrR).c_str())
                                break;//something is wrong.
                            }

                            this->t_arg->tmp_relR[idx] = this->t_arg->tmp_relR[
                                    this->t_arg->outerPtrR - 1];
            //                remove++;
                            this->t_arg->outerPtrR--;
            */

        }
        this->t_arg->tmp_relR = &this->t_arg->tmp_relR[fat_tuple_size];
        this->t_arg->outerPtrR -= fat_tuple_size;
        DEBUGMSG("Stop clean %d; T%d left with:%s", fat_tuple[0].key, tid,
                 print_relation(this->t_arg->tmp_relR, this->t_arg->outerPtrR).c_str())

    } else {

        for (auto i = 0; i < fat_tuple_size; i++) {

            int idx = find_index(this->t_arg->tmp_relS, this->t_arg->outerPtrS, &fat_tuple[i]);
            if (idx == -1) {
                break;//it must be an ack signal, and does not stored in this thread.
            }

            this->t_arg->tmp_relS[idx] = this->t_arg->tmp_relS[this->t_arg->outerPtrS - 1];
            //                remove++;
            this->t_arg->outerPtrS--;

        }


        DEBUGMSG("Stop clean %d; T%d left with:%s", fat_tuple[0].key, tid,
                 print_relation(this->t_arg->tmp_relS, this->t_arg->outerPtrS).c_str())

        //#ifdef DEBUG
        //        if (tid == 0) {
        //            DEBUGMSG("T0 after remove S (expected): %s,(actual): %s", print_window(window0.S_Window).c_str(),
        //                     print_tuples(this->t_arg->tmp_relS,
        //                                  this->t_arg->outerPtrS).c_str())
        //        } else if (tid == 1) {
        //            DEBUGMSG("T1 after remove S (expected): %s,(actual): %s, size: %d", print_window(window1.S_Window).c_str(),
        //                     print_tuples(this->t_arg->tmp_relS,
        //                                  this->t_arg->outerPtrS).c_str(), this->t_arg->outerPtrS)
        //        }
        //#endif
    }
}

long PMJJoiner::
merge(int32_t tid, int64_t* matches,
    /*void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *),*/ void* output) {
    auto* arg = (t_pmj*) t_arg;
    int stepR;
    int stepS;
    /***Handling Left-Over***/
    DEBUGMSG("TID:%d in Clean up stage, current matches:%ld", tid, *matches)
    stepR = arg->innerPtrR;
    stepS = arg->innerPtrS;

    tuple_t* out_relR;
    tuple_t* out_relS;

    auto relRsz = stepR*sizeof(tuple_t)
        + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
    out_relR = (tuple_t*) malloc_aligned(relRsz);

    relRsz = stepS*sizeof(tuple_t)
        + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
    out_relS = (tuple_t*) malloc_aligned(relRsz);

#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t *chainedbuf = (chainedtuplebuffer_t *) output;
#else
    chainedtuplebuffer_t* chainedbuf = nullptr;
#endif

    sorting_phase(tid, arg->tmp_relR + arg->outerPtrR, stepR,
                  arg->tmp_relS + arg->outerPtrS, stepS,
                  matches, &arg->Q,
                  out_relR,
                  out_relS,
        //                  arg->out_relR + arg->outerPtrR,
        //                  arg->out_relS + arg->outerPtrS,
                  timer, chainedbuf);
    timer->matches_in_sort = *matches;
    //    MSG("left over phase: Join during run sort matches:%ld, Q size: %ld\n", *matches, arg->Q.size());
    merging_phase(matches, &arg->Q, timer, chainedbuf);
    //    MSG("left over phase: Clean up stage: Join during run merge matches:%ld\n", *matches);
    return *matches;
}

PMJJoiner::PMJJoiner(int
                     sizeR, int
                     sizeS, int
                     nthreads) {
    t_arg = new t_pmj(sizeR, sizeS);
    uint32_t mod = int(1e9)+7;
}

void PMJJoiner::join(int32_t tid, tuple_t* tuple, bool IStuple_R, int64_t* matches,
    /*void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *),*/ void* output) {
    auto* arg = (t_pmj*) t_arg;

    //store tuples.
    if (IStuple_R) {
        arg->tmp_relR[arg->outerPtrR + arg->innerPtrR] = *tuple;
        arg->innerPtrR++;
    } else {
        arg->tmp_relS[arg->outerPtrS + arg->innerPtrS] = *tuple;
        arg->innerPtrS++;
    }
    int stepR = progressive_step_R;
    int stepS = progressive_step_S;

    tuple_t* out_relR;
    tuple_t* out_relS;
#ifdef JOIN_RESULT_MATERIALIZE
    chainedtuplebuffer_t *chainedbuf = (chainedtuplebuffer_t *) output;
#else
    chainedtuplebuffer_t* chainedbuf = nullptr;
#endif
    if (arg->outerPtrR < arg->sizeR - stepR && arg->outerPtrS < arg->sizeS - stepS) {//normal process
        //check if it is ready to start process.
        if (arg->innerPtrR >= stepR
            && arg->innerPtrS >= stepS) {//start process and reset inner pointer.
            //            auto relRsz = stepR * sizeof(tuple_t)
            //                          + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
            //            out_relR = (tuple_t *) malloc_aligned(relRsz);
            //
            //            relRsz = stepS * sizeof(tuple_t)
            //                     + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
            //            out_relS = (tuple_t *) malloc_aligned(relRsz);


            /***Sorting***/
            sorting_phase(tid, arg->tmp_relR
                              + arg->outerPtrR, stepR,
                          arg->tmp_relS + arg->outerPtrS, stepS,
                          matches, &arg->Q,
                //                          out_relR,
                //                          out_relS,
                          arg->out_relR + arg->outerPtrR,
                          arg->out_relS + arg->outerPtrS,
                          timer, chainedbuf);
            arg->outerPtrR += stepR;
            arg->outerPtrS += stepS;
            DEBUGMSG("Join during run creation:%ld", *matches)

            /***Reset Inner Pointer***/
            arg->innerPtrR -= stepR;
            arg->innerPtrS -= stepS;

            //            delete out_relR;
            //            delete out_relS;
        }
    } else if (arg->outerPtrR + arg->innerPtrR == arg->sizeR &&
        arg->outerPtrS + arg->innerPtrS == arg->sizeS) {//received everything

        /***Handling Left-Over***/
        stepR = arg->sizeR - arg->outerPtrR;
        stepS = arg->sizeS - arg->outerPtrS;

        //        auto relRsz = stepR * sizeof(tuple_t)
        //                      + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
        //        out_relR = (tuple_t *) malloc_aligned(relRsz);
        //
        //        relRsz = stepS * sizeof(tuple_t)
        //                 + RELATION_PADDING(1, CACHELINEPADDING(1));//TODO: think why we need to patch this.
        //        out_relS = (tuple_t *) malloc_aligned(relRsz);


        sorting_phase(tid,
                      arg->tmp_relR + arg->outerPtrR,
                      stepR,
                      arg->tmp_relS + arg->outerPtrS,
                      stepS,
                      matches, &arg->Q,
                      arg->out_relR + arg->outerPtrR,
                      arg->out_relS + arg->outerPtrS,
                      timer, chainedbuf);

        merging_phase(matches, &arg->Q, timer, chainedbuf);

        //        delete out_relR;
        //        delete out_relS;
    }

}

/**
 *
 *
 * @param relR
 * @param relS
 * @param nthreads
 * @return
 */
RippleJoiner
rpj(int32_t tid, relation_t* rel_R, relation_t* rel_S, void* pVoid) {

    RippleJoiner joiner(rel_R, rel_S, 1);

    uint32_t index_R = 0;//index of rel_R
    uint32_t index_S = 0;//index of rel_S

    uint32_t cur_step = 0;

#ifndef NO_TIMING
    START_MEASURE(joiner.timer)
#endif

    // just a simple nested loop with progressive response, R and S have the same input rate
    do {
        if (index_R < rel_R->num_tuples) {
            joiner.join(tid, &rel_R->tuples[index_R], true, &joiner.matches, /*NULL,*/ pVoid);
            index_R++;
        }
        if (index_S < rel_S->num_tuples) {
            joiner.join(tid, &rel_S->tuples[index_S], false, &joiner.matches, /*NULL,*/ pVoid);
            index_S++;
        }
    } while (index_R < rel_R->num_tuples || index_S < rel_S->num_tuples);

#ifndef NO_TIMING
    END_MEASURE(joiner.timer)
#endif
    return joiner;
}

/**
 *
 * TODO:一个是升级成wanderjoin（就是加索引）一个是要用estimation结果来指导fetch.
 *
 * RIPPLE JOIN algorithm to be used in each thread.
 * @param tid
 * @param tuple
 * @param ISTupleR
 * @param htR
 * @param htS
 * @param matches
 * @param pVoid
 * @param timer
 * @return
 */

void RippleJoiner::join(int32_t tid, tuple_t* tuple, bool ISTupleR, int64_t* matches,
    /*void *(*thread_fun)(const tuple_t *, const tuple_t *, int64_t *),*/ void* out) {
    DEBUGMSG("tid: %d, tuple: %d, R?%d\n", tid, tuple->key, ISTupleR);
    if (ISTupleR) {
#ifndef NO_TIMING
        BEGIN_MEASURE_BUILD_ACC(timer)
#endif
        samList.t_windows->R_Window.push_back(tuple);
#ifndef NO_TIMING
        END_MEASURE_BUILD_ACC(timer)//accumulate hash table build time.
#endif
        match_single_tuple(samList.t_windows->S_Window, tuple, matches, /*thread_fun,*/ timer, ISTupleR, out);
    } else {
        //        samList.t_windows->S_Window.push_back(tuple->key);
#ifndef NO_TIMING
        BEGIN_MEASURE_BUILD_ACC(timer)
#endif
        samList.t_windows->S_Window.push_back(tuple);
#ifndef NO_TIMING
        END_MEASURE_BUILD_ACC(timer)//accumulate hash table build time.
#endif
        match_single_tuple(samList.t_windows->R_Window, tuple, matches, /*thread_fun,*/  timer, ISTupleR, out);
    }
    // Compute estimation result
    long estimation_result = 0;
    if (samList.t_windows->R_Window.size() > 0 && samList.t_windows->S_Window.size() > 0) {
        estimation_result =
            ((int) relR->num_tuples*(int) relS->num_tuples)
                /
                    ((int) samList.t_windows->R_Window.size()*(int) samList.t_windows->S_Window.size())
                *
                    (int) (*matches);
    } else {
        estimation_result = *matches;
    }
    //    fprintf(stdout, "estimation result: %d \n", estimation_result);
}

/**
 *
 * @param relR
 * @param relS
 * @param nthreads
 */
RippleJoiner::RippleJoiner(relation_t* relR, relation_t* relS, int
nthreads) : relR(
    relR), relS(relS) {
    samList.num_threads = nthreads;
    samList.t_windows = new t_window();
}

/**
 *
 * @param tid
 * @param tuple
 * @param htR
 * @param htS
 * @param cleanR
 */
void RippleJoiner::clean(int32_t tid, tuple_t* tuple, bool cleanR) {
    if (cleanR) {
        samList.t_windows->R_Window.remove(tuple);
    } else {
        samList.t_windows->S_Window.remove(tuple);
    }
}


