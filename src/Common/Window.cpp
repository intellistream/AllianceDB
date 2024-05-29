//
// Created by tony on 22/11/22.
//

#include <Common/Window.h>

using namespace std;
using namespace INTELLI;
using namespace OoOJoin;

void OoOJoin::Window::setRange(OoOJoin::tsType ts, OoOJoin::tsType te) {
  startTime = ts;
  endTime = te;
}

OoOJoin::Window::Window(OoOJoin::tsType ts, OoOJoin::tsType te) {
  setRange(ts, te);
}

void OoOJoin::Window::init(size_t sLen, size_t rLen, size_t _sysId) {
  windowS = INTELLI::C20Buffer<TrackTuplePtr>(sLen);
  windowR = INTELLI::C20Buffer<TrackTuplePtr>(rLen);
  windowID = _sysId;
}

bool OoOJoin::Window::feedTupleS(OoOJoin::TrackTuplePtr ts) {
  if (ts->eventTime > endTime || ts->eventTime < startTime) {
    return false;
  }
  windowS.append(ts);
  return true;
}

bool OoOJoin::Window::feedTupleR(OoOJoin::TrackTuplePtr tr) {
  if (tr->eventTime > endTime || tr->eventTime < startTime) {
    return false;
  }
  windowR.append(tr);
  return true;
}

bool OoOJoin::Window::reset() {
  windowS.reset();
  windowR.reset();
  return true;
}