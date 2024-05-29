//
// Created by tony on 29/12/22.
//

#include <TestBench/ZipfDataLoader.h>
#include <filesystem>

using namespace INTELLI;
using namespace OoOJoin;

void ZipfDataLoader::generateKey() {
  zipfDataLoader_zipfKey = cfgGlobal->tryU64("zipfDataLoader_zipfKey", 1, true);

  if (zipfDataLoader_zipfKey) {
    zipfDataLoader_zipfKeyFactor = cfgGlobal->tryDouble("zipfDataLoader_zipfKeyFactor", 0.5, true);
    /**
     * @brief use zipf for key
     */
    INTELLI_INFO("Use zipf for key, factor=" + to_string(zipfDataLoader_zipfKeyFactor));
    md.setSeed(1);
    keyS = md.genZipfInt<keyType>(testSize, keyRange, zipfDataLoader_zipfKeyFactor);
    md.setSeed(100);
    keyR = md.genZipfInt<keyType>(testSize, keyRange, zipfDataLoader_zipfKeyFactor);
  } else {
    md.setSeed(1);
    keyS = md.genRandInt<keyType>(testSize, keyRange, 1);
    md.setSeed(100);
    keyR = md.genRandInt<keyType>(testSize, keyRange, 1);
  }
  INTELLI_INFO("Finish the generation of keys");
}

void ZipfDataLoader::generateValue() {
  zipfDataLoader_zipfValue = cfgGlobal->tryU64("zipfDataLoader_zipfValue", 1, true);

  if (zipfDataLoader_zipfValue) {
    zipfDataLoader_zipfValueFactor = cfgGlobal->tryDouble("zipfDataLoader_zipfValueFactor", 0.5, true);
    /**
     * @brief use zipf for value
     */
    INTELLI_INFO("Use zipf for value, factor=" + to_string(zipfDataLoader_zipfValueFactor));
    md.setSeed(999);
    valueS = md.genZipfInt<valueType>(testSize, valueRange, zipfDataLoader_zipfValueFactor);
    md.setSeed(114514);
    valueR = md.genZipfInt<valueType>(testSize, valueRange, zipfDataLoader_zipfValueFactor);
  } else {
    md.setSeed(999);
    valueS = md.genRandInt<valueType>(testSize, valueRange, 1);
    md.setSeed(114514);
    valueR = md.genRandInt<valueType>(testSize, valueRange, 1);
  }
  INTELLI_INFO("Finish the generation of value");

}

void ZipfDataLoader::generateEvent() {
  zipfDataLoader_zipfEvent = cfgGlobal->tryU64("zipfDataLoader_zipfEvent", 1, true);

  if (zipfDataLoader_zipfEvent) {
    zipfDataLoader_zipfEventFactor = cfgGlobal->tryDouble("zipfDataLoader_zipfEventFactor", 0.5, true);
    /**
     * @brief use zipf for event
     */
    INTELLI_INFO("Use zipf for event time, factor=" + to_string(zipfDataLoader_zipfEventFactor));
    md.setSeed(7758);
    eventS =
        md.genZipfTimeStamp<tsType>(testSize, (windowLenMs + maxArrivalSkewMs) * 1000,
                                    zipfDataLoader_zipfEventFactor);
    md.setSeed(258);
    eventR =
        md.genZipfTimeStamp<tsType>(testSize, (windowLenMs + maxArrivalSkewMs) * 1000,
                                    zipfDataLoader_zipfEventFactor);
  } else {
    uint64_t tsGrow = 1000 * timeStepUs / eventRateKTps;
    md.setSeed(7758);
    eventS = md.genSmoothTimeStamp<tsType>(testSize, timeStepUs, tsGrow);
    md.setSeed(258);
    eventR = md.genSmoothTimeStamp<tsType>(testSize, timeStepUs, tsGrow);
  }
  INTELLI_INFO("Finish the generation of event time");
}

