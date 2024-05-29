/*! \file ZipfDataLoader.h*/


#ifndef _INCLUDE_TESTBENCH_ZIPFDATALOADER_H_
#define _INCLUDE_TESTBENCH_ZIPFDATALOADER_H_

#include <Utils/ConfigMap.hpp>
#include <Common/Tuples.h>
#include <assert.h>
#include <Utils/IntelliLog.h>
#include <TestBench/AbstractDataLoader.h>
#include <vector>
#include <Utils/MicroDataSet.hpp>

using namespace INTELLI;
namespace OoOJoin {

/**
* @ingroup ADB_TESTBENCH The test bench to feed data into operators
* @{
 *
 */
/**
 * @defgroup ADB_TESTBENCH_DATALOADERS The classes of dataloader
 * @{
 */
/**
 * @class ZipfDataLoader TestBench/ZipfDataLoader.h
 * @brief The dataloader which allows zipf distribution of key, value, event time and arrival skewness
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @note:
 * - Must have a global config by @ref setConfig
 * - Can also have a modification config by
 *@note  Default behavior
* - create
* - setConfig and setModConfig (optional), generate R and S internally
* - call getTupleVectorS/R
 * @note require configs:
 * - zipfDataLoader_zipfKey U64, in key, 1 for zipf, 0 for random
 * - zipfDataLoader_zipfValue U64, in value, 1 for zipf, 0 for random
 * - zipfDataLoader_zipfEvent U64, in event time,  1 for zipf, 0 for random
 * - zipfDataLoader_zipfSkew U64, in arrival skewness,  1 for zipf, 0 for random
 * - zipfDataLoader_zipfKeyFactor Double, in key,0~1
 * - zipfDataLoader_zipfValueFactor Double, in value, 0~1
 * - zipfDataLoader_zipfEventFactor Double, in event time, 0~1
 * - zipfDataLoader_zipfSkewFactor Double, in event time, 0~1
 * - windowLenMs U64 The real world window length in ms
 * - timeStepUs U64 The simulation step in us
 * - maxArrivalSkewMs U64 The maximum real-world arrival skewness in ms
 * - eventRateKTps U64 The real-world rate of spawn event, in KTuples/s
 * - keyRange U64 The range of Key
 * - valueRange U64 The range of value
 * - generateByKV U64 Whether generate by using existing kv from file, default 0
 * - fileDataLoader_rFile String The file name of r tuple, valid under generateByKV=1
 * - fileDataLoader_sFile String The file name of s tuple, valid under generateByKV=1
 */
class ZipfDataLoader : public AbstractDataLoader {
 protected:
  tsType windowLenMs, timeStepUs, watermarkTimeMs, maxArrivalSkewMs, eventRateKTps;
  uint64_t zipfDataLoader_zipfKey, zipfDataLoader_zipfValue, zipfDataLoader_zipfEvent, zipfDataLoader_zipfSkew;
  uint64_t inOrderData;
  vector<keyType> keyS, keyR;
  vector<valueType> valueS, valueR;
  vector<tsType> eventS, eventR;
  vector<tsType> arrivalS, arrivalR;
  size_t testSize, keyRange, valueRange;
  double zipfDataLoader_zipfKeyFactor, zipfDataLoader_zipfValueFactor, zipfDataLoader_zipfEventFactor,
      zipfDataLoader_zipfSkewFactor;
  MicroDataSet md;

  vector<TrackTuplePtr> keyValueRTuple{};
  vector<TrackTuplePtr> keyValueSTuple{};
  bool generateByKV{};

  void spilt(const std::string s, const std::string &c, vector<std::string> &v) {
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while (std::string::npos != pos2) {
      v.push_back(s.substr(pos1, pos2 - pos1));

      pos1 = pos2 + c.size();
      pos2 = s.find(c, pos1);
    }
    if (pos1 != s.length())
      v.push_back(s.substr(pos1));
  }

  vector<tsType> genArrivalTime(vector<tsType> eventTime, vector<tsType> arrivalSkew) {
    vector<tsType> ru = vector<tsType>(eventTime.size());
    size_t len = (eventTime.size() > arrivalSkew.size()) ? arrivalSkew.size() : eventTime.size();
    if(inOrderData)
    {
      for (size_t i = 0; i < len; i++) {
      ru[i] = eventTime[i];
     }
    }
    else{
       for (size_t i = 0; i < len; i++) {
      ru[i] = eventTime[i] + arrivalSkew[i];
      }
    }
   
    return ru;
  }

