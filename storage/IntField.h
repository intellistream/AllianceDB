//
// Created by Shuhao Zhang on 7/9/19.
//

#ifndef ALLIANCEDB_INTFIELD_H
#define ALLIANCEDB_INTFIELD_H


#include "Field.h"

class IntField : public Field {

public:
    IntField(int i);

private:
    const int value;
};


#endif //ALLIANCEDB_INTFIELD_H
