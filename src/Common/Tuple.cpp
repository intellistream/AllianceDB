#include <Common/Tuple.hpp>
AllianceDB::Tuple::~Tuple() = default;
AllianceDB::Tuple::Tuple(AllianceDB::keyType k) : key(k), payload(0) {}
AllianceDB::Tuple::Tuple(AllianceDB::keyType k, AllianceDB::valueType v) : key(k), payload(v) {}
AllianceDB::Tuple::Tuple(AllianceDB::tsType t, AllianceDB::keyType k, AllianceDB::valueType v)
    : timestamp(t), key(k), payload(v) {}

std::string AllianceDB::Tuple::toString() {
  std::string tmp;
  tmp.append("\t\tkey:" + std::to_string(key));
  tmp.append("\t\tvalue:" + std::to_string(payload));
  tmp.append("\t\ttimestamp:" + std::to_string(timestamp));
  return tmp;
}
