
#ifndef CYCLONITE_SYSTEMS_UPDATE_STAGES
#define CYCLONITE_SYSTEMS_UPDATE_STAGES

#include <cstdint>
#include <metrix/enum.h>

namespace cyclonite::systems {
enum class SystemUpdateStageList : uint32_t
{
    FRAME_START = 0,
    TRANSFER_START = 1,
    TRANSFER_END = 2,
    FRAME_END = 3,
    LAST_STAGE = FRAME_END
};

template<typename T>
concept SystemStageListConcept = requires(T) {
    requires std::is_enum_v<T>;

    { T::LAST_STAGE };
};
}

#endif // CYCLONITE_SYSTEMS_UPDATE_STAGES
