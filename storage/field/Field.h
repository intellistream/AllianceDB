//
// Created by Shuhao Zhang on 7/9/19.
//

#ifndef ALLIANCEDB_FIELD_H
#define ALLIANCEDB_FIELD_H

#include <ostream>

class Field {
public:
    void serialize(std::ostream dos);

    virtual bool equals(Field field)= 0;

};


#endif //ALLIANCEDB_FIELD_H
