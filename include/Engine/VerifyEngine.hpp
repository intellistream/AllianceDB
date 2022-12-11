#ifndef ALLIANCEDB_SRC_ENGINE_VERIFYNGINE_HPP_
#define ALLIANCEDB_SRC_ENGINE_VERIFYNGINE_HPP_

#include "Common/Param.hpp"
#include "Common/Stream.hpp"
#include "Engine/EagerEngine.hpp"
#include "Utils/Executor.hpp"

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
  const StreamPtr R;
  const StreamPtr S;
  Param param;

public:
  VerifyEngine(const StreamPtr R, const StreamPtr S, const Param &param);

  void Start() override;
};

} // namespace AllianceDB

#endif // ALLIANCEDB_SRC_ENGINE_VERIFYNGINE_HPP_
