#ifndef ALLIANCEDB_SRC_ENGINE_VERIFYNGINE_HPP_
#define ALLIANCEDB_SRC_ENGINE_VERIFYNGINE_HPP_

#include <Common/Stream.hpp>
#include <Engine/EagerEngine.hpp>
#include <Utils/Executor.hpp>

namespace AllianceDB {
typedef std::shared_ptr<class VerifyEngine> VerifyEnginePtr;
class VerifyEngine : public EagerEngine {
 private:

  class VerifyThread : Executor {

   public:
    void Process() override;
  };

 public:
  void Start(StreamPtr streamR, StreamPtr streamS, int threads, int window_length, int slide_length);
};

} // AllianceDB

#endif //ALLIANCEDB_SRC_ENGINE_VERIFYNGINE_HPP_
