#ifndef ALLIANCEDB_SRC_ENGINE_EAGERENGINE_HPP_
#define ALLIANCEDB_SRC_ENGINE_EAGERENGINE_HPP_

#include <Common/Stream.hpp>
namespace AllianceDB {
typedef std::shared_ptr<class EagerEngine> EagerEnginePtr;
class EagerEngine {
  virtual void Start() = 0;
};

} // AllianceDB

#endif //ALLIANCEDB_SRC_ENGINE_EAGERENGINE_HPP_
