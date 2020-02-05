//
// Created by Shuhao Zhang on 8/9/19.
//

#ifndef ALLIANCEDB_SORTMERGEJOIN_MULTIPASS_H
#define ALLIANCEDB_SORTMERGEJOIN_MULTIPASS_H

#include "../util/types.h"              /* relation_t, tuple_t, result_t */


/**
 * "m-pass sort-merge join"
 *
 * A Sort-Merge Join variant with partitioning and complete
 * sorting of both input relations. The merging step in this algorithm tries to
 * overlap the first merging and transfer of remote chunks. However, in compared
 * to the other variant (m-way), merge phase still takes a significant amount
 * of time as it is done through a multi-pass merging scheme of sorted-runs.
 *
 * @param relR input relation R
 * @param relS input relation S
 * @param joincfg configuration parameters of the join
 *
 * @warning this algorithm must be run with number of threads that is power of 2
 *
 * \ingroup Joins
 */
result_t *sortmergejoin_multipass(relation_t *relR, relation_t *relS, joinconfig_t *joincfg, int exp_id);

#endif //ALLIANCEDB_SORTMERGEJOIN_MULTIPASS_H
