//
// Created by 86183 on 2023/1/5.
//

#include <Operator/MSWJ/Manager/BufferSizeManager.h>
#include <Operator/MSWJ/Profiler/TupleProductivityProfiler.h>
#include <Operator/MSWJ/Common/MSWJDefine.h>

using namespace MSWJ;

BufferSizeManager::BufferSizeManager(StatisticsManager *statisticsManager, TupleProductivityProfiler *profiler) :
    statisticsManager(statisticsManager),
    productivityProfiler(profiler) {}

/**
 *
 * @param L  - buffer-size manager's adaptive time interval
 * @param g  K* serach param
 */
auto BufferSizeManager::kSearch(int stream_id) -> int {
  int max_DH = statisticsManager->getMaxD(stream_id);
  if (max_DH == 0) {
    return 1;
  }

  int k = 0;

  auto recall = productivityProfiler->getRequirementRecall();
  while (k <= max_DH && y(k) < recall) {
    k = k + g;
  }
  return k == 0 ? 1 : k;
}

//Calculate the buffer size using the y function in K-Slack algorithm
auto BufferSizeManager::y(int K) -> double {
  //Calculate the select ratio
  double sel_radio = productivityProfiler->getSelectRatio(K);

  int m = streamCount;

  //Numerator of y function
  int numerator = 0;
  for (int i = 1; i <= m; i++) {
    int res = 1;
    for (int j = 1; j <= m; j++) {
      if (j == i) {
        continue;
      }
      std::string key = "Stream_" + std::to_string(j);
      int wj = cfg->getU64(key);
      int nj = wj / b;
      int sum = 0;
      for (int l = 1; l <= nj; l++) {
        sum += statisticsManager->wil(l, j, K);
      }
      res *= sum;
    }
    numerator += statisticsManager->fDk(0, i, K) * res;
  }

  //Denominator of y function
  int denominator = 0;
  for (int i = 1; i <= m; i++) {
    int res = 1;
    for (int j = 1; j <= m; j++) {
      if (j == i) {
        continue;
      }
      std::string key = "Stream_" + std::to_string(j);
      res *= cfg->getU64(key);;
    }
    denominator += res;
  }

  if (denominator == 0 || numerator == 0 || sel_radio == 0) {
    return 1;
  }

  return static_cast<int>(sel_radio * numerator / denominator);
}

auto BufferSizeManager::setConfig(INTELLI::ConfigMapPtr config) -> void {
  cfg = std::move(config);
}



