//
// Created by Shuhao Zhang on 7/9/19.
//

#include "IntField.h"

IntField::IntField(int i) : _value(i) {

}

bool IntField::equals(IntField field) {
    return (field)._value == _value;
}

