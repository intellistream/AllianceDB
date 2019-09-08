//
// Created by Shuhao Zhang on 7/9/19.
//

#ifndef ALLIANCEDB_TUPLE_H
#define ALLIANCEDB_TUPLE_H

#include "TupleDesc.h"
#include "field/Field.h"

/**
 * Tuple maintains information about the contents of a tuple.
 * Tuples have a specified schema specified by a TupleDesc object and contain Field objects
 * with the data for each field.
 */
class Tuple {

private:
    Field *fields;
    TupleDesc _td;
//    RecordId rid; // source on disk -- may be null

public:
    Tuple(TupleDesc td);

    /**
     * /**
     * Change the value of the ith field of this tuple.
     *
     * @param i
     *            index of the field to change. It must be a valid index.
     * @param f
     *            new value for the field.
     */
    void setField(int i, Field f);


    /**
     * @return the value of the ith field, or null if it has not been set.
     *
     * @param i
     *            field index to return. Must be a valid index.
     */
    Field getField(int i);

    TupleDesc getTupleDesc();
};


#endif //ALLIANCEDB_TUPLE_H
