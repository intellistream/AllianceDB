//
// Created by Shuhao Zhang on 7/9/19.
//

#ifndef ALLIANCEDB_TUPLEITERATOR_H
#define ALLIANCEDB_TUPLEITERATOR_H


#include <vector>
#include "DbIterator.h"

class TupleIterator : DbIterator {

public:
    TupleIterator(TupleDesc desc, std::vector<Tuple> vector, TupleDesc td);

    void open();//Opens the iterator. This must be called before any of the other methods.

    TupleDesc getTupleDesc() {
        return TupleDesc(nullptr);
    }

protected:
    TupleDesc td;

};


#endif //ALLIANCEDB_TUPLEITERATOR_H
