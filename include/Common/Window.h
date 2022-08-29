#ifndef ALLIANCEDB_INCLUDE_COMMON_WINDOW_H_
#define ALLIANCEDB_INCLUDE_COMMON_WINDOW_H_

#include <memory>
#include <vector>
#include <Common/Tuple.hpp>
#include <Common/Stream.hpp>

namespace AllianceDB {
typedef std::shared_ptr<class Window> WindowPtr;
class Window : Stream {

};
}
#endif //ALLIANCEDB_INCLUDE_COMMON_WINDOW_H_
