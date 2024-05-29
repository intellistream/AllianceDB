//
// Created by 86183 on 2023/1/9.
//

#include <iostream>
#include <utility>
#include <Operator/MSWJ/Profiler/TupleProductivityProfiler.h>
#include <Operator/MSWJ/Common/MSWJDefine.h>

using namespace MSWJ;

auto TupleProductivityProfiler::getJoinRecordMap() -> std::vector<int> {
  return joinRecordMap;
}

auto TupleProductivityProfiler::addJoinRecord(int stream_id, int count) -> void {
  joinRecordMap[stream_id] = count;
}

auto TupleProductivityProfiler::updateCrossJoin(int Di, size_t res) -> void {
  crossJoinMap[Di] = res;
  crossJoinPos.push_back(Di);
}

auto TupleProductivityProfiler::updateJoinRes(int Di, size_t res) -> void {
  joinResultMap[Di] += res;
  joinResultPos.push_back(Di);
}

auto TupleProductivityProfiler::getSelectRatio(int K) -> double {
  if (crossJoinPos.empty() || joinResultPos.empty()) {
    return 1;
  }

  int mSum = 0;
  int mxSum = 0;

  int temp = 0;
  for (int d = 0; d < joinResultPos.size(); d++) {
    if (temp == crossJoinPos[d])continue;
    temp = crossJoinPos[d];
    if (joinResultPos[d] > K) {
      continue;
    }
    mSum += joinResultMap[joinResultPos[d]];
    mxSum += crossJoinMap[crossJoinPos[d]];
  }

  //border situation
  if (mxSum == 0) {
    return 1;
  }

  int mdmSum = 0;
  int mxdmSum = 0;

  temp = 0;
  for (int d = 0; d < joinResultPos.size(); d++) {
    if (temp == crossJoinPos[d])continue;
    temp = crossJoinPos[d];
    mxdmSum += crossJoinMap[crossJoinPos[d]];
    mdmSum += joinResultMap[joinResultPos[d]];
  }
  double res = (mSum * 1.0 / mxSum) * (mxdmSum * 1.0 / mdmSum);
  return res;
}

auto TupleProductivityProfiler::getRequirementRecall() -> double {
  if (crossJoinMap.empty()) {
    return 1;
  }
  size_t maxD = 0;

  for (int i = 0; i < joinResultPos.size(); i++) {
    maxD = std::max(maxD, crossJoinMap[joinResultPos[i]]);
  }

  int nTrueL = 0;
  int temp = -1;
  for (int d = 0; d < std::min(maxD, joinResultPos.size()); d++) {
    if (temp == joinResultPos[d])continue;
    temp = joinResultPos[d];
    nTrueL += joinResultMap[joinResultPos[d]];
  }

  int nTruePL = 0;
  temp = -1;
  for (int d = 0; d < maxD; d++) {
    if (temp == joinResultPos[d])continue;
    temp = joinResultPos[d];
    if (d < maxD * (1 - (P - L) / L))continue;
    nTruePL += joinResultMap[joinResultPos[d]];
  }

  int nProdPL = 0;
  temp = -1;
  for (int d = 0; d < maxD; d++) {
    if (temp == joinResultPos[d])continue;
    temp = joinResultPos[d];
    if (d < maxD * (1 - (P - L) / L))continue;
    nProdPL += joinResultMap[joinResultPos[d]];
  }

  if (nTrueL == 0) {
    return userRecall;
  }

  double requirementRecall = (userRecall * (nTruePL + nTrueL) - nProdPL) * 1.0 / nTrueL;
  return std::max(requirementRecall, (double) 1);
}

TupleProductivityProfiler::TupleProductivityProfiler(INTELLI::ConfigMapPtr config) :
    cfg(std::move(config)) {
  joinRecordMap.resize(streamCount + 1);
  crossJoinMap.resize(maxDelay);
  joinResultMap.resize(maxDelay);
}

auto TupleProductivityProfiler::setConfig(INTELLI::ConfigMapPtr config) -> void {
  cfg = std::move(config);
}