void ZipfDataLoader::generateArrival() {
  vector<tsType> skewS, skewR;
  zipfDataLoader_zipfSkew = cfgGlobal->tryU64("zipfDataLoader_zipfSkew", 1, true);

  if (zipfDataLoader_zipfSkew) {
    zipfDataLoader_zipfSkewFactor = cfgGlobal->tryDouble("zipfDataLoader_zipfSkewFactor", 0.5, true);
    /**
     * @brief use zipf for event
     */
    INTELLI_INFO("Use zipf for arrival skew, factor=" + to_string(zipfDataLoader_zipfSkewFactor));
    md.setSeed(1024);
    skewS = md.genZipfInt<tsType>(testSize, maxArrivalSkewMs * 1000, zipfDataLoader_zipfSkewFactor);
    md.setSeed(4096);
    skewR = md.genZipfInt<tsType>(testSize, maxArrivalSkewMs * 1000, zipfDataLoader_zipfSkewFactor);

  } else {
    md.setSeed(1024);
    skewS = md.genRandInt<valueType>(testSize, maxArrivalSkewMs * 1000, 1);
    md.setSeed(4096);
    skewR = md.genRandInt<valueType>(testSize, maxArrivalSkewMs * 1000, 1);
  }
  arrivalS = genArrivalTime(eventS, skewS);
  arrivalR = genArrivalTime(eventR, skewR);
  INTELLI_INFO("Finish the generation of arrival time");
}

void ZipfDataLoader::generateFinal() {
  if (generateByKV) {
    sTuple = genTuplesByKV(keyValueSTuple, eventS, arrivalS);
    rTuple = genTuplesByKV(keyValueRTuple, eventR, arrivalR);
  } else {
    sTuple = genTuples(keyS, valueS, eventS, arrivalS);
    rTuple = genTuples(keyR, valueR, eventR, arrivalR);
  }
}

bool ZipfDataLoader::setConfig(ConfigMapPtr cfg) {

  cfgGlobal = cfg;
  /**
   * @brief load some common settings
   */
  md.setSeed(999);
  windowLenMs = cfg->tryU64("windowLenMs", 10);
  timeStepUs = cfg->tryU64("timeStepUs", 40);
  watermarkTimeMs = cfg->tryU64("watermarkTimeMs", 10);
  maxArrivalSkewMs = cfg->tryU64("maxArrivalSkewMs", 10 / 2);
  eventRateKTps = cfg->tryU64("eventRateKTps", 10);
  keyRange = cfg->tryU64("keyRange", 10, true);
  valueRange = cfg->tryU64("valueRange", 1000, true);
  generateByKV = cfg->tryU64("generateByKV", 0, true);
  inOrderData = cfg->tryU64("inOrderData", 0);
  string fnameR, fnameS;
  fnameR = cfg->tryString("fileDataLoader_rFile", "../../benchmark/datasets/rTuple.csv", true);
  fnameS = cfg->tryString("fileDataLoader_sFile", "../../benchmark/datasets/sTuple.csv", true);

  if (generateByKV) {
    testSize = (windowLenMs + maxArrivalSkewMs) * eventRateKTps;
    keyValueSTuple = loadDataFromCsvCheckSize(testSize, fnameS);
    keyValueRTuple = loadDataFromCsvCheckSize(testSize, fnameR);
    size_t minSRSize = keyValueSTuple.size();
    if (minSRSize < keyValueRTuple.size()) {
      minSRSize = keyValueRTuple.size();
    }
    INTELLI_INFO("load " + to_string(keyValueSTuple.size()) + " s Tuples, and " + to_string(keyValueRTuple.size())
                     + " R tuples");

    INTELLI_INFO("expect test size =" + to_string(testSize));
    if (testSize > minSRSize) {
      INTELLI_ERROR("Too few data, exit");
      exit(-1);
    }
    generateEvent();
    generateArrival();
    generateFinal();
    //exit(-1);
  } else {
    testSize = (windowLenMs + maxArrivalSkewMs) * eventRateKTps;
    generateKey();
    generateValue();
    generateEvent();
    generateArrival();
    generateFinal();
  }

  return true;
}

