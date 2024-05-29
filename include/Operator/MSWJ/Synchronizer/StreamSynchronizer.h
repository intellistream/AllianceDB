//
// Created by 86183 on 2023/1/7.
//

#ifndef DISORDERHANDLINGSYSTEM_SYNCHRONIZER_H
#define DISORDERHANDLINGSYSTEM_SYNCHRONIZER_H

#include <vector>
#include <queue>
#include <set>
#include <list>
#include <mutex>
#include <Operator/MSWJ/Operator/StreamOperator.h>
#include <Operator/MSWJ/Common/MSWJDefine.h>
#include <Utils/ConfigMap.hpp>
#include <shared_mutex>

namespace MSWJ {
class Synchronizer {
 public:
  explicit Synchronizer(int streamCount, StreamOperator *streamOperator, INTELLI::ConfigMapPtr config);

  ~Synchronizer() = default;

  Synchronizer(const Synchronizer &) = delete;

  Synchronizer &operator=(const Synchronizer &) = delete;

  Synchronizer(Synchronizer &&) = delete;

  Synchronizer &operator=(Synchronizer &&) = delete;

  // Synchronize process
  auto synchronizeStream(const TrackTuplePtr &tuple) -> void;

  auto setConfig(INTELLI::ConfigMapPtr config) -> void;

  INTELLI::ConfigMapPtr cfg{};

  // Tsync
  int tSync{};

  // The number of streams that have tuples in the current buffer
  int ownStream{};

  // SyncBuf buffer mapping
  std::vector<std::priority_queue<TrackTuplePtr, std::deque<TrackTuplePtr>, TrackTuplePtrComparator>> synBufferMap{};

 private:
  // Connector
  StreamOperator *streamOperator;

  //write and read lock
  mutable std::shared_mutex mu;
};
}

#endif //DISORDERHANDLINGSYSTEM_SYNCHRONIZER_H
