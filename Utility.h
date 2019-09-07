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
    static Type *getTypes(int len) {
        Type *types = new Type[len];
        for (int i = 0; i < len; ++i)
            types[i] =  INT_TYPE();
        return types;
    }

    static Field getField(int n);
};


#endif //ALLIANCEDB_UTILITY_H
