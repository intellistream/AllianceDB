//
// Created by Shuhao Zhang on 7/9/19.
//

#ifndef ALLIANCEDB_DBITERATOR_H
#define ALLIANCEDB_DBITERATOR_H


#include "Tuple.h"
#include "TupleDesc.h"

class DbIterator {

public:
    void open();//Opens the iterator. This must be called before any of the other methods.

    /** Returns true if the iterator has more tuples.
        * @return true if the iterator has more tuples.
    */
    bool hasNext();

    /**
        * Returns the next tuple from the operator (typically implementing by reading
        * from a child operator or an access method).
    */
    Tuple next();

    /**
        * Resets the iterator to the start.
    */
    void rewind();

    /**
        * Returns the TupleDesc associated with this DbIterator.
        * @return the TupleDesc associated with this DbIterator.
    */
    virtual TupleDesc getTupleDesc();

    /**
        * Closes the iterator. When the iterator is closed, calling next(),
        * hasNext(), or rewind() should fail.
    */
    void close();

};


#endif //ALLIANCEDB_DBITERATOR_H
