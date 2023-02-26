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
    EagerEngine(const Param &param);
    void Run(Context &ctx);

private:
    const Param &param;
    std::vector<JoinPtr> windows;
    JoinPtr New();
};

}  // namespace AllianceDB

#endif  // ALLIANCEDB_SRC_ENGINE_EAGERENGINE_HPP_
