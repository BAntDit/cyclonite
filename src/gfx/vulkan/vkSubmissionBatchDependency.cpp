
#include "vkSubmissionBatchDependency.h"
#include "gfx/signal.h"
#include <cassert>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
SubmissionBatchDependency::SubmissionBatchDependency(core::ResourceWeakRef signalRef,
                                                     PipelineStageFlagBits stageMask,
                                                     uint64_t ccompletionValue)
  : signalRef_{ signalRef }
  , stageMask_{ stageMask }
  , completionValue_{ ccompletionValue }
{
    auto signal = signalRef_.lock();
    assert(signal.valid());
}

auto SubmissionBatchDependency::value() const -> uint64_t
{
    auto signal = signalRef_.lock();
    assert(signal.valid());

    return signal.as<gfx::Signal>().value();
}

auto SubmissionBatchDependency::type() const -> SignalType 
{
    auto signal = signalRef_.lock();
    assert(signal.valid());

    return signal.as<gfx::Signal>().type();
}
}
#endif
