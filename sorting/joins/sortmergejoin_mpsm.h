//
// Created by Shuhao Zhang on 8/9/19.
//

#ifndef ALLIANCEDB_SORTMERGEJOIN_MPSM_H
#define ALLIANCEDB_SORTMERGEJOIN_MPSM_H

#include "../utils/types.h"              /* relation_t, tuple_t, result_t */

/**
 * "mpsm sort-merge join"
 *
 * "Massively Parallel Sort-Merge joins in main-memory multi-core database
 * systems", Albutiu et al., PVLDB'12. MPSM is a Partial-Sort-Scan-Join.
 *
 * @param relR input relation R
 * @param relS input relation S
 * @param joincfg configuration parameters of the join
 *
 * @warning this algorithm must be run with number of threads that is power of 2
 *
 * \ingroup Joins
 */
result_t *
sortmergejoin_mpsm(relation_t * relR, relation_t * relS, joinconfig_t * joincfg, int exp_id, int window_size);

#endif //ALLIANCEDB_SORTMERGEJOIN_MPSM_H
