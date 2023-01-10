#ifndef ALLIANCEDB_INCLUDE_COMMON_WINDOW_H_
#define ALLIANCEDB_INCLUDE_COMMON_WINDOW_H_

#include "Common/Stream.hpp"
#include "Common/Tuple.hpp"

#include <memory>
#include <vector>

namespace AllianceDB
{
using Window    = std::vector<TuplePtr>;
using WindowPtr = std::shared_ptr<Window>;

}  // namespace AllianceDB
#endif  // ALLIANCEDB_INCLUDE_COMMON_WINDOW_H_
