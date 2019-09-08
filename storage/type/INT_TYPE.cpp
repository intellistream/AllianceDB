//
// Created by Shuhao Zhang on 7/9/19.
//

#include "INT_TYPE.h"
#include "../field/IntField.h"

Field parse(std::istream dis) {
    int v;
    dis >> v;
    return IntField(v);
}
