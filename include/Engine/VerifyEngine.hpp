#ifndef ALLIANCEDB_SRC_ENGINE_VERIFYNGINE_HPP_
#define ALLIANCEDB_SRC_ENGINE_VERIFYNGINE_HPP_

#include "Common/Param.hpp"
#include "Common/Result.hpp"
#include "Common/Stream.hpp"
#include "Engine/EagerEngine.hpp"
#include "Utils/Executor.hpp"

#include <thread>

namespace AllianceDB {
typedef std::shared_ptr<class VerifyEngine> VerifyEnginePtr;
class VerifyEngine {
private:
  const StreamPtr R, S;
  Param param;
  std::thread t;
  ResultPtr result;

public:
  VerifyEngine(const Param &param, const StreamPtr R, const StreamPtr S);
  void Start();
  void Run();
  bool Wait();
  ResultPtr Result();
};

} // namespace AllianceDB

#endif // ALLIANCEDB_SRC_ENGINE_VERIFYNGINE_HPP_
