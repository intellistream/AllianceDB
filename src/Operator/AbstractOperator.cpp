//
// Created by tony on 22/11/22.
//

#include <Operator/AbstractOperator.h>

#include <utility>

using namespace OoOJoin;

bool OoOJoin::AbstractOperator::feedTupleS(OoOJoin::TrackTuplePtr ts) {
  assert(ts);
  return true;
}

bool OoOJoin::AbstractOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
  assert(tr);
  return true;
}

bool OoOJoin::AbstractOperator::start() {
  return true;
}

bool OoOJoin::AbstractOperator::stop() {
  return true;
}

size_t OoOJoin::AbstractOperator::getResult() {
  return 0;
}

size_t OoOJoin::AbstractOperator::getAQPResult() {
  return getResult();
}

bool OoOJoin::AbstractOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
  config = std::move(cfg);
  if (config == nullptr) {
    return false;
  }
  if (config->existU64("windowLen")) {
    windowLen = config->getU64("windowLen");
  }
  if (config->existU64("slideLen")) {
    slideLen = config->getU64("slideLen");
  }
  if (config->existU64("sLen")) {
    sLen = config->getU64("sLen");
  } else {
    OP_ERROR("empty buffer for S stream \n");

    return false;
  }
  if (config->existU64("rLen")) {
    rLen = config->getU64("rLen");
  } else {

    OP_ERROR("empty buffer for R stream \n");
    return false;
  }
  return true;
}

ConfigMapPtr OoOJoin::AbstractOperator::getTimeBreakDown() {
  return nullptr;
}
double OoOJoin::AbstractOperator::getLazyRunningThroughput() {
  return 0;
}
