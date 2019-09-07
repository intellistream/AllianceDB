//
// Created by Shuhao Zhang on 7/9/19.
//

#ifndef ALLIANCEDB_TUPLEDESC_H
#define ALLIANCEDB_TUPLEDESC_H


#include "type/Type.h"

using namespace std;

class TupleDescription {

public:
    TupleDescription(Type *pType);

    /**
     * Create a new TupleDesc with typeAr.length fields with fields of the
     * specified types, with associated named fields.
     *
     * @param typeAr
     *            array specifying the number of and types of fields in this
     *            TupleDesc. It must contain at least one entry.
     * @param fieldAr
     *            array specifying the names of the fields. Note that names may
     *            be null.
     */
    TupleDescription(Type *typeAr, string *fieldAr);

public:
    static TupleDescription merge(TupleDescription td1, TupleDescription td2);

    int numFields();

    class TDItem {
    public:
        Type fieldType;
    };


private:
    TDItem *tdItems;

    Type getFieldType(int i);

    string getFieldName(int i);
};


#endif //ALLIANCEDB_TUPLEDESC_H
