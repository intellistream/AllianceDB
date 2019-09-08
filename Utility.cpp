//
// Created by Shuhao Zhang on 7/9/19.
//
#include <vector>
#include <iostream>
#include "Utility.h"
#include "storage/field/IntField.h"

using namespace std;

TupleIterator *Utility::createTupleList(int width, int *tupdata) {
    int i = 0;
    int length = sizeof(tupdata) / sizeof(int);

    vector<Tuple> tuplist;

    while (i < length) {
        Tuple tup(Utility::getTupleDesc(width));

        for (int j = 0; j < width; ++j)
            tup.setField(j, getField(tupdata[i++]));
        tuplist.push_back(tup);
    }

    TupleIterator *result = new TupleIterator(Utility::getTupleDesc(width), tuplist, TupleDesc(nullptr));
    result->open();
    return result;

}

Field Utility::getField(int n) {
    return IntField(n);
}

Type *getTypes(int len) {
    Type *types = new Type[len];
    for (int i = 0; i < len; ++i)
        types[i] = INT_TYPE();
    return types;
}

static bool compareTuples(Tuple t1, Tuple t2) {
    if (t1.getTupleDesc().numFields() != t2.getTupleDesc().numFields())
        return false;

    for (int i = 0; i < t1.getTupleDesc().numFields(); ++i) {
        if ((t1.getTupleDesc().getFieldType(i).getType() != (t2.getTupleDesc().getFieldType(i)).getType()))
            return false;
        if (!(t1.getField(i).equals(t2.getField(i))))
            return false;
    }

    return true;
}

void Utility::matchAllTuples(DbIterator expected, DbIterator actual) {
    bool matched = false;
    while (expected.hasNext()) {
        Tuple expectedTup = expected.next();
        matched = false;
        actual.rewind();

        Tuple next = nullptr;
        while (actual.hasNext()) {
            next = actual.next();
            if (compareTuples(expectedTup, next)) {
                matched = true;
                break;
            }
        }

        if (!matched) {
            cout << "expected tuple not found: " + expectedTup << endl;
            exit(-1);
        }
    }
}


