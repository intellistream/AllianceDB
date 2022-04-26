//
// Created by tony on 18/03/22.
//
#include <JoinProcessor/SplitJoinIRJP.h>
using namespace INTELLI;
void SplitJoinIRJP::joinS(TuplePtr ts) {
  size_t timeNow = ts->subKey;
  expireR(timeNow);
  //update the table
  hashtableS->emplace(ts->key, 0, ts->subKey);
  //single thread join window r and tuple s
  TuplePtrQueueLocalS->push(ts);
  //look up the hash table for join
  auto findMatchR = hashtableR->find(ts->key);
  if (findMatchR != hashtableR->end()) {
    size_t matches = findMatchR->second.size();
    joinedResult += matches;
    //printf("JP%ld:S[%ld](%ld) find %ld R matches\r\n", sysId, ts->subKey + 1, ts->key, matches);
    //joinedResult ++;
  }
  /*size_t rSize = TuplePtrQueueLocalR->size();
   for (size_t i = 0; i < rSize; i++) {
     if (TuplePtrQueueLocalR->front()[i]->key == ts->key) {
       joinedResult++;
     }
   }*/
  /* size_t rSize = windowR.size();
   for (size_t i = 0; i < rSize; i++) {
     if (windowR.data()[i]->key == ts->key) {
       joinedResult++;
     }
   }*/

}
void SplitJoinIRJP::joinR(TuplePtr tr) {
  size_t timeNow = tr->subKey;
  expireS(timeNow);
  TuplePtrQueueLocalR->push(tr);
  //update the table
  hashtableR->emplace(tr->key, 0, tr->subKey);
  //look up the hash table for join
  auto findMatchS = hashtableS->find(tr->key);
  if (findMatchS != hashtableS->end()) {
    size_t matches = findMatchS->second.size();
    joinedResult += matches;
    //joinedResult ++;
  }
}
void SplitJoinIRJP::expireS(size_t cond) {
  size_t pos = 0;
  size_t startTime;

  if (slideLenGlobal == 0) {
    startTime = (cond < windowLenGlobal) ? 0 : (cond - windowLenGlobal);
  } else {
    size_t windowNo = oldestWindowBelong(cond);
    startTime = windowNo * slideLenGlobal;
  }
  if (!TuplePtrQueueLocalS->empty()) {
    TuplePtr ts = *TuplePtrQueueLocalS->front();
    pos = ts->subKey;
    while (pos < startTime) {
      // delete from window
      TuplePtrQueueLocalS->pop();
      //delete from table S
      //hashtableS->erase(ts->key);
      hashtableS->eraseWithSubKey(ts->key, ts->subKey);
      //move next
      if (!TuplePtrQueueLocalS->empty()) {
        ts = *TuplePtrQueueLocalS->front();
        pos = ts->subKey;
      } else {
        pos = startTime;
      }
    }
  }
}
void SplitJoinIRJP::expireR(size_t cond) {
  size_t pos = 0;
  size_t startTime;

  if (slideLenGlobal == 0) {
    startTime = (cond < windowLenGlobal) ? 0 : (cond - windowLenGlobal);
  } else {
    size_t windowNo = oldestWindowBelong(cond);
    startTime = windowNo * slideLenGlobal;
  }

  if (!TuplePtrQueueLocalR->empty()) {
    TuplePtr tr = *TuplePtrQueueLocalR->front();
    pos = tr->subKey;
    //  cout<<"pos="+ to_string(pos)+", startTime="+ to_string(slideLen)<<endl;
    while (pos < startTime) {
      // delete from window
      TuplePtrQueueLocalR->pop();
      //delete from table R
      //hashtableS->erase(tr->key);
      hashtableR->eraseWithSubKey(tr->key, tr->subKey);
      //move next
      if (!TuplePtrQueueLocalR->empty()) {
        tr = *TuplePtrQueueLocalR->front();
        pos = tr->subKey;
      } else {
        pos = startTime;
      }
    }
  }

}
void SplitJoinIRJP::inlineMain() {
  UtilityFunctions::bind2Core(cpuBind);
  hashtableS = make_shared<dpHashtable>();
  hashtableR = make_shared<dpHashtable>();
  // cout<< "this is split join IR, id "+ to_string(sysId)<<endl;
  while (1) {
    //stop cmd
    if (TuplePtrQueueInS->empty() && TuplePtrQueueInR->empty()) {
      if (!cmdQueueIn->empty()) {
        join_cmd_t cmdIn = *cmdQueueIn->front();
        cmdQueueIn->pop();
        if (cmdIn == CMD_STOP) {
          sendAck();
          // cout << "join processor " << sysId << " end" << endl;
          return;
        }
      }
    }
    //tuple S
    while (!TuplePtrQueueInS->empty()) {
      TuplePtr ts = *TuplePtrQueueInS->front();
      TuplePtrQueueInS->pop();
      sCnt++;
      if (sCnt == sysId + 1) //should process this S
      {
        joinS(ts);
      }
      if (sCnt == sMax) {
        sCnt = 0;
      }
      //joinS(ts);
    }
    while (!TuplePtrQueueInR->empty()) {
      TuplePtr tr = *TuplePtrQueueInR->front();
      TuplePtrQueueInR->pop();
      joinR(tr);
    }
  }
}
