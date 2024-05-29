//
// Created by tony on 22/11/22.
//

#include <Common/Tuples.h>

using namespace OoOJoin;

OoOJoin::Tuple::Tuple(OoOJoin::keyType k) { key = k; }

OoOJoin::Tuple::Tuple(OoOJoin::keyType k, OoOJoin::valueType v) {
  key = k;
  payload = v;
  eventTime = 0;
}

OoOJoin::Tuple::Tuple(OoOJoin::keyType k, OoOJoin::valueType v, tsType et) {
  key = k;
  payload = v;
  eventTime = et;
}

std::string OoOJoin::Tuple::toString() const {
  std::string tmp;
  tmp.append("\t\tkey:" + std::to_string(key));
  tmp.append("\t\tvalue:" + std::to_string(payload));
  tmp.append("\t\tevent time:" + std::to_string(eventTime));
  return tmp;
}

std::string OoOJoin::OoOTuple::toString() {
  std::string tmp;
  tmp.append(Tuple::toString());
  tmp.append("\t\tarrival time:" + std::to_string(arrivalTime));
  return tmp;
}

std::string OoOJoin::TrackTuple::toString() {
  std::string tmp;
  tmp.append(OoOTuple::toString());
  tmp.append("\t\tprocessed time:" + std::to_string(processedTime));
  return tmp;
}