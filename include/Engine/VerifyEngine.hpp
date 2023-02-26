#ifndef ALLIANCEDB_SRC_ENGINE_VERIFYNGINE_HPP_
#define ALLIANCEDB_SRC_ENGINE_VERIFYNGINE_HPP_

#include "Common/Context.hpp"
#include "Common/Result.hpp"
#include "Common/Stream.hpp"
#include "Engine/EagerEngine.hpp"
#include "Utils/Executor.hpp"

#include <thread>

namespace AllianceDB
{
typedef std::shared_ptr<class VerifyEngine> VerifyEnginePtr;
class VerifyEngine
{
private:
    const Param &param;
    std::thread t;

public:
    VerifyEngine(const Param &param);
    void Run(Context &ctx);
    bool Wait();
};

}  // namespace AllianceDB

#endif  // ALLIANCEDB_SRC_ENGINE_VERIFYNGINE_HPP_
