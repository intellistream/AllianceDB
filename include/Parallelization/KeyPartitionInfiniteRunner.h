/*! \file   KeyPartitionInfiniteRunner.h*/

//
// Created by tony on 27/06/23.
//

#ifndef INTELLISTREAM_INCLUDE_PARALLELIZATION_keyinfRUNNER_H_
#define INTELLISTREAM_INCLUDE_PARALLELIZATION_keyinfRUNNER_H_
#include <Parallelization/KeyPartitionRunner.h>
namespace OoOJoin {
/**
 * @class KeyPartitionInfiniteWorker Parallelization/KeyPartitionInfiniteRunner.h
 * @ingroup Parallelization
 * @brief the working thread class of key partition parallelization
 */
    class KeyPartitionInfiniteWorker : public OoOJoin::KeyPartitionWorker {
    protected:
        /**
         * @brief the time struct of the whole system
         */
        virtual void inlineMain();
        /**
         * @brief the inline main function of decentralized mode, assume there is no central distributor of datastream
         */
        void decentralizedMain();

        /**
         * @brief the inline main function of centralized mode, assume there is a central distributor of datastream
         */
        void centralizedMain();

    public:

        KeyPartitionInfiniteWorker() {}
        ~KeyPartitionInfiniteWorker() {}

    };
/**
 * @ingroup Parallelization
 * @typedef KeyPartitionInfiniteWorkerPtr
 * @brief The class to describe a shared pointer to @ref KeyPartitionInfiniteWorker
 */
    typedef std::shared_ptr<class KeyPartitionInfiniteWorker> KeyPartitionInfiniteWorkerPtr;
/**
 * @ingroup Parallelization
 * @def newKeyPartitionInfiniteWorker
 * @brief (Macro) To creat a new @ref newKeyPartitionInfiniteWorker under shared pointer.
 */
#define newKeyPartitionInfiniteWorker std::make_shared<OoOJoin::KeyPartitionInfiniteWorker>
/**
 * @class KeyPartitionInfiniteRunner Parallelization/KeyPartitionInfiniteRunner.h
 * @ingroup Parallelization
 * @brief the top class of key partition parallelization, which manages the @ref KeyPartitionInfiniteWorker
 * @note default behaviors
 * - create
 * - call @ref setConfig
 * - call @ref setDataSet
 * -call @ref runStreaming
 * -call @ref getResult, @ref getAQPResult etc for the results
 */
    class KeyPartitionInfiniteRunner : public KeyPartitionRunner {
    protected:
        //std::vector<OoOJoin::KeyPartitionInfiniteWorkerPtr> myWorker;

    public:
        KeyPartitionInfiniteRunner() {}
        ~KeyPartitionInfiniteRunner() {}
        /**
       * @brief set the dataset to feed
       * @param _r The r tuples
       * @param _s The s tuples
       */
        void setDataSet(std::vector<TrackTuplePtr> _r, std::vector<TrackTuplePtr> _s);
        void setConfig(INTELLI::ConfigMapPtr _cfg);
        /**
         * @brief to run the multithread streaming process
         */
        virtual void runStreaming(void);
        /**
      * @brief to compute the throughput after run a test
      * @return the throughput in tuples/s
      */
        virtual double getThroughput();

        /**
         *  @brief to compute the latency t such that fraction of latency is below t
         *  @param fraction The fraction you want to set
         * @return the latency in us
         */
        virtual double getLatencyPercentage(double fraction);

        /**
        * @brief get the joined sum result
        * @return The result
        */
        virtual size_t getResult();

        /**
         * @brief get the joined sum result under AQP
         * @return The result
         */
        virtual size_t getAQPResult();
    };
/**
 * @ingroup Parallelization
 * @typedef KeyPartitionInfiniteRunnerPtr
 * @brief The class to describe a shared pointer to @ref KeyPartitionInfinite
 */
    typedef std::shared_ptr<class OoOJoin::KeyPartitionInfiniteRunner> KeyPartitionInfiniteRunnerPtr;
/**
 * @ingroup Parallelization
 * @def newKeyPartitionInfiniteRunner
 * @brief (Macro) To creat a new @ref  KeyPartitionInfiniteRunner under shared pointer.
 */
#define newKeyPartitionInfiniteRunner std::make_shared<OoOJoin::KeyPartitionInfiniteRunner>
} // OoOJoin

#endif //INTELLISTREAM_INCLUDE_PARALLELIZATION_KEYPARTITIONRUNNER_H_
