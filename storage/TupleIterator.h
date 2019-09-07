//
// Created by Shuhao Zhang on 7/9/19.
//

#ifndef ALLIANCEDB_TUPLEITERATOR_H
#define ALLIANCEDB_TUPLEITERATOR_H


#include <vector>
#include "DbIterator.h"

class TupleIterator : DbIterator {

public:
    TupleIterator(TupleDescription desc, std::vector<Tuple> vector, TupleDescription td);

    void open();//Opens the iterator. This must be called before any of the other methods.

    TupleDescription getTupleDesc() {
        return TupleDescription(nullptr);
    }

protected:
    TupleDescription td;

};


#endif //ALLIANCEDB_TUPLEITERATOR_H
