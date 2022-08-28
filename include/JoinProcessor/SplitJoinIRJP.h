/*! \file SplitJoinIRJP.h*/
//
// Created by tony on 22/03/22.
//

#ifndef _JOINPROCESSOR_SPLITJOINIRJP_H_
#define _JOINPROCESSOR_SPLITJOINIRJP_H_
#include <thread>
#include <Common/Types.hpp>
#include <Utils/UtilityFunctions.hpp>
#include <barrier>
#include <JoinProcessor/SplitJoinJP.h>
#include <Utils/DupicatedHashTable.hpp>
using namespace AllianceDB;
using namespace std;
namespace AllianceDB {
typedef AllianceDB::DupicatedHashTable<keyType, keyType> dpHashtable;
//typedef <keyType, keyType> dpHashtable;
typedef shared_ptr<dpHashtable> dpHashtablePtr;
/**
 * @ingroup JOINPROCESSOR_EAGER
  * @class SplitJoinIRJP JoinProcessor/SplitJoinIRJP.h
  * @brief The class of  split join join processor, which also shares the intermediate results (IR)
  * @see SplitJoinJP
  */
class SplitJoinIRJP : public SplitJoinJP {

 protected:
  dpHashtablePtr hashtableS, hashtableR;
  virtual void inlineMain();

  void expireS(size_t cond);
  void expireR(size_t cond);
  void joinS(TuplePtr ts);
  void joinR(TuplePtr tr);
 public:
  SplitJoinIRJP() {

  }
  ~SplitJoinIRJP() {

  }
  /**
 * @brief init the join processor with buffer/queue length and id
 * @param sLen The length of S queue and buffer
 * @param rLen The length of r queue and buffer
 * @param _sysId The system id
 */
  void init(size_t sLen, size_t rLen, size_t _sysId) {
    SplitJoinJP::init(sLen, rLen, _sysId);
    //windowS = C20Buffer<TuplePtr>(sLen);
    // windowR = C20Buffer<TuplePtr>(rLen)
  }

};
typedef std::shared_ptr<SplitJoinIRJP> SplitJoinIRJPPtr;
}
#endif //ALIANCEDB_INCLUDE_JOINPROCESSOR_SPLITJP_H_
