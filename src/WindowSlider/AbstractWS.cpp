//
// Created by tony on 10/03/22.
//

#include <WindowSlider/AbstractWS.h>
using namespace INTELLI;
AbstractWS::AbstractWS(size_t _sLen, size_t _rLen) {
  sLen = _sLen;
  rLen = _rLen;
  TuplePtrQueueInS = newTuplePtrQueue(sLen);
  TuplePtrQueueInR = newTuplePtrQueue(rLen);
  reset();
}
AbstractWS::~AbstractWS() {

}
