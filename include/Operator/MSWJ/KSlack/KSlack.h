//
// Created by 86183 on 2023/1/4.
//

#ifndef DISORDERHANDLINGSYSTEM_K_SLACK_H
#define DISORDERHANDLINGSYSTEM_K_SLACK_H

#include <cstddef>
#include <queue>
#include <set>
#include <Operator/MSWJ/Common/MSWJDefine.h>
#include <Operator/MSWJ/Synchronizer/StreamSynchronizer.h>
#include <Operator/MSWJ/Manager/StatisticsManager.h>
#include <Operator/MSWJ/Manager/BufferSizeManager.h>

namespace MSWJ {

class KSlack {
 public:
  // Constructor with required parameters
  explicit KSlack(int streamId, BufferSizeManager *bufferSizeManager, StatisticsManager *statisticsManager,
                  Synchronizer *synchronizer);

  // Destructor
  ~KSlack() = default;

  // Disable copy constructor and copy assignment operator
  KSlack(const KSlack &) = delete;

  KSlack &operator=(const KSlack &) = delete;

  // Handles the disordering of incoming tuples
  auto disorderHandling(const TrackTuplePtr &tuple) -> void;

  // Sets the configuration for the KSlack instance
  auto setConfig(const INTELLI::ConfigMapPtr &config) -> void;

  // Configuration object
  INTELLI::ConfigMapPtr cfg{};

  // Buffer size, equivalent to the K value in the paper. Note that buffer size is measured in time units, not in number of tuples.
  size_t bufferSize{0};

  // Current time, equivalent to the T value in the paper.
  int currentTime{};

  // Stream ID of the KSlack instance
  int streamId{};

  // Priority queue used as the buffer (min heap)
  std::priority_queue<TrackTuplePtr, std::deque<TrackTuplePtr>, TrackTuplePtrComparator> buffer{};

 private:
  // Buffer size manager
  BufferSizeManager *bufferSizeManager{};

  // Statistics manager
  StatisticsManager *statisticsManager{};

  // Synchronizer object
  Synchronizer *synchronizer{};

};
}

#endif //DISORDERHANDLINGSYSTEM_K_SLACK_H
