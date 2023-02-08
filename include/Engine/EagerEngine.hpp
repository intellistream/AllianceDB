#ifndef ALLIANCEDB_SRC_ENGINE_EAGERENGINE_HPP_
#define ALLIANCEDB_SRC_ENGINE_EAGERENGINE_HPP_

#include <memory>
#include <thread>

#include "Common/Context.hpp"
#include "Common/Result.hpp"
#include "Common/Stream.hpp"
#include "Common/Window.h"
#include "Join/Join.hpp"

namespace AllianceDB
{
class EagerEngine
{
public:
    EagerEngine(Context &ctx);
    void Run();
    ResultPtr Result();

private:
    const Param &param;
    const StreamPtr ss, sr;
    ResultPtr res;
    std::vector<JoinPtr> algo;
    Context &ctx;
};

}  // namespace AllianceDB

#endif  // ALLIANCEDB_SRC_ENGINE_EAGERENGINE_HPP_
