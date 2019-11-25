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
#include <list>


class localJoiner {

public:
    virtual long join(int32_t tid, tuple_t *tuple,
                      bool tuple_R, hashtable_t *htR, hashtable_t *htS, int64_t *matches, void *pVoid,
                      T_TIMER *timer) = 0;
};


class RippleJoiner : public localJoiner {

    private:relation_t *relR;
    private:relation_t *relS;

    t_window_list samList;


public:
    long
    join(int32_t tid, tuple_t *tuple, bool tuple_R, hashtable_t *htR, hashtable_t *htS, int64_t *matches, void *pVoid,
         T_TIMER *timer) override;

    RippleJoiner(relation_t *relR, relation_t *relS, int nthreads);
};


class PMJJoiner : public localJoiner {

public:
    long
    join(int32_t tid, tuple_t *tuple, bool tuple_R, hashtable_t *htR, hashtable_t *htS, int64_t *matches, void *pVoid,
         T_TIMER *timer) override;
};

class SHJJoiner : public localJoiner {

public:
    long
    join(int32_t tid, tuple_t *tuple, bool tuple_R, hashtable_t *htR, hashtable_t *htS, int64_t *matches, void *pVoid,
         T_TIMER *timer) override;
};

long
shj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid, T_TIMER *timer);

long
pmj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid, T_TIMER *timer);

long
rpj(int32_t tid, relation_t *rel_R, relation_t *rel_S, void *pVoid, T_TIMER *timer);

#endif //ALLIANCEDB_LOCALJOINER_H
