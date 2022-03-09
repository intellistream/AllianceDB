//
// Created by Wang Chenyu on 4/9/21.
//

#include <TestBench/OneWayHashJoin.h>
#include <iostream>

void INTELLI::OneWayHashJoin::execute(INTELLI::Result &joinResult, INTELLI::RelationCouple &relationCouple) {
  bool windowFullR = false;
  bool windowFullS = false;
  INTELLI::WindowCouple windowCouple = INTELLI::WindowCouple(WINDOW_SIZE);
  while (!relationCouple.relationR.empty() && !relationCouple.relationS.empty()) {
    //R
    //Push relationR's tuple into windowR + hashtableR
    INTELLI::TuplePtr tuple = relationCouple.relationR.front();
    relationCouple.relationR.pop();
    windowCouple.windowR.push(tuple);
    windowCouple.hashtableR.emplace(tuple->key, 0);

    //match the tuple with hashtableS
    if (windowCouple.hashtableS.find(tuple->key) != windowCouple.hashtableS.end()) joinResult++;
    //Expire the oldest item in windowR + hashtableR if window is full
    if (windowFullR) {
      windowCouple.hashtableR.erase(windowCouple.windowR.front()->key);
      windowCouple.windowR.pop();
    } else {
      //This may reduce the compare time.
      if (windowCouple.windowR.size() >= windowCouple.windowSize) windowFullR = true;
    }

    //S
    //Push relationS's tuple into windowS + hashtableS
    tuple = relationCouple.relationS.front();
    relationCouple.relationS.pop();
    windowCouple.windowS.push(tuple);
    windowCouple.hashtableS.emplace(tuple->key, 0);

    //match the tuple with hashtableR
    if (windowCouple.hashtableR.find(tuple->key) != windowCouple.hashtableR.end()) joinResult++;
    //Expire the oldest item in windowS + hashtableS if window is full
    if (windowFullS) {
      windowCouple.hashtableS.erase(windowCouple.windowS.front()->key);
      windowCouple.windowS.pop();
    } else {
      //This may reduce the compare time.
      if (windowCouple.windowS.size() >= windowCouple.windowSize) windowFullS = true;
    }
  }
}