vector<TrackTuplePtr> ZipfDataLoader::getTupleVectorS() {
  return sTuple;
}

vector<TrackTuplePtr> ZipfDataLoader::getTupleVectorR() {
  return rTuple;
}

std::vector<TrackTuplePtr> ZipfDataLoader::loadDataFromCsv(std::string fname,
                                                           std::string separator,
                                                           std::string newLine) {
  std::vector<TrackTuplePtr> ru;
  ifstream ins;
  ins.open(fname);
  assert(separator.data());
  assert(newLine.data());
  if (ins.fail()) {
    INTELLI_ERROR("Can't open file [" + fname + "]");
    return ru;
  }
  std::string readStr;
  /**
   * header is of << "key,value,eventTime,arrivalTime,processedTime\n";
   * should read the first line
   */
  std::getline(ins, readStr, newLine.data()[0]);
  vector<std::string> cols;
  // readStr.erase(readStr.size()-1);
  spilt(readStr, separator, cols);
  size_t idxKey = 0, idxValue = 0;
  size_t validCols = 4;
  size_t validRows = 0;
  if (cols.size() < validCols) {
    INTELLI_ERROR("Invalid csv header [" + fname + "] return nothing");
    return ru;
  }

  /**
   * @note parase the first row here
   */
  for (size_t i = 0; i < cols.size(); i++) {
    if (cols[i] == "key") {
      idxKey = i;
    }
    if (cols[i] == "value") {
      idxValue = i;
    }
  }
  /**
   * @note deciding the valid rows
   */
  while (std::getline(ins, readStr, newLine.data()[0])) {
    if (cols.size() >= validCols) {
      validRows++;
    }
  }
  INTELLI_INFO("valid rows=" + to_string(validRows));
  // cout<<"valid rows="<<validRows<<endl;
  ru = std::vector<TrackTuplePtr>(validRows);
  /**
   * re-open file
   */
  ins.close();
  ins.open(fname);
  /**
   * jump the header,
   */
  size_t loadRow = 0;
  std::getline(ins, readStr, newLine.data()[0]);
  while (std::getline(ins, readStr, newLine.data()[0])) {
    vector<std::string> cols2;
    // readStr.erase(readStr.size()-1);
    spilt(readStr, separator, cols2);
    if (cols.size() >= validCols) {
      keyType k;
      valueType v;
      tsType et, at;
      istringstream k_ss(cols2[idxKey]);
      k_ss >> k;
      istringstream v_ss(cols2[idxValue]);
      v_ss >> v;
      TrackTuplePtr tp = newTrackTuple(k, v, et, at);
      tp->processedTime = 0;
      ru[loadRow] = tp;
      loadRow++;
    }
  }
  ins.close();
  return ru;
}

std::vector<TrackTuplePtr> ZipfDataLoader::loadDataFromCsvCheckSize(size_t size,
                                                                    std::string fname,
                                                                    std::string separator,
                                                                    std::string newLine) {
  std::vector<TrackTuplePtr> ru0, ru1;
  ru0 = loadDataFromCsv(fname, separator, newLine);
  ru1 = std::vector<TrackTuplePtr>(size);
  size_t ru0Size = ru0.size();
  for (size_t i = 0; i < size; i++) {
    size_t idx = i % ru0Size;
    TrackTuplePtr tp = newTrackTuple(ru0[idx]->key, ru0[idx]->payload, ru0[idx]->eventTime, ru0[idx]->arrivalTime);
    tp->processedTime = 0;
    ru1[i] = tp;
  }
  INTELLI_INFO(fname+"is read");
  return ru1;
}
