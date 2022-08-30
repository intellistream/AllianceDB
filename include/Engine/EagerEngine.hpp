#ifndef ALLIANCEDB_SRC_ENGINE_EAGERENGINE_HPP_
#define ALLIANCEDB_SRC_ENGINE_EAGERENGINE_HPP_

#include <Common/Stream.hpp>
namespace AllianceDB {
typedef std::shared_ptr<class EagerEngine> EagerEnginePtr;
class EagerEngine {

 public:
  virtual void Run(StreamPtr streamR, StreamPtr streamS, int threads, int window_length, int slide_length) = 0;
};

} // AllianceDB

#endif //ALLIANCEDB_SRC_ENGINE_EAGERENGINE_HPP_
