//
// Created by Shuhao Zhang on 7/9/19.
//

#ifndef ALLIANCEDB_UTILITY_H
#define ALLIANCEDB_UTILITY_H


#include "storage/TupleIterator.h"
#include "storage/type/Type.h"
#include "storage/type/INT_TYPE.h"

class Utility {

public:
    static TupleIterator *createTupleList(int width, int *tupdata);

    /**
     * @return a TupleDesc with n fields of type Type.INT_TYPE
     */
    static TupleDesc getTupleDesc(int n) {
        return TupleDesc(getTypes(n));
    }

    /**
    * @return a Type array of length len populated with Type.INT_TYPE
    */
    static Type *getTypes(int len);

    static Field getField(int n);


    static void matchAllTuples(DbIterator expected, DbIterator actual);
};


#endif //ALLIANCEDB_UTILITY_H