  vector<OoOJoin::TrackTuplePtr> genTuples(vector<keyType> keyS,
                                           vector<valueType> valueS,
                                           vector<tsType> eventS,
                                           vector<tsType> arrivalS) {
    size_t len = keyS.size();
    vector<OoOJoin::TrackTuplePtr> ru = vector<OoOJoin::TrackTuplePtr>(len);
    for (size_t i = 0; i < len; i++) {
      ru[i] = newTrackTuple(keyS[i], valueS[i], eventS[i], arrivalS[i]);
    }
    return ru;
  }

  vector<OoOJoin::TrackTuplePtr> genTuplesByKV(std::vector<TrackTuplePtr> keyValueData,
                                               vector<tsType> eventS,
                                               vector<tsType> arrivalS) {
    size_t len = keyValueData.size();
    if (len >= testSize) {
      len = testSize;
    }
    vector<OoOJoin::TrackTuplePtr> ru = vector<OoOJoin::TrackTuplePtr>(len);
    for (size_t i = 0; i < len; i++) {
      ru[i] = newTrackTuple(keyValueData[i]->key, keyValueData[i]->payload, eventS[i], arrivalS[i]);
    }
    return ru;
  }

  /**
  * @brief load a key value dataset from csv file
  * @param fname The name of file
  * @param separator The separator in .csv, default is ","
  * @param newLine THe indicator of a new line. default is "\n"
  * @return The vector of TrackTuplePtr
  */
  std::vector<TrackTuplePtr> loadDataFromCsv(std::string fname,
                                             std::string separator = ",",
                                             std::string newLine = "\n");
  /**
 * @brief load a key value dataset from csv file and check the size to be aligned with
   *  @param size The size to be checked
 * @param fname The name of file
 * @param separator The separator in .csv, default is ","
 * @param newLine THe indicator of a new line. default is "\n"
 * @return The vector of TrackTuplePtr
 */
  std::vector<TrackTuplePtr> loadDataFromCsvCheckSize(size_t size, std::string fname,
                                                      std::string separator = ",",
                                                      std::string newLine = "\n");

  /**
   * @brief generate the vector of key
   */
  void generateKey();

  /**
   * @brief enerate the vector of key
   */
  void generateValue();

  /**
   *
   *  @brief generate the vector of event
   */
  void generateEvent();

  /**
   * @brief  generate the vector of arrival
   */
  void generateArrival();

  /**
   * @brief generate the final result of s and r
   */
  void generateFinal();

 public:
  ConfigMapPtr cfgGlobal;
  vector<TrackTuplePtr> sTuple, rTuple;

  ZipfDataLoader() = default;

  ~ZipfDataLoader() = default;

  /**
 * @brief Set the GLOBAL config map related to this loader
 * @param cfg The config map
  * @return bool whether the config is successfully set
 */
  bool setConfig(ConfigMapPtr cfg) override;

  /**
* @brief Set the modification config map related to this loader
* @param cfg The config map
* @return bool whether the config is successfully set
*/
  bool setModConfig(ConfigMapPtr cfg) override {
    return AbstractDataLoader::setModConfig(cfg);
  }

  /**
   * @brief get the vector of s tuple
   * @return the vector
   */
  vector<TrackTuplePtr> getTupleVectorS() override;

  /**
  * @brief get the vector of R tuple
  * @return the vector
  */
  vector<TrackTuplePtr> getTupleVectorR() override;
};

/**
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @typedef ZipfDataLoaderPtr
 * @brief The class to describe a shared pointer to @ref ZipfDataLoader

 */
typedef std::shared_ptr<class ZipfDataLoader> ZipfDataLoaderPtr;
/**
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @def newZipfDataLoader
 * @brief (Macro) To creat a new @ref  ZipfDataLoader under shared pointer.
 */
#define newZipfDataLoader std::make_shared<OoOJoin::ZipfDataLoader>
/**
 * @}
 */
/**
 *
 */
}
#endif //INTELLISTREAM_INCLUDE_TESTBENCH_ZipfDataLoader_H_
