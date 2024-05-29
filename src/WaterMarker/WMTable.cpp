//
// Created by tony on 31/12/22.
//

#include <WaterMarker/WMTable.h>
#include <WaterMarker/LatenessWM.h>
#include <WaterMarker/ArrivalWM.h>

using namespace OoOJoin;

OoOJoin::WMTable::WMTable() {
  wmMap["arrival"] = newArrivalWM();
  wmMap["lateness"] = newLatenessWM();
}
