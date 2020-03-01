//
// Created by Shuhao Zhang on 15/11/19.
//

#ifndef ALLIANCEDB_SORT_COMMON_H
#define ALLIANCEDB_SORT_COMMON_H


#include <cstdint>
#include <cstdio>
#include "../utils/types.h"

#define scalarflag 0

/** utility method to check whether arrays are sorted */
int
is_sorted_helper(int64_t *items, uint64_t nitems);


#endif //ALLIANCEDB_SORT_COMMON_H
