//
// Created by 86183 on 2023/1/4.
//

#include <iostream>
#include <future>
#include <Operator/MSWJ/KSlack/KSlack.h>
#include <Operator/MSWJ/Common/MSWJDefine.h>
#include <Operator/MSWJ/Synchronizer/StreamSynchronizer.h>
#include <Operator/MSWJ/Manager/BufferSizeManager.h>

using namespace MSWJ;

KSlack::KSlack(int streamId, BufferSizeManager *bufferSizeManager, StatisticsManager *statisticsManager,
               Synchronizer *synchronizer) :
    bufferSizeManager(bufferSizeManager),
    statisticsManager(statisticsManager),
    synchronizer(synchronizer) {
  this->streamId = streamId;
}

// K-Slack algorithm for processing unordered streams
auto KSlack::disorderHandling(const TrackTuplePtr &tuple) -> void {
  tuple->streamId = streamId;
  if (tuple->isEnd) {
    // Add remaining elements in buffer to output
    while (!buffer.empty()) {
      auto syn_tuple = buffer.top();
      // Add to synchronizer
      synchronizer->synchronizeStream(syn_tuple);
      buffer.pop();
    }
    return;
  }

  //update local time
  currentTime = std::max(currentTime, (int) tuple->eventTime);

  // Adjust K value every L time units
  if (currentTime != 0 && currentTime % L == 0) {
    bufferSize = bufferSizeManager->kSearch(streamId);
  }

  // Calculate tuple's delay T - ts for recording in statistics manager
  tuple->delay = currentTime - tuple->eventTime;

  // Add tuple to output and statistics manager's historical records and T value
  statisticsManager->addRecord(streamId, tuple);
  statisticsManager->addRecord(streamId, currentTime, bufferSize);

  // Dequeue all tuples in buffer that satisfy the condition and add them to output
  while (!buffer.empty()) {

    auto tuple = buffer.top();

    // Condition: ei. ts + Ki <= T
    if (tuple->eventTime + bufferSize > currentTime) {
      break;
    }

    // Satisfies condition, add to output
    // Add to synchronizer
    synchronizer->synchronizeStream(tuple);

    buffer.pop();
  }

  // Add tuple to buffer
  buffer.push(tuple);
}

auto KSlack::setConfig(const INTELLI::ConfigMapPtr &config) -> void {
  cfg = config;
  g = cfg->getU64("g");
  L = cfg->getU64("L");
  P = cfg->getU64("P");
  b = cfg->getU64("b");
  maxDelay = cfg->getU64("maxDelay");
  streamCount = cfg->getU64("StreamCount");
  userRecall = cfg->getDouble("userRecall");
  confidenceValue = cfg->getDouble("confidenceValue");
}




