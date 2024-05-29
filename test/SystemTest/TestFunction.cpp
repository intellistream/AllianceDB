
#include <OoOJoin.h>
#include <vector>

using namespace std;
using namespace OoOJoin;

constexpr static int SECONDS = 1000000;
constexpr static int MICRO_SECONDS = 1;
constexpr static int MILLION_SECONDS = 1000;

vector<tsType> genArrivalTime(vector<tsType> eventTime, vector<tsType> arrivalSkew) {
  vector<tsType> ru = vector<tsType>(eventTime.size());
  size_t len = (eventTime.size() > arrivalSkew.size()) ? arrivalSkew.size() : eventTime.size();
  for (size_t i = 0; i < len; i++) {
    ru[i] = eventTime[i] + arrivalSkew[i];
  }
  return ru;
}

void bubble_sort(vector<OoOJoin::TrackTuplePtr> &arr) {
  size_t i, j;
  TrackTuplePtr temp;
  size_t len = arr.size();
  for (i = 0; i < len - 1; i++)
    for (j = 0; j < len - 1 - i; j++)
      if (arr[j]->arrivalTime > arr[j + 1]->arrivalTime) {
        temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
}

vector<OoOJoin::TrackTuplePtr> genTuples(vector<keyType> keyS, vector<tsType> eventS, vector<tsType> arrivalS) {
  size_t len = keyS.size();
  vector<OoOJoin::TrackTuplePtr> ru = vector<OoOJoin::TrackTuplePtr>(len);
  for (size_t i = 0; i < len; i++) {
    ru[i] = newTrackTuple(keyS[i], 0, eventS[i], arrivalS[i]);
  }
  bubble_sort(ru);
  return ru;
}

vector<TrackTuplePtr> genTuplesSmooth(size_t testSize,
                                      uint64_t keyRange,
                                      uint64_t rateKtps,
                                      uint64_t groupUnit,
                                      uint64_t maxSkewUs,
                                      uint64_t seed = 999) {
  MicroDataSet ms(seed);
  uint64_t tsGrow = 1000 * groupUnit / rateKtps;
  vector<keyType> keyS = ms.genRandInt<keyType>(testSize, keyRange, 1);
  vector<tsType> eventS = ms.genSmoothTimeStamp<tsType>(testSize, groupUnit, tsGrow);
  vector<tsType> arrivalSkew = ms.genRandInt<tsType>(testSize, maxSkewUs, 1);
  vector<tsType> arrivalS = genArrivalTime(eventS, arrivalSkew);
  vector<TrackTuplePtr> sTuple = genTuples(keyS, eventS, arrivalS);
  return sTuple;
}

/**
   * @brief Try to get an U64 from config map, if not exist, use default value instead
   * @param cfg The config map
   * @param key The key
   * @param defaultValue The default
   * @return The returned value
   */
uint64_t tryU64(ConfigMapPtr config, string key, uint64_t defaultValue = 0) {
  uint64_t ru = defaultValue;
  if (config->existU64(key)) {
    ru = config->getU64(key);
    // INTELLI_INFO(key + " = " + to_string(ru));
  } else {
    //  WM_WARNNING("Leaving " + key + " as blank, will use " + to_string(defaultValue) + " instead");
  }
  return ru;
}

/**
   * @brief Try to get an String from config map, if not exist, use default value instead
   * @param cfg The config map
   * @param key The key
   * @param defaultValue The default
   * @return The returned value
   */
string tryString(ConfigMapPtr config, string key, string defaultValue = "") {
  string ru = defaultValue;
  if (config->existString(key)) {
    ru = config->getString(key);
    //INTELLI_INFO(key + " = " + (ru));
  } else {
    // WM_WARNNING("Leaving " + key + " as blank, will use " + (defaultValue) + " instead");
  }
  return ru;
}

MSWJOperatorPtr mswjConfiguration(ConfigMapPtr cfg) {
  MSWJOperatorPtr mswj;
  cfg->edit("g", (uint64_t) 10 * MILLION_SECONDS);
  cfg->edit("L", (uint64_t) 50 * MILLION_SECONDS);
  cfg->edit("userRecall", 0.4);
  cfg->edit("b", (uint64_t) 10 * MILLION_SECONDS);
  cfg->edit("confidenceValue", 0.5);
  cfg->edit("P", (uint64_t) 10 * SECONDS);
  cfg->edit("maxDelay", (uint64_t) INT16_MAX);
  cfg->edit("StreamCount", (uint64_t) 2);
  cfg->edit("Stream_1", (uint64_t) 0);
  cfg->edit("Stream_2", (uint64_t) 0);

  auto tupleProductivityProfiler = std::make_shared<MSWJ::TupleProductivityProfiler>(cfg);
  auto statisticsManager = std::make_shared<MSWJ::StatisticsManager>(tupleProductivityProfiler.get(), cfg);
  auto bufferSizeManager = std::make_shared<MSWJ::BufferSizeManager>(statisticsManager.get(),
                                                                     tupleProductivityProfiler.get());
  auto streamOperator = std::make_shared<MSWJ::StreamOperator>(tupleProductivityProfiler.get(), cfg);
  auto synchronizer = std::make_shared<MSWJ::Synchronizer>(2, streamOperator.get(), cfg);

  bufferSizeManager->setConfig(cfg);

  auto kSlackS = std::make_shared<MSWJ::KSlack>(1, bufferSizeManager.get(), statisticsManager.get(),
                                                synchronizer.get());
  auto kSlackR = std::make_shared<MSWJ::KSlack>(2, bufferSizeManager.get(), statisticsManager.get(),
                                                synchronizer.get());

  mswj = std::make_shared<MSWJOperator>(bufferSizeManager, tupleProductivityProfiler,
                                        synchronizer,
                                        streamOperator, statisticsManager, kSlackR, kSlackS);
  mswj->setConfig(cfg);

  return mswj;
}

int runTestBenchAdj(const ConfigMapPtr &cfg, const string &configName = "config.csv", const string &outPrefix = "") {
  INTELLI_INFO("Load global config from" + configName + ", output prefix = " + outPrefix);
  OperatorTablePtr opTable = newOperatorTable();

  size_t OoORu = 0, realRu = 0;
  tsType windowLenMs, timeStepUs, watermarkTimeMs;
  string operatorTag = "IMA";
  string loaderTag = "random";

  windowLenMs = cfg->tryU64("windowLenMs", 10, true);
  timeStepUs = cfg->tryU64("timeStepUs", 40, true);
  watermarkTimeMs = cfg->tryU64("watermarkTimeMs", 10, true);
  //maxArrivalSkewMs = cfg->tryU64("maxArrivalSkewMs", 10 / 2);

  operatorTag = cfg->tryString("operator", "IMA");
  loaderTag = cfg->tryString("dataLoader", "random");
  INTELLI_INFO("window len= " + to_string(windowLenMs) + "ms ");
  INTELLI_INFO("Try use " + operatorTag + " operator");

  AbstractOperatorPtr iawj;
  MSWJOperatorPtr mswj;

  if (operatorTag == "IAWJ") {
    iawj = newIAWJOperator();
  } else if (operatorTag == "MSWJ") {
    mswj = mswjConfiguration(cfg);
  } else {
    iawj = opTable->findOperator(operatorTag);
  }

  if (operatorTag != "MSWJ" && iawj == nullptr) {
    iawj = newIAWJOperator();
    INTELLI_INFO("No " + operatorTag + " operator, will use IAWJ instead");
  }

  //Global configs
  cfg->edit("windowLen", (uint64_t) windowLenMs * 1000);
  cfg->edit("timeStep", (uint64_t) timeStepUs);
  cfg->edit("watermarkTime", (uint64_t) watermarkTimeMs * 1000);



  TestBench tb, tbOoO;
  tbOoO.setDataLoader(loaderTag, cfg);

  cfg->edit("rLen", (uint64_t) tbOoO.sizeOfS());
  cfg->edit("sLen", (uint64_t) tbOoO.sizeOfR());
  cfg->edit("latenessMs", (uint64_t) 0);
  cfg->edit("earlierEmitMs", (uint64_t) 0);

  if (operatorTag == "MSWJ") {
    tbOoO.setOperator(mswj, cfg);
  } else {
    tbOoO.setOperator(iawj, cfg);
  }

  INTELLI_INFO("/****run OoO test of  tuples***/");
  OoORu = tbOoO.OoOTest(true);

  INTELLI_DEBUG("OoO Confirmed joined " + to_string(OoORu));
  INTELLI_DEBUG("OoO AQP joined " + to_string(tbOoO.AQPResult));

  ConfigMap generalStatistics;
  generalStatistics.edit("AvgLatency", (double) tbOoO.getAvgLatency());
  generalStatistics.edit("95%Latency", (double) tbOoO.getLatencyPercentage(0.95));
  generalStatistics.edit("Throughput", (double) tbOoO.getThroughput());

  INTELLI_DEBUG("95% latency (us)=" + to_string(tbOoO.getLatencyPercentage(0.95)));
  INTELLI_DEBUG("Throughput (TPs/s)=" + to_string(tbOoO.getThroughput()));

  tbOoO.saveRTuplesToFile(outPrefix + "_tuples.csv", true);
  tbOoO.saveRTuplesToFile(outPrefix + "_arrived_tuplesR.csv", false);
  tbOoO.saveSTuplesToFile(outPrefix + "_arrived_tuplesS.csv", false);

  ConfigMapPtr resultBreakDown = tbOoO.getTimeBreakDown();
  if (resultBreakDown != nullptr) {
    resultBreakDown->toFile(outPrefix + "_breakdown.csv");
  }

  tb.setDataLoader(loaderTag, cfg);

  if (operatorTag == "MSWJ") {
    tb.setOperator(newIAWJOperator(), cfg);
  } else {
    tb.setOperator(newIAWJOperator(), cfg);
  }

  realRu = tb.inOrderTest(true);

  INTELLI_DEBUG("Expect " + to_string(realRu));
  double err = OoORu;
  err = (err - realRu) / realRu;
  generalStatistics.edit("Error", (double) err);

  INTELLI_DEBUG("OoO AQP joined " + to_string(tbOoO.AQPResult));

  err = tbOoO.AQPResult;
  err = (err - realRu) / realRu;
  generalStatistics.edit("AQPError", (double) err);

  INTELLI_DEBUG("Error = " + to_string(err));

  generalStatistics.toFile(outPrefix + "_general.csv");

  return 1;
}

int runTestBenchPretrain(const ConfigMapPtr &cfg,
                         const string &configName = "config.csv",
                         const string &outPrefix = "") {
  INTELLI_INFO("Load global config from" + configName + ", output prefix = " + outPrefix);
  OperatorTablePtr opTable = newOperatorTable();

  size_t OoORu = 0;
  tsType windowLenMs, timeStepUs, watermarkTimeMs;
  string operatorTag = "IMA";
  string loaderTag = "random";

  windowLenMs = cfg->tryU64("windowLenMs", 10, true);
  timeStepUs = cfg->tryU64("timeStepUs", 40, true);
  watermarkTimeMs = cfg->tryU64("watermarkTimeMs", 10, true);
  //maxArrivalSkewMs = cfg->tryU64("maxArrivalSkewMs", 10 / 2);

  operatorTag = cfg->tryString("operator", "IMA");
  loaderTag = cfg->tryString("dataLoader", "random");
  INTELLI_INFO("window len= " + to_string(windowLenMs) + "ms ");
  INTELLI_INFO("Try use " + operatorTag + " operator");

  AbstractOperatorPtr iawj;
  MSWJOperatorPtr mswj;

  if (operatorTag == "IAWJ") {
    iawj = newIAWJOperator();
  } else if (operatorTag == "MSWJ") {
    mswj = mswjConfiguration(cfg);
  } else {
    iawj = opTable->findOperator(operatorTag);
  }

  if (operatorTag != "MSWJ" && iawj == nullptr) {
    iawj = newIAWJOperator();
    INTELLI_INFO("No " + operatorTag + " operator, will use IAWJ instead");
  }

  //Global configs
  cfg->edit("windowLen", (uint64_t) windowLenMs * 1000);
  cfg->edit("timeStep", (uint64_t) timeStepUs);
  cfg->edit("watermarkTime", (uint64_t) watermarkTimeMs * 1000);

  TestBench tbOoO;
  tbOoO.setDataLoader(loaderTag, cfg);

  cfg->edit("rLen", (uint64_t) tbOoO.sizeOfS());
  cfg->edit("sLen", (uint64_t) tbOoO.sizeOfR());
  cfg->edit("latenessMs", (uint64_t) 0);
  cfg->edit("earlierEmitMs", (uint64_t) 0);

  if (operatorTag == "MSWJ") {
    tbOoO.setOperator(mswj, cfg);
  } else {
    tbOoO.setOperator(iawj, cfg);
  }

  INTELLI_INFO("/****run OoO test of  tuples***/");
  OoORu = tbOoO.OoOTest(true);

  INTELLI_DEBUG("OoO Confirmed joined " + to_string(OoORu));
  INTELLI_DEBUG("OoO AQP joined " + to_string(tbOoO.AQPResult));

  ConfigMap generalStatistics;
  generalStatistics.edit("AvgLatency", (double) tbOoO.getAvgLatency());
  generalStatistics.edit("95%Latency", (double) tbOoO.getLatencyPercentage(0.95));
  generalStatistics.edit("Throughput", (double) tbOoO.getThroughput());

  INTELLI_DEBUG("95% latency (us)=" + to_string(tbOoO.getLatencyPercentage(0.95)));
  INTELLI_DEBUG("Throughput (TPs/s)=" + to_string(tbOoO.getThroughput()));

  tbOoO.saveRTuplesToFile(outPrefix + "_tuples.csv", true);
  tbOoO.saveRTuplesToFile(outPrefix + "_arrived_tuplesR.csv", false);
  tbOoO.saveSTuplesToFile(outPrefix + "_arrived_tuplesS.csv", false);

  return 1;
}
