//
// Created by Shuhao Zhang on 7/9/19.
//

#ifndef ALLIANCEDB_SYMMETRICHASHJOIN_H
#define ALLIANCEDB_SYMMETRICHASHJOIN_H


#include "../storage/TupleIterator.h"
#include "JoinPredicate.h"

class SymmetricHashJoin {


public:
    SymmetricHashJoin(JoinPredicate predicate, TupleIterator *R, TupleIterator *S,
                          TupleDescription comboTD, JoinPredicate _predicate);

    void symHashJoin(TupleIterator R, TupleIterator S);

private:
    JoinPredicate _predicate;
    TupleIterator *_R = nullptr;
    TupleIterator *_S = nullptr;
    TupleDescription _comboTD = nullptr;

    TupleIterator *inner;
    TupleIterator *outter;

};


#endif //ALLIANCEDB_SYMMETRICHASHJOIN_H
