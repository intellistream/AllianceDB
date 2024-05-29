//
// Created by tony on 27/06/23.
//

#include <Parallelization/KeyPartitionInfiniteRunner.h>
#include <string>

void OoOJoin::KeyPartitionInfiniteWorker::decentralizedMain() {

    INTELLI_INFO("Start tid" + to_string(myId) + " run");
    INTELLI::UtilityFunctions::bind2Core(myId);
    size_t testSize = (rTuple.size() > sTuple.size()) ? sTuple.size() : rTuple.size();
    size_t pos=0;
    gettimeofday(&tSystem, nullptr);
    testOp->syncTimeStruct(tSystem);
    testOp->start();
    while (pos<testSize)
    {
        auto newTs=sTuple[pos];
        if(isMySTuple(newTs))
        {
            testOp->feedTupleS(newTs);
        }
        auto newTr=rTuple[pos];
        if(isMyRTuple(newTr))
        {
            testOp->feedTupleR(newTr);
        }
        pos++;
    }
    testOp->stop();
    INTELLI_INFO("End tid" + to_string(myId) + " run");
}

void OoOJoin::KeyPartitionInfiniteWorker::inlineMain() {
    decentralizedMain();
}

void OoOJoin::KeyPartitionInfiniteRunner::setConfig(INTELLI::ConfigMapPtr _cfg) {
    cfg = _cfg;
    threads = cfg->tryU64("threads", 1, true);
    myWorker = std::vector<OoOJoin::KeyPartitionWorkerPtr>(threads);
    for (uint64_t i = 0; i < threads; i++) {
        myWorker[i] = newKeyPartitionInfiniteWorker();
        myWorker[i]->setConfig(cfg);
        myWorker[i]->setId(i, threads);
    }
}
void OoOJoin::KeyPartitionInfiniteRunner::runStreaming() {
    for (uint64_t i = 0; i < threads; i++) {
        myWorker[i]->prepareRunning();
    }
    for (uint64_t i = 0; i < threads; i++) {
        myWorker[i]->startThread();
    }
    INTELLI_INFO("Start " + to_string(threads) + " threads");
    for (uint64_t i = 0; i < threads; i++) {
        myWorker[i]->joinThread();
    }
    INTELLI_INFO("Finish " + to_string(threads) + " threads");
}
size_t OoOJoin::KeyPartitionInfiniteRunner::getResult() {
    size_t ru = 0;
    for (uint64_t i = 0; i < threads; i++) {
        ru += myWorker[i]->getResult();
    }
    return ru;
}

void OoOJoin::KeyPartitionInfiniteRunner::setDataSet(std::vector<TrackTuplePtr> _r, std::vector<TrackTuplePtr> _s) {
    for (uint64_t i = 0; i < threads; i++) {
        myWorker[i]->setDataSet(_r, _s);
    }
}

size_t OoOJoin::KeyPartitionInfiniteRunner::getAQPResult() {
    size_t ru = 0;
    for (uint64_t i = 0; i < threads; i++) {
        ru += myWorker[i]->getAQPResult();
    }
    return ru;
}

double OoOJoin::KeyPartitionInfiniteRunner::getThroughput() {
    double ru = 0;
    uint64_t nz = 0;
    for (uint64_t i = 0; i < threads; i++) {
        ru += myWorker[i]->getThroughput();
        if (myWorker[i]->getThroughput() != 0) {
            nz++;
        }
    }
    return ru / nz;
}

double OoOJoin::KeyPartitionInfiniteRunner::getLatencyPercentage(double fraction) {
    double ru = 0;
    uint64_t nz = 0;
    for (uint64_t i = 0; i < threads; i++) {
        ru += myWorker[i]->getLatencyPercentage(fraction);
        if (myWorker[i]->getLatencyPercentage(fraction) != 0) {
            nz++;
        }
    }
    return ru / nz;
}