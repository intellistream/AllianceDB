//
// Created by Shuhao Zhang on 7/9/19.
//

#ifndef ALLIANCEDB_INTFIELD_H
#define ALLIANCEDB_INTFIELD_H


#include "Field.h"

class IntField : public Field {

public:
    IntField(int i);

    const int _value;

    bool equals(IntField field);
};


#endif //ALLIANCEDB_INTFIELD_H
