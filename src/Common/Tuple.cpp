//
// Created by shuhao.zhang on 26/4/22.
//

#include <Common/Types.hpp>
#include "Common/Tuple.hpp"
AllianceDB::Tuple::~Tuple() = default;
AllianceDB::Tuple::Tuple(AllianceDB::keyType k) : key(k), payload(0) {}