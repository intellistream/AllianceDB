//
// Created by 86183 on 2023/1/6.
//

#include <map>
#include <list>
#include <complex>
#include <iostream>
#include <utility>
#include <Operator/MSWJ/Manager/StatisticsManager.h>
#include <Operator/MSWJ/Common/MSWJDefine.h>
#include <Operator/MSWJ/Profiler/TupleProductivityProfiler.h>

using namespace MSWJ;

StatisticsManager::StatisticsManager(TupleProductivityProfiler *profiler, INTELLI::ConfigMapPtr config)
    : cfg(std::move(config)), productivityProfiler(profiler) {
  recordMap.resize(streamCount + 1);
  kSyncMap.resize(streamCount + 1);
  RStatMap.resize(streamCount + 1);
  tMap.resize(streamCount + 1);
  kMap.resize(streamCount + 1);
  histogramMap.resize(streamCount + 1);
  histogramPos.resize(streamCount + 1);
}

auto StatisticsManager::addRecord(int streamId, const TrackTuplePtr &tuple) -> void {
  recordMap[streamId].push_back(*tuple);
}

auto StatisticsManager::addRecord(int streamId, int T, int K) -> void {
  tMap[streamId] = T;
  kMap[streamId] = K;
  kSyncMap[streamId].push_back(getKsync(streamId));
}

auto StatisticsManager::getMaxD(int streamId) -> int {
  int maxD = 0;

  if (recordMap[streamId].empty()) {
    return maxD;
  }

  for (auto it : recordMap[streamId]) {
    maxD = std::max(maxD, (int) it.delay);
  }
  return maxD;
}

// Get the R_stat value for a given stream ID
auto StatisticsManager::getRStat(int streamId) -> int {
  std::vector<TrackTuple> record = recordMap[streamId];
  if (record.empty()) {
    return 1;
  }

  // Current R_stat value
  int RStat = 2;
  if (RStatMap[streamId] != 0) {
    RStat = RStatMap[streamId];
  }

  // Create two windows
  std::list<int> window1List;
  std::list<int> window0List;

  // Maintain sum variables for computing the average
  int sumW0 = 0;
  int sumW1 = 0;

  // Extract data from the record within the current R_stat window size, compute frequency, and estimate probability using the frequency
  for (int i = record.size() - 1; i >= std::max((int) record.size() - RStat, 0); i--) {
    int xi = getD(record[i].delay);
    window1List.push_back(xi);
    sumW1 += xi;
  }

  double eCut = 0;
  while (window1List.size() > 1) {
    //Divide the window into w1 and w2 two parts
    int w1Front = window1List.front();
    window0List.push_back(w1Front);
    window1List.pop_front();
    sumW1 -= w1Front;
    sumW0 += w1Front;

    //update e_cut
    double m = 1 / (1 / window0List.size() + 1 / window1List.size());
    double confidenceValue = confidenceValue / (window1List.size() + window0List.size());
    eCut = std::sqrt((1 / 2 * m) * std::log1p(4 / confidenceValue));

    if (std::abs(sumW1 * 1.0 / window1List.size() - sumW0 * 1.0 / window0List.size()) >= eCut) {
      RStatMap[streamId] = window1List.size();
      break;
    }
  }

  return window1List.size();
}

// Get the K_sync value, K_sync = i_T - k_i - min{i_T - k_i| i∈[1,latch_]}
auto StatisticsManager::getKsync(int streamId) -> int {
  if (tMap[streamId] == 0
      || kMap[streamId] == 0
      || recordMap.empty()) {
    return 0;
  }

  int miniTKi = tMap[streamId] - kMap[streamId];

  for (int i = 0; i < recordMap.size(); i++) {
    if (recordMap[i].empty())continue;
    miniTKi = std::min(miniTKi, tMap[i] - kMap[i]);
  }
  return tMap[streamId] - kMap[streamId] - miniTKi;
}

// Get the average K_sync value for a given stream ID, to predict future K_sync values
auto StatisticsManager::getAvgKsync(int streamId) -> int {
  int sumKsynci = 0;
  int R_stat = getRStat(streamId);

  std::vector<int> kSyncList = kSyncMap[streamId];
  if (kSyncList.empty()) {
    return 0;
  }

  // Compute the sum of K_sync values within the R_stat window size, then compute the average
  for (int i = kSyncList.size() - 1; i >= std::max((int) kSyncList.size() - R_stat, 0); i--) {
    sumKsynci += kSyncMap[streamId][i];
  }
  int avgKsynci = sumKsynci / R_stat;

  return avgKsynci == 0 ? 0 : avgKsynci;
}

// The formula used is on page 7 of the referenced paper.
auto StatisticsManager::getFutureKsync(int stream_id) -> int {
  if (kSyncMap.empty()) {
    return 0;
  }

  int avgksynci = getAvgKsync(stream_id);
  int minKsync = INT32_MAX;

  //Find the minimum value of all avgKsync for j != 1
  for (int i = 0; i < kSyncMap.size(); i++) {
    if (kSyncMap[i].empty())continue;
    minKsync = std::min(minKsync, getAvgKsync(i));
  }

  if (minKsync == INT32_MAX) {
    minKsync = 0;
  }
  return avgksynci - minKsync;
}

