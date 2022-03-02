//
// Created by Wang Chenyu on 13/10/21.
//

#include <JoinMethods/CellJoin.h>
#include <thread>
#include <Common/Types.h>
#include <iostream>
#include <WindowSlider/AbstractEagerWS.h>
//Variables that are shared among threads
INTELLI::tupleKeyQueue deliveryR[THREAD_NUMBER]; //Used to send R tuple to all worker threads
INTELLI::tupleKeyQueue deliveryS[THREAD_NUMBER]; //Used to send S tuple to all worker threads
INTELLI::numberType threadJoinResult[THREAD_NUMBER]; //Join result per thread

//Main thread will serve as a tuple distributor
void INTELLI::CellJoin::execute(INTELLI::Result &joinResult, INTELLI::RelationCouple &relationCouple) {

  //Tuple distributor
  int count = 0;
  while (!relationCouple.relationR.empty() || !relationCouple.relationS.empty()) {
    //R
    //Get the current R we need to distribute
    if (!relationCouple.relationR.empty()) {
      INTELLI::TuplePtr tupleR = relationCouple.relationR.front();
      relationCouple.relationR.pop();
      //Push it to all threads
      for (int id = 0; id < THREAD_NUMBER; id++) {

        deliveryR[id].push(tupleR->key);
      }
    }


    //S
    //Get the current S we need to distribute
    if (!relationCouple.relationS.empty()) {
      INTELLI::TuplePtr tupleS = relationCouple.relationS.front();
      relationCouple.relationS.pop();
      //Push it to a specific thread according to calculation
      int tupleDestinationS = (++count % WINDOW_SIZE) % THREAD_NUMBER; //TODO: OR count++? you try it. thanks

      deliveryS[tupleDestinationS].push(tupleS->key);
    }
  }
  //Check

//    for (int id = 0; id < THREAD_NUMBER; id++) {
//        std::cout << id << "-S" << deliveryS[id].size() << "\n";
//        std::cout << id << "-R" << deliveryR[id].size() << "\n";
//        std::cout << std::endl;
//    }




  std::thread threadArray[THREAD_NUMBER];
  //Thread creation
  int base = WINDOW_SIZE / THREAD_NUMBER;
  int module = WINDOW_SIZE % THREAD_NUMBER;
  for (int id = 0; id < THREAD_NUMBER; id++) {
    int threadWindowSize = (id < module) ? base + 1 : base;
//        std::cout << id << "-" << threadWindowSize << "\n";
    threadArray[id] = std::thread(threadWork, id, threadWindowSize);
  }

  //Join and get the join result
  for (int id = 0; id < THREAD_NUMBER; id++) {
    threadArray[id].join();
    std::cout << id << " result " << threadJoinResult[id] << "\n";
    joinResult.joinNumber += threadJoinResult[id];
  }
}

void INTELLI::CellJoin::threadWork(int id, numberType windowSizeS) {
  //if (id == 0) {
  //while (!deliveryR[id].empty()) {
  //std::cout << deliveryR[id].front() << " ";
  //deliveryR[id].pop();
  //}
  //}

  INTELLI::WindowCouple windowCouple = INTELLI::WindowCouple(WINDOW_SIZE, windowSizeS);
  bool windowFullR = false;
  bool windowFullS = false;
  int windowRat = 0;
  int windowSat = 0;
  bool flagR = true;
  bool flagS = true;

  while (!deliveryR[id].empty() || !deliveryS[id].empty()) {
    //R
    if (flagR == false && flagS == false) {
      windowRat = 0;
      windowSat = 0;
      flagR = true;
      flagS = true;
    }
    if (!deliveryR[id].empty() && flagR) {
      //Push relationR's tuple_key into windowR + hashtableR
      INTELLI::numberType tuple_key = deliveryR[id].front();

      //if (id == -1)
      //std::cout << tuple_key << "\n";

      deliveryR[id].pop();

      windowCouple.windowR.push(std::make_shared<Tuple>(tuple_key));
      windowCouple.hashtableR.emplace(tuple_key, 0);
      windowRat++;
      //match the tuple_key with hashtableS
      if (windowCouple.hashtableS.find(tuple_key) != windowCouple.hashtableS.end()) {

        threadJoinResult[id]++;
      }

      if (windowRat == WINDOW_SIZE) {
        flagR = false;
      }
      //Expire the oldest item in windowR + hashtableR if window is full
      if (windowFullR) {
        windowCouple.hashtableR.erase(windowCouple.windowR.front()->key);
        windowCouple.windowR.pop();
      } else {
        //This may reduce the compare time.
        if (windowCouple.windowR.size() >= windowCouple.windowSizeR) windowFullR = true;
      }
    }
    //S
    if (!deliveryS[id].empty() && flagS) {
      //Push relationS's tuple_key into windowS + hashtableS
      INTELLI::numberType tuple_key = deliveryS[id].front();
//            if (id == 0)
//                std::cout << tuple_key << "\n";
      deliveryS[id].pop();
      windowCouple.windowS.push(std::make_shared<Tuple>(tuple_key));
      windowCouple.hashtableS.emplace(tuple_key, 0);
      windowSat++;
      //match the tuple_key with hashtableR
      if (windowCouple.hashtableR.find(tuple_key) != windowCouple.hashtableR.end()) {

        threadJoinResult[id]++;
      }
      if (windowSat == windowSizeS) {
        flagS = false;
      }
      //Expire the oldest item in windowS + hashtableS if window is full
      if (windowFullS) {
        windowCouple.hashtableS.erase(windowCouple.windowS.front()->key);
        windowCouple.windowS.pop();
      } else {
        //This may reduce the compare time.
        if (windowCouple.windowS.size() >= windowCouple.windowSizeS) windowFullS = true;
      }
    }
  }
}
