//
// Created by Shuhao Zhang on 7/9/19.
//
#include <vector>
#include "Utility.h"
#include "storage/IntField.h"

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
