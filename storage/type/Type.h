//
// Created by Shuhao Zhang on 7/9/19.
//

#ifndef ALLIANCEDB_TYPE_H
#define ALLIANCEDB_TYPE_H


#include <istream>
#include "../field/Field.h"

class Type {

private:
    static const int STRING_LEN = 128;
    int _v;
public:
    int getType();
    int getLen();
    Field parse(std::istream dis);
};

#endif //ALLIANCEDB_TYPE_H
