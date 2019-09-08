//
// Created by Shuhao Zhang on 7/9/19.
//

#include "TupleDesc.h"

using namespace std;

TupleDesc::TupleDesc(Type *pType) {

}

TupleDesc TupleDesc::merge(TupleDesc td1, TupleDesc td2) {
    Type *types;
    types = new Type[td1.numFields() + td2.numFields()];
    int size = sizeof(types);
    string *names = new string[size];

    for (int i = 0; i < td1.numFields(); i++) {
        types[i] = td1.getFieldType(i);
        names[i] = td1.getFieldName(i);
    }
    for (int i = 0; i < td2.numFields(); i++) {
        types[td1.numFields() + i] = td2.getFieldType(i);
        names[i + td1.numFields()] = td2.getFieldName(i);
    }
    return {types, names};

}

int TupleDesc::numFields() {
    return 0;
}

Type TupleDesc::getFieldType(int i) {
    return tdItems[i].fieldType;
}

string TupleDesc::getFieldName(int i) {
    return nullptr;
}

TupleDesc::TupleDesc(Type *typeAr, string *fieldAr) {

}

