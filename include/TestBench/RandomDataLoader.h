/*! \file RandomDataLoader.h*/
//
// Created by tony on 29/12/22.
//

#ifndef _INCLUDE_TESTBENCH_RandomDataLoader_H_
#define _INCLUDE_TESTBENCH_RandomDataLoader_H_

#include <TestBench/AbstractDataLoader.h>
#include <Utils/MicroDataSet.hpp>

using namespace INTELLI;
namespace OoOJoin {

/**
* @ingroup ADB_TESTBENCH The test bench to feed data into operators
* @{
 *
 */
/**
 * @class RandomDataLoader TestBench/RandomDataLoader.h
 * @brief The dataloader which produces random key, random value and random skewness
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @note:
 * Default behavior
 * - create
 * - setConfig and setModConfig (optional), generate R and S internally
 * - call getTupleVectorS/R and getSizeS/R;
 * @note require configs:
 * - "windowLenMs" U64 The real world window length in ms
 * - "timeStepUs" U64 The simulation step in us
 * - "watermarkTimeMs" U64 The real world watermark generation period in ms
 * - "maxArrivalSkewMs" U64 The maximum real-world arrival skewness in ms
 * - "eventRateKTps" U64 The real-world rate of spawn event, in KTuples/s
 * - "keyRange" U64 The range of Key
 */
class RandomDataLoader : public AbstractDataLoader {
 protected:
  tsType windowLenMs, timeStepUs, watermarkTimeMs, maxArrivalSkewMs, eventRateKTps;

  static vector<tsType> genArrivalTime(vector<tsType> eventTime, vector<tsType> arrivalSkew) {
    vector<tsType> ru = vector<tsType>(eventTime.size());
    size_t len = (eventTime.size() > arrivalSkew.size()) ? arrivalSkew.size() : eventTime.size();
    for (size_t i = 0; i < len; i++) {
      ru[i] = eventTime[i] + arrivalSkew[i];
    }
    return ru;
  }

  static void std_sort(vector<OoOJoin::TrackTuplePtr> &arr) {
    std::sort(arr.begin(), arr.end(), [](const TrackTuplePtr &t1, const TrackTuplePtr &t2) {
      return t1->arrivalTime < t2->arrivalTime;
    });
  }

  static vector<OoOJoin::TrackTuplePtr> genTuples(vector<keyType> keyS,
                                                  vector<tsType> eventS,
                                                  vector<tsType> arrivalS) {
    size_t len = keyS.size();
    vector<OoOJoin::TrackTuplePtr> ru = vector<OoOJoin::TrackTuplePtr>(len);
    for (size_t i = 0; i < len; i++) {
      ru[i] = newTrackTuple(keyS[i], 0, eventS[i], arrivalS[i]);
    }
    std_sort(ru);
    return ru;
  }

  static vector<TrackTuplePtr> genTuplesSmooth(size_t testSize,
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
    vector<TrackTuplePtr> genTuple = genTuples(keyS, eventS, arrivalS);
    return genTuple;
  }

 public:
  ConfigMapPtr cfgGlobal;
  vector<TrackTuplePtr> sTuple, rTuple;

  RandomDataLoader() = default;

  ~RandomDataLoader() = default;

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
    assert(cfg);
    return true;
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
 * @typedef RandomDataLoaderPtr
 * @brief The class to describe a shared pointer to @ref RandomDataLoader

 */
typedef std::shared_ptr<class RandomDataLoader> RandomDataLoaderPtr;
/**
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @def newRandomDataLoader
 * @brief (Macro) To creat a new @ref RandomDataLoader under shared pointer.
 */
#define newRandomDataLoader std::make_shared<OoOJoin::RandomDataLoader>

/**
 *
 */
}
#endif //INTELLISTREAM_INCLUDE_TESTBENCH_RandomDataLoader_H_
