//
// Created by 86183 on 2023/1/7.
//

#include <future>
#include <utility>
#include <Operator/MSWJ/Synchronizer/StreamSynchronizer.h>
#include <Operator/MSWJ/Operator/StreamOperator.h>
#include <shared_mutex>

using namespace MSWJ;

Synchronizer::Synchronizer(int streamCount, StreamOperator *streamOperator, INTELLI::ConfigMapPtr config)
    : cfg(std::move(config)),
      streamOperator(streamOperator) {
  synBufferMap.resize(streamCount + 1);
}

// Receives a stream tuple from the K-slack operator
auto Synchronizer::synchronizeStream(const TrackTuplePtr &tuple) -> void {
  int streamId = tuple->streamId;

  if (tuple->eventTime > tSync) {
    // Push the tuple into the synchronization buffer for this stream
    if (synBufferMap[streamId].empty()) {
      // This is the first tuple for this stream, the synchronization buffer is about to receive tuples
      ownStream++;
    }
    synBufferMap[streamId].push(tuple);

    // Check if the synchronization buffer contains tuples for all streams
    while (ownStream == streamCount) {
      // Find the minimum event time across all synchronization buffers
      tSync = INT32_MAX;
      // Acquire a shared lock to read from the synchronization buffers
      {
        std::shared_lock<std::shared_mutex> lock(mu);
        for (const auto &it : synBufferMap) {
          if (it.empty()) {
            continue;
          }
          tSync = std::min(tSync, (int) it.top()->eventTime);
        }
      }
      // Acquire an unique lock to write to the synchronization buffers
      {
        std::unique_lock<std::shared_mutex> lock(mu);
        for (auto &it : synBufferMap) {
          if (it.empty()) {
            continue;
          }
          // Output all tuples in the synchronization buffer with event time equals to tSync
          while (!it.empty() && it.top()->eventTime == tSync) {
            auto item = it.top();
            streamOperator->mswjExecution(item);
            it.pop();
          }

          if (it.empty()) {
            ownStream--;
          }
        }
      }
    }
  } else {
    // Output the tuple directly
    streamOperator->mswjExecution(tuple);
  }
}

auto Synchronizer::setConfig(INTELLI::ConfigMapPtr config) -> void {
  cfg = std::move(config);
}

