
#include "vkSubmissionBatchDependency.h"
#include "gfx/signal.h"
#include <cassert>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
SubmissionBatchDependency::SubmissionBatchDependency(core::ResourceWeakRef signalRef, PipelineStageFlagBits stageMask)
  : signalRef_{ signalRef }
  , stageMask_{ stageMask }
  , value_{ 0 }
{
    auto signal = signalRef_.lock();
    assert(signal.valid());

    // store signal value here
    // because meanwhile dependency creation signal is not in pending state
    value_ = signal.as<gfx::Signal>().value();
}
}
#endif
