#ifndef ALLIANCEDB_SRC_ENGINE_EAGERENGINE_HPP_
#define ALLIANCEDB_SRC_ENGINE_EAGERENGINE_HPP_

#include "Common/Param.hpp"
#include "Common/Result.hpp"
#include "Common/Stream.hpp"
#include "Common/Window.h"
#include "Join/Join.hpp"

#include <thread>
#include <memory>

namespace AllianceDB {

typedef std::shared_ptr<class EagerEngine> EagerEnginePtr;
// Why not template here? Because we may need to be able to choose different
// join algorithms at runtime.
class EagerEngine {
private:
  Param param;
  const StreamPtr ss, sr;
  ResultPtr res;
  std::thread run_thread;
  std::vector<std::unique_ptr<std::thread>> join_threads;
  WindowPtr wr, ws;
  JoinPtr algo;
  size_t cntw = 0;
  size_t cntr = 0, cnts = 0;
  void LaunchJoin();
  bool IsFullR();
  bool IsFullS();

public:
  EagerEngine(const Param &param, const StreamPtr sr, const StreamPtr ss);
  void Run();
  void Start();
  void Wait();
  ResultPtr Result();
  void RunJoin(size_t rbeg, size_t rend, size_t sbeg, size_t send, WindowJoinResult &res);
};

} // namespace AllianceDB

#endif // ALLIANCEDB_SRC_ENGINE_EAGERENGINE_HPP_
