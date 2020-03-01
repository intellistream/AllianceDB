//
// Created by Shuhao Zhang on 14/11/19.
//

#ifndef ALLIANCEDB_AVXSORT_H
#define ALLIANCEDB_AVXSORT_H

#include "../utils/types.h"


void
scalarsort_tuples(tuple_t ** inputptr, tuple_t ** outputptr, uint64_t nitems);

void
avxsort_tuples(tuple_t **inputptr, tuple_t **outputptr, uint64_t nitems);

#endif //ALLIANCEDB_AVXSORT_H
