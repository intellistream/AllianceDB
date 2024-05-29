#include <Operator/MSWJOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>

bool OoOJoin::MSWJOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
  if (!OoOJoin::MeanAQPIAWJOperator::setConfig(cfg)) {
    return false;
  }
  streamOperator->setConfig(cfg);

  return true;
}

bool OoOJoin::MSWJOperator::start() {
  streamOperator->syncTimeStruct(timeBaseStruct);
  return streamOperator->start();
}

bool OoOJoin::MSWJOperator::stop() {
  streamOperator->stop();
  timeBreakDownAll = timeTrackingEnd(timeBreakDownAll);
  return true;
}

bool OoOJoin::MSWJOperator::feedTupleS(OoOJoin::TrackTuplePtr ts) {
  ts->streamId = 1;
  kSlackS->disorderHandling(ts);
  return true;
}

bool OoOJoin::MSWJOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
  tr->streamId = 2;
  kSlackR->disorderHandling(tr);
  return true;
}

size_t OoOJoin::MSWJOperator::getResult() {
  return streamOperator->getResult();
}

size_t OoOJoin::MSWJOperator::getAQPResult() {
  return streamOperator->getAQPResult();
}




