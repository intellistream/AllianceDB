/*! \file OoOJoin.h*/
//
// Created by tony on 22/11/22.
//

#ifndef INTELLISTREAM_OOOJOIN_H
#define INTELLISTREAM_OOOJOIN_H

/**
 *
 * @mainpage Introduction
 * The OoOJoin project contains the basic ideas and evaluation on a novel join operator that supports error-bounded processing
 * on out-of-order tuples
 * @section Sys_overview  System Overview
 * The whole work is complete join operator, including basic windowing, buffering and join computation. However, watermark
 * generation is the major focus.
 * \image html System.png
 * @section BENCH_MARK Benchmark Tips
 * usage: ./benchmark [configfile]
 * @note Require configs in configfile
 * - "windowLenMs" U64 The real world window length in ms
 * - "timeStepUs" U64 The simulation step in us
 * - "wmTag" String The name of watermark generator, see WMTable class for all valid tags
 * - "watermarkTimeMs" U64 The time of watermark assertion in ms, if use the ArrivalWM to generate watermark
 * - "latenessMs" U64 The time of max allowed event lateness in ms, if use the LatenessWM to generate watermark
 * - "maxArrivalSkewMs" U64 The maximum real-world arrival skewness in ms
 * - "eventRateKTps" U64 The real-world rate of spawn event, in KTuples/s
 * - "keyRange" U64 The range of Key
 * - "operator" String The operator to be used, see OperatorTable class for all valid tags
 * - "dataLoader"  String The dataloader to be used, see DataLoaderTable class for all valid tags
 * @section Code_overview  Code Overview
 * The code structure aligns well with system design.
 * \image html UML_ALL.png
 * @section sec_extend How to extend
 * @subsection subsec_extend_dataloader How to extend a new DataLoader
 * - copy the TestBench/AbstractDataLoader.h, to your extension of TestBench/xxx.h, assume xxx is your new dataloader
 * - make it inherited from AbstractDataLoader
 * - creat the corresponding TestBench/xxx.cpp, fill in the virtual functions
 * - your extension may require extra specific configs, please name as xxx_yyy, and then state them in TestBench/xxx.h and leave a default value in your setConfig
 * - edit the TestBench/DataLoaderTable.cpp so the benchmark system can find it by tag
 * - assign "dataLoader" in global config file into your own tag of xxx when running, so the data will be loaded by your xxx
 * @subsection subsec_extend_operator How to extend a new Operator
 * - copy the Operator/AbstractOperator.h, to your extension of Operator/xxx.h, assume xxx is your new Operator
 * - make it inherited from AbstractOperator
 * - creat the corresponding Operator/xxx.cpp, fill in the virtual functions
 * - your extension may require extra specific configs, please name as xxx_yyy, and then state them in Operator/xxx.h and leave a default value in your setConfig
 * - edit the Operator/OperatorTable.cpp so the benchmark system can find it by tag
 * - assign "operator" in global config file into your own tag of xxx when running, so the data will be processed by your xxx operator
 */
/**
*
*/
//The groups of modules
/**
 * @mainpage Code Structure
 *  @section Code_Structure  Code Structure
 */
/**
 * @subsection code_stru_common Common
 * This folder contains the common definitions of Window, Tuple, etc.
 * * @defgroup INTELLI_COMMON_BASIC Basic Definitions and Data Structures
 * @{
 * We define the classes of Tuple, Window, queue, etc. here
 **/
#include <Common/Tuples.h>
#include <Common/Window.h>
#include <Common/StateOfKey.h>
#include <Common/LinearVAE.h>
#include <Common/LinearSVI.h>
/**
 * @}
 */
/***
 *  @subsection code_stru_utils Utils
* This folder contains the public utils shared by INTELISTREAM team and some third party dependencies.
 **/
/**
* @defgroup INTELLI_UTIL Shared Utils
* @{
*/
#include <Utils/MicroDataSet.hpp>
#include <Utils/SPSCQueue.hpp>
#include <Utils/ConfigMap.hpp>
/**
 * @ingroup INTELLI_UTIL
* @defgroup INTELLI_UTIL_OTHERC20 Other common class or package under C++20 standard
* @{
* This package covers some common C++20 new features, such as std::thread to ease the programming
*/
#include <Utils/C20Buffers.hpp>
#include <Utils/ThreadPerf.hpp>
#include <Utils/IntelliLog.h>
/**
 * @}
 */
/**
 *
 * @}
 */
/**
 *  @defgroup ADB_JOINALGOS The specific join algorithms
 *  @{
 */
#include <JoinAlgos/AbstractJoinAlgo.h>
#include <JoinAlgos/NestedLoopJoin.h>
#include <JoinAlgos/JoinAlgoTable.h>
#include <JoinAlgos/NPJ/NPJ.h>
/**
 * @}
 */
/**
 *  @defgroup ADB_OPERATORS The top entity of join operators
 *  @{
 */
#include <Operator/AbstractOperator.h>
#include <Operator/IAWJOperator.h>
#include <Operator/MeanAQPIAWJOperator.h>
#include <Operator/OperatorTable.h>
#include <Operator/MSWJOperator.h>
#include <Operator/AIOperator.h>
#include <Operator/LinearSVIOperator.h>
#include <Operator/IAWJSelOperator.h>
/**
 * @}
 */
/**
*  @defgroup Parallelization The parallelization part
*  @{
*/
#include <Parallelization/KeyPartitionRunner.h>
#include <Parallelization/KeyPartitionInfiniteRunner.h>
#include <Parallelization/RoundRobinRunner.h>
/**
 * @}
 */
/**
 * @defgroup ADB_TESTBENCH The test bench
 * @{
 */
#include <TestBench/TestBench.h>
#include <TestBench/DataLoaderTable.h>
/**
 * @}
 */
/**
 * @defgroup ADB_WATERMARKER The watermark generator
 * This package is the key to the whole operator and generate the watermark to control processing
 * @{
 */
#include <WaterMarker/AbstractWaterMarker.h>
#include <WaterMarker/LatenessWM.h>
/**
 * @}
 */
#endif //INTELLISTREAM_OOOJOIN_H
