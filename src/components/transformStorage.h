
#ifndef CYCLONITE_COMPONENTS_TRANSFORM_STORAGE
#define CYCLONITE_COMPONENTS_TRANSFORM_STORAGE

#include "transform.h"
#include <vector>

namespace cyclonite::components {
class TransformStorage
{
public:
    using component_type = Transform;

    using storage_type = std::vector<component_type>;

    TransformStorage() = default;

    void reserve(size_t capacity);

    [[nodiscard]] auto capacity() const -> size_t { return storage_.capacity(); }

    [[nodiscard]] auto size() const -> size_t { return storage_.size(); }

    [[nodiscard]] auto get(uint32_t index) const -> Transform const&;

    [[nodiscard]] auto get(uint32_t index) -> Transform&;

    auto create(uint32_t index, enttx::Entity parent) -> Transform&;

    [[nodiscard]] auto begin() const -> std::vector<Transform>::const_iterator { return storage_.cbegin(); }

    [[nodiscard]] auto end() const -> std::vector<Transform>::const_iterator { return storage_.cend(); }

    [[nodiscard]] auto begin() -> std::vector<Transform>::iterator { return storage_.begin(); }

    [[nodiscard]] auto end() -> std::vector<Transform>::iterator { return storage_.end(); }

private:
    constexpr static size_t chunk_size_v = 256;

    void resizeIndicesIfNecessary(uint32_t index);

    void reserveStoreIfNecessary(uint32_t componentIdx);

    std::vector<uint32_t> entityIdxToComponent_; // maybe better use unordered map
    std::vector<uint32_t> componentIdxToEntityIdx_;
    std::vector<component_type> storage_;
};
}

#endif // CYCLONITE_COMPONENTS_TRANSFORM_STORAGE
