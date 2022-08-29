#ifndef ALLIANCEDB_SRC_ENGINE_EAGERENGINE_HPP_
#define ALLIANCEDB_SRC_ENGINE_EAGERENGINE_HPP_

namespace AllianceDB {
typedef std::shared_ptr<class EagerEngine> EagerEnginePtr;
class EagerEngine {

 public:
  void Run();
};

} // AllianceDB

#endif //ALLIANCEDB_SRC_ENGINE_EAGERENGINE_HPP_
