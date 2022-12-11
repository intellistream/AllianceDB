#include "Common/Tuple.hpp"

using namespace AllianceDB;

Tuple::~Tuple() = default;
Tuple::Tuple(KeyType k) : key(k), val(0) {}
Tuple::Tuple(KeyType k, ValType v) : key(k), val(v) {}
Tuple::Tuple(KeyType k, ValType v, StreamType st, TsType ts)
    : ts(ts), st(st), key(k), val(v) {}

std::string Tuple::toString() {
  std::string tmp;
  tmp.append("\t\tkey:" + std::to_string(key));
  tmp.append("\t\tvalue:" + std::to_string(val));
  tmp.append("\t\ttimestamp:" + std::to_string(ts));
  return tmp;
}
