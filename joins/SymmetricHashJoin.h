//
// Created by Shuhao Zhang on 7/9/19.
//

#ifndef ALLIANCEDB_SYMMETRICHASHJOIN_H
#define ALLIANCEDB_SYMMETRICHASHJOIN_H


#include "../storage/TupleIterator.h"
#include "JoinPredicate.h"
#include "../execution/Operator.h"

class SymmetricHashJoin : Operator {


public:
    SymmetricHashJoin(JoinPredicate predicate, TupleIterator *R, TupleIterator *S);

    void symHashJoin(TupleIterator R, TupleIterator S);

    void open();

private:
    JoinPredicate _predicate;
    TupleIterator *_R = nullptr;
    TupleIterator *_S = nullptr;
    TupleDesc _comboTD = nullptr;

    TupleIterator *inner;
    TupleIterator *outter;

};


#endif //ALLIANCEDB_SYMMETRICHASHJOIN_H
