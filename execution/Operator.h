//
// Created by Shuhao Zhang on 8/9/19.
//

#ifndef ALLIANCEDB_OPERATOR_H
#define ALLIANCEDB_OPERATOR_H


#include "../storage/DbIterator.h"

class Operator : DbIterator {

public:
    void open();

private:
    bool _open;
};


#endif //ALLIANCEDB_OPERATOR_H
