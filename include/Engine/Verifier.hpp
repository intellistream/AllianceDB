#ifndef _VERIFYBENCH_H_
#define _VERIFYBENCH_H_

#include <Common/Types.hpp>
#include <Common/Tuple.hpp>
#include <Common/Relations.hpp>
#include  <Common/Result.hpp>
#include <WindowSlider/AbstractEagerWS.h>
#include <WindowSlider/VerifyWS.h>
#include <Engine/Executor.hpp>

using namespace INTELLI;
namespace INTELLI {

/**
 * @ingroup INTELLI_COMMON_VERIFY
 * @class Verify  Common/Verify.h
 */
class Verifier : public Executor<VerifyWS> {
};
}

#endif
