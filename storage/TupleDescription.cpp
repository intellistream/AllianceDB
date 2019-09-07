//
// Created by Shuhao Zhang on 7/9/19.
//

#include "TupleDescription.h"

using namespace std;

TupleDescription::TupleDescription(Type *pType) {

}

TupleDescription TupleDescription::merge(TupleDescription td1, TupleDescription td2) {
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

int TupleDescription::numFields() {
    return 0;
}

Type TupleDescription::getFieldType(int i) {
    return tdItems[i].fieldType;
}

string TupleDescription::getFieldName(int i) {
    return nullptr;
}

TupleDescription::TupleDescription(Type *typeAr, string *fieldAr) {

}

