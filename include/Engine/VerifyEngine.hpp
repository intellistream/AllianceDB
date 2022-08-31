#ifndef ALLIANCEDB_SRC_ENGINE_VERIFYNGINE_HPP_
#define ALLIANCEDB_SRC_ENGINE_VERIFYNGINE_HPP_

#include <Common/Stream.hpp>
#include <Engine/EagerEngine.hpp>
#include <Utils/Executor.hpp>

namespace AllianceDB {
typedef std::shared_ptr<class VerifyEngine> VerifyEnginePtr;
class VerifyEngine : public EagerEngine {
 private:
  class VerifyThread : public Executor {
   public:
    explicit VerifyThread(int id);
    void Process() override;
    TuplePtr NextTuple();
    std::string id();
   private:
    int ID;
  };
  const StreamPtr streamR;
  const StreamPtr streamS;
  const int threads;
  const int window_length;
  const int slide_length;

 public:
  VerifyEngine(const StreamPtr streamR,
               const StreamPtr streamS,
               int threads,
               int window_length,
               int slide_length);

  void Start() override;
};

} // AllianceDB

#endif //ALLIANCEDB_SRC_ENGINE_VERIFYNGINE_HPP_
