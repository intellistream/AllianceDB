//
// Created by tony on 2022/2/9.
//
#include<JoinProcessor/HandShakeHashJP.h>
using namespace INTELLI;
void HandShakeHashJP::setupQueue() {
TuplePtrQueueInS = newTuplePtrQueue(sQueue);
TuplePtrQueueInR = newTuplePtrQueue(rQueue);
selfWindowS = newTuplePtrQueue(sQueue);
selfWindowR = newTuplePtrQueue(rQueue);
cmdQueueIn = newCmdQueue(1);
cmdQueueOut = newCmdQueue(1);

}
void HandShakeHashJP::inlineRun() {
//first bind to the core
UtilityFunctions::bind2Core(cpuBind);
setupQueue();
waitInitBar();
cout << "jp " << sysId << " is ready,S=" << timeOffsetS << ", R= " << timeOffsetR << endl;

while (1) {
//cmd
if (TuplePtrQueueInS->empty() && TuplePtrQueueInR->empty()) {
if (!cmdQueueIn->empty()) {

join_cmd_t cmdIn = *cmdQueueIn->front();
cmdQueueIn->pop();
if (cmdIn == CMD_STOP) {
sendAck();
cout << "join processor " << sysId << " end" << endl;
return;
}
}
}
while (!TuplePtrQueueInR->empty()) { //cout<<"r is full"<<endl;
paraseTupleR();
}
while (!TuplePtrQueueInS->empty()) {//cout<<" s is full"<<endl;
paraseTupleS();
}
}
}
void HandShakeHashJP::paraseTupleR() {
TuplePtr trIn = *TuplePtrQueueInR->front();
TuplePtrQueueInR->pop();
size_t tNow;
if (isTimeBased()) {
tNow = getTimeStamp();
//cout<<tNow<<endl;
//trIn->subKey=tNow;
expireS(tNow);

}
//join the trIn with S window
//build S
size_t sSize = selfWindowS->size();
/*hashtable hashtableS;
for (size_t i = 0; i < sSize; i++) {
  TuplePtr ts = selfWindowS->front()[i];
  hashtableS.emplace(ts->key, 0, ts->subKey);
}
//probe R
auto findMatchS = hashtableS.find(trIn->key);
if (findMatchS != hashtableS.end()) {
  size_t matches = findMatchS->second.size();
  joinedResult += matches;
 // printf("JP%ld:R[%ld](%ld) find %ld S matches\r\n", sysId, trIn->subKey + 1, trIn->key, matches);
}*/
for (size_t i = 0; i < sSize; i++) {
TuplePtr ts = selfWindowS->front()[i];
if (ts->key == trIn->key) {
joinedResult++;
}
}

selfWindowR->push(trIn);
expireR(tNow);
// late expire of R
if (!isTimeBased()) {

/*if (selfWindowR->size()>windowLen)
{   TuplePtr tr = *selfWindowR->front();
  if(leftJP!= nullptr)
  {
    leftJP->feedTupleR(tr);
  }
  selfWindowR->pop();
}*/
expireR(countR);
}
}
void HandShakeHashJP::paraseTupleS() {
TuplePtr tsIn = *TuplePtrQueueInS->front();
TuplePtrQueueInS->pop();
size_t tNow;
if (isTimeBased()) {
tNow = getTimeStamp();
//tsIn->subKey=tNow;

expireR(tNow);
}
//join the tsIn with R window
//build R
size_t rSize = selfWindowR->size();
/*hashtable hashtableR;
for (size_t i = 0; i < rSize; i++) {
  TuplePtr tr= selfWindowR->front()[i];
  hashtableR.emplace(tr->key, 0, tr->subKey);
}
//probe S
auto findMatchR = hashtableR.find(tsIn->key);
if (findMatchR != hashtableR.end()) {
  size_t matches = findMatchR->second.size();
  joinedResult += matches;
 // printf("JP%ld:R[%ld](%ld) find %ld S matches\r\n", sysId, tsIn->subKey + 1, tsIn->key, matches);
}*/
for (size_t i = 0; i < rSize; i++) {
TuplePtr tr = selfWindowR->front()[i];
if (tr->key == tsIn->key) {
joinedResult++;
}
}
selfWindowS->push(tsIn);
expireS(tNow);
// late expire of S
if (!isTimeBased()) {

/* if (selfWindowS->size()>windowLen)
 {   TuplePtr ts = *selfWindowS->front();
   if(rightJP!= nullptr)
   {
     rightJP->feedTupleS(ts);
   }
   selfWindowS->pop();
 }*/
expireS(countS);
}

}
void HandShakeHashJP::expireS(size_t cond) {
size_t distance = 0;
if (!selfWindowS->empty()) {
TuplePtr ts = *selfWindowS->front();
distance = cond - ts->subKey;
// cout<<ts->subKey<<cond<<endl;
while (distance > windowLen + timeOffsetS) {
selfWindowS->pop();
if (rightJP != nullptr) {
// printf("jp%d, send to S to right\r\n",sysId);
rightJP->feedTupleS(ts);
} else {
//printf("jp%d, no right\n",sysId);
}
if (!selfWindowS->empty()) {
ts = *selfWindowS->front();
distance = cond - ts->subKey;
} else {
distance = 0;
}
}
}
}

void HandShakeHashJP::expireR(size_t cond) {
size_t distance = 0;
if (!selfWindowR->empty()) {
TuplePtr tr = *selfWindowR->front();
distance = cond - tr->subKey;
// cout<<"expire R"<<cond<<":"<<distance<<endl;
while (distance > windowLen + timeOffsetR) {
selfWindowR->pop();

if (leftJP != nullptr) {//("jp%d, send to R to left\r\n",sysId);
leftJP->feedTupleR(tr);
} else {
//printf("jp%d, no left\n",sysId);
}
if (!selfWindowR->empty()) {
tr = *selfWindowR->front();
distance = cond - tr->subKey;
} else {
distance = 0;
}
}
}
}
void HandShakeHashJP::feedTupleS(TuplePtr ts) {
if (!isTimeBased()) {
ts->subKey = countS;
countS++;
// printf("JP %d, s=%d\r\n",sysId,countS);
} else {
//ts->subKey=getTimeStamp();
}
CellJoinJP::feedTupleS(ts);

}

void HandShakeHashJP::feedTupleR(TuplePtr tr) {
if (!isTimeBased()) {
tr->subKey = countR;
countR++;
// printf("JP %d, R=%d\r\n",sysId,countR);
} else {
//tr->subKey=getTimeStamp();
}
CellJoinJP::feedTupleR(tr);

}
