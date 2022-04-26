//
// Created by shuhao.zhang on 26/4/22.
//

#include <Common/Types.hpp>
#include "Common/Tuple.hpp"
INTELLI::Tuple::~Tuple() = default;
INTELLI::Tuple::Tuple(INTELLI::keyType k) : key(k), payload(0) {}