// This function calculates the probability distribution function fD for a given delay d
auto StatisticsManager::fD(int d, int stream_id) -> double {
  int R_stat = getRStat(stream_id);

  if (recordMap[stream_id].empty()) {
    return -1;
  }

  std::vector<TrackTuple> record = recordMap[stream_id];
  if (record.empty()) {
    return 1;
  }

  //取出record里面R_stat大小范围的数据,并计算频率，用频率估计概率
  //Extract the Rstat-sized range of data from record, and calculate probability, and using frequency to estimate probability
  std::map<int, int> rate_map;
  for (int i = record.size() - 1; i >= std::max((int) record.size() - R_stat, 0); i--) {
    int Di = getD(record[i].delay);
    rate_map[Di] = rate_map.find(Di) == rate_map.end() ? 1 : rate_map[Di] + 1;
  }

  //Use a histogram to simulate the probabilities
  if (histogramMap[stream_id].empty()) {
    histogramMap[stream_id].resize(maxDelay);
  }

  double sumP = 0;
  for (auto it : rate_map) {
    if (histogramMap[stream_id][it.first] != 0) {
      // The probability for Di has already been calculated, so take the average
      histogramMap[stream_id][it.first] = (histogramMap[stream_id][it.first] + it.second * 1.0 / R_stat) / 2;
    } else {
      histogramMap[stream_id][it.first] = it.second * 1.0 / R_stat;
      histogramPos[stream_id].push_back(it.first);
    }
  }

  for (int i = 0; i < histogramPos[stream_id].size(); i++) {
    sumP += histogramMap[stream_id][histogramPos[stream_id][i]];
  }

  // If the probability for d already exists, return it
  if (histogramMap[stream_id][d] != 0) {
    // Normalize the histogram if it has been updated before returning the probability for d
    if (sumP != 0) {
      for (int i = 0; i < histogramPos[stream_id].size(); i++) {
        if (histogramMap[stream_id][histogramPos[stream_id][i]] != 0) {
          histogramMap[stream_id][histogramPos[stream_id][i]] /= sumP;
        }
      }
    }
    return histogramMap[stream_id][d];
  }

  // If d is not recorded, use line interpolation estimation, double pointer center diffusion method
  int hi_size = histogramMap[stream_id].size();

  bool flag = false;
  int left = d - 1, right = d + 1;
  while (left >= 0 && right < hi_size) {
    if (histogramMap[stream_id][left] != 0 && histogramMap[stream_id][right] != 0) {
      flag = true;
      break;
    }
    if (histogramMap[stream_id][left] == 0) {
      left--;
    }
    if (histogramMap[stream_id][right] == 0) {
      right++;
    }
  }

  if (!flag) {
    if (left < 0) {
      left = d + 1, right = d + 2;
      while (right < hi_size) {
        if (histogramMap[stream_id][left] != 0 && histogramMap[stream_id][right] != 0) {
          break;
        }
        if (histogramMap[stream_id][left] == 0) {
          left++;
        }
        if (histogramMap[stream_id][right] == 0) {
          right++;
        }
      }
    } else if (right >= hi_size) {
      if (d - 1 < 0 || d - 2 < 0) {
        // Indicates that there are no more elements to the left, so the loop should be terminated using break.
        right = left;
      } else {
        left = d - 2, right = d - 1;
        while (left >= 0) {
          if (histogramMap[stream_id][left] != 0 && histogramMap[stream_id][right] != 0) {
            break;
          }
          if (histogramMap[stream_id][left] == 0) {
            left--;
          }
          if (histogramMap[stream_id][right] == 0) {
            right--;
          }
        }
      }
    }
  }

  if (left == right) {
    histogramMap[stream_id][left] /= 2;
    histogramMap[stream_id][d] = histogramMap[stream_id][left];
    histogramPos[stream_id].push_back(d);
    return histogramMap[stream_id][d];
  }

  double pL = histogramMap[stream_id][left];
  double pR = histogramMap[stream_id][right];

  // At this point, left and right are pointing to indices with actual data, so linear interpolation can be used to estimate probability.
  // The equation of the line is: y = (pR - pL) / (right - left) * (x - left) + pL
  double pD = (pR - pL) / (right - left) * (d - left) + pL;

  if (pD < 0) {
    pD = 1;
  }
  // Update the histogram at index 'd' with the estimated probability and append 'd' to the histogram position vector.
  histogramMap[stream_id][d] = pD;
  histogramPos[stream_id].push_back(d);

  // Normalize the histogram.
  sumP += pD;
  for (int i = 0; i < histogramPos[stream_id].size(); i++) {
    if (histogramMap[stream_id][histogramPos[stream_id][i]] != 0) {
      histogramMap[stream_id][histogramPos[stream_id][i]] /= sumP;
    }
  }

  return histogramMap[stream_id][d];
}

auto StatisticsManager::fDk(int d, int streamId, int K) -> double {
  int kSync = getFutureKsync(streamId);
  double res = 0;

  if (d == 0) {
    for (int i = 0; i <= (kSync + K) / g; i++) {
      res += fD(i, streamId);
    }
  } else {
    res = fD(d + (K + kSync) / g, streamId);
  }

  return std::abs(res);
}

auto StatisticsManager::wil(int l, int streamId, int K) -> int {
  std::string key = "Stream_" + std::to_string(streamId);
  int wi = cfg->getU64(key);;
  int ni = wi / b;
  double res = 0;
  double ri = productivityProfiler->getJoinRecordMap()[streamId] * 1.0 / wi;

  if (l <= ni - 1 && l >= 1) {
    for (int i = 0; i <= (l - 1) * b / g; i++) {
      res += fDk(i, streamId, K);
    }
    res = std::ceil(ri * b * res);
  } else if (l == ni) {
    for (int i = 0; i <= (ni - 1) * b / g; i++) {
      res += fDk(i, streamId, K);
    }
    res = std::ceil(ri * (wi - (ni - 1) * b) * res);
  }

  return std::abs(res);
}

auto StatisticsManager::setConfig(INTELLI::ConfigMapPtr config) -> void {
  cfg = std::move(config);
}



