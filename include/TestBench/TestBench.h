/*! \file TestBench.h*/
//
// Created by tony on 23/11/22.
//

#ifndef INTELLISTREAM_INCLUDE_TESTBENCH_TESTBENCH_H_
#define INTELLISTREAM_INCLUDE_TESTBENCH_TESTBENCH_H_

#include <Common/Window.h>
#include <Operator/AbstractOperator.h>
#include <Utils/IntelliLog.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <TestBench/DataLoaderTable.h>

using namespace INTELLI;
#define TB_INFO INTELLI_INFO
#define TB_ERROR INTELLI_ERROR
#define TB_WARNNING INTELLI_WARNING
namespace OoOJoin {

/**
* @ingroup ADB_TESTBENCH The test bench to feed data into operators
* @{
 *
 */
/**
 * @class TestBench TestBench/TestBench
 * @brief The test bench class to feed data
 * @ingroup ADB_TESTBENCH
 * @note Require config if used
 * -"timeStep" U64 The simulation time step in us
 */
class TestBench {
 protected:
  //be used to make a comparation
  void inOrderSort(std::vector<TrackTuplePtr> &arr);

  //true sort
  void OoOSort(std::vector<TrackTuplePtr> &arr);

  void inlineTest();

  void forceInOrder(std::vector<TrackTuplePtr> &arr);
  /**
   * @breif gather the data to prepare for an ai pretrain
   */
  void aiPretrain();
  tsType timeStep = 1;

 private:

  void inlineTestOfCommon();

  void inlineTestOfMSWJ();

 public:
  std::vector<TrackTuplePtr> rTuple{};
  std::vector<TrackTuplePtr> sTuple{};
  AbstractOperatorPtr testOp{};
  ConfigMapPtr opConfig{};
  size_t AQPResult = 0;

  TestBench() = default;

  ~TestBench() = default;

  /**
   * @brief load a dataset according to the tag
   * @param tag the name tag of DataLoader
   * @param globalCfg the global config file to load dataset
   */
  void setDataLoader(const std::string &tag, ConfigMapPtr globalCfg);

  /**
   * @brief get the size of loaded s tuple
   * @return the size
   */
  size_t sizeOfS() const {
    return sTuple.size();
  }

  /**
  * @brief get the size of loaded r tuple
  * @return the size
  */
  size_t sizeOfR() const {
    return rTuple.size();
  }

  /**
   * @brief set the dataset to feed
   * @param _r The r tuples
   * @param _s The s tuples
   */
  void setDataSet(std::vector<TrackTuplePtr> _r, std::vector<TrackTuplePtr> _s);

  /**
  * @brief set the operator to test
  * @param op shared pointer to operator
   * @param cfg the config file to operator
   * @return whether the setting is successful
  */
  bool setOperator(AbstractOperatorPtr op, ConfigMapPtr cfg);

  /**
  * @brief test the operator under in order arrival
  * @param additionalSort whether or not additionally sort the input
  * @return the joined result
  */
  size_t inOrderTest(bool additionalSort);

  /**
  * @brief test the operator under OoO arrival
  * @param additionalSort whether or not additionally sort the input
  * @return the joined result
  */
  size_t OoOTest(bool additionalSort);

  /**
   * @brief print the rTuples to logging system
   * @param skipZero Whether skip the tuples whose processed time is zero
   */
  void logRTuples(bool skipZero = false);

  /**
   * @brief save the  rTuples to a file
   * @param fname the name of file
   * @param skipZero Whether skip the tuples whose processed time is zero
   * @return whether the file is written
   */
  bool saveRTuplesToFile(string fname, bool skipZero = false);

  /**
  * @brief save the  sTuples to a file
  * @param fname the name of file
  * @param skipZero Whether skip the tuples whose processed time is zero
  * @return whether the file is written
  */
  bool saveSTuplesToFile(string fname, bool skipZero = false);

  /**
   * @brief to compute average latency after run a test
   * @return the latency in us
   */
  double getAvgLatency();

  /**
   * @brief to compute the throughput after run a test
   * @return the throughput in tuples/s
   */
  double getThroughput();

  /**
   *  @brief to compute the latency t such that fraction of latency is below t
   *  @param fraction The fraction you want to set
   * @return the latency in us
   */
  double getLatencyPercentage(double fraction);

  /**
   * @brief get the break down information of processing time from the join operator tested
   * @warning should check the nullptr of output
   * @return The ConfigMapPtr which contains breakdown information, null if no breakdown supported
   */
  ConfigMapPtr getTimeBreakDown();
};
/**
 * @}
 */
}
#endif //INTELLISTREAM_INCLUDE_TESTBENCH_TESTBENCH_H_
